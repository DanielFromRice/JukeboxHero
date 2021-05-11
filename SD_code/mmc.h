/*
 * mmc.h
 *
 *  Created on: Apr 17, 2012
 *      Author: kimballr
 */

#ifndef MMC_H_
#define MMC_H_

/**
 * streaming byte read complete
 */
#ifdef __GNUC__
void on_pfread_complete(uint16_t value) __attribute__((always_inline));
#else
void on_pfread_complete(uint16_t value);
#endif

//#define FORWARD(d)  on_pfread_complete((uint16_t)(d))  /* Data forwarding function (Console out in this example) */

#endif /* MMC_H_ */
