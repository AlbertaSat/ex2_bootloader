/*
 * bl_launch.c
 *
 *  Created on: May 17, 2021
 *      Author: Robert Taylor
 */

#include "bl_launch.h"
#include "bl_config.h"
#include <stdbool.h>
#include "bl_eeprom.h"

unsigned short crc16();

bool start_application(void) {
    if (verify_application() == true) {
        image_info app_info = eeprom_get_app_info();
        uint32_t JumpAddress = (uint32_t)app_info.addr;
        eeprom_shutdown();
        ((void (*)(void))JumpAddress)();
    }
    return false;
}

void start_golden() {
    if (verify_golden() == true) {
        image_info app_info = eeprom_get_golden_info();
        uint32_t JumpAddress = (uint32_t)app_info.addr;
        eeprom_shutdown();
        ((void (*)(void))JumpAddress)();
    }
    return false;
}

bool verify_application() {
    image_info app_info = {0};
    app_info = eeprom_get_app_info();
    if (app_info.exists) {
        if (crc16((char *)app_info.addr, app_info.size) == app_info.crc) {
            return true;
        } else return false;
    } else return false;
}

bool verify_golden() {
    image_info app_info = eeprom_get_golden_info();
    if (app_info.exists) {
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
