#ifdef NOT
//*****************************************************************************
//
// bl_uart.c : Functions to transfer data via the UART port.
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
#include "bl_config.h"
extern long long int APP_START_ADDRESS;
#if defined(UART_ENABLE_UPDATE)
#include "HL_sci.h"
#include "bl_uart.h"
#include "sci_common.h"
#include "bl_ymodem.h"
#include "HL_sys_common.h"
#include "printf.h"
#include "bl_launch.h"
#include "bl_eeprom.h"
#include "HL_reg_system.h"
#include "ti_fee.h"



//crc16 function declaration
unsigned short crc16 (char *ptr, int count);

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

    Size = Ymodem_Receive(UART, &tab_1024[0], address);
    if (Size > 0) {
        printf("\n\r The application image has been programmed successfully!\n\r---------------------------\n\r Name: ");
        printf(fileName);
        Int2Str(Number, Size);
        printf("\n\n\r Size:     ");
        printf(Number);
        printf("  Bytes\n\r");
        printf("---------------------------\n\n\n\n\r");
    } else {
        printf("\n\rFailed to receive the file!\n\r");
    }
    return Size;
}

//*****************************************************************************
//
//! This function performs the update on the selected port.
//!
//! This function is called directly by the boot loader or it is called as a
//! result of an update request from the application.
//!
//! \return Never returns.
//
//*****************************************************************************
void UpdaterUART(void) {
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
        printf("=======================================================\r\n\n");

        key = UART_getKey(UART);

        if (key == '0') {
            // Set Application Start Address
            printf("Enter The Application Start Address \n\r\n\r");
            sciReceive(UART, 10, &receive_command[0]);
            printf("\n\r\n\r The Entered Address Is: ");
            int c;
            for (c = 0; c < 8; c++) {
                printf("%c",receive_command[c]);
            }
            printf("\r\n");

           uint32_t address_num;
           if (true) {
               address_num = 0x00200020;
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
            app_info = eeprom_get_app_info();
            int size;

            if (app_info.addr != 0) {
                size = UART_Download(app_info.addr);
                if (size) {
                    app_info.size = size;
                    app_info.exists = 1;
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
           // Upload CRC of application image
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

        else {
           printf("Invalid Number !! \n\r\n\r");
        }
    }
}

#endif
#endif

