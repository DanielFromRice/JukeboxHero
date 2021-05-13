/*
 * display_change.c
 *
 *  Created on: May 11, 2021
 *      Author: Kevin Lata
 *      This source file is in charge of showing/updating the display on the strip LEDs
 */
#include "display_change.h"


uint8_t *led1col1row =(uint8_t *) &off; // All of these are state variables for each individual LED in the strip LED display
uint8_t *led1col2row =(uint8_t *) &off;
uint8_t *led1col3row =(uint8_t *) &off;
uint8_t *led1col4row = (uint8_t *)&off;
uint8_t *led2col1row =(uint8_t *) &off;
uint8_t *led2col2row =(uint8_t *) &off;
uint8_t *led2col3row =(uint8_t *) &off;
uint8_t *led2col4row =(uint8_t *) &off;
uint8_t *led3col1row = (uint8_t *)&off;
uint8_t *led3col2row = (uint8_t *)&off;
uint8_t *led3col3row =(uint8_t *) &off;
uint8_t *led3col4row =(uint8_t *) &off;
uint8_t *led4col1row = (uint8_t *)&off;
uint8_t *led4col2row =(uint8_t *) &off;
uint8_t *led4col3row =(uint8_t *) &off;
uint8_t *led4col4row =(uint8_t *) &off;

void change_display(uint8_t inval){ // This function updates the display given the signal from the main board to update the display
    led1col4row = led1col3row; // These lines update each LED to be the value that the previous LED on the same column previously was
    led1col3row = led1col2row; // starting from the bottom row going on upwards
    led1col2row = led1col1row;

    led2col4row = led2col3row;
    led2col3row = led2col2row;
    led2col2row = led2col1row;

    led3col4row = led3col3row;
    led3col3row = led3col2row;
    led3col2row = led3col1row;

    led4col4row = led4col3row;
    led4col3row = led4col2row;
    led4col2row = led4col1row;

    switch(inval){ // Depending on the value input into this function, this switch case turns on the appropriate LED on the top row
    case 1: // for the current song/display iteration
        led1col1row =(uint8_t *) &blue;
        led2col1row =(uint8_t *) &off;
        led3col1row =(uint8_t *) &off;
        led4col1row =(uint8_t *) &off;
        break;
    case 2:
        led1col1row =(uint8_t *) &off;
        led2col1row =(uint8_t *) &green;
        led3col1row =(uint8_t *) &off;
        led4col1row =(uint8_t *) &off;
        break;
    case 4:
        led1col1row =(uint8_t *) &off;
        led2col1row =(uint8_t *) &off;
        led3col1row =(uint8_t *) &red;
        led4col1row =(uint8_t *) &off;
        break;
    case 8:
        led1col1row =(uint8_t *) &off;
        led2col1row =(uint8_t *) &off;
        led3col1row =(uint8_t *) &off;
        led4col1row =(uint8_t *) &yellow;
        break;
    default:
        led1col1row =(uint8_t *) &off;
        led2col1row =(uint8_t *) &off;
        led3col1row = (uint8_t *) &off;
        led4col1row =(uint8_t *) &off;
        break;
    }

    send_leds();
}

void write_correct(void) { // This function displays the bottom row of the array as all green to show the player that they
    led1col4row =(uint8_t *) &green; // broke the correct beam/their score increased
    led2col4row =(uint8_t *) &green;
    led3col4row = (uint8_t *) &green;
    led4col4row =(uint8_t *) &green;

    send_leds();
}


void write_incorrect(void) { // This function displays the bottom row of the array as all red to show the player that they
    led1col4row =(uint8_t *) &red; // broke the incorrect beam/their score decreased
    led2col4row =(uint8_t *) &red;
    led3col4row = (uint8_t *) &red;
    led4col4row =(uint8_t *) &red;

    send_leds();
}


void write_score(int score, int song_length) { // This function displays the player's final score on the strip LEDs
    led1col1row =(uint8_t *) &off; // depending on how well the player did, more rows of LEDs will light up if the player did a better job
    led1col2row =(uint8_t *) &off; // and got a higher score
    led1col3row =(uint8_t *) &off;
    led1col4row = (uint8_t *)&red;
    led2col1row =(uint8_t *) &off;
    led2col2row =(uint8_t *) &off;
    led2col3row =(uint8_t *) &off;
    led2col4row =(uint8_t *) &red;
    led3col1row = (uint8_t *)&off;
    led3col2row = (uint8_t *)&off;
    led3col3row =(uint8_t *) &off;
    led3col4row =(uint8_t *) &red;
    led4col1row = (uint8_t *)&off;
    led4col2row =(uint8_t *) &off;
    led4col3row =(uint8_t *) &off;
    led4col4row =(uint8_t *) &red;

    if (score > (song_length / 8)) {
        led1col3row =(uint8_t *) &yellow;
        led2col3row =(uint8_t *) &yellow;
        led3col3row = (uint8_t *) &yellow;
        led4col3row =(uint8_t *) &yellow;
    }
    if (score > (song_length / 4)) {
        led1col2row =(uint8_t *) &green;
        led2col2row =(uint8_t *) &green;
        led3col2row = (uint8_t *) &green;
        led4col2row =(uint8_t *) &green;
    }
    if (score > (song_length / 2)) {

        led1col1row =(uint8_t *) &green;
        led2col1row =(uint8_t *) &green;
        led3col1row = (uint8_t *) &green;
        led4col1row =(uint8_t *) &green;
    }

    send_leds();
}

static void send_leds(void) { // This function just sends the necessary frames via SPI to get the desired display up on the strip LEDs
    rgb_send_start();
    rgb_send_frame(led1col1row, false);
    rgb_send_frame(led1col2row, false);
    rgb_send_frame(led1col3row, false);
    rgb_send_frame(led1col4row, false);
    rgb_send_frame(led2col1row, false);
    rgb_send_frame(led2col2row, false);
    rgb_send_frame(led2col3row, false);
    rgb_send_frame(led2col4row, false);
    rgb_send_frame(led3col1row, false);
    rgb_send_frame(led3col2row, false);
    rgb_send_frame(led3col3row, false);
    rgb_send_frame(led3col4row, false);
    rgb_send_frame(led4col1row, false);
    rgb_send_frame(led4col2row, false);
    rgb_send_frame(led4col3row, false);
    rgb_send_frame(led4col4row, false);
    rgb_send_end();
}
