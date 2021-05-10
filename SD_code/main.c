/*
 * pcmplayer.c - playback a 15.625kHz 8bit raw sound file.
 *
 * The snippet plays a file called 'sound0.raw' from an SDCard. You
 * should use the Audacity program to resample the sound file you
 * want to play at 15625Hz. Export the file as 8bit unsigned data without
 * a header.
 *
 * P1.2 - PWM output, output to a low pass RC filter. I'm using 400ohm, 100nF
 *
 * Inspired from here:
 *
 *  http://www.arduino.cc/playground/Code/PCMAudio
 *
 *  3596 bytes of code on a msp430g2553 with msp430-gcc 4.5.3
 *
 *
 *  Modules taken from: https://gist.github.com/RickKimball/2407353
 */

#include <msp430.h>
#include <stdint.h>

#include "SPI.h"
#include "diskio.h"
#include "pff.h"
#include "mmc.h"
#include "sd_funcs.h"


static char *fileName = "num.bin";

/*
 * on_pfread_complete() - called when pf_read has a uint8_t for us.
 *
 * pf_read is called in a loop getting bytes. When we get one, this routine
 * waits for CCR0 to roll over to 0. Once, it has overflowed, we set
 * the next duty cycle using scaled PCM (pulse code modulation)
 *
 */
void on_pfread_complete(register uint16_t sample) {
    /* set sample*4, scale up the value.
     * Our sample is scale 0-255, our PWM cycle is from 0-1023
     * (16000000/15625)
     */
    sample <<= 2;
    while( !(TA0CTL & TAIFG) ); // wait for CCR0 to roll over
    TACCR1 = sample;
    TA0CTL &= ~TAIFG;
}

uint8_t output[16] = {1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2};
uint8_t test_byte = 0xFD;
struct packet pkt;

int main(void) {
    FATFS fs; /* File system object */

    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    DCOCTL = 0; // Run at 1 MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    __delay_cycles(0xffff); // delay for power up and stabilization

    sd_init(&fs);
    spi_initialize();


    /* Initialize Button */
//    P1DIR &= ~BIT3;     // P1.3 Input


    if (sd_open(fileName) != FR_OK) {
        while(1);
    }

    if (sd_read_packet(&pkt) != FR_OK) {
        while(1);
    }

    while(1);


    return 0;
}
