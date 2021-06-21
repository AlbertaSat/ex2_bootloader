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
}
