/*
 * bl_launch.c
 *
 *  Created on: May 17, 2021
 *      Author: Robert Taylor
 */

#include "bl_launch.h"
#include "bl_eeprom.h"
#include <stdbool.h>
#include <system.h>

bool start_application(void) {
    if (verify_application() == true) {
        image_info app_info = {0};
        eeprom_get_app_info(&app_info);
        uint32_t JumpAddress = (uint32_t)app_info.addr;
        ((void (*)(void))JumpAddress)();
    }
    return false;
}

void start_golden() {
    if (verify_golden() == true) {
        image_info app_info = {0};
        eeprom_get_golden_info(&app_info);
        uint32_t JumpAddress = (uint32_t)app_info.addr;
        ((void (*)(void))JumpAddress)();
    }
}
