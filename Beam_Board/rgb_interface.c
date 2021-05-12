/*
 * rgb_interface.c
 *
 *  Created on: Mar 12, 2021
 *      Author: ckemere
 */
#include "rgb_interface.h"
const uint8_t blue[] = {0xF0, 10, 0, 0};
const uint8_t green[] = {0xF0, 0, 10, 0};
const uint8_t red[] = {0xF0, 0, 0, 10};
const uint8_t yellow[] = {0xF0, 0, 10, 10};
const uint8_t magenta[] = {0xF0, 10, 0, 10};
const uint8_t off[] = {0xE0, 0, 0, 0};
const uint8_t start_frame[] = {0,0,0,0};
const uint8_t end_frame[] = {0,0,0,0};
void rgb_init_spi(void){
    //COPI on p1.2, SCLK on p1.4
    P1SEL |= BIT2 + BIT4;
    P1SEL2 |= BIT2 + BIT4;
    UCA0CTL1=UCSWRST; //disable serial interface
    UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;    // data cap at 1st clk edge, MSB first, master mode, synchronous
    UCA0CTL1 |= UCSSEL_2;                           // select SMCLK
    UCA0BR0 = 0;                                    //set frequency
    UCA0BR1 = 0;                                    //
    UCA0CTL1 &= ~UCSWRST;           // Initialize USCI state machine
}
void rgb_set_LEDs(const uint8_t *LED1, const uint8_t *LED2, const uint8_t *LED3, const uint8_t *LED4) {
    rgb_send_start();
    rgb_send_frame(LED1, false);
    rgb_send_frame(LED2, false);
    rgb_send_frame(LED3, false);
    rgb_send_frame(LED4, false);
    rgb_send_end();
}
//writes a 32 bit frame to the spi buffer
void rgb_send_frame(const uint8_t *frame1, bool wait_for_completion){
    int byte1;
    for (byte1=0;byte1<4;byte1++){//send 32 bit frame in 8 bit chunks
        UCA0TXBUF=frame1[byte1];
        while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
    }
    if (wait_for_completion)
        while (!(IFG2 & UCA0RXIFG));  // USCI_A0 RX buffer ready? (indicates transfer complete)
}
void rgb_send_start() {
    rgb_send_frame(start_frame, false);
}
void rgb_send_end() {
    rgb_send_frame(end_frame, true);
}
