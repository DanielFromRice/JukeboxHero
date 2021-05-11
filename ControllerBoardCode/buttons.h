/*
 * buttons.h
 *
 *  Created on: May 10, 2021
 *      Author: Daniel Rothfusz
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdbool.h>
#include <msp430.h>
#include <stdint.h>

void button_init(void);
void button_update(void);
void button_ready(void);


#endif /* BUTTONS_H_ */
