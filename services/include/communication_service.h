/*
 * Copyright (C) 2015  University of Alberta
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

#ifndef COMMUNICATION_SERVICE_H
#define COMMUNICATION_SERVICE_H

#include <FreeRTOS.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <stdio.h>

#include "services.h"

/*Valid values*/
#define MIN_FREQ = 2200
#define MAX_FREQ = 2290

typedef enum {
    UHF_SET_SCW = 20,
    UHF_SET_FREQ,
    UHF_SET_PIPE_TIMEOUT,
    UHF_SET_BEACON_T,
    UHF_SET_AUDIO_T,
    UHF_SET_PARAMS, // Merge last 4
    UHF_RESTORE_DEFAULT,
    UHF_LOW_PWR,
    UHF_SET_DEST,
    UHF_SET_SRC,
    UHF_SET_MORSE,
    UHF_SET_MIDI,
    UHF_SET_BEACON_MSG,
    UHF_SET_I2C,
    UHF_WRITE_FRAM,
    UHF_SECURE,
    UHF_GET_FULL_STAT, // 0-244,249,250
    UHF_GET_CALL_SIGN, // 245-246
    UHF_GET_MORSE,
    UHF_GET_MIDI,
    UHF_GET_BEACON_MSG,
    UHF_GET_FRAM,
    UHF_SET_ECHO,
    UHF_SET_BCN,
    UHF_SET_PIPE,
    UHF_GET_SECURE_KEY,
    UHF_GET_SWVER,
    UHF_GET_PLDSZ,
    UHF_SET_CRC16_ENABLE,
    UHF_SET_RF_MODE
} UHF_Subtype;

SAT_returnState communication_service_app(csp_packet_t *pkt);

SAT_returnState start_communication_service(void);

#endif /* COMMUNICATION_SERVICE_H_ */
