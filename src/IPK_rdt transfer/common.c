/*
 * File:     common.c
 * Date:     2011-04-29
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  RDT transmit, common file
 */


// Header files
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>

#include "common.h"


// Enums, structures, messages, etc.
// -----------------------------------------------------------------------------

/** Error messages */
const char *ECODEMSG[] =
{
    /* EOK */
    "Everything OK.\n",
    /* EPARAM */
    "Error: Bad command line parameters!\n",
    /* EOUT_MEM */
    "Error: Out of memory. Sorry!\n",

    /* ESOC_CREATE */
    "Socekt Error: Unable to create socket!\n",
    /* ESOC_CONNECT */    
    "Socekt Error: Unable to connect!\n",
    /* ESOC_BIND */  
    "Socekt Error: Unable to bind!\n",
    /* ESOC_WRITE */     
    "Socekt Error: Unable to write!\n",
    /* ESOC_READ */    
    "Socekt Error: Unable to read!\n",
    /* ESOC_CLOSE */      
    "Socekt Error: Unable to close!\n",
    /* ESOC_INV */
    "Socket Error: Socket descriptor is invalid!\n",

    /* EUNKNOWN */
    "Error: Unknown!\n"
};


// Function declarations
// -----------------------------------------------------------------------------

/**
 * Prints error message to stderr
 * @param ecode error code
 */
void print_ecode(int ecode, ...)
{
    if (ecode < EOK || ecode > EUNKNOWN)
    { 
        ecode = EUNKNOWN; 
    }

    va_list arg;

    va_start(arg, ecode);

    vfprintf(stderr, ECODEMSG[ecode], arg);

    va_end(arg);
}

/**
 * Frees a pointer if it's not NULL
 * @param *p pointer to free
 */
void free2(void *p)
{
    if (p != NULL)
    {
        free(p);
    }
}

/**
 * Processes arguments of command line
 * @param argc Argument count
 * @param argv Array of string with arguments
 */
tParams get_params(int argc, char *argv[])
{
    tParams result = 
    {  // init structure
        .source_port = 0,
        .dest_port = 0,
        .remote_addr = 0x7f000001,
        .ecode = EOK
    };

    char ch;
    opterr = 0;

    while ((ch = getopt(argc,argv,"s:d:")) != -1) 
    {
		switch(ch) 
        {
		    case 's': result.source_port = atol(optarg); break;
		    case 'd': result.dest_port = atol(optarg); break;
		}
	}

    if (argc != 5 || result.source_port == 0 || result.dest_port == 0)
    {
        result.ecode = EPARAM;
    }

    return result;
}

/*
 * Initializes UDT descriptor
 * @param local_port Specifies a local port to which UDT binds.
 * @param *udt
 */
int udt_init(in_port_t local_port, int *udt_res)
{
	int udt = socket(AF_INET, SOCK_DGRAM, 0);
	fcntl(udt, F_SETFL, O_NONBLOCK);

	if (udt <= 0) 
    {
        return ESOC_CREATE;
	}

	struct sockaddr_in sa;
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(0);
	sa.sin_port = htons(local_port);

	int result = bind(udt, (const struct sockaddr*) &sa, sizeof(sa));

	if (result < 0) 
    {
        return ESOC_BIND;
	}

    *udt_res = udt;

	return EOK;
}

/*
 * Reads a received datagram in UDT buffer pool, if such exists.
 * @param udt Determines UDT descriptor as initialized by udt_init() function.
 * @param buff A pointer to buffer used for store the available data.
 * @param nbytes Length of the buffer.
 * @param addr A pointer to variable to be filled with IP address of the sender.
 *             This can be NULL if such information is not required.
 * @param port A pointer to variable to be filled with Port used by the sender.
 *             This can be NULL if such information is not required.
 */
int udt_recv(int udt, void *buff, size_t nbytes, 
                      in_addr_t *addr, in_port_t *port, int *nrecv_res)
{
	struct stat info;

	if (fstat(udt, &info) != 0) 
    {
        return ESOC_INV;
	}

	struct sockaddr_in sa;
	bzero(&sa, sizeof(sa));
	socklen_t salen = sizeof(sa);

	ssize_t nrecv = recvfrom(udt, buff, nbytes, MSG_DONTWAIT, 
                                 (struct sockaddr *) &sa, &salen);

	if (addr != NULL) 
    {
        (*addr) = ntohl(sa.sin_addr.s_addr);
    }

	if (port != NULL) 
    {
        (*port) = ntohs(sa.sin_port);
    }

	if (nrecv < 0 )
    {
        nrecv = 0;
    }

    *nrecv_res = nrecv;

	return EOK;
}

/*
 * Sends a new UDT datagram with data provided 
 * to the specified address and port.
 * @param udt Determines UDT descriptor as initialized by udt_init() function.
 * @param addr IP address of the remote node.
 * @param port Port on the remote node used for distinguishing
               different connections.
 * @param buff A buffer containing RDT packet.
 * @param nbytes The lenght of the buffer with data.
 */
int udt_send(int udt, in_addr_t addr, in_port_t port, void *buff, size_t nbytes)
{
	struct stat info;

	if (fstat(udt, &info) != 0) 
    {
        return ESOC_INV;
	}

	struct sockaddr_in sa;
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(addr);
	sa.sin_port = htons(port);

	ssize_t nsend = sendto(udt, buff, nbytes, 0, 
                                (const struct sockaddr *) &sa, sizeof(sa));

    if (nsend == (ssize_t)nbytes)
    { 
        return EOK;
    }

	return ESOC_WRITE;
}

/**
 * Correctly ends work with udt
 * @param udt descriptor to close
 */
int udt_close(int udt)
{
    if (close(udt) < 0)
    {
        return ESOC_CLOSE;
    }
    
    return EOK;
}

/* end of common.c */
