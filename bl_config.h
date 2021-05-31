//*****************************************************************************
//
// bl_config.h - The configurable parameters of the boot loader.
// Author    : QJ Wang. qjwang@ti.com

// Copyright (c) 2008-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
//
//*****************************************************************************

#ifndef __BL_CONFIG_H__
#define __BL_CONFIG_H__

#include <stdint.h>

//*****************************************************************************
//
// The following defines are used to configure the operation of the boot
// loader.  For each define, its interactions with other defines are described.
// First is the dependencies (in other words, the defines that must also be
// defined if it is defined), next are the exclusives (in other words, the
// defines that can not be defined if it is defined), and finally are the
// requirements (in other words, the defines that must be defined if it is
// defined).
//
// The following defines must be defined in order for the boot loader to
// operate:
//
//     One of CAN_ENABLE_UPDATE, SPI_ENABLE_UPDATE, SCI_ENABLE_UPDATE
//     APP_START_ADDRESS
//
//*****************************************************************************

//*****************************************************************************
//
// The frequency (MHz) of the crystal used to clock the microcontroller.
//
// This defines the crystal frequency used by the microcontroller running the
// boot loader.
//
// Depends on: None
// Exclusive of: None
// Requires: None
//
//*****************************************************************************
#define   TMS570LC43
#define CRYSTAL_FREQ            16             //MHz
#define SYS_CLK_FREQ            150            //MHz

//*****************************************************************************
// Selects the UART as the port for communicating with the boot loader.
// Exclusive of: CAN_ENABLE_UPDATE, SPI_ENABLE_UPDATE,
// Requires: UART_FIXED_BAUDRATE, BUFFER_SIZE
//*****************************************************************************
#define UART_ENABLE_UPDATE

//*****************************************************************************
// The starting address of the application.  This must be a multiple of 32K(sector size)
// bytes (making it aligned to a page boundary), and can not be 0 (the first sector is 
// boot loader). 
//
// The flash image of the boot loader must not be larger than this value.
//*****************************************************************************
//#define APP_START_ADDRESS       0x00200020

//*****************************************************************************
// The blocknumber to store the update status of the application image
// It contains Application Start Address, Application Image Size, etc
//
//*****************************************************************************
#define EXISTS_FLAG 0xA5A5A5A5

#define APP_STATUS_BLOCKNUMBER 2 // status byte reports 1 if program present
#define APP_STATUS_OFFSET 0
#define APP_STATUS_LEN sizeof(image_info)

#define GOLD_STATUS_BLOCKNUMBER 3
#define GOLD_STATUS_OFFSET 0
#define GOLD_STATUS_LEN sizeof(image_info)

#define GOLD_MINIMUM_ADDR 0x00018000
#define GOLD_DEFAULT_ADDR 0x00018000
#define GOLD_START_BANK 0

#define APP_MINIMUM_ADDR 0x00200000
#define APP_DEFAULT_ADDR 0x00200000
#define APP_START_BANK 1

#define BOOTLOADER_MAX_ADDR 0x0017FFF

#define BOOT_TYPE_BLOCK 1
#define BOOT_TYPE_OFFSET 0
#define BOOT_TYPE_LEN 1

// Representation of data which will be stored in FEE flash
typedef struct __attribute__((packed)) {
    uint32_t exists; // 1 for exists, 0 for does not exist
    uint32_t size;
    uint32_t addr;
    uint16_t crc;
} image_info;

/* UART is used in all the boot modes*/
#define UART              sciREG1   /* Use UART port 1 for UART boot */

#define BUFFER_SIZE             64       /*words in the data buffer used for receiving packets*/


//#define   DEBUG_MSG_L3
#endif // __BL_CONFIG_H__
