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

uint8_t beam_values[154]; // Should hold all of the beam pattern data and have extra room just in case
uint8_t beam_values_size; // Value letting us know how long the beam pattern array is

unsigned char *PRxData; // Pointers and variables used in receiving data from the main board
unsigned char RXByteCtr;

int main_reciept_flag = 0; // flag for checking if you've gotten a value from the main board
int sequence_iter = 0; // general use iterating variable
int playertics = 0; // ticking time for evaluating a player's performance
uint8_t current_button = 0; // variable to check against to see what button should be pressed, should be between 0-3
int thresh = 20000; //threshold amount for how many tics until the player loses/gets no points
int listenout = 0; // outcome variable for listening to a player's interaction
int current_check; // Variable for checking if the wrong beam was broken (equivalent to wrong button press)
int playerscore = 0; // value containing the player's score
int main_flag = 0; // i2c interrupt flag for the middle of the game
int player_flag = 0; // player interrupt flag (player has broken a beam)
int new_iter = 0; // additional iterating variable
uint8_t real_one = 0; //variable for changing the LED display to the correct iteration
int has_scored = 0; // variable to keep track if the player has scored already or not
uint8_t p2reg = 0; // variable to be used in checking if the correct beam was broken

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

    uint8_t status = 0; // variable used for receiving signals from the main board during the "gaming" state (ListenForInt)

    int Button_LED_map[] = {0, BIT0,BIT1,0,BIT2,0,0,0,BIT3};
    uint8_t current_bit; //hex value that represents dec number 0-3

    init_buttons(); // initialization functions
    rgb_init_spi();

    while(1){
        if(state == Start){ // In this state the board will receive the beam array pattern from the main board and store it
            receive_bytes(&beam_values_size); // It receives the size of the data to be received first
            for (sequence_iter = 0;sequence_iter < beam_values_size;sequence_iter++){
                receive_bytes(&beam_values[sequence_iter]); // It gets it one byte at a time which is why this for loop is necessary
            }
            for (new_iter = sequence_iter+1;new_iter <sequence_iter+4;new_iter++){
                beam_values[new_iter] = 0; // this pads the beam_values array after it recieved all of the relevant data with 0 values
            }
            new_iter = 0;
            change_display(beam_values[0]); // These three lines load the first three display updates onto the strip display
            change_display(beam_values[1]); // Such that by the time the game starts (the board listens to player interrupts)
            change_display(beam_values[2]); // The bottom most row has an LED on (since display updates go from the top down)
            sequence_iter = 3; // Values set to whatever necessary to move onto the nect state
            main_flag = 0;
            player_flag = 0;
            state = ListenForInt;
//            init_wdt();
        }
        else if (state==ListenForInt){
            while(listenout == 0){ // use another one to exit
//                __bis_SR_register(LPM0_bits + GIE);
                receive_bytes(&status); // Goes to sleep until an interrupt takes place
                if (main_flag == 1){ // main board i2c run interrupt
                    if (sequence_iter > beam_values_size-5){ // In this case, the song has finished playing
                        listenout = 99; // listenout = 99 is end of song flag
                        sequence_iter = 0; // resets value since it will exit this state soon
                    }
                    else if (sequence_iter < beam_values_size){ // In this case, the song is in the middle of playing and display should update
                        current_button = beam_values[sequence_iter-3]; // The button corresponding to the bottom LED was at the top 3 iters ago
                        real_one = beam_values[sequence_iter]; // Current value that determines how to update the top row of the display
                        current_bit = Button_LED_map[current_button]; // essentially passes it to a lookup table to get its associated value
                        //current_check = ((BIT0 + BIT1 + BIT2 + BIT3) & (~current_bit));
                        change_display(real_one); // changes the display according to the correct value
                        playertics = 0; // resets player timer since the display has just changed
                        sequence_iter = sequence_iter + 1; // updates where in the array we currently are
                        has_scored = 0; // flag for designating that the display has been updated
                    }
                    main_flag = 0; // interrupt flag for getting update from main board is cleared
                }
                if (player_flag == 1){ // player run interrupt was triggered
                    if ((playertics < thresh)&&((~p2reg & current_bit) == current_bit)){ // correct beam was broken and player didnt time out
                        playerscore = playerscore+1; //  player's score increases by 1
                        write_correct(); // LEDs display that the player got this note correctly/got a point, bottom row lights up green
                    }
                    else if (playertics < thresh) {
                        playerscore = playerscore-1; // player broke the wrong beam
                        write_incorrect(); // LEDs display that the player got this note incorrectly/lost a point, bottom row lights up red
                    }
                    player_flag = 0; // clears associated interrupt flag
                    has_scored =1; // designates that after a display update, the player responded, either correctly or incorrectly
                }
                playertics = playertics+1; // player timer increases
            }
            playertics = 0; // Upon exiting this state, reset certain values
            listenout = 0;
            sequence_iter = 0;
            state = SendToBoard;
        }
        else if (state == SendToBoard){ // This state displays on the LED strips the player's performance
            //send playerscore to the board, not sure how this happens
            write_score(playerscore, beam_values_size); // Depending on what fraction of the total notes the player got correct
//            playerscore = 0; // they may receive 0, 1, 2, 3, or 4 rows worth of LEDs be on
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
    if(has_scored ==0){ // PLayer broke a beam after the display has been updated
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
  main_flag =1; // signal from main board has been received
  UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags
  if (RXByteCtr)                            // Check RX byte counter
    __bic_SR_register_on_exit(LPM0_bits);      // Exit LPM0 if data was
}
