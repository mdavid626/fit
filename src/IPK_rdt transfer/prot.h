/*
 * File:     prot.h
 * Date:     2011-04-29
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Protocol header file
 */

#ifndef PROT_H_
#define PROT_H_

#define MAX_ROW_LENGTH 80
#define MAX_MSG_LENGTH 100
#define WINDOW_SIZE 10

/** */
typedef struct udp_seg_s
{
    // Sequence number
    uint32_t seqn;

    // Message
    char msg[MAX_MSG_LENGTH];
    int msg_len;

    // Checksum
    uint32_t checksum;

    // Ack
    char ack[8];
    int ack_len;

    // Flags
    int recv_flag;
    int special_packet;
} tSeg;

/**
 * Init data structure
 * @param *data
 * @param count
 */
static inline void init_data(tSeg *data, int count)
{
    for (int i = 0; i < count; i++)
    {
        tSeg *item = &(data[i]);
        
        item->seqn = 0;

        item->msg[0] = '\0';
        item->msg_len = 0;

        item->checksum = 0;
        
        item->ack[0] = '\0';
        item->ack_len = 0;

        item->recv_flag = 0;
        item->special_packet = 0;
    }
}

/**
 * Compute Fletcher checksum
 * @param *data
 * @param len
 */
static inline uint32_t fletcher32(uint16_t *data, size_t len) 
{
    uint32_t sum1 = 0xffff;
    uint32_t sum2 = 0xffff; 

    while (len) 
    {
        unsigned tlen = len > 360 ? 360 : len;
        len -= tlen;

        do 
        {
            sum1 += *data++; 
            sum2 += sum1;
        }
        while (--tlen);

        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    } 

    // Second reduction step to reduce sums to 16 bits
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);

    return sum2 << 16 | sum1; 
}

#endif
/* end of prot.h */
