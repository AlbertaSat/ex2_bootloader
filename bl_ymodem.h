/**
 ******************************************************************************
 * ymodem.h for Hercules MCU SCI Bootloader
 * Copytight (c) 2012 QJ Wang <qjwang@ti.com>
 *
 *******************************************************************************
  */

#ifndef _YMODEM_H_
#define _YMODEM_H_


#include "HL_sci.h"
#include "HL_sys_common.h"
#include "bl_config.h"

#define PACKET_SEQNUM_LOC       1    /* The byte 1, the byte 2 is its complement*/

#define PACKET_HEADER           3    /* SOH 00 FF; or STX 01 FE, etc */
#define PACKET_CRC              2    /* 2 bytes CRC */
#define PACKET_SIZE             128  /* The packet in SOH, normally for 1st packet used for filename and file szie*/
#define PACKET_1K_SIZE          1024 /* For packet in STX */

#define FILENAME_LEN            96   /* The filename cannot include space */
#define FILESIZE_LEN            16   /* I use unsigned int which is 4 bytes long */

#define SOH                     0x01  /* start of 128-byte data packet */
#define STX                     0x02  /* start of 1024-byte data packet */
#define EOT                     0x04  /* end of transmission */
#define ACK                     0x06  /* acknowledge */
#define NAK                     0x15  /* negative acknowledge */
#define CAN                     0x18  /* two of these in succession aborts transfer */
#define CRC                     0x43  /* 'C' == 0x43, request 16-bit CRC */

/* 0x500000 takes 178258017 CPU cycles, 1.114 seconds for 160Mhz CPU*/
/* 0x480000 takes 160432221 CPU cycles, 1.003 seconds for 160Mhz CPU*/
#if defined (TMS570LS04) || defined (RM42)   //LS04 and RM42 runs 80MHz
#define WAIT_TIMEOUT            0x240000   //Timeout=1.003 seconds
#elif defined (RM57) || defined (TMS570LC43)
#define WAIT_TIMEOUT            0xB60000   //Timeout=1.003 seconds
#else
#define WAIT_TIMEOUT            0x480000   //Timeout=1.003 seconds
#endif

#define MAX_ALLOWED_ERRORS            10

char Ymodem_Transmit (sciBASE_t *sci, uint8_t *buf, char* fileName, uint32_t fileSize);
int  Ymodem_Receive (sciBASE_t *sci, char *buf);
int _cal_crc16_(unsigned char*, int);

#endif  /* _YMODEM_H_ */

