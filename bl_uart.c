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


//crc16 function declaration
unsigned short crc16 (char *ptr, int count);

uint32_t JumpAddress;
void get_software_Version(void);
void get_hardware_Info(void);

extern uint32_t g_pulUpdateSuccess[8];
extern uint32_t g_ulUpdateStatusAddr;
extern uint32_t g_ulUpdateBufferSize;    //32 bytes or 8 32-bit words
extern char fileName[FILENAME_LEN];
char tab_1024[1024] = {    0  };

uint8_t receive_command[8]={0};  //format maybe #0x12345678$
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
void UART_Download()
{
  char Number[10] = "          ";
  int Size = 0;

  UART_putString(UART, "\n\r Waiting for the file to be sent ... ");
  UART_putString(UART, "\n\r Use Transfer->Send File and Ymodem Protocol from HyperTerminal \n\r");

  Size = Ymodem_Receive(UART, &tab_1024[0]);
  if (Size > 0)
  {
    UART_putString(UART, "\n\r The application image has been programmed successfully!\n\r---------------------------\n\r Name: ");
    UART_putString(UART, fileName);
    Int2Str(Number, Size);
    UART_putString(UART, "\n\n\r Size:     ");
    UART_putString(UART, Number);
    UART_putString(UART, "  Bytes\n\r");
    UART_putString(UART, "---------------------------\n\n\n\n\r");
  }
  else
  {
    UART_putString(UART, "\n\rFailed to receive the file!\n\r");
  }
}

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
void UART_Upload(void)
{
	  uint32_t status = 0;
	  uint32_t *updateInfo;
	  uint32_t imageSize;
	  uint32_t imageAddress;

	  updateInfo = (uint32_t *)g_ulUpdateStatusAddr;

	  imageAddress = updateInfo[1];
	  imageSize = updateInfo[2];


	  UART_putString(UART, "\n\n\rSelect Receive File and Ymodel protocol in the drop-down menu... (press any key to abort)\n\r");

	  if (UART_getKey(UART) == CRC )
	  {
			/* Transmit the flash image through ymodem protocol */
			status = Ymodem_Transmit(UART, (uint8_t*)imageAddress, (char*)"UploadedApplicationImage.bin", imageSize);

			if (status != 0)
			{
				UART_putString(UART, "\n\rError occured while transmitting\n\r");
			}
			else
			{
				UART_putString(UART, "\n\rApplication image has been transmitted successfully \n\n\n\r");
			}
	  }
	  else
	  {
		  UART_putString(UART, "\r\n\nAborted by user.\n\r");
	  }

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
void UpdaterUART(void)
{
	  char key = 0;

	  while (1)
	  {
//		    LITE_TOPRIGHT_LED;
			UART_putString(UART, "\r================== Main Menu ==========================\r\n");
            UART_putString(UART, "  0. Receive The Application Start Address\r\n");
			UART_putString(UART, "  1. Write the binary image to the Internal Flash of MCU \r\n");
			UART_putString(UART, "  2. Download the binary image from the internal flash \r\n");
			UART_putString(UART, "  3. Execute the first written image( upper address in memory) \r\n");
			UART_putString(UART, "  4. Get Bootloader Version \r\n");
			UART_putString(UART, "  5. Get Device Information \r\n");
            UART_putString(UART, "  6. Execute the second written image (lower address in memory) \r\n");
			UART_putString(UART, "  7. Calculate the CRC of the working image \r\n");
		    UART_putString(UART, "  8. Calculate the CRC of the golden image \r\n");
			UART_putString(UART,   "=======================================================\r\n\n");

			key = UART_getKey(UART);


			if (key == 0x30)
			            {

			               UART_putString(UART, "Enter The Application Start Address \n\r\n\r");
			               sciReceive(UART, 8, &receive_command[0]);
			               UART_putString(UART, "\n\r\n\r The Entered Address Is: \n\r\n\r");
			               UART_putString(UART,receive_command);
			               UART_putString(UART, "\n\r\n\r The  Address received \n\r\n\r");
			               //unsigned char ascii_to_num[8]={0};
			                 int i;
			                    for ( i=0;i<sizeof(receive_command);i++)
			                    {
			                        if (receive_command[i]>= 48 && receive_command[i]<=57)
			                        {
			                            ascii_to_num[i]= receive_command[i]-'0';
			                        }
			                        else if (receive_command[i]>= 97 && receive_command[i]<=102)
			                        {
			                            ascii_to_num[i]= receive_command[i]-87;
			                        }
			                            else if (receive_command[i]>= 65 && receive_command[i]<=70)
			                        {
			                            ascii_to_num[i]= receive_command[i]-55;
			                        }

			                    }



			               /*
			               UART_putString(UART, "\n\r\n\r");
			               UART_putString(UART, " \n\r\n\r bit 7\n\r\n\r");
			               UART_putString(UART,&receive_command[7]);
			               UART_putString(UART, "\n\r\n\rbit 6\n\r\n\r");
			               UART_putString(UART,&receive_command[6]);
			               UART_putString(UART, "\n\r\n\rbit 5\n\r\n\r");
			               UART_putString(UART,&receive_command[5]);
			               UART_putString(UART, "\n\r\n\rbit 4\n\r\n\r");
			               UART_putString(UART,&receive_command[4]);
			               UART_putString(UART, "\n\r\n\rbit 3\n\r\n\r");
			               UART_putString(UART,&receive_command[3]);
			               UART_putString(UART, "\n\r\n\rbit 2\n\r\n\r");
			               UART_putString(UART,&receive_command[2]);
			               UART_putString(UART, "\n\r\n\rbit 1\n\r\n\r");
			               UART_putString(UART,&receive_command[1]);
			               UART_putString(UART, "\n\r\n\rbit 0\n\r\n\r");
			               UART_putString(UART,&receive_command[0]);
			               */


			               uint32_t address_num =   (uint32_t)ascii_to_num[0] << 28 |
			                                        (uint32_t)ascii_to_num[1] << 24 |
			                                        (uint32_t)ascii_to_num[2] << 20 |
			                                        (uint32_t)ascii_to_num[3] << 16 |
			                                        (uint32_t)ascii_to_num[4] << 12 |
			                                        (uint32_t)ascii_to_num[5] << 8  |
			                                        (uint32_t)ascii_to_num[6] << 4  |
			                                        (uint32_t)ascii_to_num[7];

			                //uint32_t  *pstart_address;
			                //pstart_address =&APP_START_ADDRESS;

			                //*pstart_address=address_num;
			                APP_START_ADDRESS=address_num;
			                //UART_putString(UART, " \n\r\n\r The Address is updated. \n\r\n\r");
			                //UART_putString(UART,APP_START_ADDRESS);
			                printf("%d",APP_START_ADDRESS);

			                if (image_num <=1 )
			                {
			                image_num_arr[image_num] = address_num;
			                image_num= image_num+1;
			                }



			                 /*
			               UART_putString(UART, " \n\r\n\r The Address is Received. \n\r\n\r");

			                cint32_2_char[7]= ( char) ((APP_START_ADDRESS & 0x0000000F)    );
			                cint32_2_char[6]= ( char) ((APP_START_ADDRESS & 0x000000F0)>>4 );
			                cint32_2_char[5]= ( char) ((APP_START_ADDRESS & 0x00000F00)>>8 );
			                cint32_2_char[4]= ( char) ((APP_START_ADDRESS & 0x0000F000)>>12);
			                cint32_2_char[3]= ( char) ((APP_START_ADDRESS & 0x000F0000)>>16);
			                cint32_2_char[2]= ( char) ((APP_START_ADDRESS & 0x00F00000)>>20);
			                cint32_2_char[1]= ( char) ((APP_START_ADDRESS & 0x0F000000)>>24);
			                cint32_2_char[0]= ( char) ((APP_START_ADDRESS & 0xF0000000)>>28);
			                */

			                /*
			                cint32_2_char[7]= 9;
			                cint32_2_char[6]= 8;
			                cint32_2_char[5]= 7;
			                cint32_2_char[4]= 6;
			                cint32_2_char[3]= 5;
			                cint32_2_char[2]= 4;
			                cint32_2_char[1]= 3;
			                cint32_2_char[0]= 2;
			                */

			                /*
			                UART_putString(UART, " \n\r\n\r The Address is converted to unsigned char. \n");
			                //cint32_2_char[7]=7;
			                UART_putString(UART,cint32_2_char);
			                //UART_putString(UART,receive_command);
			                UART_putString(UART, " \n\r\n\r last line \n\r\n\r");
			                */

			            }


			else if (key == 0x31)
			{



        	    write_count=write_count+1;
			   /* Download user application in the Flash */
			    if ( (write_count>=3)  &&  (APP_START_ADDRESS==image_num_arr[1]) &&  ((*(uint32_t *) APP_STATUS_ADDRESS)==0x5A5A5A5A)  )
			     {
			     UART_putString(UART, "The golden image is already written, overwrite is prevented !! \n\r\n\r");
			     break;
			     }
			    else
			    {
			        UART_Download();
			    }



			}
		    else if (key == 0x32)
		    {
		      /* Upload user application from the Flash */
		      UART_Upload();
		    }
			else if (key == 0x33)
			{
			    crc_out= crc16( (char*)image_num_arr[0] , size_WI);

			    if (crc_out== crc_WI)
			    {
				UART_putString(UART, "working image CRC passed, it is running now !! \n\r\n\r");
		        JumpAddress = (uint32_t)image_num_arr[0];
		        ((void (*)(void))JumpAddress)();
			    }
			    else
			    {
			    UART_putString(UART, "working image CRC failed, the golden image is running instead !! \n\r\n\r");
			    JumpAddress = (uint32_t)image_num_arr[1];
			    ((void (*)(void))JumpAddress)();

			    }
			}
			else if (key == 0x34)
			{
				get_software_Version();
			}
			else if (key == 0x35)
			{
				get_hardware_Info();
			}
            else if (key == 0x36)
            {
                // single blinky LED address= 0x00200020, size= 16372 bytes
                 crc_out= crc16( (char*)image_num_arr[1], size_GI);
                // single blinky LED 0x00200020, 16372
                 if (crc_out= crc_GI)
                 {
                 UART_putString(UART, "CRC passed, golden image is running !! \n\r\n\r");
                 JumpAddress = (uint32_t)image_num_arr[1];
                 ((void (*)(void))JumpAddress)();
                 }
                 else{

                     UART_putString(UART, "golden image CRC failed, the working image is running instead !! \n\r\n\r");
                     JumpAddress = (uint32_t)image_num_arr[0];
                     ((void (*)(void))JumpAddress)();
                 }
            }

            else if (key == 0x37)
                   {
                       // single blinky LED 0x00200020, 16372
                   crc_out= crc16( (char*) image_num_arr[0], size_WI);
                   printf("CRC of working image  :0x%X\n",crc_out);
                   UART_putString(UART, "CRC of working image is calculated, please check the CSS console \n\r\n\r");
                   }

            else if (key == 0x38)
                   {
                       // single blinky LED 0x00200020, 16372
                   crc_out= crc16( (char*) image_num_arr[1], size_GI);

                   printf("CRC of golden image  :0x%X\n",crc_out);
                   UART_putString(UART, "CRC of golden image is calculated, please check the CSS console \n\r\n\r");
                   }
			else
			{
				UART_putString(UART, "Invalid Number !! \n\r\n\r");
			}
	  }
}


unsigned short crc16( char *ptr, int count)
{
   int  crc;
   char i;
   crc = 0;
   while (--count >= 0)
   {
      crc = crc ^  ( ((int)*ptr)  << 8  ) ;
      ptr=ptr+1;
      i = 8;
      do
      {
         if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
         else
            crc = crc << 1;
      } while(--i);
   }
   return (crc);
}


#endif


