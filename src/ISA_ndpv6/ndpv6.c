/*
 * File:     ndpv6.c
 * Date:     2011-04-29
 * Encoding: UTF-8
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  NDPv6 Proxy
 */


// Header files
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h> // ethernet functions
#include <netinet/if_ether.h> // ethernet frame
#include <netinet/ip6.h> // IPv6 packet
#include <netinet/icmp6.h> // ICMPv6 packet

#include <libnet.h>
#include <pcap.h>

// get IPv6 addresses
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

// get MAC address
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>



// Macros
// -----------------------------------------------------------------------------

// PCap maximum packet size (bytes)
#define BUF_SIZE 65536

// IPv6 address size (bytes)
#define IPV6_ADDR_SIZE 16

// ICMPv6 type in IPv6 Next header
#define IPV6_ICMPV6 0x3A

// Used to determine interface in pcap packet handler function
#define IFACE_IN 'i'

// Used to determine interface in pcap packet handler function
#define IFACE_OUT 'o'

// Is interface IFACE_IN?
#define CHECK_IFACE_IN (*args == IFACE_IN)

// For debugging purposes
#define WHERE fprintf(stderr, "[LOG%s:%d\n", __FILE__, __LINE__);


/**
 * Free resources allocated by PCap
 */
#define free_pcap pcap_freecode(&fp_in);\
                  pcap_freecode(&fp_out);\
                  if (pcap_in != NULL) pcap_close(pcap_in);\
                  if (pcap_in != NULL) pcap_close(pcap_out);
                  
/**
 * Free resources allocated by Libnet
 */
#define free_libnet if (lc_in != NULL) libnet_destroy(lc_in);\
                    if (lc_out != NULL) libnet_destroy(lc_out);
                  
/**
 * Free resource containing interface's IPv6 addresses
 * @param ip6_iface structure for storing IPv6 addresses
 */
#define free_iface(ip6_iface) for (int i = 0; i < (ip6_iface).len; i++) free((ip6_iface).addr[i]);\
                              if ((ip6_iface).addr != NULL) free((ip6_iface).addr);
 
/**
 * Free resources containing interfaces' IPv6 addresses (both)
 */
#define free_iface_all  free_iface(ip6_in)\
                        free_iface(ip6_out)
                   
/**
 * Free Neighbor Cache (for both interfaces)
 */
#define free_nt if (nt_in.items != NULL) free(nt_in.items);\
                if (nt_out.items != NULL) free(nt_out.items);

/**
 * Free all allocated resources
 */
#define free_all free_pcap\
                 free_libnet\
                 free_iface_all\
                 free_nt

                   
                   
// Enums, structures, messages, etc.
// -----------------------------------------------------------------------------

/** Program error codes */
enum tecodes
{
    EOK = 0,         /**< Without error */
    EPARAM,          /**< Bad command line parameters */
    EOUT_MEM,        /**< Out of memory */

	EPCAP_OPEN,      /**< PCAP Open device error */
    EPCAP_BLOCK,     /**< PCAP Non-Block mode setting error */
    EPCAP_COMPILE,   /**< PCAP Compile filter error */
    EPCAP_SETFIL,    /**< PCAP Set filter error */
    EPCAP_REC,       /**< PCAP Packet received error */
    
    ESEL,            /**< Error during select() */
    EGETIPS,         /**< Error during getting IPv6 addresses */
    
    ELIBNET_INIT,    /**< Error Libnet init */
    ELIBNET_WRITE,   /**< Error Libnet write */
    
    ESIG_ACTION,     /**< Error sigaction */
    ESIG_BLOCK,      /**< Error sigblock */
	
    EUNKNOWN         /**< Unknown error */
};

/** Neighbor Cache entry states */
enum nt_states
{
    NT_INCOMPLETE = 0,    /**< Incomplete */
    NT_STALE,             /**< Stale */
    NT_DELAY,             /**< Delay */
    NT_PROBE,             /**< Probe */
    NT_REACHABLE          /**< Reachable */
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
	char* in;            /**< Inner interface name */
	char* out;           /**< Outer interface name */
    int ecode;           /**< Error code (enum tecodes) */
} tParams;

/**
 * Structure for IPv6 addresses
 */
typedef struct
{
    struct in6_addr **addr;    /**< Array of IPv6 address pointers */
    int len;                   /**< Total number of addresses */
    char *iface;               /**< Name of the interface */
} t_ip6;

/**
 * Neighbor Cache entry
 */
typedef struct
{
    struct in6_addr addr;       /**< IPv6 address */
    u_int8_t ether[ETH_ALEN];   /**< MAC address */
    int state;                  /**< State of entry */
} t_nt_item;

/**
 * Neighbor Cache
 */ 
typedef struct
{
    t_nt_item *items;           /**< Array of entries */
    int len;                    /**< Total number of entries */
    int allocated_len;          /**< Allocated array length */
    char *fname;                /**< Interface name */
} t_nt;

/** Error messages */
const char *ECODEMSG[] =
{
    /* EOK */
    "Everything OK.\n",
    /* EPARAM */
    "Error: Bad command line parameters!\n",
    /* EOUT_MEM */
    "Error: Out of memory!\n",
	
	/* EPCAP_OPEN */
	"PCap: Could not open device \"%s\": \"%s\"!\n",
    /* EPCAP_BLOCK */
    "PCap: Could not set device \"%s\" to non-blocking mode: \"%s\"!\n",
    /* EPCAP_COMPILE */
    "PCao: Could not parse filter \"%s\": \"%s\"1\n",
    /* EPCAP_SETFIL */
    "PCap: Could not install filter \"%s\": \"%s\"!\n",
    /* EPCAP_REC */
    "PCap: error occured during packet receiving on \"%s\": \"%s\"!\n",
    
    /* ESEL */
    "Error: select()\n",
    /* EGETIPS */
    "Error: unable to get IPv6 addresses!\n",
    
    /* ELIBNET_INIT */
    "Libnet: unable to init on %s: %s!\n",
    /* ELIBNET_WRITE */
    "Libnet: unable to write %s!\n", 
    
    /* ESIG_ACTION */
    "Error: sigaction!\n",
    /* ESIG_BLOCK */
    "Error: sigblock!\n",
 
    /* EUNKNOWN */
    "Error: Unknown!\n"
};




// Global Variables
// -----------------------------------------------------------------------------

// PCap handler for both interfaces
struct pcap *pcap_in, *pcap_out;

// PCap filter structures for both interfaces
struct bpf_program fp_in, fp_out;
    
// Libnet context for both int.
libnet_t *lc_in, *lc_out;

// MAC addresses of both int.
u_int8_t ether_in[ETH_ALEN], ether_out[ETH_ALEN];

// IPv6 addresses of both int.
t_ip6 ip6_in, ip6_out;

// Neighbor Cache for both int.
t_nt nt_in, nt_out;



// Functions
// -----------------------------------------------------------------------------

/**
 * Prints error message to stderr
 * @param ecode error code
 */
int print_ecode(int ecode, ...)
{
    if (ecode < EOK || ecode > EUNKNOWN)
    { 
        ecode = EUNKNOWN; 
    }

    va_list arg;

    va_start(arg, ecode);

    vfprintf(stderr, ECODEMSG[ecode], arg);

    va_end(arg);
    
    return ecode;
}

/**
 * Processes arguments of command line
 * @param argc Argument count
 * @param argv Array of string with arguments
 */
tParams get_params(int argc, char *argv[])
{
    tParams result = 
    {  // Init structure
	    .in = NULL,
		.out = NULL,
        .ecode = EOK
    };

    char ch;
    opterr = 0;

    // -i IN, -o OUT
    while ((ch = getopt(argc,argv,"i:o:")) != -1) 
    {
		switch(ch) 
        {
		    case 'i': result.in = optarg; break;
		    case 'o': result.out = optarg; break;
		}
	}

    if (argc != 5 || result.in == NULL || result.out == NULL)
    {
        result.ecode = EPARAM;
    }

    return result;
}

/**
 * Get MAC address of an interface and store it in *ether_host
 * @param *int_name interface name
 * @param *ether_host MAC address (in network byte order)
 */
void get_mac_address(char *int_name, u_int8_t *ether_host)
{
    // Open socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, int_name, IFNAMSIZ - 1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);
    
    // Store address
    for (int i = 0; i < ETH_ALEN; i++)
    {
        ether_host[i] = ifr.ifr_hwaddr.sa_data[i];
    }

    close(fd);
}

/**
 * Print MAC address to file in form XX:XX:XX:XX:XX:XX
 * @param *file print destination
 * @param *ether_host pointer to MAC address (in network byte order)
 */
int print_mac(FILE *file, u_int8_t *ether_host)
{
    return fprintf(file, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                            ether_host[0], ether_host[1],
                            ether_host[2], ether_host[3],
                            ether_host[4], ether_host[5]);
}

/**
 * Get IPv6 addresses of an interface (in network byte order)
 * @param *ip6_iface store addresses here
 * @param *iface name of the interface
 */
int get_ip6_addr(t_ip6 *ip6_iface, char *iface)
{
    // init
    ip6_iface->addr = NULL;
    ip6_iface->len = 0;
    ip6_iface->iface = iface;
    
    struct ifaddrs *ifaddr, *ifa;

    // get IPv6 addresses from system
    if (getifaddrs(&ifaddr) == -1) 
    {
        return EGETIPS;
    }
  
    int size = 4;
    ip6_iface->addr = (struct in6_addr**)malloc(size * sizeof(struct in6_addr*));
    
    if (ip6_iface->addr == NULL)
    {
        freeifaddrs(ifaddr);
        return EOUT_MEM;
    }

    // store addresses
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        // Store only IPv6 addresses
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET6)
        {
            continue;
        }
        
        // Only addresses of interface named iface
        if (strcmp(ifa->ifa_name, iface) != 0)
        {
            continue;
        }
        
        if (ip6_iface->len == size)
        { // Buffer too small
            size *= 2;
            struct in6_addr **rebuf = (struct in6_addr**)realloc(ip6_iface->addr, size * sizeof(struct in6_addr*));

            if (rebuf == NULL)
            { 
                free_iface(*ip6_iface);
                freeifaddrs(ifaddr);
                return EOUT_MEM;
            } 

            ip6_iface->addr = rebuf;
        }
        
        ip6_iface->addr[ip6_iface->len] = (struct in6_addr*)malloc(sizeof(struct in6_addr));
        
        *ip6_iface->addr[ip6_iface->len] = *(&((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr);
        
        ip6_iface->len += 1;
    }

    freeifaddrs(ifaddr);
    
    return EOK;
}

/**
 * Compare two IPv6 addresses (byte to byte)
 * Returns 0 if addresses are equal, -1 if not
 * @param *a first IPv6 address to compare
 * @param *b second IPv6 address to compare 
 */
int cmp_in6_addr(const struct in6_addr *a, const struct in6_addr *b)
{
    for (int i = 0; i < IPV6_ADDR_SIZE; i++)
    {
        if (a->s6_addr[i] != b->s6_addr[i])
        {
            return -1;
        }
    }
    
    return 0;
}

/**
 * Compare two MAC addresses (byte to byte)
 * Returns 0 if addresses are equal, -1 if not
 * @param *a first MAC address to compare
 * @param *b second MAC address to compare
 */
int cmp_mac_addr(u_int8_t *a, u_int8_t *b)
{
    for (int i = 0; i < ETH_ALEN; i++)
    {
        if (a[i] != b[i])
        {
            return -1;
        }
    }
    
    return 0;
}

/**
 * Check for unspecified IPv6 address 
 * Unspecified IPv6 address: ::
 * Returns -1 if unspecified, 0 if not
 * @param *ip IPv6 address
 */
int check_unspec_ip(const struct in6_addr *ip)
{
    for (int i = 0; i < IPV6_ADDR_SIZE; i++)
    {
        if (ip->s6_addr[i] != 0)
        {
            return 0;
        }
    }
    
    return -1;
}

/**
 * Helper function to icmpv6_cksum(), calculates sum
 * If noth is 1, call noths() to data.
 * @param *addr data
 * @param len data length
 * @param ntoh conversion
 */
uint32_t cksum_sum(uint16_t *addr, int len, int ntoh) 
{ 
    uint16_t * w = addr; 
    uint16_t ret = 0; 
    uint32_t sum = 0; 

    while (len > 1) 
    { 
        ret = *w++;
        
        if (ntoh) 
        {
            ret = ntohs(ret); 
        }
        
        sum += ret; 
        len -= 2; 
    } 

    if (len == 1)
    { 
        *(unsigned char*)(&ret) = *(unsigned char*)w; 
        sum += ret; 
    } 

    return sum; 
}

/**
 * Calculate ICMPv6 checksum (in host byte order)
 * @param *src_ip source IPv6 address
 * @param *dst_ip destination IPv6 address
 * @param *payload pointer to ICMPv6 payload
 * @param payload_len ICMPv6 payload length
 */
uint16_t icmpv6_cksum(struct in6_addr* src_ip, struct in6_addr* dst_ip, 
                             char* payload, int payload_len) 
{ 
    uint32_t sum = 0; 
    uint16_t val = 0; 

    // IPv6 pseudo header 
    sum += cksum_sum(src_ip->s6_addr16, IPV6_ADDR_SIZE, 1); 
    sum += cksum_sum(dst_ip->s6_addr16, IPV6_ADDR_SIZE, 1); 
    
    val = payload_len;
    sum += cksum_sum(&val, 2, 0); 
    
    val = 58; 
    sum += cksum_sum(&val, 2, 0); 

    // ICMPv6 packet
    sum += cksum_sum((uint16_t*)payload, payload_len, 1); 

    // perform 16-bit one's complement of sum     
    sum += sum >> 16;
    return (uint16_t)~sum;
}

/**
 * Init Neighbor Cache
 * @param *nt Neighbor Cache pointer
 * @param *fname interface name
 */
void init_nt(t_nt *nt, char *fname)
{
    nt->items = NULL;
    nt->len = 0;
    nt->allocated_len = 0;
    nt->fname = fname;
}

/**
 * Search entry in Neighbor Cache by IPv6 address
 * Return 0 if founded an entry, -1 if not
 * @param *nt Neighbor Cache pointer
 * @param *addr IPv6 address to search (in network byte order)
 * @param **item store founded item here
 */
int search_nt(t_nt *nt, struct in6_addr *addr, t_nt_item **item)
{
    for (int i = 0; i < nt->len; i++)
    {
        if (cmp_in6_addr(&(nt->items[i].addr), addr) == 0)
        {
            if (item != NULL)
            {
                *item = nt->items + i;
            }
            
            return 0;
        }
    }

    return -1;
}

/**
 * Add entry to Neighbor Cache
 * @param *nt Neighbor Cache pointer
 * @param *addr IPv6 address (in network byte order)
 * @param *ether MAC address (in network byte order)
 * @param state state of entry
 */
int add_nt(t_nt *nt, struct in6_addr *addr, u_int8_t *ether, int state)
{
    t_nt_item *old_item;
    
    // Search for the IPv6 address in Neighbor Cache
    // If found, update entry
    if (search_nt(nt, addr, &old_item) == 0)
    {
        for (int i = 0; i < ETH_ALEN; i++)
        {
            old_item->ether[i] = ether[i];
        }
        
        old_item->state = state;
        
        return EOK;
    }

    if (nt->len == nt->allocated_len)
    { // Buffer too small
        nt->allocated_len *= 2;
        
        if (nt->allocated_len == 0)
        {
            nt->allocated_len = 1;
        }
        
        t_nt_item *rebuf = (t_nt_item*)realloc(nt->items, nt->allocated_len * sizeof(t_nt_item));

        if (rebuf == NULL)
        { 
            free(nt->items);
            return EOUT_MEM;
        }
        
        nt->items = rebuf;
    }
    
    // Store entry
    t_nt_item *item = nt->items + nt->len;
    
    for (int i = 0; i < IPV6_ADDR_SIZE; i++)
    {
        item->addr.s6_addr[i] = addr->s6_addr[i];
    }
    
    for (int i = 0; i < ETH_ALEN; i++)
    {
        item->ether[i] = ether[i];
    }
    
    item->state = state;
    
    nt->len += 1;
    
    return EOK;
}

/**
 * Print information about packet (not ICMPv6 NS or NA packet)
 * "IN: time, srcmac > dstmac, ipv6src > ipv6dst"
 * "OUT: time, srcmac > dstmac, ipv6src > ipv6dst"
 * @param *args pointer to a character, which can be 'i' or 'o' (choose interface)
 * @param *sec timestamp of packet
 * @param *ether_shost packet source MAC address (in network byte order)
 * @param *ether_dhost packet destination MAC address (in network byte order)
 * @param *ip6_src packet source IPv6 address (in network byte order)
 * @param *ip6_dst packet destination IPv6 address (in network byte order)
 * @param newline if not 0 then put new line
 */
int print_info(u_char *args, const time_t *sec, u_int8_t *ether_shost, u_int8_t *ether_dhost,
               struct in6_addr *ip6_src, struct in6_addr *ip6_dst, int newline)
{
    char str[INET6_ADDRSTRLEN];
    struct tm tm = *localtime(sec);
    
    // IN or OUT (interface)
    printf(CHECK_IFACE_IN ? "IN:  " : "OUT: ");
    
    // Time (hh:mm:ss)
    printf("%.2d:%.2d:%.2d, ", tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    // Source MAC address
    //printf("%s > ", ether_ntoa((struct ether_addr *)ether->ether_shost));
    print_mac(stdout, ether_shost);
    printf(" > ");
    
    // Destination MAC address
    //printf("%s, ", ether_ntoa((struct ether_addr *)ether->ether_dhost));
    print_mac(stdout, ether_dhost);
    printf(", ");

    // Source IPv6 address
    printf("%s > ", inet_ntop(AF_INET6, ip6_src, str, INET6_ADDRSTRLEN));
    
    // Destination IPv6 address
    printf("%s", inet_ntop(AF_INET6, ip6_dst, str, INET6_ADDRSTRLEN));
    
    if (newline)
    {
        printf("\n");
    }
    
    return EOK;
}

/**
 * Print information about ICMPv6 NS or NA packet
 * "IN: time, srcmac > dstmac, ipv6src > ipv6dst, ICMPv6[NS|NA] target, [slla|tlla]"
 * "OUT: time, srcmac > dstmac, ipv6src > ipv6dst, ICMPv6[NS|NA] target, [slla|tlla]"
 * @param *args pointer to a character, which can be 'i' or 'o' (choose interface)
 * @param *sec timestamp of packet
 * @param *ether_shost packet source MAC address (in network byte order)
 * @param *ether_dhost packet destination MAC address (in network byte order)
 * @param *ip6_src packet source IPv6 address (in network byte order)
 * @param *ip6_dst packet destination IPv6 address (in network byte order)
 * @param *target target IPv6 address (in ICMPv6 payload) (in network byte order)
 * @param *icmp_options MAC address in ICMPv6 payload (options field) (in network byte order)
 * @param type type of packet: NS or NA
 */
int print_link_info(u_char *args, const time_t *sec, u_int8_t *ether_shost, u_int8_t *ether_dhost,
                    struct in6_addr *ip6_src, struct in6_addr *ip6_dst, struct in6_addr *target, 
                    u_int8_t *icmp_options, int type)
{
    char str[INET6_ADDRSTRLEN];
    
    print_info(args, sec, ether_shost, ether_dhost, ip6_src, ip6_dst, 0);
    
    printf(", ");

    if (type == ND_NEIGHBOR_SOLICIT)
    {
        printf("ICMPv6 NS %s, ", inet_ntop(AF_INET6, target, str, INET6_ADDRSTRLEN));
    }
    else if (type == ND_NEIGHBOR_ADVERT)
    {
        printf("ICMPv6 NA %s, ", inet_ntop(AF_INET6, target, str, INET6_ADDRSTRLEN)); 
    }
    
    print_mac(stdout, icmp_options);
    
    printf("\n");
    
    return EOK;
}

/**
 * Initialize PCap
 * @param **pcap pcap handler
 * @param *dev interface name
 * @param *errbuf error buffer
 * @param *fp filter pointer
 * @param *filter_exp filter expression
 */
int init_pcap(struct pcap **pcap, char *dev, char *errbuf, 
              struct bpf_program *fp, char *filter_exp)
{
    // Open device (P-mode, no timeout)
    *pcap = pcap_open_live(dev, BUF_SIZE, 1, -1, errbuf);
    
    if (*pcap == NULL)
    {
        return EPCAP_OPEN;
    }
    
    // Set device to non-blocking mode
    if (pcap_setnonblock(*pcap, 1, errbuf) < 0)
    {
        pcap_close(*pcap);
        return EPCAP_BLOCK;
    }
    
    // Compile filer (only IPv6 packets)
    if (pcap_compile(*pcap, fp, filter_exp, 0, 0) < 0)
    {
        pcap_close(*pcap);
        return EPCAP_COMPILE;
    }
    
    // Set filter
    if (pcap_setfilter(*pcap, fp) < 0) 
    {
        pcap_freecode(fp);
        pcap_close(*pcap);
        return EPCAP_SETFIL;
    }
    
    return EOK;
}

/**
 * Handle received packet
 * @param *args arguments ('i' or 'o')
 * @param *header packet length, etc.
 * @param *packet packet data
 */
void packet_received(u_char *args, 
                     const struct pcap_pkthdr *header,
					 const u_char *packet)
{
    // Check packet length
    // ------------------------------------------------------------------------
    
    if (header->len > header->caplen)
    {
        // Drop packet
        return;
    }
    
    if (!ETHER_IS_VALID_LEN(header->caplen))
    {
        // Drop packet
        return;
    }
    
    
    // Setup packet header structures (ethernet header, IPv6 header)
    // ------------------------------------------------------------------------
    
    // Ethernet header
    struct ether_header *ether = (struct ether_header*) packet;
    // payload: (u_char*)(packet + sizeof(struct ether_header))
    // payload length: header->caplen - ETHER_HDR_LEN - ETHER_CRC_LEN
    
    // Ethernet payload = IPv6 packet
    struct ip6_hdr *ip = (struct ip6_hdr*)(packet + sizeof(struct ether_header));
    // src ipv6: ip->ip6_src (in6_addr)
    // dst ipv6: ip->ip6_dst (in6_addr)
    // payload: (u_char*)(ip + sizeof(struct ip6_hdr))
    // payload length: ip->ip6_plen    
    
    // IPv6 payload = ICMPv6 (set later, only when ICMPv6 packet received)
    struct icmp6_hdr *icmp = (struct icmp6_hdr*)(packet + ETHER_HDR_LEN + sizeof(struct ip6_hdr));;
    struct in6_addr *target = NULL;
    u_int8_t *icmp_options = NULL;
    
    
    // Consult Neighbor Cache to find an entry for the source IPv6 address
    // If no entry exists, one is created in the STALE state.
    // ------------------------------------------------------------------------
    
    if (check_unspec_ip(&ip->ip6_src) == 0 && 
        cmp_mac_addr(CHECK_IFACE_IN ? ether_in : ether_out, ether->ether_shost) != 0)
    {
        t_nt *nt = CHECK_IFACE_IN ? &nt_in : &nt_out;
        t_nt_item *item;
        
        if (search_nt(nt, &ip->ip6_src, &item) != 0)
        {
            // No entry exists
            add_nt(nt, &ip->ip6_src, ether->ether_shost, NT_STALE);
        }
        else
        {
            // An entry found
            // Adjust entry state (?)
        }
    }
    
    
    // Print information to stdout and setup ICMPv6 structures
    // ------------------------------------------------------------------------
    
    if (ip->ip6_nxt == IPV6_ICMPV6 &&
       (icmp->icmp6_type == ND_NEIGHBOR_SOLICIT || 
        icmp->icmp6_type == ND_NEIGHBOR_ADVERT))
    {
        // IPv6 payload is ICMPv6 NS or NA packet
        
        target = (struct in6_addr*)(icmp->icmp6_data8 + 4);
        
        // skip IPv6 address, ICMPv6 Options Type and Length (+2)
        icmp_options = (u_int8_t*)(target) + IPV6_ADDR_SIZE + 2;
        
        print_link_info(args, &(header->ts.tv_sec), 
                        ether->ether_shost, ether->ether_dhost, 
                        &ip->ip6_src,  &ip->ip6_dst, target, icmp_options, icmp->icmp6_type);
    }
    else 
    {
        // IPv6 payload is NOT ICMPv6 packet
        
        print_info(args, &(header->ts.tv_sec), 
                   ether->ether_shost, ether->ether_dhost, 
                   &ip->ip6_src,  &ip->ip6_dst, 1);
    }
    

    // Check source MAC address and destination IPv6 address
    // If these addresses belong to this host, drop the packet
    // ------------------------------------------------------------------------
    
    // Check source MAC address
    if (cmp_mac_addr(ether_in, ether->ether_shost) == 0 ||
        cmp_mac_addr(ether_out, ether->ether_shost) == 0)
    {
        // Drop packet
        return;
    }

    // Check destination IPv6 address (both interfaces)
    t_ip6 *ip6_iface = &ip6_in;
    int len = ip6_in.len;
    
    for (int i = 0; i < len; i++)
    {
        if (cmp_in6_addr(ip6_iface->addr[i], &ip->ip6_dst) == 0)
        {
            // Drop packet
            return;
        }
    }
    
    ip6_iface = &ip6_out;
    len = ip6_out.len;
    
    for (int i = 0; i < len; i++)
    {
        if (cmp_in6_addr(ip6_iface->addr[i], &ip->ip6_dst) == 0)
        {
            // Drop packet
            return;
        }
    }
    
    
    // Set source and destination MAC addresses
    // ------------------------------------------------------------------------
    
    // The source address will be the address of the outgoing
    // interface.
    u_int8_t *f_ether_src = CHECK_IFACE_IN ? ether_out : ether_in;
    u_int8_t *f_ether_dst;
    
    if (ip->ip6_dst.s6_addr[0] == 0xFF)
    {
        // Multicast ff00::/8
        
        f_ether_dst = ether->ether_dhost;
    }
    else
    {
        // Unicast
        
        // The destination address will be the address in the neighbor
        // entry corresponding to the destination IPv6 address.
        t_nt_item *item;
        t_nt *s_nt = CHECK_IFACE_IN ? &nt_out : &nt_in;
        
        if (search_nt(s_nt, &ip->ip6_dst, &item) == 0)
        {
            f_ether_dst = item->ether;
        }
        else
        {
            // Not found an entry in Neighbor Cache
            // Drop the packet
            return;
        }
    }
    
    
    // Handle ICMPv6 ND and NA packet
    // ------------------------------------------------------------------------
    if (ip->ip6_nxt == IPV6_ICMPV6 && 
        (icmp->icmp6_type == ND_NEIGHBOR_SOLICIT || 
         icmp->icmp6_type == ND_NEIGHBOR_ADVERT))
    {
        // Replace link layer address within payload
        for (int i = 0; i < ETH_ALEN; i++)
        {
            icmp_options[i] = f_ether_src[i];            
        }
        
        icmp->icmp6_cksum = 0;
        u_int16_t checksum = icmpv6_cksum(&ip->ip6_src, &ip->ip6_dst, (char*)(icmp), 
                                          header->caplen - ETHER_HDR_LEN - sizeof(struct ip6_hdr));
        icmp->icmp6_cksum = htons(checksum);
    }
    

    // Sending packet
    // ------------------------------------------------------------------------
    
    // Choose interface
    libnet_t *lc = CHECK_IFACE_IN ? lc_out : lc_in;
    
    // Build the new ethernet packet  
    libnet_build_ethernet(f_ether_dst, // destination MAC 
                          f_ether_src, // source MAC
                          ETHERTYPE_IPV6,
                          (u_char*)(packet + sizeof(struct ether_header)), // payload
                          //header->caplen - ETHER_HDR_LEN - ETHER_CRC_LEN,  // payload size
                          header->caplen - ETHER_HDR_LEN, // payload size
                          lc, // libnet context
                          0);
    
    // Send the packet
    if (libnet_write(lc) < 0)
    {
        print_ecode(ELIBNET_WRITE, libnet_geterror(lc));
    }
    
    // Clear libnet contex
    libnet_clear_packet(lc);
}

/**
 * Print Neighbor Cache to stdout
 * @param *nt Neighbor Cache pointer
 */
void print_nt(t_nt *nt)
{
    char str[INET6_ADDRSTRLEN];

    printf("%s - len: %d, allocated_len: %d\n", nt->fname, nt->len, nt->allocated_len);
    printf("--------------------------------------------------------------------------------\n");
    
    printf("#\tIPv6%44sMAC                State\n", "");
    for (int i = 0; i < nt->len; i++)
    {
        t_nt_item *item = nt->items + i;
        
        printf("%.2d\t", i + 1);
        printf("%-48s", inet_ntop(AF_INET6, &(item->addr), str, INET6_ADDRSTRLEN));
        print_mac(stdout, item->ether);
        printf("      %d\n", item->state);
    }
}

/**
 * Signal handler for handling signals SIGTERM, SIGQUIT, SIGINT
 * @param s - signal number
 */
static void signal_handler(int s)
{
    // Print some info
    printf("\n********************************************************************************\n\n");
    
    print_nt(&nt_in);
    
    printf("\n\n");
    
    print_nt(&nt_out);
    
    printf("\n********************************************************************************\n");

    printf("Releasing resources...");

    // Free resources
    if (s == SIGTERM || s == SIGQUIT || s == SIGINT)
    {
        free_all
    }
    
    printf("Bye...\n");
    
    exit(EXIT_SUCCESS);
}


// Function main
// -----------------------------------------------------------------------------

/**
 * Main program
 */
int main(int argc, char *argv[])
{
    // Process parameters
    // -------------------------------------------------------------------------
    
    tParams params = get_params(argc, argv);

    if (params.ecode != EOK)
    { // Wrong parameters
        print_ecode(params.ecode);
        return EXIT_FAILURE;
    }
    
   
    // Init variables
    // -------------------------------------------------------------------------
    
    // Turn off stdout buffer
    setbuf(stdout, NULL);
    
    // Set signal handler for signals SIGTERM, SIGQUIT, SIGINT
    signal(SIGTERM, &signal_handler);
	signal(SIGQUIT, &signal_handler);
	signal(SIGINT, &signal_handler);

    // PCap variables
    int result;
    pcap_in = NULL;
    pcap_out = NULL;
    char errbuf[PCAP_ERRBUF_SIZE];
	char filter_exp[] = "ether proto \\ip6";	// The filter expression
    u_char fl_in = IFACE_IN;
    u_char fl_out = IFACE_OUT;
    
    // Init PCap for both interfaces
    for (int i = 0; i < 2; i++)
    {
        struct pcap *pcap = NULL;
        char *dev = (i == 0) ? params.in : params.out;
        struct bpf_program *fp = (i == 0) ? &fp_in : &fp_out;
        
        result = init_pcap(&pcap, dev, errbuf, fp, filter_exp);

        switch (result)
        {
            case EPCAP_OPEN: print_ecode(result, dev, errbuf);                      break;
            case EPCAP_COMPILE: print_ecode(result, filter_exp, pcap_geterr(pcap)); break;
            case EPCAP_SETFIL: print_ecode(result, filter_exp, pcap_geterr(pcap));  break;
            case EPCAP_BLOCK: print_ecode(result, dev, errbuf);                     break;
        }
        
        if (result != EOK)
        {
            if (i == 1)
            {
                pcap_freecode(&fp_in);
                pcap_close(pcap_in);
            }
            
            return EXIT_FAILURE;
        }
        
        if (i == 0)
        {
            pcap_in = pcap;
        }
        else
        {
            pcap_out = pcap;
        }
    }
    
    // Get MAC address of this host
    get_mac_address(params.in, ether_in);
    get_mac_address(params.out, ether_out);
    
    // Get IPv6 addresses of both interfaces
    if ((result = get_ip6_addr(&ip6_in, params.in)) < 0 || 
        (result = get_ip6_addr(&ip6_out, params.out)) < 0)
    {
        print_ecode(result);
        
        free_pcap
        free_iface_all
        return EXIT_FAILURE;
    }
    
    // Socket descriptors
    int s_in = pcap_fileno(pcap_in);
    int s_out = pcap_fileno(pcap_out);
    
    // Set of examined descriptors
    fd_set rset;
    
    // Init Neighbor Cache
    init_nt(&nt_in, "NT IN");
    init_nt(&nt_out, "NT OUT");
    
    // Init Libnet
    char errbuf2[LIBNET_ERRBUF_SIZE];
    
    if ((lc_in = libnet_init(LIBNET_LINK, params.in, errbuf2)) == NULL)
    {
        print_ecode(ELIBNET_INIT, params.in, errbuf2);
        free_pcap
        free_iface_all
        free_nt
        return EXIT_FAILURE;
    }
    
    if ((lc_out = libnet_init(LIBNET_LINK, params.out, errbuf2)) == NULL)
    {
        print_ecode(ELIBNET_INIT, params.out, errbuf2);
        libnet_destroy(lc_in);
        free_pcap
        free_iface_all
        free_nt
        return EXIT_FAILURE;
    }
    
   
    // Main loop
    // -------------------------------------------------------------------------
    
    while (1)
    {
        // Empty set of examined descriptors
        FD_ZERO(&rset);
        
        // Add pcap_in and pcap_out socket into set of examined sockets
        FD_SET(s_in, &rset);
        FD_SET(s_out, &rset);

        int nb  = select(FD_SETSIZE, &rset, NULL, NULL, /*&wait*/ NULL);
        
        if (nb <= 0)
        {
            free_all
            print_ecode(ESEL);
            return EXIT_FAILURE;
        }
        
        if (FD_ISSET(s_in, &rset))
        {
            // Read from socket IN
            if (pcap_dispatch(pcap_in, 1, packet_received, &fl_in) < 0)
            {
                free_all
                print_ecode(EPCAP_REC, params.in, pcap_geterr(pcap_in));
                return EXIT_FAILURE;
            }
        }
        
        if (FD_ISSET(s_out, &rset))
        {
            // Read from socket OUT
            if (pcap_dispatch(pcap_out, 1, packet_received, &fl_out) < 0)
            {
                free_all
                print_ecode(EPCAP_REC, params.out, pcap_geterr(pcap_out));
                return EXIT_FAILURE;
            }
        }
    }
}

/* end of ndpv6.c */
