/*
 * display_change.c
 *
 *  Created on: May 11, 2021
 *      Author: Kevin Lata
 */
#include "display_change.h"


uint8_t *led1col1row =(uint8_t *) &off;
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

void change_display(uint8_t inval){
    led1col4row = led1col3row;
    led1col3row = led1col2row;
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

    switch(inval){
    case 1:
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

void write_correct(void) {
    led1col4row =(uint8_t *) &green;
    led2col4row =(uint8_t *) &green;
    led3col4row = (uint8_t *) &green;
    led4col4row =(uint8_t *) &green;

    send_leds();
}


void write_incorrect(void) {
    led1col4row =(uint8_t *) &red;
    led2col4row =(uint8_t *) &red;
    led3col4row = (uint8_t *) &red;
    led4col4row =(uint8_t *) &red;

    send_leds();
}


void write_score(int score, int song_length) {
    led1col1row =(uint8_t *) &off;
    led1col2row =(uint8_t *) &off;
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

static void send_leds(void) {
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
