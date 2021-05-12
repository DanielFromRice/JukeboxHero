/*
 * display_change.h
 *
 *  Created on: May 11, 2021
 *      Author: Kevin Lata
 */

#ifndef DISPLAY_CHANGE_H_
#define DISPLAY_CHANGE_H_
#include <msp430.h>
#include <stdint.h>

#include "rgb_interface.h"

void change_display(uint8_t inval);
void write_correct(void);
void write_incorrect(void);
void write_score(int score, int song_length);
static void send_leds(void);

#endif /* DISPLAY_CHANGE_H_ */
