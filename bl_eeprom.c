/*
 * bl_eeprom.c
 *
 *  Created on: May 17, 2021
 *      Author: Robert Taylor
 */

#include "bl_eeprom.h"
#include "ti_fee.h"
#include "FreeRTOSConfig.h"
#include "privileged_functions.h"

unsigned short crc16();

void sw_reset(SW_RESET_REASON reason) {

    boot_info info;
    eeprom_init();
    info = eeprom_get_boot_info();
    info.reason.swr_reason =  reason;
    eeprom_set_boot_info(info);
    eeprom_shutdown();
    systemREG1->SYSECR = (0x10) << 14;

}

// Returns false on failure
bool eeprom_init() {
    int delayCount = 0;
    TI_Fee_Init();
    while (TI_Fee_GetStatus(0) != IDLE) {
        delayCount++;
        if (delayCount > 10000) { // timeout after trying this many times
            return false;
        }
    }
    return true;
}

void eeprom_shutdown() {
    TI_Fee_Shutdown();
}

char eeprom_get_boot_type() {
    char bootType;
    TI_Fee_ReadSync(BOOT_TYPE_BLOCK, BOOT_TYPE_OFFSET, (uint8_t *)(&bootType), BOOT_TYPE_LEN);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);
    TI_Fee_ErrorCodeType error = TI_FeeErrorCode(0);
    return bootType;
}

void eeprom_set_boot_type(char boot) {
    TI_Fee_WriteSync(BOOT_TYPE_BLOCK, (uint8_t *)&boot);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);

}

void eeprom_set_app_info(image_info i) {

    TI_Fee_WriteSync(APP_STATUS_BLOCKNUMBER, (uint8_t *)(&i));
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);

}

image_info eeprom_get_app_info() {
    image_info out = {0};
    TI_Fee_ReadSync(APP_STATUS_BLOCKNUMBER, APP_STATUS_OFFSET, (uint8_t *)(&out), APP_STATUS_LEN);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);
    TI_Fee_ErrorCodeType error = TI_FeeErrorCode(0);
    return out;
}

void eeprom_set_golden_info(image_info i) {

    TI_Fee_WriteSync(GOLD_STATUS_BLOCKNUMBER, (uint8_t *)&i);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);

}

image_info eeprom_get_golden_info() {

    image_info out = {0};
    TI_Fee_ReadSync(GOLD_STATUS_BLOCKNUMBER, GOLD_STATUS_OFFSET, (uint8_t *)(&out), GOLD_STATUS_LEN);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);
    TI_Fee_ErrorCodeType error = TI_FeeErrorCode(0);

    return out;
}

boot_info eeprom_get_boot_info() {

    boot_info out = {0};
    TI_Fee_ReadSync(BOOT_INFO_BLOCKNUMBER, BOOT_INFO_OFFSET, (uint8_t *)(&out), BOOT_INFO_LEN);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);
    TI_Fee_ErrorCodeType error = TI_FeeErrorCode(0);

    return out;
}

void eeprom_set_boot_info(boot_info b) {

    TI_Fee_WriteSync(BOOT_INFO_BLOCKNUMBER, (uint8_t *)&b);
    TI_FeeJobResultType res = TI_Fee_GetJobResult(0);

}

bool verify_application() {
    image_info app_info = {0};
    app_info = priv_eeprom_get_app_info();
    if (app_info.exists == EXISTS_FLAG) {
        if (crc16((char *)app_info.addr, app_info.size) == app_info.crc) {
            return true;
        } else return false;
    } else return false;
}

bool verify_golden() {
    image_info app_info = eeprom_get_golden_info();
    if (app_info.exists == EXISTS_FLAG) {
        if (crc16((char *)app_info.addr, app_info.size) == app_info.crc) {
            return true;
        } else return false;
    } else return false;
}

unsigned short crc16( char *ptr, int count)
{
   uint16_t crc;
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



