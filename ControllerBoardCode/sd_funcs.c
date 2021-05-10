/*
 * sd_funcs.c
 *
 *  Created on: May 9, 2021
 *      Author: Daniel Rothfusz
 */

#include "sd_funcs.h"


void
sd_init(FATFS * fs) {
    volatile FRESULT res;

    spi_initialize();

    res = FR_DISK_ERR;
    do {
        res = disk_initialize();
    } while(res != FR_OK);

    do {
        res = pf_mount(fs);
    } while ( res != FR_OK );
}


FRESULT
sd_open(const char * filname) {
    return pf_open(filname);
}


FRESULT
sd_read_byte(void * addr) {
    WORD bytes_read;
    FRESULT res;
    res = pf_read(addr, 1, &bytes_read);
    if (res != 0) {
        return res;
    } else if (bytes_read != 1) {
        return FR_DISK_ERR;
    } else {
        return FR_OK;
    }
}


FRESULT
sd_read_packet(struct packet * pckt) {
    WORD bytes_read;
    FRESULT res;
    res = pf_read(pckt, sizeof(struct packet), &bytes_read);
    if (res != 0) {
        return res;
    } else if (bytes_read != sizeof(struct packet)) {
        return FR_DISK_ERR;
    } else {
        return FR_OK;
    }
}


