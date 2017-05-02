/*
 * File:     rdtclient.c
 * Date:     2011-04-29
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  RDT client
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
#include <signal.h>
#include <sys/time.h>

#include "common.h"
#include "prot.h"


// Macros
// -----------------------------------------------------------------------------

#define BUFFER_SIZE 500
#define RESEND_TIMEOUT_MS 300
#define RESEND_MULT 1.5


/**
 * Close socket
 */
#define clean_soc           udt_close(udt);

/**
 * Sets timer to specified time
 * @param s
 * @param ms
 */
#define set_timer(s, ms)    itv.it_interval.tv_sec = 0;\
	                        itv.it_interval.tv_usec = 0;\
	                        itv.it_value.tv_sec = (s);\
	                        itv.it_value.tv_usec = (ms) * 1000;\
	                        setitimer(ITIMER_REAL, &itv, NULL);

/**
 * Stops timer by setting it_value to zero
 */
#define stop_timer          set_timer(0, 0)

/**
 * Starts timer
 * @param ms
 */
#define start_timer(ms)     set_timer((ms) / 1000, (ms) % 1000)


// Global variables
// -----------------------------------------------------------------------------
int send;


// Functions
// -----------------------------------------------------------------------------

/**
 * Read data from file f, max count, store in *data
 * @param **data
 * @param *count
 * @param seqn
 * @param *last
 * @param *f
 */
int read_data(tSeg *data, unsigned int *count, unsigned int seq_n, 
              int *last, FILE *f)
{
    int ch = 0;
    int i = 0;
    int newline = 1;
    tSeg *item = &(data[0]);

    *count = 0;
   
    while (1)
    {
        // get new character
        ch = fgetc(f);
        
        if (ch == '\n' || ch == EOF || i >= MAX_ROW_LENGTH)
        {
            newline = 0;

            // store new line
            if (ch == '\n' && i < MAX_ROW_LENGTH)
            {
                item->msg[i] = '\n';
                i += 1;
                newline = 1;
            }

            // we have a new message
            item->seqn = seq_n + *count;
            item->msg[i] = '\0';
            item->msg_len = strlen(item->msg);

            if (i != 0  || (i == 0 && ch != EOF))
            {
                *count += 1;
            }

            i = 0;
        
            // last char arrived or buffer full, break while
            if (ch == EOF || *count >= WINDOW_SIZE)
            {
                if (ch == EOF)
                {
                    // send special flag segment
                    data[*count].special_packet = 1;
                    *count += 1;
                    *last = 1;
                }

                break;
            }

            item = &(data[*count]);
            
            if (newline == 1)
            { // new line was stored, we can skip storing new line...
                continue;
            }
        }

        // store character
        item->msg[i] = ch;
        i += 1;
    }

    return EOK;
}

/**
 * Send data to server
 * @param *data
 * @param count
 * @param udt
 * @param *params
 */
int send_data(tSeg *data, int count, int udt, tParams *params)
{
    char buffer[BUFFER_SIZE];

    tSeg *item;
    int result = EOK;

    for (int i = 0; i < count; i++)
    {
        item = &(data[i]);

        if (item->recv_flag == 1)
        {
            continue;
        }

        // Construct flag packet

        if (item->special_packet == 1)
        {
            // Copy count
            buffer[0] = (char)count - 1;
            buffer[1] = '\0';

            // Compute checksum
            uint32_t checksum = fletcher32((uint16_t*)buffer, 1);

            memcpy(buffer + 1, &checksum, 4); 

            // Send
            result = udt_send(udt, params->remote_addr, params->dest_port,
                              buffer, 5);

            if (result != EOK) 
            {
                return result;
            }

            continue;
        }

        // Construct data packet

        // Copy sequence number
        uint32_t seqn = item->seqn;
        memcpy(buffer, &seqn, 4); 

        // Copy message
        strcpy(buffer + 4, item->msg);
        
        // Compute checksum
        size_t len = (size_t)(((item->msg_len + 4) / 2.0) + 0.5);
        uint32_t checksum = fletcher32((uint16_t*)buffer, len);

        memcpy(buffer + item->msg_len + 4, &checksum, 4); 
                                                                            
        // Send
        result = udt_send(udt, params->remote_addr, params->dest_port,
                          buffer, item->msg_len + 8);

        if (result != EOK) 
        {
            return result;
        }
    }

    return EOK;
}

/**
 * Check ack packet. If not valid returns < 1
 * @param seq_n
 * @param *buffer
 * @param recv_res
 */
int check_ack(int seq_n, char *buffer, int recv_res)
{
    if (recv_res == 5)
    {
        // Last packet flag ack
        
        // export checksum
        uint32_t a_checksum;
        memcpy(&a_checksum, buffer + 1, 4);
        buffer[1] = '\0';
        
        // Check checksum
        if (a_checksum != fletcher32((uint16_t*)buffer, 1))
        {
            return -1;
        }

        return (int)(buffer[0]);
    }

    if (recv_res != 8)
    {
        return -1;
    }

    // Data packet ack

    // Extract checksum
    uint32_t checksum;
    memcpy(&checksum, buffer + recv_res - 4, 4);

    // Check checksum    
    if (checksum != fletcher32((uint16_t*)buffer, 2))
    {
        return -1;
    }

    // Extract sequence number
    uint32_t sn;
    memcpy(&sn, buffer, 4);

    return (sn - seq_n);
}

/**
 * Timer tick handler
 * @param sig
 */
void sigalrm_handler(int sig)
{
    if (sig != SIGALRM)
    {
        return;
    }

    // mark flag for resending
    send = 1;

    // reinstall
    signal(SIGALRM, sigalrm_handler);
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

    // Init timer
    signal(SIGALRM, sigalrm_handler);

	struct itimerval itv;
    stop_timer

    int resend_timeout_ms = RESEND_TIMEOUT_MS;


    // Init UDP socket
    // ------------------------------------------------------------------------- 
   
    int udt;
	int result = udt_init(params.source_port, &udt);

    if (result != EOK)
    {
        print_ecode(result);
        return EXIT_FAILURE;
    }

   
    // Read and send data 
    // -------------------------------------------------------------------------
    
    char buffer[BUFFER_SIZE];
    in_addr_t addr;
    in_port_t port;
    int nrecv_res;

    unsigned int count = 0;
    unsigned int seq_n = 1;
    int index;
    
    // Flags
    int read_new = 1;
    send = 0;
    int last = 0;
    int full;

    while (1)
    {
        if (read_new == 1)
        {
            init_data(data, WINDOW_SIZE);
    
            // Fetch new data
            result = read_data(data, &count, seq_n, &last, stdin);

            if (result != EOK)
            {
                print_ecode(result);
                clean_soc
                return EXIT_FAILURE;
            }

            read_new = 0;
            send = 1;
        }

        // Send data
        if (send == 1)
        {
            // Stop timer
            stop_timer

            result = send_data(data, count, udt, &params);
            
            if (result != EOK)
            {
                print_ecode(result);
                clean_soc
                return EXIT_FAILURE;
            }

            send = 0;

            resend_timeout_ms *= RESEND_MULT;

            // Start timer
            start_timer(resend_timeout_ms);
        }

        // Read data from server
        result = udt_recv(udt, buffer, BUFFER_SIZE, &addr, &port, &nrecv_res);

        // Unable to read from socket
        if (result != EOK)
        {
            print_ecode(result);
            clean_soc
            return EXIT_FAILURE;
        }

        // Listen only on localhost, on source port <- don't, because proxy
        // Skip if 0 bytes received
        //if (addr != params.remote_addr || port != params.dest_port || 
        //    nrecv_res == 0)
        if (nrecv_res == 0)
        {
            continue;
        }

        // Check received message
        index = check_ack(seq_n, buffer, nrecv_res);

        if (index >= 0)
        {
            data[index].recv_flag = 1;
        }
        else
        {
            continue;
        }

        // Check if all message is sent and acked
        full = 1;

        for (unsigned int i = 0; i < count; i++)
        {
            if (data[i].recv_flag == 0)
            {
                full = 0;
            }
        }

        if (full == 1)
        {
            // All sent and received ack
            // Stop timer
            stop_timer

            // Read new data

            read_new = 1;
            send = 1;
            seq_n += WINDOW_SIZE;
            resend_timeout_ms = RESEND_TIMEOUT_MS;
        }
        else
        {
            continue;
        }

        // If no data readed end
        if (last == 1)
        {
            break;
        }
   }


    // Free resourses
    // -------------------------------------------------------------------------

    clean_soc   

    return EXIT_SUCCESS;
}

/* end of rdtserver.c */
