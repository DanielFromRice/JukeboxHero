/*
 * rgb_interface.h
 *
 *  Created on: Mar 12, 2021
 *      Author: ckemere
 */
#ifndef RGB_INTERFACE_H_
#define RGB_INTERFACE_H_
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
extern const uint8_t blue[4];
extern const uint8_t green[4];
extern const uint8_t red[4];
extern const uint8_t yellow[4];
extern const uint8_t magenta[4];
extern const uint8_t off[4];
void rgb_init_spi(void);
void rgb_set_LEDs(const uint8_t *LED1, const uint8_t *LED2, const uint8_t *LED3, const uint8_t *LED4);
void rgb_send_frame(const uint8_t *frame1, bool wait_for_completion);
void rgb_send_start();
void rgb_send_end();
#endif /* RGB_INTERFACE_H_ */
