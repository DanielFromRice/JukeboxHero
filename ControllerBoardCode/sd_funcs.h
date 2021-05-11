/*
 * sd_funcs.h
 *
 *  Created on: May 9, 2021
 *      Author: Daniel Rothfusz
 */

#ifndef SD_FUNCS_H_
#define SD_FUNCS_H_

#include <stdint.h>
#include "SPI.h"
#include "pff.h"

struct packet{
    uint16_t duration;
    uint16_t note_s;
    uint16_t note_a;
    uint16_t note_t;
    uint16_t note_b;
    uint8_t beam_state;
};


void sd_init();
FRESULT sd_open(const char * filname);
FRESULT sd_read_byte(void * addr);
FRESULT sd_read_packet(struct packet * pckt);


#endif /* SD_FUNCS_H_ */
