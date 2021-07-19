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

#if defined (UART_ENABLE_UPDATE)
//#include "bl_uart.h"
#endif

#include "HL_system.h"
#include "sci_common.h"
#include "HL_rti.h"
#include "HL_gio.h"
#include "ti_fee.h"
#include "bl_eeprom.h"
#include "bl_launch.h"


#include "bl_config.h"
#include "HL_sci.h"
#include "sci_common.h"
#include "bl_ymodem.h"
#include "HL_sys_common.h"
#include "printf.h"
#include "bl_launch.h"
#include "bl_eeprom.h"
#include "HL_reg_system.h"
#include "ti_fee.h"

uint32_t JumpAddress;
void get_software_Version(void);
void get_hardware_Info(void);

extern char fileName[FILENAME_LEN];
char tab_1024[1024] = {    0  };

uint8_t receive_command[10]={0};  //format maybe #0x12345678$
uint32_t address_num=0;

int crc_GI= 0xB371;
int crc_WI= 0x4D07;
int size_WI= 16564;
int size_GI= 16372;
int crc_out;
int write_count=0;
//uint8_t cint32_2_char[8]={0};
unsigned char ascii_to_num[8]={0};
int image_num=0;
uint32_t image_num_arr[2]={0};


/*****************************************************************************
* bl_main
******************************************************************************/
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

void delay(unsigned int delayval) {
	  while(delayval--);
}
//*****************************************************************************
//
//  bl_uart_api
//
//
//*****************************************************************************
/**
  * @brief  Download a file via serial port
  * @param  None
  * @retval None
  */
int UART_Download(uint32_t address) {
    char Number[10] = "          ";
    int Size = 0;

    printf("\n\r Waiting for the file to be sent ... ");
    printf("\n\r Use the ymodem protocol in your chosen serial terminal \n\r");
    eeprom_shutdown();
    Size = Ymodem_Receive(UART, &tab_1024[0], address);
    eeprom_init();
    if (Size > 0) {
        printf("\n\r The application image has been programmed successfully!\n\r---------------------------\n\r Name: ");
        printf(fileName);
        Int2Str(Number, Size);
        printf("\n\n\r Size:     ");
        printf(Number);
        printf("  Bytes\n\r");
        printf("---------------------------\n\n\n\n\r");
    } else {
        printf("\n\rFailed to receive tconstRunStarthe file!\n\r");
    }
    return Size;
}

bool start_application(void);
void start_golden();

char get_boot_type(int rstsrc, boot_info *b_inf) {

    char stored_boot_type = eeprom_get_boot_type();

    switch(rstsrc) {
    case POWERON_RESET:
        return stored_boot_type;
    case DEBUG_RESET:
    case EXT_RESET:
        return 'B';
    case SW_RESET:
        if (b_inf->attempts >= 5) {
            b_inf->attempts = 0;
            if (stored_boot_type == 'A') {
                return 'G';
            } else {
                return 'B';
            }
        } else {
            b_inf->attempts += 1;
            return stored_boot_type;
        }
    default:
        return 'B';
    }
}

void main(int rstsrc) {
    char bootType;
    bool fee_init = eeprom_init();
    boot_info b_inf;

    if (!fee_init) {
        bootType = 'B'; // EEPROM didn't work, emergency mode
    } else {
        b_inf = eeprom_get_boot_info();
        bootType = get_boot_type(rstsrc, &b_inf);
    }

    b_inf.count += 1;
    eeprom_set_boot_info(b_inf);

    switch(bootType) {
    case 'A': start_application(); // no break to automatically attempt start golden on failure
    case 'G': start_golden(); break;
    case 'B':
    default: break;
    }

    // if we make it here the golden image didn't work

    /* Initialize SCI Routines to receive Command and transmit data */
	sciInit();

	// Golden Image address cannot change in this version of the bootloader. as such,
	// bootloader will reset golden image info if it is different from the default
	image_info golden_info = {0};
	golden_info = eeprom_get_golden_info();
	if (golden_info.addr != GOLD_DEFAULT_ADDR) {
	    golden_info.addr = GOLD_DEFAULT_ADDR;
	    golden_info.size = 0;
	    golden_info.crc = 0;
	    golden_info.exists = 0;
	    eeprom_set_golden_info(golden_info);
	}

    char key = 0;

    while (1) {
        int delayCount = 0;
        do {
            TI_Fee_MainFunction();
            delayCount++;
            if (delayCount > 10000) { // timeout after trying this many times
                printf("\r\nTI FEE error\r\n");
                break;
            }
        } while (TI_Fee_GetStatus(0) != IDLE);

        printf("\r\n================== Main Menu ==========================\r\n");
        printf("  0. Set Application Start Address\r\n");
        printf("  1. Upload New Application Binary\r\n");
        printf("  2. Upload New Golden Binary\r\n");
        printf("  3. Set next boot to Application \r\n");
        printf("  4. Get Bootloader Version \r\n");
        printf("  5. Get Device Information \r\n");
        printf("  6. Set next boot to Golden image \r\n");
        printf("  7. Upload CRC of application image \r\n");
        printf("  8. Upload CRC of golden image \r\n");
        printf("  9. Reboot\r\n");
        printf("  A. Ping\r\n");
        printf("  B. Erase application image\r\n");
        printf("=======================================================\r\n\n");

        key = UART_getKey(UART);

        if (key == '0') {
            // Set Application Start Address
            printf("Enter The Application Start Address in Hex. Format: 0xNNNNNNNN\n\r\n\r");
            sciReceive(UART, 10, &receive_command[0]);
            printf("\n\r\n\r The Entered Address Is: ");
            int c;
            for (c = 0; c < 10; c++) {
                printf("%c",receive_command[c]);
            }
            printf("\r\n");

           uint32_t address_num;
           if (Str2Int(receive_command, &address_num)) {
               image_info app_info;
               app_info = eeprom_get_app_info();
               app_info.addr = address_num;
               app_info.crc = 0x7398;
               eeprom_set_app_info(app_info);
           }
        }

        else if (key == '1') {
            // Upload New Application Binary
            image_info app_info = {0};
            int size;
            app_info = eeprom_get_app_info();
            if (app_info.addr != 0) {
                size = UART_Download(app_info.addr);
                if (size) {
                    app_info.size = size;
                    app_info.exists = EXISTS_FLAG;
                    eeprom_set_app_info(app_info);
                }
            } else printf("Set application address before flashing");
        }

        else if (key == '2') {
            // Upload New Golden Binary
            image_info app_info = {0};
            app_info = eeprom_get_golden_info();
            int size;

            if (app_info.addr != 0) {
                size = UART_Download(app_info.addr);
                if (size) {

                    app_info.size = size;
                    app_info.exists = EXISTS_FLAG;
                    eeprom_set_golden_info(app_info);
                }
            } else printf("Set Golden Image address before flashing");
        }

        else if (key == '3') {
            // Set next boot to Application

            if (verify_application()) {
                printf("CRC passed, Next Boot set to Application\r\n");
                eeprom_set_boot_type('A');
            } else {
                printf("CRC failed, next boot unchanged\r\n");
            }
        }

        else if (key == '4') {
            get_software_Version();
        }

        else if (key == '5') {
            get_hardware_Info();
        }

        else if (key == '6') {
            // Set next boot to Golden image
            if (verify_golden()) {
                printf("CRC passed, Next Boot set to Golden Image\r\n");
                eeprom_set_boot_type('G');
            } else {
                printf("CRC failed, next boot unchanged\r\n");
            }
        }

        else if (key == '7') {
            // Upload CRC of application image
            printf("Enter application CRC (hex): ");
            sciReceive(UART, 4, &receive_command[0]);
            int i;
            for (i = 0; i < 4; i++) {
                if (receive_command[i] >= 48 && receive_command[i] <= 57) {
                    ascii_to_num[i] = receive_command[i]-'0';
                }
                else if (receive_command[i] >= 97 && receive_command[i] <= 102) {
                    ascii_to_num[i] = receive_command[i]-87;
                }
                    else if (receive_command[i] >= 65 && receive_command[i] <= 70) {
                    ascii_to_num[i] = receive_command[i]-55;
                }
            }

           uint16_t crc_num =   (uint32_t)ascii_to_num[0] << 12 |
                                (uint32_t)ascii_to_num[1] << 8  |
                                (uint32_t)ascii_to_num[2] << 4  |
                                (uint32_t)ascii_to_num[3];
           image_info app_info = eeprom_get_app_info();
           app_info.crc = crc_num;
           eeprom_set_app_info(app_info);
        }

        else if (key == '8') {
           //Upload CRC of golden image
           printf("Enter Golden Image CRC (hex): ");
           sciReceive(UART, 4, &receive_command[0]);
           int i;
           for (i = 0; i < 4; i++) {
               if (receive_command[i] >= 48 && receive_command[i] <= 57) {
                   ascii_to_num[i] = receive_command[i]-'0';
               }
               else if (receive_command[i] >= 97 && receive_command[i] <= 102) {
                   ascii_to_num[i] = receive_command[i]-87;
               }
                   else if (receive_command[i] >= 65 && receive_command[i] <= 70) {
                   ascii_to_num[i] = receive_command[i]-55;
               }
           }

          uint16_t crc_num =   (uint32_t)ascii_to_num[0] << 12 |
                               (uint32_t)ascii_to_num[1] << 8  |
                               (uint32_t)ascii_to_num[2] << 4  |
                               (uint32_t)ascii_to_num[3];
          image_info gold_info = eeprom_get_golden_info();
          gold_info.crc = crc_num;
          eeprom_set_golden_info(gold_info);
        }

        else if (key == '9') {
            // Reboot
            printf("Rebooting...\r\n");
            eeprom_shutdown();
            systemREG1->SYSECR = (0x10) << 14;
            break;
        }

        else if (key == 'A') {
            printf("\r\nPing\r\n");
        }

        else if (key == 'B') {
            image_info erase;
            erase.crc = 0;
            erase.exists = 0;
            erase.addr = 0;
            erase.size = 0;
            eeprom_set_app_info(erase);
        }

        else {
           printf("Invalid Number !! \n\r\n\r");
        }
    }
}




