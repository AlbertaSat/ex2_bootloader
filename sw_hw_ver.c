//*****************************************************************************
//
// sw_hw_ver.c : Funcions that are used to find the Software and the Hardware information
// Author      : QJ Wang. qjwang@ti.com
// Date        : 9-19-2012
//
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
//*****************************************************************************

/* Include Files */
#include "bl_config.h"

#if defined (UART_ENABLE_UPDATE)
#include "HL_sys_common.h"
#include "HL_system.h"
#include "HL_sci.h"
#include "sci_common.h"


#define DIE_ID1 		(*(volatile unsigned int *) 0xFFFFE1F0)
#define DIE_ID2 		(*(volatile unsigned int *) 0xFFFFE1F4)
#define DIE_ID3 		(*(volatile unsigned int *) 0xFFFFE1F8)
#define DIE_ID4	 	    (*(volatile unsigned int *) 0xFFFFE1FC)

//extern void sciSend_32bitdata(sciBASE_t *sci, unsigned int data);

/** @fn get_software_Version(void)
*   @brief Get the Software Version of the Demo
*/
void get_software_Version(void)
{
	UART_putString(UART, "\n\r\ The BootLoader Version: V1.0 \n\n\r" );
	return;
}

/** @fn get_hardware_Info(void)
*   @brief Get the Hardware Information of the Micro
*		- Device ID
*  		- LOT Number
*  		- WAFFER Number + DIE Locations ( X & Y)
* 		- Package Type + Flash Memory 
*	@note: The above Hardware informations are stored in the OTP locations	
*/
void get_hardware_Info(void)
{
	unsigned int LOT_NUM, WAFER_LOC_NUM;
	LOT_NUM   =  ((DIE_ID1 & 0xFFC00000) >> 22) | ((DIE_ID2 & 0x00003FFF) << 10);
	WAFER_LOC_NUM = (DIE_ID1 & 0x003FFFFF);
	
	UART_putString(UART, "\n\r Device Information: \r\n\ ");

	UART_putString(UART, "DEV:  ");
	UART_send32BitData(UART, systemREG1->DEVID);
	UART_putString(UART, "  \r\n ");
	UART_putString(UART, "LOT NUM:  ");
	UART_send32BitData(UART, LOT_NUM);
	UART_putString(UART, " \r\n ");
	UART_putString(UART, "WAFER LOC NUM:  ");
	UART_send32BitData(UART, (WAFER_LOC_NUM));
	UART_putString(UART, " \n\n\r\n ");

	return;
}
#endif
