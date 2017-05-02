/*
 * File:     myripresponse.c
 * Date:     2015-11-17
 * Encoding: UTF-8
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  RIP Responser
 */

// Includes
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include<arpa/inet.h>
#include <sys/socket.h>
#include "rip.h"


// Constants
// ----------------------------------------------------------------------------
/** Program error codes */
enum tecodes
{
    EOK = 0,
    EPARAM,
    
    ESOCKET,

    EUNKNOWN
};

/** Error messages */
const char *ECODEMSG[] =
{
    /* EOK */
    "Everything OK.\n",
    /* EPARAM */
    "Error: Bad command line parameters!\n",
    
    /* ESOCKET */
    "Error: socket\n",
	
    /* EUNKNOWN */
    "Error: Unknown!\n"
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
	char* interface;
    struct in_addr ip;
    struct in_addr netmask;
    struct in_addr nexthop;
    uint32_t metric;
    uint16_t route_tag;
    char* password;
    int ecode; 
} tParams;


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
    // Init structure
    tParams result = 
    {  
	    .interface = NULL,
        .ip.s_addr = 0,
        .netmask.s_addr = 0,
        .nexthop.s_addr = 0,
        .metric = htonl(1),
        .route_tag = 0,
        .password = NULL,
        .ecode = EOK
    };
    
    opterr = 0;
    char ch;
    int number;
    bool ip_defined = false;

    while ((ch = getopt(argc, argv, "i:r:n:m:t:p:")) != -1) 
    {
		switch(ch) 
        {
		    case 'i': 
                result.interface = optarg;
                break;
            case 'r': 
                // input = 192.168.1.0/24
                // indexOf "/"
                number = strcspn(optarg, "/");
                if (number >= 0 && number < (int)strlen(optarg)-1)
                {
                    // optarg = 192.168.1.0
                    optarg[number] = 0;
                    if (inet_aton(optarg, &result.ip) == 0)
                    {
                        result.ecode = EPARAM;
                    }
                    else
                    {
                        // rest - "24"
                        char* mask_text = optarg + number + 1;
                        int mask;
                        
                        if (sscanf(mask_text, "%d", &mask) == 1 &&
                            mask >= 8 && mask <= 30)
                        {
                            // viz. http://stackoverflow.com/a/218748
                            uint32_t netmask = 0xFFFFFFFF;
                            netmask <<= 32 - mask;
                            result.netmask.s_addr = htonl(netmask);
                            ip_defined = true;
                        }
                        else
                        {
                            result.ecode = EPARAM;
                        }
                    }
                }
                else
                {
                    result.ecode = EPARAM;
                }
                break;
            case 'n': 
                if (inet_aton(optarg, &result.nexthop) == 0)
                    result.ecode = EPARAM;
                break;
            case 'm': 
                if (sscanf(optarg, "%d", &number) == 1 && number >= 0 && number <= 16) 
                    result.metric = htonl(number);
                else 
                    result.ecode = EPARAM;
                break;
            case 't': 
                if (sscanf(optarg, "%d", &number) == 1 && number >= 0 && number <= 65535) 
                    result.route_tag = htons((uint16_t)number);
                else 
                    result.ecode = EPARAM;
                break;
            case 'p': 
                result.password = optarg; 
                break;
            default: 
                result.ecode = EPARAM; 
                break;
		}
	}
    
    if (!ip_defined)
        result.ecode = EPARAM;

    return result;
}

int create_rip_packet(u_char* packet, tParams *params)
{
    struct ripv2hdr* rip = (struct ripv2hdr*)packet;
    rip->version = RIP_V2;
    rip->command = RIP_COMMAND_RESPONSE;
    rip->reserved = 0;
    int size = sizeof(struct ripv2hdr);    
    
    if (params->password != NULL)
    {
        struct ripauth* auth = (struct ripauth*)(packet + sizeof(struct ripv2hdr));
        auth->reserved = RIP_V2_AUTH;
        auth->auth_type = htons(RIP_V2_SIMPLE_PASSWORD);
    
        u_char* password = (u_char*)auth + sizeof(struct ripauth);
        int input_len = strlen(params->password);
        for (int i = 0; i < RIP_V2_SIMPLE_PASSWORD_SIZE; i++)
        {
            password[i] = i < input_len ? params->password[i] : 0;
        }
        size += sizeof(struct ripauth) + RIP_V2_SIMPLE_PASSWORD_SIZE;
    }
    
    struct ripv2route* route = (struct ripv2route*)(packet + size);
    route->addr_family = htons(RIP_V2_ADDRESS_FAMILY);
    route->route_tag = params->route_tag;
    route->ip = params->ip.s_addr;
    route->subnet_mask = params->netmask.s_addr;
    route->next_hop = params->nexthop.s_addr;
    route->metric = params->metric;
    size += sizeof(struct ripv2route);
    return size;
}

void print_help()
{
    printf("RIPv2 Responser\n");
    printf("Usage:\n");
    printf(" {-i <interface_name>}\n");
    printf(" -r <IPv4>/[8-30]: route\n");
    printf(" {-n <IPv4>}: next hop\n");
    printf(" {-m [0-16]}: metric\n");
    printf(" {-t [0-65535]}: router tag\n");
    printf(" {-p <password>}\n");
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
    
    // socket
    int sock;
    struct sockaddr_in server;
    
    server.sin_addr.s_addr = inet_addr(RIP_MULTICAST_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(RIP_PORT);
    
    printf("creating socket... ");
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        print_ecode(ESOCKET);
        return EXIT_FAILURE;
    }
    
    printf("OK\nbinding socket... ");
    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        print_ecode(ESOCKET);
        return EXIT_FAILURE;
    }
    
    u_char ttl = 1;
    u_char loop = 1;
    
    printf("OK\nsetting socket parameters... ");
    
    // nastaveni TTL
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
        print_ecode(ESOCKET);
        return EXIT_FAILURE;
    }
    
    if (params.interface != NULL)
    {
        if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, params.interface, strlen(params.interface)) < 0)
        {
            print_ecode(ESOCKET);
            return EXIT_FAILURE;
        }
    }
    
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
    {
        print_ecode(ESOCKET);
        return EXIT_FAILURE;
    }
    
    u_char packet[1024];
    
    printf("OK\ncreating RIPv2 Response packet... ");
    int size = create_rip_packet(packet, &params);
    
    printf("OK\nsending packet... ");
    if (sendto(sock, packet, size, 0, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        print_ecode(ESOCKET);
        return EXIT_FAILURE;
    }
    
    printf("OK\npacket sent\n");
    
    close(sock);
    
    return EXIT_SUCCESS;    
}