/*
 * buttons.c
 *
 *  Created on: May 10, 2021
 *      Author: Daniel Rothfusz
 */

#include "buttons.h"

bool bsel_pressed, bup_pressed, bdown_pressed;
bool bsel_edge, bup_edge, bdown_edge;
uint8_t bsel_reg, bup_reg, bdown_reg;


/*
 * Prep button registers
 */
void button_init(void) {
    P2DIR &= ~(BIT0 + BIT1 + BIT2); // set to input
    P2REN |= BIT0 + BIT1 + BIT2; // enable pullup/down resistors
    P2OUT |= BIT0 + BIT1 + BIT2; // set resistors to pull up
}


/*
 * Initializes button variables and button registers
 */
void button_ready(void) {
    bsel_pressed = false;
    bup_pressed = false;
    bdown_pressed = false;
    bsel_reg = bup_reg = bdown_reg = 0xAA; // 0b10101010 so that neither value is set by default
}

/*
 *
 */
void button_update(void) {
    // Shift registers to prep for new entry
    bsel_reg = bsel_reg << 1;
    bup_reg = bup_reg << 1;
    bdown_reg = bdown_reg << 1;

    // Update the registers with the new value
    if ((P2IN & BIT0) != BIT0) {
        bsel_reg |= 0x01;
    }
    if ((P2IN & BIT1) != BIT1) {
        bup_reg |= 0x01;
    }
    if ((P2IN & BIT2) != BIT2) {
        bdown_reg |= 0x01;
    }

    // If fully on or off, set b#pressed (all 1's is on)
    if (bsel_reg == 0xFF) {
        if (!bsel_pressed)
            bsel_edge = true;
        bsel_pressed = true;
    } else if (bsel_reg == 0) {
        bsel_pressed = false;
    }
    if (bup_reg == 0xFF) {
        if (!bup_pressed)
            bup_edge = true;
        bup_reg = true;
    } else if (bup_reg == 0) {
        bup_reg = false;
    }
    if (bdown_reg == 0xFF) {
        if (!bdown_reg)
            bdown_edge = true;
        bdown_reg = true;
    } else if (bdown_reg == 0) {
        bdown_reg = false;
    }
}

