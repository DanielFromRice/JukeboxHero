/**
 * File: usci_spi.c - msp430 USCI SPI implementation
 *
 */
#include <msp430.h>

#include <stdint.h>
#include "SPI.h"

#ifndef __MSP430_HAS_USCI__
#error "Error! This MCU doesn't have a USCI peripheral"
#endif

/**
 * USCI flags for various the SPI MODEs
 *
 * Note: The msp430 UCCKPL tracks the CPOL value. However,
 * the UCCKPH flag is inverted when compared to the CPHA
 * value described in Motorola documentation.
 */

#define SPI_MODE0 (UCMSB | UCMST | UCSYNC | UCCKPH)            /* CPOL=0 CPHA=0 */
#define SPI_MODE1 (UCMSB | UCMST | UCSYNC)                     /* CPOL=0 CPHA=1 */
#define SPI_MODE2 (UCMSB | UCMST | UCSYNC | UCCKPL | UCCKPH)   /* CPOL=1 CPHA=0 */
#define SPI_MODE3 (UCMSB | UCMST | UCSYNC | UCCKPL)            /* CPOL=1 CPHA=1 */

/**
 * utility macros for extracting hi/lo byte data from a word value
 */
#ifndef LOBYTE
#define LOBYTE(w) ((w)&0xFF)
#define HIBYTE(w) ((w)>>8)
#endif

/**
 * spi_initialize() - Configure USCI UCB0 for SPI mode
 *
 * P2.0 - CS (active low)
 * P1.5 - SCLK
 * P1.6 - SIMO/MOSI
 * P1.7 - SOMI/MISO
 *
 *
 * USCI UCA0 PINS:
 *
 * P1.1 - SOMI
 * P1.2 - SIMO
 * P1.4 - SCLK
 * P1.5 - CS (active low)
 */
void spi_initialize(void) {
    UCA0CTL1 = UCSWRST | UCSSEL_2;  // Put USCI in reset mode, source USCI clock from SMCLK
    UCA0CTL0 = SPI_MODE0;          // Use SPI MODE 0 - CPOL=0 CPHA=0

    UCA0MCTL = 0; // CLEARED FOR USCA

//    P1SEL  |= BIT5 | BIT6 | BIT7;   // configure P1.5, P1.6, P1.7 for USCI
//    P1SEL2 |= BIT5 | BIT6 | BIT7;   // SCLK, MOSI, MISO
    P1SEL |= BIT1 | BIT2 | BIT4;
    P1SEL2 |= BIT1 | BIT2 | BIT4;

    UCA0BR0 = LOBYTE(SPI_250kHz);   // set initial speed to 250kHz (16MHz/400000)
    UCA0BR1 = HIBYTE(SPI_250kHz);

//    P2OUT |= BIT0;                  // CS on P2.0. start out disabled
//    P2DIR |= BIT0;                  // CS configured as output
    P1OUT |= BIT5;
    P1DIR |= BIT5;

    UCA0CTL1 &= ~UCSWRST;           // release USCI for operation
}

/**
 * spi_send() - send a byte and recv response
 */
uint8_t spi_send(const uint8_t c) {
    while (!(IFG2 & UCA0TXIFG))
        ; // wait for previous tx to complete

    UCA0TXBUF = c; // setting TXBUF clears the TXIFG flag

    while (!(IFG2 & UCA0RXIFG))
        ; // wait for an rx character?

    return UCA0RXBUF; // reading clears RXIFG flag
}

/**
 * spi_receive() - send dummy btye then recv response
 */

uint8_t spi_receive(void) {

    while (!(IFG2 & UCA0TXIFG))
        ; // wait for any previous xmits to complete

    UCA0TXBUF = 0xFF; // Send dummy packet to get data back.

    while (!(IFG2 & UCA0RXIFG))
        ; // wait to recv a character?

    return UCA0RXBUF; // reading clears RXIFG flag
}

/**
 * spi_set_divisor() - set new clock divider for USCI
 *
 * USCI speed is based on the SMCLK divided by BR0 and BR1
 * initially we start slow (400kHz) to conform to SDCard
 * specifications then we speed up once initialized (SPI_DEFAULT_SPEED)
 *
 * returns the previous setting
 */

uint16_t spi_set_divisor(const uint16_t clkdiv) {
    uint16_t prev_clkdiv = UCA0BR1 << 8 | UCA0BR0;

    UCA0CTL1 |= UCSWRST; // go into reset state
    UCA0BR0 = LOBYTE(clkdiv);
    UCA0BR1 = HIBYTE(clkdiv);
    UCA0CTL1 &= ~UCSWRST; // release for operation

    return prev_clkdiv;
}
