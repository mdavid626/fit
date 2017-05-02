/*
 * File:     rdtserver.c
 * Date:     2011-04-29
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  RDT server
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
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "prot.h"


// Macros
// -----------------------------------------------------------------------------

#define BUFFER_SIZE 500
#define LAST_WAIT_MS 1000

/**
 * Close socket
 */
#define clean_soc       udt_close(udt);


// Functions
// -----------------------------------------------------------------------------

/**
 * Check received message validity
 * @param *item
 * @param nr
 * @param *count
 * @param *last
 * @param *next
 * @param *buffer
 * @param nrecv_res
 */
int check(tSeg *item, unsigned int nr, unsigned int *count, 
          int *last, int *next, int *special, char *buffer, int nrecv_res)
{
    // Check last segment flag
    if (nrecv_res == 5)
    {
        // Extract checksum: last 4 bytes
        uint32_t f_checksum = 0;
        memcpy(&f_checksum, buffer + nrecv_res - 4, 4);

        // Check checksum
        buffer[nrecv_res - 4] = '\0';

        if (f_checksum != fletcher32((uint16_t*)buffer, 1))
        {
            return -1;
        }

        *last = 1;
        *special = 1;
        *count = (unsigned int)(buffer[0]);

        // Create ACK

        // Copy first byte
        memcpy(item->ack, buffer, 1);

        // Copy checksum
        memcpy(item->ack + 1, &f_checksum, 4);

        // Set length
        item->ack_len = 5;

        // Return index
        return *count;
    }

    // At least 9 bytes in message
    if (nrecv_res < 8)
    {
        return -1;
    }

    // Extract checksum: last 4 bytes
    uint32_t checksum = 0;
    memcpy(&checksum, buffer + nrecv_res - 4, 4);

    // Check checksum
    buffer[nrecv_res - 4] = '\0';
    size_t len = (size_t)(((nrecv_res - 4) / 2.0) + 0.5);

    if (checksum != fletcher32((uint16_t*)buffer, len))
    {
        return -1;
    }

    // Store checksum
    item->checksum = checksum;

    // Extract seqn: first 4 bytes
    uint32_t seqn = 0;
    memcpy(&seqn, buffer, 4);

    // Check if seqn is OK
    if (seqn < nr || seqn > (nr + 2 * WINDOW_SIZE))
    {
        return -1;
    }

    if (seqn >= (nr + WINDOW_SIZE))
    {                                                                   
        *next = 1;
    }

    // Store seqn
    item->seqn = seqn;

    // Store message
    strcpy(item->msg, buffer + 4);

    // Create ACK

    // Copy seqn
    memcpy(item->ack, buffer, 4);

    // Create checksum
    uint32_t ack_checksum = fletcher32((uint16_t*)item->ack, (size_t)2);
    memcpy(item->ack + 4, &ack_checksum, 4);

    // Set length
    item->ack_len = 8;

    *special = 0;

    // return index
    return (seqn - nr);
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
    { // wrong parameters
        print_ecode(params.ecode);
        return EXIT_FAILURE;
    }

    
    // Init variables
    // -------------------------------------------------------------------------

    tSeg data[WINDOW_SIZE + 1];

    init_data(data, WINDOW_SIZE);


    // Init UDP socket
    // ------------------------------------------------------------------------- 
   
    int udt;
	int result = udt_init(params.source_port, &udt);

    if (result != EOK)
    {
        print_ecode(result);
        return EXIT_FAILURE;
    }

    
    // Read and print out data from client
    // -------------------------------------------------------------------------

    char buffer[BUFFER_SIZE];
    in_addr_t addr;
    in_port_t port;
    int nrecv_res;

    unsigned int count = WINDOW_SIZE;
    int index;
    tSeg item;
    unsigned int nr = 1;

    // Flags
    int last = 0;
    int full;
    int next = 0;
    int dup = 0;
    int special = 0;
    int last_2 = 0;
    int last_delay = 0;
    int last_delay_count = 0;

    while (1)
    {
        // Read data from client
        result = udt_recv(udt, buffer, BUFFER_SIZE, &addr, &port, &nrecv_res);

        // Unable to read from socket
        if (result != EOK)
        {
            print_ecode(result);
            clean_soc
            return EXIT_FAILURE;
        }

        // Listen only on localhost, on source port <- dont, because proxy
        // Skip if 0 bytes received
        //if (addr != params.remote_addr || port != params.dest_port || 
        //    nrecv_res == 0)
        if (last_2 == 0 && nrecv_res == 0 && count != 0)
        {
            //continue;
        }

        // Check message
        index = check(&item, nr, &count, &last, &next, 
                      &special, buffer, nrecv_res);

        if (index >= 0)
        {
            // Message accepted

            if (next == 1)
            {

                 // Init data
                count = WINDOW_SIZE;
                nr += WINDOW_SIZE;

                init_data(data, WINDOW_SIZE);

                next = 0;

                // Last window flag received yet, "reload"
                if (last_delay == 1)
                {
                    last = 1;
                    count = last_delay_count;
                    last_delay = 0;
                }

                if (special == 0)
                {
                    index -= WINDOW_SIZE;
                }

                dup = 0;
            }

            if (dup == 1 && special == 1 && last_delay == 0 && count != 0)
            {
                // Last window flag received, but not "started" new window
                // save, ant when new window starts reload...
                last_delay = 1;
                last_delay_count = count;
                count = WINDOW_SIZE;
                last = 0;
            }

            data[index] = item;

            // Set received flag
            data[index].recv_flag = 1;

            // Sending ACK
            result = udt_send(udt, params.remote_addr, params.dest_port, 
                              data[index].ack, data[index].ack_len);

            if (result != EOK)
            {
                print_ecode(result);
                clean_soc
                return EXIT_FAILURE;
            }
        }
        else
        {
            // Message not accepted
            if (last_2 == 0)
            {            
                continue;
            }
        }

        // Check if we have all segments in current window
        full = 1;

        for (unsigned int i = 0; i < count; i++)
        {
            if (data[i].recv_flag == 0)
            {
                full = 0;
                break;
            } 
        }

        if (full != 1)
        {
            if (last_2 == 0)
            {
                continue;
            }
        }

        // Yes, all packets received

        // Print out
        if (dup == 0)
        {
            for (unsigned int i = 0; i < count; i++)
            {            
                fprintf(stdout, "%s", data[i].msg);
            }
            dup = 1;
        }

        // Check if we had received last window flag
        if (last == 1)
        { 
            if (last_2 == 1)
            {
                // Done our job, exiting...
                break;
            }
            
            //last = 0;

            // One more round, it is possible not all ack-s were delivered
            usleep(LAST_WAIT_MS * 1000);
            last_2 = 1;
        }
    }


    // Free resourses
    // -------------------------------------------------------------------------

    clean_soc

    return EXIT_SUCCESS;
}

/* end of rdtserver.c */
