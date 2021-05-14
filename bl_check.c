//*****************************************************************************
//
// bl_check.c: Code to check for a forced update.
// Author    : QJ Wang. qjwang@ti.com
// Date      : 9-19-2012
//
// Copyright (c) 2006-2011 Texas Instruments Incorporated.  All rights reserved.
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

#include "HL_gio.h"
#include "bl_config.h"
#include "bl_check.h"
#include "HL_sci.h"
#include "sci_common.h"

//*****************************************************************************
//
//  bl_check_api
//
//*****************************************************************************

//*****************************************************************************
//
// This global is used to remember if a forced update occurred.
//
//*****************************************************************************

extern unsigned int g_pulUpdateSuccess[8];
extern unsigned int g_pulUpdateFail[8];
extern unsigned int g_ulUpdateStatusAddr;

#ifdef ENABLE_UPDATE_CHECK
uint32_t g_ulForced;
#endif


#if (FORCED_UPDATE_PORT == GPIO_PORTA_BASE)
#define gioPORT gioPORTA
#else
#define gioPORT gioPORTB
#endif


//*****************************************************************************
//
// Checks a GPIO for a forced update.
//
// This function checks the state of a GPIO to determine if a update is being
// requested.
//
// \return Returns a non-zero value if an update is being requested and zero
// otherwise.
//
//*****************************************************************************
#ifdef ENABLE_UPDATE_CHECK
uint32_t
CheckGPIOForceUpdate(void)
{
    /** bring GIO module out of reset */
    gioREG->GCR0      = 1;
    gioREG->ENACLR = 0xFF;
    gioREG->LVLCLR    = 0xFF;  

    // Set the pin as input
    gioPORT->DIR &= ~(1 << FORCED_UPDATE_PIN);

    // Enable the pull up/down.
    gioPORT->PULDIS &= ~(1 << FORCED_UPDATE_PIN);

    // Enable the weak pull up.
    gioPORT->PSL |= 1 << FORCED_UPDATE_PIN;

    // Check the pin to see if an update is being requested.

    if ((gioPORT->DIN & (0x1 << FORCED_UPDATE_PIN) ) == 0)
    {
        // Remember that this was a forced update.
        g_ulForced = 1;
        return(1);
    }

    // No update is being requested so return 0.
    return(0);
}
#endif

//*****************************************************************************
//
// Checks if an update is needed or is being requested.
//
// This function detects if an update is being requested or if there is no
// valid code presently located on the microcontroller.  This is used to tell
// whether or not to enter update mode.
//
// \return Returns a non-zero value if an update is needed or is being
// requested and zero otherwise.
//
//*****************************************************************************
uint32_t
CheckForceUpdate(void)
{
    uint32_t *pulApp;

#ifdef ENABLE_UPDATE_CHECK
    g_ulForced = 0;
#endif

    //
    // See if the last word of 1st sector to see it is 0xFFFFFFFF. If it is 0xFFFFFFFF,
    // there is no application code in the flash. The update is required
    //
#ifdef DEBUG_MSG
    UART_putString(UART, "\r Application update status: ");
#endif
    pulApp = (uint32_t *)g_ulUpdateStatusAddr;
#ifdef DEBUG_MSG
    UART_send32BitData(sciREG1, pulApp[0]);
    UART_putString(UART, "\r Application image address: ");
    UART_send32BitData(sciREG1, pulApp[1]);
    UART_putString(UART, "\r Application image size: ");
    UART_send32BitData(sciREG1, pulApp[2]);
    UART_putString(UART, "\r ");
#endif

    if((pulApp[0] != g_pulUpdateSuccess[0]))
    {
        return(1);    //1 means Need New UPDATE
    }

// if the checked location is 0x5A5A5A5A, check GPIO forced update
#ifdef ENABLE_UPDATE_CHECK
    // If simple GPIO checking is configured, determine whether or not to force an update.
    return(CheckGPIOForceUpdate());
#else
    // GPIO checking is not required so, if we get here, a valid image exists and no update is needed.
    return(0);
#endif

}

