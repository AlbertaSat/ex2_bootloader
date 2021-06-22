/*
 * el_flash.c
 *
 *  Created on: Jun. 17, 2021
 *      Author: robert
 */

#include "el_flash.h"
#include "HL_sci.h"
#include "HL_spi.h"
#include "HL_gio.h"
#include "HL_het.h"

void do_flash_electra() {

    gioSetBit(hetPORT1, nCONFIG, 0);
    int d;
    for (d=0; d<10000; d++){}
    gioSetBit(hetPORT1, nCONFIG, 1);
    while (gioGetBit(hetPORT1, nSTATUS)); // wait for target to release nSTATUS
    uint16_t newest = 0;
    //spiREG1->DAT0 = 0;
    while (!gioGetBit(hetPORT1, CONF_DONE)) {
        char high = sciReceiveByte(UART);
        char low = sciReceiveByte(UART);
        newest = (low << 8) | high;

        while ((spiREG1->FLG & 0x0200) == 0); // Wait until TXINTFLG is set for previous transmission
        spiREG1->DAT0 = newest;
        while ((spiREG1->FLG & 0x0100) == 0); // Wait until RXINTFLG is set when new value is received
        uint8_t _ = ((unsigned char)spiREG1->BUF);  // throw away received value
    }
    while(!gioGetBit(hetPORT1, CONF_DONE));

    return;
}
