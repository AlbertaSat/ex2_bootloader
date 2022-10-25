/*
 * leop_eeprom.h
 *
 *  Created on: Oct, 2021
 *      Author: Grace Yi
 */

#ifndef LEOP_EEPROM_H_
#define LEOP_EEPROM_H_

#include <stdbool.h>
#include <stdint.h>
#include "HL_system.h"
#include "bl_eeprom.h"

typedef struct __attribute__((packed)) {
    uint32_t exists_flag;
    bool status;
} leop_status_t;

bool eeprom_get_leop_status();
bool eeprom_set_leop_status();
void eeprom_reset_leop_status();
#endif /* LEOP_EEPROM_H_ */
