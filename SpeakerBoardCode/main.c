/*
 *  MSP430 Jukebox Hero Speaker board code
 *
 *  Author:
 *      Daniel Rothfusz, Michael Angino
 *
 *  Based on I2C code from msp430g2xx3_uscib0_i2c_09.c
 *      by D. Dang, Texas Instruments
 *
 */
#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


int receive_bytes(void* pointer_to_memory);

void init_wdt(void);

#define SOP         1
#define ALT         2
#define TEN         3
#define BAS         4

#define BOARDNUM    SOP  // CHANGE THIS VALUE

#define SOP_ADDR    0x48
#define ALT_ADDR    0x49
#define TEN_ADDR    0x4A
#define BAS_ADDR    0x4B


unsigned char *PRxData;                     // Pointer to RX data
unsigned char RXByteCtr;
//volatile unsigned char RxBuffer[128];       // Allocate 128 byte of RAM
uint16_t in_data, ticks;
uint8_t new_data_flag;


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  uint16_t i2c_addr;
  switch(BOARDNUM) {
  case SOP:
      i2c_addr = SOP_ADDR;
      break;
  case ALT:
      i2c_addr = ALT_ADDR;
      break;
  case TEN:
      i2c_addr = TEN_ADDR;
      break;
  case BAS:
      i2c_addr = BAS_ADDR;
      break;
  default:
      i2c_addr = 0xFF;
  }

  // I2C setup
  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
  UCB0I2COA = i2c_addr;                         // Own Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
  IE2 |= UCB0RXIE;                          // Enable RX interrupt

  // Buzzer/timer setup
  P2SEL |= BIT5;
  P2DIR |= BIT5;

  TA1CTL |= TASSEL_2 + MC_1;                 // Set TA1 to use SMCLK, Up mode
  TA1CCTL2 = OUTMOD_2;                      // Output mode 3 - set/reset

  in_data = 0;
  ticks = 0;

  // Operational code
  while(1) {
      if (new_data_flag) {
          TA1CCR0 = 0;
          TA1CCR2 = 0;
          new_data_flag = 0;
          init_wdt();
          ticks = 0;
      }
      if (ticks == 4) {
          TA1CCR0 = in_data;
          TA1CCR2 = in_data >> 1;
          IE1 &= ~WDTIE;
      }
      ticks += 1;
      receive_bytes(&in_data);
  }

  return 0;

}

void init_wdt(void) {
    BCSCTL3 |= LFXT1S_2;      // ACLK = VLO
    WDTCTL = WDT_ADLY_1_9;    // WDT 1.9ms (~43.3ms since clk 12khz), ACLK, interval timer
    IE1 |= WDTIE;             // Enable WDT interrupt
}

/*
 * Returns the number of bytes received
 */
int receive_bytes(void* pointer_to_memory){
    PRxData = (unsigned char *)pointer_to_memory;    // Start of RX buffer
       RXByteCtr = 0;                          // Clear RX byte count
       __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupts
       __no_operation();                // Remain in LPM0 until master finishes TX
       return RXByteCtr;
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
    ticks += 1;
    __bic_SR_register_on_exit(LPM0_bits); // exit LPM0 when returning to program (clear LPM0 bits)
}

//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C master
// to the MSP430 memory.
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
  *PRxData++ = UCB0RXBUF;                   // Move RX data to address PRxData
  RXByteCtr++;                              // Increment RX byte count
}

//------------------------------------------------------------------------------
// The USCI_B0 state ISR is used to wake up the CPU from LPM0 in order to
// process the received data in the main program. LPM0 is only exit in case
// of a (re-)start or stop condition when actual data was received.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCIAB0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags
  if (RXByteCtr)  {                          // Check RX byte counter
    new_data_flag = 1;
    __bic_SR_register_on_exit(LPM0_bits);      // Exit LPM0 if data was
  }
}
