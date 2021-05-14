/********************************************************************************************************
*
* bl_main.c      : The main filefor the boot loader.
* Author         : QJ Wang. qjwang@ti.com
* Date           : 2-18-2016
*
* Copyright (c) 2006-2011 Texas Instruments Incorporated.  All rights reserved.
* Software License Agreement
*
* Texas Instruments (TI) is supplying this software for use solely and
* exclusively on TI's microcontroller products. The software is owned by
* TI and/or its suppliers, and is protected under applicable copyright
* laws. You may not combine this software with "viral" open-source
* software in order to form a larger program.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
* NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
* NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
* CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
* DAMAGES, FOR ANY REASON WHATSOEVER.
*
* Author:  Qingjun Wang  (qjwang@ti.com)
*---------------------------------------------------------------------------------------------------------------------
* Revision History
*---------------------------------------------------------------------------------------------------------------------
* Version          Date        Author         Description
*---------------------------------------------------------------------------------------------------------------------
* 00.10.00       19/09/2012    QJ Wang        1. Initial Version
* 00.10.01       10/01/2014    QJ Wang        1. Changes for Rev2.01.00 F021 Flash API
* 00.10.02       11/12/2014    QJ Wang        1. Changes for Rev2.01.01 F021 Flash API
* 00.20.00       18/02/2016    QJ Wang        1. Rewrote code for y-modem protocol, and CRC16
*                                             2. Changes in bl_flash.c, bl_link.cmd
*                                             3. Changed App start address to 0x10100, The 0x10000~0x10100 is left for
*                                                application update status
*                                             4. Added TMS570LS0714 devices
***********************************************************************************************************/

#include "bl_config.h"
long long int APP_START_ADDRESS= 0x00200020;
#if defined (CAN_ENABLE_UPDATE)

#include "bl_can.h"
#include "HL_can.h"
#endif

#if defined (SPI_ENABLE_UPDATE)
#include "bl_spi.h"
#include "HL_spi.h"
#endif

#if defined (UART_ENABLE_UPDATE)
#include "bl_uart.h"
#endif

#include "HL_system.h"
#include "bl_check.h"
#include "sci_common.h"
#include "HL_rti.h"
#include "HL_gio.h"


/*****************************************************************************
* bl_main
******************************************************************************/
#if defined (SPI_ENABLE_UPDATE) || defined(UART_ENABLE_UPDATE) || defined(CAN_ENABLE_UPDATE)

/*****************************************************************************
*
* This holds the current remaining size in bytes to be downloaded.
*
******************************************************************************/
uint32_t g_ulTransferSize;

/*****************************************************************************
*
* This holds the current address that is being written to during a download
* command.
*
******************************************************************************/
uint32_t g_ulTransferAddress;

/*****************************************************************************
*
* This is the data buffer used during transfers to the boot loader.
*
******************************************************************************/
uint32_t g_pulDataBuffer[BUFFER_SIZE];

/*****************************************************************************
*
* This is the data buffer used for update status.
*
* g_pulUpdateSuccess[] are used to store application update status and application
* image's version etc
******************************************************************************/

uint32_t g_pulUpdateSuccess[] = {0x5A5A5A5A, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
uint32_t g_ulUpdateStatusAddr = APP_STATUS_ADDRESS;

uint32_t g_ulUpdateBufferSize = 32; /*32 bytes or 8 32-bit words*/

#define E_PASS     		0
#define E_FAIL     		0x1U
#define E_TIMEOUT  		0x2U

/*****************************************************************************
*
* This is an specially aligned buffer pointer to g_pulDataBuffer to make
* copying to the buffer simpler.  It must be offset to end on an address that
* ends with 3.
*
******************************************************************************/
uint8_t *g_pucDataBuffer;

/*****************************************************************************
*
* This holds the current address that is being written to during a download
* command.
*
******************************************************************************/
void delay(unsigned int delayval) {
	  while(delayval--);
}


void main(void)
{
    uint32_t fnRetValue;

    /* Initialize RTI driver */
    gioInit();
    gioSetDirection(gioPORTB, 0xFFFF);

    /*Enable RTI to check if interrupt affect flash erase/program*/
    rtiInit();
    /* Enable RTI Compare 0 interrupt notification */
    //rtiEnableNotification(rtiREG1,rtiNOTIFICATION_COMPARE0);

    /* Enable IRQ - Clear I flag in CPS register */
    /* Note: This is usually done by the OS or in an svc dispatcher */
    //_enable_IRQ_interrupt_();

    /* Start RTI Counter Block 0 */
    //rtiStartCounter(rtiREG1,rtiCOUNTER_BLOCK0);

    /* Initialize SCI Routines to receive Command and transmit data */
	sciInit();

#if defined (SPI_ENABLE_UPDATE)
	UART_putString(UART, "\r Hercules MCU SPI BootLoader ");
#endif
#if defined (CAN_ENABLE_UPDATE)
	UART_putString(UART, "\r Hercules MCU CAN BootLoader ");
#endif
#if defined (UART_ENABLE_UPDATE)
	UART_putString(UART, "\r Hercules MCU UART BootLoader ");
#endif

	UART_putString(UART, "\r TI Safety MCU Application Team, qjwang@ti.com \r\r");

	//
	//  See if an update should be performed.
	//
    fnRetValue = CheckForceUpdate();

    if ( !fnRetValue )
    {
#ifdef DEBUG_MSG
    	UART_putString(UART, "\r Jump to application...  ");
#endif
        g_ulTransferAddress = (uint32_t)APP_START_ADDRESS;
        ((void (*)(void))g_ulTransferAddress)();
    }

    //
	//  Configure the microcontroller.
	//
	//EnterBootLoader
	#ifdef CAN_ENABLE_UPDATE
	    ConfigureCANDevice(CAN_PORT);
	#endif
	#ifdef SPI_ENABLE_UPDATE
	    ConfigureSPIDevice(SPI_PORT);
	#endif
	#ifdef UART_ENABLE_UPDATE
	    ConfigureUartDevice();
	#endif

	//
	// Branch to the update handler. Use can1
	//
	#ifdef CAN_ENABLE_UPDATE
	    UpdaterCAN(CAN_PORT);
	#endif

	#ifdef UART_ENABLE_UPDATE
	    UpdaterUART();
	#endif
	#ifdef SPI_ENABLE_UPDATE
	    UpdaterSPI(SPI_PORT);
	#endif

}


/******************************************************************************
*
* Configures the microcontroller.
*
* This function configures the peripherals and GPIOs of the microcontroller,
* preparing it for use by the boot loader.  The interface that has been
* selected as the update port will be configured, and auto-baud will be
* performed if required.
*
* \return None.
*
******************************************************************************/
#ifdef SPI_ENABLE_UPDATE
void ConfigureSPIDevice(spiBASE_t *node)
{
    //
    // Initialize the SPI1 as slave mode
    // Enable the SPI interface in slave mode.
    // Set the SPI protocol to Motorola with default clock high and data valid on the rising edge.
    //
    spiInit();
}
#endif

#ifdef UART_ENABLE_UPDATE
void ConfigureUartDevice(void)
{
    //
    // Enable the the clocks to the UART and GPIO modules.
    //
   sciInit();
}
#endif


void rtiNotification(rtiBASE_t *rtiREG, uint32 notification)
{
/*  enter user code between the USER CODE BEGIN and USER CODE END. */
    gioSetBit(gioPORTB, 7, gioGetBit(gioPORTB, 7) ^ 1);
}

#endif

