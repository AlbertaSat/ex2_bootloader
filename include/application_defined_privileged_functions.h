/*
 * Copyright (C) 2021  University of Alberta
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * application_defined_privileged_functions.h
 *
 *  Created on: May 14, 2021
 *      Author: Robert Taylor
 */

#ifndef APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS_H_
#define APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS_H_

#include "FreeRTOS.h"
#include "HL_reg_system.h"
#include "bl_eeprom.h"
#include "bl_flash.h"
#include "ti_fee.h"

void reboot_system_(char reboot_type) {
    RAISE_PRIVILEGE;
    boot_info b_inf;
    eeprom_get_boot_info(&b_inf);
    b_inf.reason.swr_reason = REQUESTED;
    b_inf.type = reboot_type;
    eeprom_set_boot_info(&b_inf);
    systemREG1->SYSECR = (0x10) << 14;
    RESET_PRIVILEGE;
}

static BaseType_t xRunningPrivileged;

void raise_privilege_() {
    xRunningPrivileged = prvRaisePrivilege();
}

void reset_privilege_() {
    RESET_PRIVILEGE;
}
#endif /* INCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS_H_ */
