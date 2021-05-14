
#ifndef __FLASH_DEFINES_H__
#define __FLASH_DEFINES_H__

#include "bl_config.h"

//The device name is defined in bl_config.h which is located in the 
//individual folder
//
//#define TMS570LS31
//#define RM48
//#define TMS570LS12
//#define RM46
//#define TMS570LS04
//#define RM42
//#define TMS570LS07
//#define RM44

//#define F021

typedef struct Sectors
{ void * start;
  unsigned int length;  // number of 32-bit words
  unsigned int bankNumber;
  unsigned int sectorNumber;
  unsigned int FlashBaseAddress;
} SECTORS;

typedef struct Banks
{ void * start;
  unsigned int length;  // number of 32-bit words
  unsigned int bankNumber;
  unsigned int FlashBaseAddress;
  unsigned int numOfSectors;
} BANKS;


#if defined (RM57) || defined (TMS570LC43)
#define NUMBEROFSECTORS 32
const SECTORS flash_sector[NUMBEROFSECTORS]=
{
  (void *)0x00000000, 0x04000, 0,  0, 0xfff87000,
  (void *)0x00004000, 0x04000, 0,  1, 0xfff87000,
  (void *)0x00008000, 0x04000, 0,  2, 0xfff87000,
  (void *)0x0000C000, 0x04000, 0,  3, 0xfff87000,
  (void *)0x00010000, 0x04000, 0,  4, 0xfff87000,
  (void *)0x00014000, 0x04000, 0,  5, 0xfff87000,
  (void *)0x00018000, 0x08000, 0,  6, 0xfff87000,
  (void *)0x00020000, 0x20000, 0,  7, 0xfff87000,
  (void *)0x00040000, 0x20000, 0,  8, 0xfff87000,
  (void *)0x00060000, 0x20000, 0,  9, 0xfff87000,
  (void *)0x00080000, 0x40000, 0, 10, 0xfff87000,
  (void *)0x000C0000, 0x40000, 0, 11, 0xfff87000,
  (void *)0x00100000, 0x40000, 0, 12, 0xfff87000,
  (void *)0x00140000, 0x40000, 0, 13, 0xfff87000,
  (void *)0x00180000, 0x40000, 0, 14, 0xfff87000,
  (void *)0x001C0000, 0x40000, 0, 15, 0xfff87000,
  (void *)0x00200000, 0x20000, 1,  0, 0xfff87000,
  (void *)0x00220000, 0x20000, 1,  1, 0xfff87000,
  (void *)0x00240000, 0x20000, 1,  2, 0xfff87000,
  (void *)0x00260000, 0x20000, 1,  3, 0xfff87000,
  (void *)0x00280000, 0x20000, 1,  4, 0xfff87000,
  (void *)0x002A0000, 0x20000, 1,  5, 0xfff87000,
  (void *)0x002C0000, 0x20000, 1,  6, 0xfff87000,
  (void *)0x002E0000, 0x20000, 1,  7, 0xfff87000,
  (void *)0x00300000, 0x20000, 1,  8, 0xfff87000,
  (void *)0x00320000, 0x20000, 1,  9, 0xfff87000,
  (void *)0x00340000, 0x20000, 1, 10, 0xfff87000,
  (void *)0x00360000, 0x20000, 1, 11, 0xfff87000,
  (void *)0x00380000, 0x20000, 1, 12, 0xfff87000,
  (void *)0x003A0000, 0x20000, 1, 13, 0xfff87000,
  (void *)0x003C0000, 0x20000, 1, 14, 0xfff87000,
  (void *)0x003E0000, 0x20000, 1, 15, 0xfff87000
};

#define NUMBEROFBANKS 2
const BANKS flash_bank[NUMBEROFBANKS]=
{
  (void *) 0x00000000, 0x200000, 0, 0xfff87000, 16,
  (void *) 0x00200000, 0x400000, 1, 0xfff87000, 16,
};

#endif   /* RM57 */

#if defined (TMS570LS31) || defined (RM48)
#define NUMBEROFSECTORS 27
const SECTORS flash_sector[NUMBEROFSECTORS]=
{
  (void *)0x00000000, 0x08000, 0,  0, 0xfff87000,
  (void *)0x00008000, 0x08000, 0,  1, 0xfff87000,
  (void *)0x00010000, 0x08000, 0,  2, 0xfff87000,
  (void *)0x00018000, 0x08000, 0,  3, 0xfff87000,
  (void *)0x00020000, 0x20000, 0,  4, 0xfff87000,
  (void *)0x00040000, 0x20000, 0,  5, 0xfff87000,
  (void *)0x00060000, 0x20000, 0,  6, 0xfff87000,
  (void *)0x00080000, 0x20000, 0,  7, 0xfff87000,
  (void *)0x000A0000, 0x20000, 0,  8, 0xfff87000,
  (void *)0x000C0000, 0x20000, 0,  9, 0xfff87000,
  (void *)0x000E0000, 0x20000, 0, 10, 0xfff87000,
  (void *)0x00100000, 0x20000, 0, 11, 0xfff87000,
  (void *)0x00120000, 0x20000, 0, 12, 0xfff87000,
  (void *)0x00140000, 0x20000, 0, 13, 0xfff87000,
  (void *)0x00160000, 0x20000, 0, 14, 0xfff87000,
  (void *)0x00180000, 0x20000, 1,  0, 0xfff87000,
  (void *)0x001A0000, 0x20000, 1,  1, 0xfff87000,
  (void *)0x001C0000, 0x20000, 1,  2, 0xfff87000,
  (void *)0x001E0000, 0x20000, 1,  3, 0xfff87000,
  (void *)0x00200000, 0x20000, 1,  4, 0xfff87000,
  (void *)0x00220000, 0x20000, 1,  5, 0xfff87000,
  (void *)0x00240000, 0x20000, 1,  6, 0xfff87000,
  (void *)0x00260000, 0x20000, 1,  7, 0xfff87000,
  (void *)0x00280000, 0x20000, 1,  8, 0xfff87000,
  (void *)0x002A0000, 0x20000, 1,  9, 0xfff87000,
  (void *)0x002C0000, 0x20000, 1, 10, 0xfff87000,
  (void *)0x002E0000, 0x20000, 1, 11, 0xfff87000

};

#define NUMBEROFBANKS 2
const BANKS flash_bank[NUMBEROFBANKS]=
{
  (void *) 0x00000000, 0x180000, 0, 0xfff87000, 15,
  (void *) 0x00180000, 0x180000, 1, 0xfff87000, 12,
};

#endif   /* TMS570LS31 and RM48 */


#if defined (TMS570LS12) || defined (RM46)
#define NUMBEROFSECTORS 16
extern const SECTORS flash_sector[NUMBEROFSECTORS]=
{
  (void *)0x00000000, 0x04000, 0,  0, 0xfff87000,
  (void *)0x00004000, 0x04000, 0,  1, 0xfff87000,
  (void *)0x00008000, 0x04000, 0,  2, 0xfff87000,
  (void *)0x0000C000, 0x04000, 0,  3, 0xfff87000,
  (void *)0x00010000, 0x04000, 0,  4, 0xfff87000,
  (void *)0x00014000, 0x04000, 0,  5, 0xfff87000,
  (void *)0x00018000, 0x08000, 0,  6, 0xfff87000,
  (void *)0x00020000, 0x20000, 0,  7, 0xfff87000,
  (void *)0x00040000, 0x20000, 0,  8, 0xfff87000,
  (void *)0x00060000, 0x20000, 0,  9, 0xfff87000,
  (void *)0x00080000, 0x20000, 0, 10, 0xfff87000,
  (void *)0x000A0000, 0x20000, 0, 11, 0xfff87000,
  (void *)0x000C0000, 0x20000, 0, 12, 0xfff87000,
  (void *)0x000E0000, 0x20000, 0, 13, 0xfff87000,
  (void *)0x00100000, 0x20000, 0, 14, 0xfff87000,
  (void *)0x00120000, 0x20000, 0, 15, 0xfff87000
};

#define NUMBEROFBANKS 1
extern const BANKS flash_bank[NUMBEROFBANKS]=
{
  (void *) 0x00000000, 0x140000, 0, 0xfff87000, 16,
};

#endif   /* TMS570LS12 and RM46 */


#if defined (TMS570LS07) || defined (RM44)
#define NUMBEROFSECTORS 12
const SECTORS flash_sector[NUMBEROFSECTORS]=
{
  (void *)0x00000000, 0x04000, 0,  0, 0xfff87000,
  (void *)0x00004000, 0x04000, 0,  1, 0xfff87000,
  (void *)0x00008000, 0x04000, 0,  2, 0xfff87000,
  (void *)0x0000C000, 0x04000, 0,  3, 0xfff87000,
  (void *)0x00010000, 0x04000, 0,  4, 0xfff87000,
  (void *)0x00014000, 0x04000, 0,  5, 0xfff87000,
  (void *)0x00018000, 0x08000, 0,  6, 0xfff87000,
  (void *)0x00020000, 0x20000, 0,  7, 0xfff87000,
  (void *)0x00040000, 0x20000, 0,  8, 0xfff87000,
  (void *)0x00060000, 0x20000, 0,  9, 0xfff87000,
  (void *)0x00080000, 0x20000, 0, 10, 0xfff87000,
  (void *)0x000A0000, 0x20000, 0, 11, 0xfff87000
};

#define NUMBEROFBANKS 1
const BANKS flash_bank[NUMBEROFBANKS]=
{
  (void *) 0x00000000, 0xC0000, 0, 0xfff87000, 12,
};

#endif   /* TMS570LS07 and RM44 */


#if defined (TMS570LS04) || defined (RM42)
#define NUMBEROFSECTORS 15
const SECTORS flash_sector[NUMBEROFSECTORS]=
{
  (void *)0x00000000, 0x02000, 0,  0, 0xfff87000,
  (void *)0x00002000, 0x02000, 0,  1, 0xfff87000,
  (void *)0x00004000, 0x02000, 0,  2, 0xfff87000,
  (void *)0x00006000, 0x02000, 0,  3, 0xfff87000,
  (void *)0x00008000, 0x02000, 0,  4, 0xfff87000,
  (void *)0x0000A000, 0x02000, 0,  5, 0xfff87000,
  (void *)0x0000C000, 0x02000, 0,  6, 0xfff87000,
  (void *)0x0000E000, 0x02000, 0,  7, 0xfff87000,
  (void *)0x00010000, 0x02000, 0,  8, 0xfff87000,
  (void *)0x00012000, 0x02000, 0,  9, 0xfff87000,
  (void *)0x00014000, 0x02000, 0, 10, 0xfff87000,
  (void *)0x00016000, 0x02000, 0, 11, 0xfff87000,
  (void *)0x00018000, 0x08000, 0, 12, 0xfff87000,
  (void *)0x00020000, 0x20000, 0, 13, 0xfff87000,
  (void *)0x00040000, 0x20000, 0, 14, 0xfff87000
};

#define NUMBEROFBANKS 1
const BANKS flash_bank[NUMBEROFBANKS]=
{
  (void *) 0x00000000, 0x60000, 0, 0xfff87000, 15,
};

#endif   /* TMS570LS04 and RM42 */


#endif /* FLASH_DEFINES_H_ */
