/*
 * bl_eeprom.h
 *
 *  Created on: May 17, 2021
 *      Author: Robert Taylor
 */

#ifndef BL_EEPROM_H_
#define BL_EEPROM_H_

#include "bl_config.h"
#include <stdbool.h>

bool eeprom_init();

void eeprom_shutdown();

char eeprom_get_boot_type();

void eeprom_set_boot_type(char boot);

void eeprom_set_app_info(image_info i);

image_info eeprom_get_app_info();

void eeprom_set_golden_info(image_info i);

image_info eeprom_get_golden_info();

#endif /* BL_EEPROM_H_ */
