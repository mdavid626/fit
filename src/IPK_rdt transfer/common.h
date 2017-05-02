/*
 * File:     common.h
 * Date:     2011-04-29
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  RDT transmit, Common header file
 */

#ifndef COMMON_H_
#define COMMON_H_

// Enums, structures, messages, etc.
// -----------------------------------------------------------------------------

/** Program error codes */
enum tecodes
{
    EOK = 0,         /**< Without error */
    EPARAM,          /**< Bad command line parameters */
    EOUT_MEM,        /**< Out of memory */

    ESOC_CREATE,     /**< Socket Error: create */
    ESOC_CONNECT,    /**< Socket Error: connect */
    ESOC_BIND,       /**< Socket Error: bind */
    ESOC_WRITE,      /**< Socket Error: write */
    ESOC_READ,       /**< Socket Error: read */
    ESOC_CLOSE,      /**< Socket Error: close */
    ESOC_INV,        /**< Socket Error: invalid */

    EUNKNOWN         /**< Unknown error */
};

/**
 * Structure containing parameters from command line
 */
typedef struct params
{
    in_port_t source_port;     /**< Source port */
    in_port_t dest_port;       /**< Destination port */
    in_addr_t remote_addr;     /**< Destinaton address */
    int ecode;                 /**< Error code (enum tecodes) */
} tParams;


// Functions declarations
// -----------------------------------------------------------------------------

/**
 * Prints error message to stderr
 */
void print_ecode(int ecode, ...);

/**
 * Frees a pointer if it's not NULL
 */
void free2(void *p);

/**
 * Processes arguments of command line
 */
tParams get_params(int argc, char *argv[]);

/*
 * Initializes UDT descriptor
 */
int udt_init(in_port_t local_port, int *udt_res);

/*
 * Reads a received datagram in UDT buffer pool, if such exists.
 */
int udt_recv(int udt, void *buff, size_t nbytes, 
                      in_addr_t *addr, in_port_t *port, int *nrecv_res);

/*
 * Sends a new UDT datagram with data provided
 * to the specified address and port.
 */
int udt_send(int udt, in_addr_t addr, in_port_t port, 
                                      void *buff, size_t nbytes);

/**
 * Correctly ends work with udt
 */
int udt_close(int udt);

#endif
/* end of common.h */
