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

int main(void) {
    WORD s1;
    volatile FRESULT res;
    FATFS fs; /* File system object */

    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    DCOCTL = 0; // Run at 1 MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
//    DCOCTL += 3;

    __delay_cycles(0xffff); // delay for power up and stabilization

    spi_initialize();

//    P1DIR |= BIT2|BIT0; // P1.2 output
//    P1SEL |= BIT2;      // P1.2 TA1 options
//    P1OUT &= ~BIT0;

    /* Initialize Button */
    P1DIR &= ~BIT3;     // P1.3 Input

    /* MY TEST CODE */
    res = FR_DISK_ERR;
    do {
        res = disk_initialize();
    } while(res != FR_OK);

    do {
        res = pf_mount(&fs);
    } while ( res != FR_OK );

    res = pf_open(fileName);
    if ( res == FR_OK ) {
        P1OUT |= BIT0;
    }
//    res = pf_read(output, 4, &s1);
//    while(1);

    s1 = 1;
    while(res == FR_OK && s1 != 0) {
        res = pf_read(output, 4, &s1);
        __delay_cycles(50000);
    }

    while(1);



    /* END MY TEST CODE, start original file code */
//    TACCR0 = SMCLK_FREQ/SAMPLE_RATE; // PWM Period 15.625k
//    TACCTL1 = OUTMOD_7;              // CCR1 reset/set, enable interrupt
//    TACCR1 = TACCR0/2;               // set initial CCR1 to 50% PWM duty cycle
//    TACTL = TASSEL_2 | MC_1;         // SMCLK, up mode
//
////    __enable_interrupt();
//
//    // play the same song over and over.
//    while(1) {
//        res = disk_initialize();
//        if ( res == FR_OK ) {
//            res = pf_mount(&fs);
//            if ( res == FR_OK ) {
//                static char *fileName = "sound0.raw";
//                res = pf_open(fileName);
//                if ( res == FR_OK ) {
//                    do {
//                        res = pf_read(0, 32768, &s1);
//                        if (res != FR_OK) {
//                            break;
//                        }
//                    } while (s1 == 32768);
//                }
//            }
//        }
//    }

    return 0;
}
