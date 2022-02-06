/*
 * privileged_functions.c
 *
 *  Created on: Jun. 14, 2021
 *      Author: Robert Taylor
 */

#include "application_defined_privileged_functions.h"

void reboot_system(char reboot_type) {
    reboot_system_(reboot_type);
}
