/*
 * File:     charmap.c
 * Date:     2011-12-01
 * Encoding: ISO-8859-2
 * Author:   David Molnar, xmolna02@stud.fit.vutbr.cz
 * Project:  Model riadenia prevadzky na svetelnej krizovatke
 */

 
#ifndef _CHARMAP_H_
#define _CHARMAP_H_

unsigned char ch_car_stop[8] = { 0x1F, 0x1B, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
/*  76543210            
 *  ...XXXXX
 *  ...XX.XX
 *  ...XXXXX
 *  ........
 *  ........
 *  ........
 *  ........
 *  ........
 */
 
unsigned char ch_car_att[8] = { 0x1F, 0x1B, 0x1F, 0x1B, 0x1F, 0x00, 0x00, 0x00 }; 
 /* 76543210            
 *  ...XXXXX
 *  ...XX.XX
 *  ...XXXXX
 *  ...XX.XX
 *  ...XXXXX
 *  ........
 *  ........
 *  ........
 */
 
unsigned char ch_car_go[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1B, 0x1F }; 
/*  76543210            
 *  ........
 *  ........
 *  ........
 *  ........
 *  ........
 *  ...XXXXX
 *  ...XX.XX
 *  ...XXXXX
 */
 
unsigned char ch_walker_stop[8] = { 0x1F, 0x1B, 0x1B, 0x1F, 0x00, 0x00, 0x00, 0x00 }; 
 /* 76543210            
 *  ...XXXXX
 *  ...XX.XX
 *  ...XX.XX
 *  ...XXXXX
 *  ........
 *  ........
 *  ........
 *  ........
 */
 
unsigned char ch_walker_go[8] = { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1B, 0x1B, 0x1F }; 
 /* 76543210            
 *  ........
 *  ........
 *  ........
 *  ........
 *  ...XXXXX
 *  ...XX.XX
 *  ...XX.XX
 *  ...XXXXX
 */
 
unsigned char ch_oran[8] = { 0x00, 0x00, 0x1F, 0x1B, 0x1B, 0x1F, 0x00, 0x00 }; 
 /* 76543210            
 *  ........
 *  ........
 *  ...XXXXX
 *  ...XX.XX
 *  ...XX.XX
 *  ...XXXXX
 *  ........
 *  ........
 */

#endif /* _CHARMAP_H_ */
