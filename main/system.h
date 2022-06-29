#ifndef __BL_CONFIG_H__
#define __BL_CONFIG_H__
#include "subsystems_ids.h"

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
#if defined(IS_ATHENA)
#define CSP_SCI sciREG2 // UART2
#else
#define CSP_SCI sciREG3 /* Use appropriate UART port */
#endif

#define PRINTF_SCI NULL
#define BUFFER_SIZE 64 /*words in the data buffer used for receiving packets*/
#define NORMAL_SERVICE_PRIO (tskIDLE_PRIORITY + 1)

typedef enum {
    SATR_PKT_ILLEGAL_APPID = 0,
    SATR_PKT_ILLEGAL_SUBSERVICE,
    SATR_OK,
    SATR_ERROR,
    SATR_RETURN_FROM_TASK,
    SATR_BUFFER_ERR,
    /*LAST*/
    SATR_LAST
} SAT_returnState;
//#define   DEBUG_MSG_L3
#endif // __BL_CONFIG_H__
