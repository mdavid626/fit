/*
 * File: myripsniffer.c
 * Date:     2015-11-17
 * Encoding: UTF-8
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  RIP sniffer
 */
 

// Includes
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include<arpa/inet.h>
#include<net/ethernet.h>
#include<netinet/udp.h>
#include<netinet/ip.h>
#include<netinet/ip6.h>
#include <pcap.h>
#include "rip.h"


// Macros
// ----------------------------------------------------------------------------

// PCap maximum packet size (bytes)
#define BUF_SIZE 65536

#define IPV4_UDP 17

/**
 * Free resources allocated by PCap
 */
#define free_pcap pcap_freecode(&fp_iface);\
                  if (pcap_iface != NULL) pcap_close(pcap_iface);


// Constants
// ----------------------------------------------------------------------------
/** Program error codes */
enum tecodes
{
    EOK = 0,
    EPARAM,

    ESIG_ACTION,
    ESIG_BLOCK,
    
    EPCAP_OPEN,
    EPCAP_BLOCK,
    EPCAP_COMPILE,
    EPCAP_SETFIL,
    EPCAP_REC,
	
    ESEL,
    
    EUNKNOWN
};

/** Error messages */
const char *ECODEMSG[] =
{
    /* EOK */
    "Everything OK.\n",
    /* EPARAM */
    "Error: Wrong command line arguments!\n",
	
    /* ESIG_ACTION */
    "Error: sigaction!\n",
    /* ESIG_BLOCK */
    "Error: sigblock!\n",
    
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
 
    /* EUNKNOWN */
    "Error: Unknown!\n"
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
	char* iface;            /**< Inner interface name */
    int ecode;           /**< Error code (enum tecodes) */
} tParams;

// Global variables
// ----------------------------------------------------------------------------

// PCap handler for both interfaces
struct pcap *pcap_iface;

// PCap filter structures for both interfaces
struct bpf_program fp_iface;


// Functions
// ----------------------------------------------------------------------------
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
	    .iface = NULL,
        .ecode = EOK
    };

    char ch;
    opterr = 0;

    while ((ch = getopt(argc,argv,"i:")) != -1) 
    {
		switch(ch) 
        {
		    case 'i': result.iface = optarg; break;
            default: result.ecode = EPARAM; break;
		}
	}

    if (argc != 3 || result.iface == NULL)
    {
        result.ecode = EPARAM;
    }

    return result;
}

/**
 * Signal handler for handling signals SIGTERM, SIGQUIT, SIGINT
 * @param s - signal number
 */
static void signal_handler(int s)
{
    // Free resources
    if (s == SIGTERM || s == SIGQUIT || s == SIGINT)
    {
        free_pcap
    }
    
    printf("\nBye...\n");
    exit(EXIT_SUCCESS);
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
    
    // Compile filer
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

void print_etherneth(struct ethhdr *eth)
{
    printf("[MAC] %.2X-%.2X-%.2X-%.2X-%.2X-%.2X -> %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
        eth->h_source[0], eth->h_source[1], eth->h_source[2], 
        eth->h_source[3], eth->h_source[4], eth->h_source[5],
        eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], 
        eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
}

void print_ipaddr(uint32_t addr, const char* format)
{
    struct in_addr ipaddress;
    ipaddress.s_addr = addr;
    printf(format, inet_ntoa(ipaddress));
}

void print_ip6addr(struct in6_addr *ip6, const char* format)
{
    char buffer[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, ip6, buffer, INET6_ADDRSTRLEN);
    printf(format, buffer);
}

void print_iph(struct iphdr *iph, struct udphdr *udph)
{
    print_ipaddr(iph->saddr, "[IP:Port] %s:");
    printf("%d -> ", ntohs(udph->source));
    print_ipaddr(iph->daddr, "%s:");
    printf("%d\n", ntohs(udph->dest));
}

void print_ip6h(struct ip6_hdr *iph, struct udphdr *udph)
{
    print_ip6addr(&iph->ip6_src, "[IPv6:Port] %s:");
    printf("%d -> ", ntohs(udph->source));
    print_ip6addr(&iph->ip6_dst, "%s:");
    printf("%d\n", ntohs(udph->dest));
}

char* get_command_desc(unsigned int command)
{
    switch (command)
    {
        case RIP_COMMAND_REQUEST: return "Request";
        case RIP_COMMAND_RESPONSE: return "Response";
        case RIP_COMMAND_TRACEON: return "Traceon";
        case RIP_COMMAND_TRACEOFF: return "Traceoff";
        case RIP_COMMAND_RESERVED: return "Reserved";
    }
    return "unknown";
}


// viz. http://stackoverflow.com/a/109025
uint32_t get_subnet_bits(uint32_t i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void print_ripv1route(struct ripv1route *route)
{
    printf("[Route] ");
    print_ipaddr(route->ip, "%s ");
    printf("[%d]\n", ntohl(route->metric));
}

void print_ripv1hdr(struct ripv1hdr* rip, uint32_t size)
{
    char* cmd_desc = get_command_desc((unsigned int)rip->command);
    printf("=========>RIPv%d %s (%d bytes)<=========\n", 
            (unsigned int)rip->version, cmd_desc, size);
    printf("[Address Family Identifier] %d\n", ntohs(rip->address_family));
}

void print_ripv1(struct ripv1hdr* rip, uint32_t size)
{
    const unsigned short route_size = sizeof(struct ripv1route);
    
    size -= sizeof(struct ripv1hdr);
    
    struct ripv1route* data = (struct ripv1route*)((u_char*)rip + sizeof(struct ripv1hdr));
    
    while (size >= route_size)
    {
        print_ripv1route(data);
        
        data += route_size;
        size -= route_size;
    }
}

void print_ripv2route(struct ripv2route *route)
{
    printf("[Route (%d/%d)] ", ntohs(route->addr_family), ntohs(route->route_tag));
    print_ipaddr(route->ip, "%s/");
    printf("%d", get_subnet_bits(route->subnet_mask));
    print_ipaddr(route->next_hop, " -> %s ");
    printf("[%d]\n", ntohl(route->metric));
}

void print_ripv2hdr(struct ripv2hdr* rip, uint32_t size)
{
    char* cmd_desc = get_command_desc((unsigned int)rip->command);
    printf("=========> RIPv%d %s (%d bytes) <=========\n", 
        (unsigned int)rip->version, cmd_desc, size);
}

void print_ripv2(struct ripv2hdr* rip, uint32_t size)
{
    const unsigned short route_size = sizeof(struct ripv2route);
    
    size -= sizeof(struct ripv2hdr);
    u_char* data = (u_char*)rip + sizeof(struct ripv2hdr);
    
    struct ripauth* auth = (struct ripauth*)data;
    
    if (size >= sizeof(struct ripauth) &&
        auth->reserved == RIP_V2_AUTH && ntohs(auth->auth_type) == RIP_V2_SIMPLE_PASSWORD)
    {
        data += sizeof(struct ripauth);
        size -= sizeof(struct ripauth);
        
        u_char* password = (u_char*)data;
        printf("[Authentication] Password: %s\n", password);
        
        data += RIP_V2_SIMPLE_PASSWORD_SIZE;
        size -= RIP_V2_SIMPLE_PASSWORD_SIZE;
    }
    
    while (size >= route_size)
    {
        print_ripv2route((struct ripv2route*)data);
        
        data += route_size;
        size -= route_size;
    }
}

void print_ripngroute(struct ripngroute *route)
{
    if (route->route_tag == 0 && route->prefix_len == 0 && route->metric == 0xFF)
    {
        print_ip6addr(&route->ip6_addr, "[Next hop] %s\n");
    }
    else
    {
        printf("[Route (%d)] ", (uint32_t)route->route_tag);
        print_ip6addr(&route->ip6_addr, "%s/");
        printf("%d ", (uint32_t)route->prefix_len); 
        printf("[%d]\n", (uint32_t)route->metric);
    }
}

void print_ripnghdr(struct ripnghdr* rip, uint32_t size)
{
    char* cmd_desc = get_command_desc((unsigned int)rip->command);
    printf("=========> RIPng %s (%d bytes) <=========\n", cmd_desc, size);
}

void print_ripng(struct ripnghdr* rip, uint32_t size)
{
    const unsigned short route_size = sizeof(struct ripngroute);
    
    size -= sizeof(struct ripnghdr);
    u_char* data = (u_char*)rip + sizeof(struct ripnghdr);
    
    while (size >= route_size)
    {
        print_ripngroute((struct ripngroute*)data);
        
        data += route_size;
        size -= route_size;
    }
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
    uint32_t p_size = header->caplen;
    
    if (p_size > header->len ||
        p_size < sizeof(struct ethhdr) ||
        args != NULL) return;
    
    struct ethhdr* ethh = (struct ethhdr*)packet;
    p_size -= sizeof(struct ethhdr);
    
    if (ntohs(ethh->h_proto) == ETH_P_IP)
    {
        struct iphdr *iph = (struct iphdr*)(packet + sizeof(struct ethhdr));
        
        if (p_size >= sizeof(struct iphdr) && iph->protocol == IPV4_UDP)
        {
            unsigned short iphdrlen = iph->ihl * 4;
            p_size -= iphdrlen;
            
            struct udphdr *udph = (struct udphdr*)((u_char*)iph + iphdrlen);
            
            if (p_size >= sizeof(struct udphdr) && 
                (ntohs(udph->source) == RIP_PORT || ntohs(udph->dest) == RIP_PORT))
            {
                struct ripv2hdr *rip = (struct ripv2hdr*)((u_char*)udph + sizeof(struct udphdr));
                p_size -= sizeof(struct udphdr);
                
                if (p_size >= sizeof(struct ripv2hdr))
                {
                    if (rip->version == RIP_V1 && p_size >= sizeof(struct ripv1hdr))
                    {
                        print_ripv1hdr((struct ripv1hdr*)rip, p_size);
                        print_etherneth(ethh);
                        print_iph(iph, udph);
                        print_ripv1((struct ripv1hdr*)rip, p_size);
                    }
                        
                    if (rip->version == RIP_V2)
                    {
                        print_ripv2hdr(rip, p_size);
                        print_etherneth(ethh);
                        print_iph(iph, udph);
                        print_ripv2(rip, p_size);
                    }
                }
            }
        }
    }
    else if (ntohs(ethh->h_proto) == ETH_P_IPV6)
    {
        struct ip6_hdr *iph = (struct ip6_hdr*)(packet + sizeof(struct ethhdr));
        
        if (p_size >= sizeof(struct ip6_hdr) &&
            iph->ip6_ctlun.ip6_un1.ip6_un1_nxt == IPV4_UDP)
        {
            struct udphdr *udph = (struct udphdr*)((u_char*)iph + sizeof(struct ip6_hdr));
            p_size -= sizeof(struct ip6_hdr);
            
            if (p_size >= sizeof(struct udphdr) &&
                ((ntohs(udph->source) == RIPng_PORT || ntohs(udph->dest) == RIPng_PORT)))
            {
                struct ripnghdr *rip = (struct ripnghdr*)((u_char*)udph + sizeof(struct udphdr));
                p_size -= sizeof(struct udphdr);
                
                if (p_size >= sizeof(struct ripnghdr) && rip->version == RIPng_V1)
                {
                    print_ripnghdr(rip, p_size);
                    print_etherneth(ethh);
                    print_ip6h(iph, udph);
                    print_ripng(rip, p_size);
                }
            }
        }
    }
}

void print_help()
{
    printf("RIPv1, RIPv2, RIPng sniffer\n");
    printf("Usage: ./myripsniffer -i <interface_name>\n");
    printf("For example: sudo ./myripsniffer -i eth0\n");
}

/**
 * Main entry point
 */
int main(int argc, char *argv[])
{
    // process arguments
    tParams params = get_params(argc, argv);

    if (params.ecode != EOK)
    { 
        print_help();
        print_ecode(params.ecode);
        return EXIT_FAILURE;
    }
    
    // Turn off stdout buffer
    setbuf(stdout, NULL);
    
    // Set signal handler for signals SIGTERM, SIGQUIT, SIGINT
    signal(SIGTERM, &signal_handler);
	signal(SIGQUIT, &signal_handler);
	signal(SIGINT, &signal_handler);
    
    // PCap variables
    int result;
    pcap_iface = NULL;
    char errbuf[PCAP_ERRBUF_SIZE]; 
	char filter_exp[] = "ether proto \\ip or ether proto \\ip6";
    
    // Init PCaP
    struct pcap *pcap = NULL;
    result = init_pcap(&pcap, params.iface, errbuf, &fp_iface, filter_exp);

    switch (result)
    {
        case EPCAP_OPEN: print_ecode(result, params.iface, errbuf);             break;
        case EPCAP_COMPILE: print_ecode(result, filter_exp, pcap_geterr(pcap)); break;
        case EPCAP_SETFIL: print_ecode(result, filter_exp, pcap_geterr(pcap));  break;
        case EPCAP_BLOCK: print_ecode(result, params.iface, errbuf);            break;
    }
    
    if (result != EOK)
    {
        free_pcap
        return EXIT_FAILURE;
    }    
    
    int s_iface = pcap_fileno(pcap);
    
    // Set of examined descriptors
    fd_set rset;
    
    printf("sniffing...\n");
    
    while (1)
    {
        // Empty set of examined descriptors
        FD_ZERO(&rset);
        
        // Add pcap_iface socket into set of examined sockets
        FD_SET(s_iface, &rset);

        int nb  = select(FD_SETSIZE, &rset, NULL, NULL, /*&wait*/ NULL);
        
        if (nb <= 0)
        {
            free_pcap
            print_ecode(ESEL);
            return EXIT_FAILURE;
        }
        
        if (FD_ISSET(s_iface, &rset))
        {
            // Read from socket IN
            if (pcap_dispatch(pcap, 1, packet_received, NULL) < 0)
            {
                free_pcap
                print_ecode(EPCAP_REC, params.iface, pcap_geterr(pcap));
                return EXIT_FAILURE;
            }
        }
    }
    
    return EXIT_SUCCESS;    
}