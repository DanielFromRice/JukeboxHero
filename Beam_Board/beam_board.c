#include <msp430.h> 


/**
 * main.c
 */

void init_wdt(void){ // initiates watch dog timer
    BCSCTL3 |= LFXT1S_2;     // ACLK = VLO
    WDTCTL = WDT_ADLY_250;    // WDT 16ms (~43.3ms since clk 12khz), ACLK, interval timer
    IE1 |= WDTIE;            // Enable WDT interrupt
}
void init_buttons() { // initiates port 2 register values for button interrupts/input
    P2DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3); // set to input
    P2REN = BIT0 + BIT1 + BIT2 + BIT3; // enable pullup/down resistors
    P2OUT = BIT0 + BIT1 + BIT2 + BIT3; // set resistors to pull up
    P2IES = BIT0 + BIT1 + BIT2 + BIT3; // listen for high to low transitions
    P2IFG &=  ~(BIT0 + BIT1 + BIT2 + BIT3); // clear any pending interrupts
    P2IE = BIT0 + BIT1 + BIT2 + BIT3; // enable interrupts for these pins
}


int main_reciept_flag = 0; // flag for checking if you've gotten a value from the main board
int sequence_iter = 0; // iterating value to use if iterating through an array of light/song values
int player_tics = 0; // ticking time for evaluating a player's performance
int current_button = 0; // variable to check against to see what button should be pressed, should be between 0-3
int thresh = 20; //threshold amount for how many tics until the player loses/gets no points
int listenout = 0; // outcome variable for listening to a player's interaction
int current_check; // for checking a wrong button press
int playerscore = 0; // value to be sent to main board

int main(void)
{
    enum state_enum{WaitForMain, ChangeDisp, ListenForInt, SendToBoard} state;
    state = WaitForMain;
    //The following sets up watchdog timer related values
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    DCOCTL = 0;                 // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_16MHZ;     // Set range
    DCOCTL = CALDCO_16MHZ;      // Set DCO step + modulation */
    BCSCTL3 |= LFXT1S_2;        // ACLK = VLO

    //P1DIR |= 0x01;
    //P2DIR |= BIT5;
	

    int Button_LED_map[] = {BIT0, BIT1, BIT2, BIT3};
    int current_bit; //hex value that represents dec number 0-3
    init_wdt();
    init_buttons();

    while(1){
        if (state == WaitForMain){//Wait for Board to send signal to change display
            while(main_reciept_flag == 0){
                __bis_SR_register(LPM3_bits + GIE); // if it can't be interrupt enabled, remove this
            }
            main_reciept_flag = 0;
            state = ChangeDisp;
        }
        else if (state==ChangeDisp){//Change the display of the strip LEDs
            // Do we have the sequence of display information given or will it be sent
            sequence_iter = sequence_iter+1;
            state = ListenForInt;
        }
        else if (state==ListenForInt){
            // current_button = next in sequence, should be evaluated here
            while(listenout == 0){
                __bis_SR_register(LPM3_bits + GIE);
                current_bit = Button_LED_map[current_button];
                current_check = ((BIT0 + BIT2 + BIT3 + BIT4) & (~current_bit));
                if ((P2IN & current_bit) != current_bit) {
                    listenout = 1; // correct beam was broken
                }
                if (((P2IN & current_check) != current_check)&&(listenout==0)) {
                    listenout = 2; // incorrect beam was broken
                }
                if ((playertics >thresh)&&(listenout==0)){ // player did not respond in time
                    listenout = 3;
                }
                playertics = playertics + 1;
            }
            if (listenout == 1){
                playerscore = 1;
            }
            if (listenout == 2){
                playerscore = -1;
            }
            if (listenout == 3){
                playerscore == 0;
            }
            playertics = 0;
            listenout = 0;
            state = SendToBoard;
        }
        else if (state == SendToBoard){
            //send playerscore to the board, not sure how this happens
            playerscore = 0;
            state = WaitForMain;
        }
        __bis_SR_register(LPM3_bits + GIE); // little bit of a time gap between states
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
    __bic_SR_register_on_exit(LPM3_bits); // exit LPM3 when returning to program (clear LPM3 bits)
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
    //testing_value = TA0R; // grab value from timerA0 counter while its running in the background
    //button_wakeup_flag = 1;
    P2IFG &= ~(BIT0 + BIT1 + BIT2 + BIT3);
    __bic_SR_register_on_exit(LPM3_bits); // exit LPM3 when returning to program (clear LPM3 bits)
}

