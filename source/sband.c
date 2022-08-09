/*
 * sband.c
 *
 *  Created on: Aug. 9, 2022
 *      Author: Robert Taylor
 */
#include "sband.h"
#include "sdr_driver.h"

// These don't exist (and aren't needed) in the bootloader
int sband_get_rate(void) { return 0; }

bool sband_enter_conf_mode() { return true; }
bool sband_enter_sync_mode() { return true; }
bool sband_enter_data_mode() { return true; }

void sband_sync() {}

int sband_transmit_ready(void) { return 1; }

bool sband_buffer_count(uint16_t *cnt) { return true; }

int sdr_sband_tx(struct sdr_interface_data *ifdata, uint8_t *data, uint16_t len) {
    return 0;
}

int sdr_sband_driver_init(sdr_interface_data_t *ifdata) {
    return 0;
}
