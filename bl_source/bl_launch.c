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

bool copy_golden_image() {
    image_info golden_info = {0};
    eeprom_get_golden_info(&golden_info);
    if (!verify_golden()) {
        return false;
    }
    image_info app_info = {0};
    memcpy(&app_info, &golden_info, sizeof(image_info));
    app_info.addr = APP_MINIMUM_ADDR; // This is an emergency, so this will do
    if (BLInternalFlashStartAddrCheck(app_info.addr, app_info.size) != 0) {
        return false;
    }

    eeprom_set_app_info(&app_info);

    if (Fapi_BlockErase(app_info.addr, app_info.size) != 0) {
        return false;
    }

    uint8_t bank = 1; // This emergency will always write to bank 1, since that's the minimum app addr
                      // Also, if it tries for some reason to write to bank 0 and an interrupt occurs,
                      // a prefetch abort will happen.
    if (Fapi_BlockProgram(bank, app_info.addr, golden_info.addr, app_info.size) != 0) {
        return false;
    }
    return true;
}

void start_golden() {
    if (verify_golden() == true) {
        if (copy_golden_image() == false) {
            return;
        }
        sw_reset("A", REQUESTED);
    }
}
