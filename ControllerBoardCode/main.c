/*
 *
 *
 *  Modules taken from: https://gist.github.com/RickKimball/2407353
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "SPI.h"
#include "diskio.h"
#include "pff.h"
#include "mmc.h"
#include "sd_funcs.h"
#include "buttons.h"

void i2c_init(void);
void i2c_send_bytes(int address, void * data, int len_of_array);

static char *fileName = "num.bin";

const char* const paths[] = {"num.bin", "lida_rose.bin", "num.bin"};
#define MAXSONG  3

extern bool bsel_edge, bup_edge, bdown_edge;

// Gamestate management
enum gamestate{song_select, loop, endgame} state;
struct packet pkt;
uint8_t data_byte, num_pkts, total_pkts;
uint16_t ticks, duration;

// Display and buttons
uint16_t display_val;



// I2C stuff
unsigned char *PTxData;                 // Pointer to TX data
unsigned char TXByteCtr;

#define BB_ADDR     0x50

#define SOP_ADDR    0x48
#define ALT_ADDR    0x49
#define TEN_ADDR    0x4A
#define BAS_ADDR    0x4B

#define NO_CHANGE   256


void init_wdt(void) {
    BCSCTL3 |= LFXT1S_2;      // ACLK = VLO
    WDTCTL = WDT_ADLY_1_9;    // WDT 1.9ms (~43.3ms since clk 12khz), ACLK, interval timer
//    WDTCTL = WDT_ADLY_16;
    IE1 |= WDTIE;             // Enable WDT interrupt
}


int main(void) {

    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    DCOCTL = 0; // Run at 1 MHz
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    __delay_cycles(0xffff); // delay for power up and stabilization

    button_init();
    button_ready();
    i2c_init();
    sd_init();

    init_wdt();

    display_val = 0;

    while (1) {
        switch(state) {
        case song_select:
            // Start read on button press
//            button_update();
//            if (bup_edge) {
//                if (display_val == MAXSONG)
//                    display_val = 1;
//                else
//                    display_val += 1;
//                bup_edge = false;
//            }
//            if (bdown_edge) {
//                if (display_val == 0)
//                    display_val = MAXSONG;
//                else
//                    display_val -= 1;
//                bdown_edge = false;
//            }
//            if (bsel_edge) {
                // Load song and enter next state
                if (sd_open("LidaRose.bin") != FR_OK) {
                    while(1); // Change this later (error handling)
                }

                IE1 &= ~WDTIE;             // Disable WDT interrupt

                // Read beam array:
                uint8_t i;
                sd_read_byte(&total_pkts);
                i2c_send_bytes(BB_ADDR, &total_pkts, 1);
                for (i = 0; i < total_pkts; i++) { // TODO: Neaten this up to write more bytes at a time
                    sd_read_byte(&data_byte);
                    i2c_send_bytes(BB_ADDR, &data_byte, 1);
                }

                sd_read_byte(&total_pkts); // Read number of packets
                // Load first packet
                if (sd_read_packet(&pkt) != FR_OK) {
                    while(1);
                }
                num_pkts = 1; // Lists number of packets read in

                state = loop; // Init loop state
                ticks = 1;
                duration = 0;

                IE1 |= WDTIE;             // Enable WDT interrupt
//            }
            break;
        case loop:
            ticks += 1;
            if (ticks >= duration) {
                IE1 &= ~WDTIE;             // Disable WDT interrupt

                // Send current packet data
                if (pkt.note_s != NO_CHANGE)
                    i2c_send_bytes(SOP_ADDR, &(pkt.note_s), 2);
                if (pkt.note_a != NO_CHANGE)
                    i2c_send_bytes(ALT_ADDR, &(pkt.note_a), 2);
//                if (pkt.note_t != NO_CHANGE)
//                    i2c_send_bytes(TEN_ADDR, &(pkt.note_t), 2);
//                if (pkt.note_b != NO_CHANGE)
//                    i2c_send_bytes(BAS_ADDR, &(pkt.note_b), 2);
                if (pkt.beam_state != 0)
                    i2c_send_bytes(BB_ADDR, &(pkt.beam_state), 1);

                ticks = 0;
                duration = pkt.duration;

                IE1 |= WDTIE;             // Enable WDT interrupt

                if (num_pkts >= total_pkts) {
                    state = endgame;
                    ticks = 0;
                    break;
                }
                if (sd_read_packet(&pkt) != FR_OK) {
                    while(1);
                }
                num_pkts += 1;
            }
            break;
        case endgame:
            // TODO: Poll for score from bb board

            IE1 &= ~WDTIE;             // Disable WDT interrupt

            // Write "off" to each board
            pkt.note_s = 0;
            pkt.note_a = 0;
            pkt.note_t = 0;
            pkt.note_b = 0;
            i2c_send_bytes(SOP_ADDR, &(pkt.note_s), 2);
            i2c_send_bytes(ALT_ADDR, &(pkt.note_a), 2);
            i2c_send_bytes(ALT_ADDR, &(pkt.note_t), 2);
            i2c_send_bytes(ALT_ADDR, &(pkt.note_b), 2);

        }
        __bis_SR_register(LPM0_bits + GIE);
    }

    return 0;
}


void i2c_init(void) {
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = 0x48;                         // Slave Address is 048h
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    IE2 |= UCB0TXIE;                          // Enable TX interrupt
}


void i2c_send_bytes(int address, void * data, int len_of_array) {
   UCB0I2CSA = address;
   PTxData = (unsigned char *)data;      // TX array start address
   TXByteCtr = len_of_array;             // Load TX byte counter
   while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
   UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
   __bis_SR_register(LPM0_bits + GIE);        // Enter LPM0 w/ interrupts
   // Remain in LPM0 until all data is TX'd
}

//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCIAB0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  if (TXByteCtr)                            // Check TX byte counter
  {
    UCB0TXBUF = *PTxData++;                 // Load TX buffer
    TXByteCtr--;                            // Decrement TX byte counter
  }
  else
  {
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
    __bic_SR_register_on_exit(LPM0_bits);      // Exit LPM0
  }
}


// Watchdog Timer interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void)
#else
#error Compiler not supported!
#endif
{
    __bic_SR_register_on_exit(LPM0_bits); // exit LPM0 when returning to program (clear LPM0 bits)
}

