#ifndef RIP_H
#define RIP_H 1

#include<netinet/ip.h>
#include<netinet/ip6.h>

#define RIP_MULTICAST_IP "224.0.0.9"
#define RIP_PORT 520
#define RIPng_PORT 521
#define RIP_V1 1
#define RIP_V2 2
#define RIPng_V1 1
#define RIP_V2_AUTH 0xFFFF
#define RIP_V2_SIMPLE_PASSWORD 2
#define RIP_V2_SIMPLE_PASSWORD_SIZE 16
#define RIP_V2_ADDRESS_FAMILY 2

#define RIP_COMMAND_REQUEST 1
#define RIP_COMMAND_RESPONSE 2
#define RIP_COMMAND_TRACEON 3
#define RIP_COMMAND_TRACEOFF 4
#define RIP_COMMAND_RESERVED 5

struct ripv1hdr
{
    uint8_t command;
    uint8_t version;
    uint16_t reserved1;
    uint16_t address_family;
    uint16_t reserved2;
} _ripv1hdr;

struct ripv1route
{
    uint32_t ip;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t metric;
} _ripv1route;

struct ripv2hdr
{
    uint8_t command;
    uint8_t version;
    uint16_t reserved;
} _ripv2hdr;

struct ripv2route
{
    uint16_t addr_family;
    uint16_t route_tag;
    uint32_t ip;
    uint32_t subnet_mask;
    uint32_t next_hop;
    uint32_t metric;
} _ripv2route;

struct ripauth
{
    uint16_t reserved;
    uint16_t auth_type;
} _ripauth;

struct ripnghdr
{
    uint8_t command;
    uint8_t version;
    uint16_t reserved;
} _ripnghdr;

struct ripngroute
{
    struct in6_addr ip6_addr;
    uint16_t route_tag;
    uint8_t prefix_len;
    uint8_t metric;
} _ripngroute;

#endif