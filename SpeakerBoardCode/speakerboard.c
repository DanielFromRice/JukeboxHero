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


#define SOP         1
#define ALT         2
#define TEN         3
#define BAS         4

#define BOARDNUM    SOP  // CHANGE THIS VALUE

#define SOP_ADDR    0x48
#define ALT_ADDR    0x49
#define TEN_ADDR    0x4A
#define BAS_ADDR    0x4B


int receive_bytes(void* pointer_to_memory);

unsigned char *PRxData;                     // Pointer to RX data
unsigned char RXByteCtr;
//volatile unsigned char RxBuffer[128];       // Allocate 128 byte of RAM
uint16_t in_data;


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
  int cnt = 2;

  // Operational code
  while(cnt == 2) {
      TA1CCR0 = in_data;
      TA1CCR2 = in_data >> 1;
      cnt = receive_bytes(&in_data);
  }

  return 0;

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
  if (RXByteCtr)                            // Check RX byte counter
    __bic_SR_register_on_exit(LPM0_bits);      // Exit LPM0 if data was
}
