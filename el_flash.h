/*
 * el_flash.h
 *
 *  Created on: Jun. 17, 2021
 *      Author: robert
 */

#ifndef EL_FLASH_H_
#define EL_FLASH_H_



#define nCONFIG     30
#define nSTATUS     16
#define CONF_DONE   18
#define UART sciREG3
#define SPI spiREG1

void do_flash_electra();
#endif /* EL_FLASH_H_ */
