/*----------------------------------------------------------------------------*/
/* sys_link_freeRTOS.cmd                                                      */
/*                                                                            */
/* 
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com  
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/*                                                                            */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN (0) */
/* USER CODE END */
/*----------------------------------------------------------------------------*/
/* Linker Settings                                                            */

--retain="*(.intvecs)"

/* USER CODE BEGIN (1) */
/* USER CODE END */

/*----------------------------------------------------------------------------*/
/* Memory Map                                                                 */

MEMORY
{
    VECTORS (X)      : origin=0x00000000 length=0x00000040
    BOOTFLASH (RX)   : origin=0x00000040 length=0x00020000 - 0x40
    FLASH0  (RX)     : origin=end(BOOTFLASH) length=0x00200000 - 0x00020000 - 0x20
    FLASH1  (RX)     : origin=0x00200000 length=0x00200000
    RAMINTVECS (RWX) : origin=0x08000000 length=0x00000020
    KRAM  (RW)     : origin=end(RAMINTVECS) length=0x00000800
    RAM     (RW)     : origin=end(KRAM) length=0x0007eb00 - 0x08
    
/* USER CODE BEGIN (2) */
/* USER CODE END */
}

/* USER CODE BEGIN (3) */
/* USER CODE END */

/*----------------------------------------------------------------------------*/
/* Section Configuration                                                      */

SECTIONS
{
    .intvecs : {} > VECTORS
    /* FreeRTOS Kernel in protected region of Flash */
    .kernelTEXT  align(32) : {} > BOOTFLASH
    .cinit       align(32) : {} > BOOTFLASH
    .pinit       align(32) : {} > BOOTFLASH
    /* Rest of code to user mode flash region */
    .text        align(32) : {} > BOOTFLASH
    .const       align(32) : {} > BOOTFLASH
    /* FreeRTOS Kernel data in protected region of RAM */
    .kernelBSS    : {} > KRAM
    .kernelHEAP   : {} > RAM
    .bss          : {} > RAM
    .data         : {} > RAM    
    .sysmem       : {} > RAM
    FEE_TEXT_SECTION align(32) : {} > BOOTFLASH
    FEE_CONST_SECTION align(32): {} > BOOTFLASH
    FEE_DATA_SECTION : {} > RAM

/* USER CODE BEGIN (4) */
	flashAPI :
	    {
	        Fapi_UserDefinedFunctions.obj (.text)
	        bl_flash.obj (.text)
	        --library = ../lib/F021_API_CortexR4_BE_L2FMC_V3D16.lib (.text)
	    } load=BOOTFLASH, run=RAM, LOAD_START(FlashApi_LoadStart), SIZE(FlashApi_LoadSize), RUN_START(FlashApi_RunStart)
	.ramIntvecs : {} load=BOOTFLASH, run=RAMINTVECS, palign=8, LOAD_START(ramint_LoadStart), SIZE(ramint_LoadSize), RUN_START(ramint_RunStart)
/* USER CODE END */
}

/* USER CODE BEGIN (5) */
/* USER CODE END */

/*----------------------------------------------------------------------------*/
/* Misc                                                                       */

/* USER CODE BEGIN (6) */
/* USER CODE END */

/*----------------------------------------------------------------------------*/
