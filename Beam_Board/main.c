/**
 * main.c
 *
 * Main file for controlling the break beam feedback board for team CLARG's
 * Jukebox Hero project.
 *
 * Author:
 *      Kevin Lata
 */

#include <msp430.h>
#include <stdint.h>
#include "rgb_interface.h"
#include "display_change.h"

void init_wdt(void);
void init_buttons(void);
int receive_bytes(void* pointer_to_memory);

uint8_t beam_values[154]; // originally 150 but needs a padding of 4
uint8_t beam_values_size;

unsigned char *PRxData;
unsigned char RXByteCtr;

int main_reciept_flag = 0; // flag for checking if you've gotten a value from the main board
int sequence_iter = 0; // iterating value to use if iterating through an array of light/song values
int playertics = 0; // ticking time for evaluating a player's performance
uint8_t current_button = 0; // variable to check against to see what button should be pressed, should be between 0-3
int thresh = 20000; //threshold amount for how many tics until the player loses/gets no points
int listenout = 0; // outcome variable for listening to a player's interaction
int current_check; // for checking a wrong button press
int playerscore = 0; // value to be sent to main board
int main_flag = 0; // i2c interrupt flag for the middle of the game
int player_flag = 0; // player interrupt flag
int new_iter = 0;
uint8_t real_one = 0;
int has_scored = 0;
uint8_t p2reg = 0;

void init_wdt(void){ // initiates watch dog timer
    BCSCTL3 |= LFXT1S_2;     // ACLK = VLO
    WDTCTL = WDT_ADLY_16;    // WDT 16ms (~43.3ms since clk 12khz), ACLK, interval timer
    IE1 |= WDTIE;            // Enable WDT interrupt
}

void init_buttons(void) { // initiates port 2 register values for button interrupts/input
    P2DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3); // set to input
    P2REN = BIT0 + BIT1 + BIT2 + BIT3; // enable pullup/down resistors
    P2OUT = BIT0 + BIT1 + BIT2 + BIT3; // set resistors to pull up
    P2IES = BIT0 + BIT1 + BIT2 + BIT3; // listen for high to low transitions
    P2IFG &=  ~(BIT0 + BIT1 + BIT2 + BIT3); // clear any pending interrupts
    P2IE = BIT0 + BIT1 + BIT2 + BIT3; // enable interrupts for these pins
}

int receive_bytes(void* pointer_to_memory){
    PRxData = (unsigned char *)pointer_to_memory;    // Start of RX buffer
       RXByteCtr = 0;                          // Clear RX byte count
       __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupts
       __no_operation();                // Remain in LPM0 until master finishes TX
       return RXByteCtr;

}

int main(void)
{

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
    UCB0I2COA = 0x50;                         // Own Address is 048h
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
    IE2 |= UCB0RXIE;                          // Enable RX interrupt

    enum state_enum{Start, ListenForInt, SendToBoard} state;
    state = Start;
    //The following sets up watchdog timer related values
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    DCOCTL = 0;                 // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_16MHZ;     // Set range
    DCOCTL = CALDCO_16MHZ;      // Set DCO step + modulation */
    BCSCTL3 |= LFXT1S_2;        // ACLK = VLO

    uint8_t status = 0;

    int Button_LED_map[] = {0, BIT0,BIT1,0,BIT2,0,0,0,BIT3};
    uint8_t current_bit; //hex value that represents dec number 0-3

    init_buttons();
    rgb_init_spi();

    while(1){
        if(state == Start){
            //
            receive_bytes(&beam_values_size);
            for (sequence_iter = 0;sequence_iter < beam_values_size;sequence_iter++){
                receive_bytes(&beam_values[sequence_iter]);
            }
            for (new_iter = sequence_iter+1;new_iter <sequence_iter+4;new_iter++){
                beam_values[new_iter] = 0;
            }
            new_iter = 0;
            change_display(beam_values[0]);
            change_display(beam_values[1]);
            change_display(beam_values[2]);
            sequence_iter = 3;
            main_flag = 0;
            player_flag = 0;
            state = ListenForInt;
//            init_wdt();
        }
        else if (state==ListenForInt){
            while(listenout == 0){ // use another one to exit
//                __bis_SR_register(LPM0_bits + GIE);
                receive_bytes(&status);
                if (main_flag == 1){ // main board i2c run interrupt
                    if (sequence_iter > beam_values_size-5){ // if the song is 150 and we padded by 3, the last actual song value will
                        listenout = 99; //display at the end of sequence when sequence_iter = 152// listenout = 99 is end of song flag
                        sequence_iter = 0;
                    }
                    else if (sequence_iter < beam_values_size){
                        current_button = beam_values[sequence_iter-3]; // it was at the top 3 iters ago
                        real_one = beam_values[sequence_iter];
                        current_bit = Button_LED_map[current_button];
                        //current_check = ((BIT0 + BIT1 + BIT2 + BIT3) & (~current_bit));
                        change_display(real_one);
                        playertics = 0;
                        sequence_iter = sequence_iter + 1;
                        has_scored = 0;
                    }
                    main_flag = 0;
                }
                if (player_flag == 1){ // player run interrupt
                    if ((playertics < thresh)&&((~p2reg & current_bit) == current_bit)){
                        playerscore = playerscore+1;
                        write_correct();
                    }
                    else if (playertics < thresh) {
                        playerscore = playerscore-1;
                        write_incorrect();
                    }
                    player_flag = 0;
                    has_scored =1;
                }
                playertics = playertics+1;
            }
            playertics = 0;
            listenout = 0;
            sequence_iter = 0;
            state = SendToBoard;
        }
        else if (state == SendToBoard){
            //send playerscore to the board, not sure how this happens
            write_score(playerscore, beam_values_size);
//            playerscore = 0;
//            state = Start;
        }
        __bis_SR_register(LPM0_bits + GIE); // little bit of a time gap between states
    }


    return 0;
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
    //timer_wakeup_flag = 1;
    __bic_SR_register_on_exit(LPM0_bits); // exit LPM3 when returning to program (clear LPM3 bits)
}

// Watchdog Timer interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void button(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) button (void)
#else
#error Compiler not supported!
#endif
{
    if(has_scored ==0){
        player_flag = 1;
    }
    p2reg = P2IN;
    P2IFG &= ~(BIT0 + BIT1 + BIT2 + BIT3);
    __bic_SR_register_on_exit(LPM0_bits); // exit LPM3 when returning to program (clear LPM3 bits)
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCIAB0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{

  *PRxData = UCB0RXBUF;                   // Move RX data to address PRxData
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
  main_flag =1;
  UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags
  if (RXByteCtr)                            // Check RX byte counter
    __bic_SR_register_on_exit(LPM0_bits);      // Exit LPM0 if data was
}
