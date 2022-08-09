/*
 * sband.h
 *
 *  Created on: Aug. 9, 2022
 *      Author: Robert taylor
 */

#ifndef INCLUDE_SBAND_H_
#define INCLUDE_SBAND_H_

#include <stdbool.h>
#include <stdint.h>

// These don't exist (and aren't needed) in the bootloader
int sband_get_rate(void);

bool sband_enter_conf_mode(void);
bool sband_enter_sync_mode(void);
bool sband_enter_data_mode(void);

void sband_sync(void);

int sband_transmit_ready(void);

bool sband_buffer_count(uint16_t *cnt);



#endif /* INCLUDE_SBAND_H_ */
