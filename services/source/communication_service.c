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
/**
 * @file communication_service.c
 * @author Arash Yazdani
 * @date 2020-09-25
 */

#include "communication_service.h"

#include <FreeRTOS.h>
#include "os_task.h"
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <main/system.h>

#include "services.h"

#include "uhf.h"
#include "service_utilities.h"

#define CHAR_LEN 1 // If using Numpy unicode string, change to 4
#define CALLSIGN_LEN 6
#define MORSE_BEACON_MSG_LEN_MAX 36
#define BEACON_MSG_LEN_MAX 60
// Update this to 108 (MIDI) and 97 (Beacon msg) when packet configuration
// is changed.
#define FRAM_SIZE 16
#define SID_byte 1
#define SINGLE_NOTE_LEN 3 // For MIDI audio notes

SAT_returnState communication_service_app(csp_packet_t *packet);

/**
 * @brief
 *      FreeRTOS communication server task
 * @details
 *      Accepts incoming communication service packets and executes the
 * application
 * @param void* param
 * @return None
 */
void communication_service(void *param) {
    csp_socket_t *sock;
    sock = csp_socket(CSP_SO_HMACREQ);
    csp_bind(sock, TC_COMMUNICATION_SERVICE);
    csp_listen(sock, SERVICE_BACKLOG_LEN);

    for (;;) {
        csp_packet_t *packet;
        csp_conn_t *conn;
        if ((conn = csp_accept(sock, DELAY_WAIT_TIMEOUT)) == NULL) {

            /* timeout */
            continue;
        }

        while ((packet = csp_read(conn, 50)) != NULL) {
            if (communication_service_app(packet) != SATR_OK) {
                // something went wrong in the service
                csp_buffer_free(packet);
            } else {
                if (!csp_send(conn, packet, 50)) {
                    csp_buffer_free(packet);
                }
            }
        }
        csp_close(conn);
    }
}

/**
 * @brief
 *      Start the communication server task
 * @details
 *      Starts the FreeRTOS task responsible for accepting incoming
 *      communication service requests
 * @param None
 * @return SAT_returnState
 *      success report
 */
SAT_returnState start_communication_service(void) {

    if (xTaskCreate((TaskFunction_t)communication_service, "communication_service", COMM_SVC_SIZE, NULL,
                    NORMAL_SERVICE_PRIO, NULL) != pdPASS) {
        ex2_log("FAILED TO CREATE TASK start_communication_service\n");
        return SATR_ERROR;
    }
    ex2_log("Communication service started\n");
    return SATR_OK;
}

/**
 * @brief
 *      Takes a CSP packet and switches based on the subservice command
 * @details
 *      Reads/Writes data from communication EHs as subservices
 * @attention
 *      Some subservices return the aggregation of error status of multiple HALs
 * @param *packet
 *      The CSP packet
 * @return SAT_returnState
 *      Success or failure
 */

SAT_returnState communication_service_app(csp_packet_t *packet) {
    uint8_t ser_subtype = (uint8_t)packet->data[SUBSERVICE_BYTE];
    int8_t status;                          // Status of HAL functions success
    SAT_returnState return_state = SATR_OK; // Remains OK unless changed by default or if statements.
    uint8_t uhf_struct_len;
    int i;           // For indexing in multiple loops
    int SID;         // The identifier in the packet
    uint8_t confirm; // For confirming the change

    switch (ser_subtype) {
    // read scw and change the respective bit and set again
    case UHF_SET_ECHO:
    case UHF_SET_BCN:
    case UHF_SET_PIPE: {
        uint8_t scw[SCW_LEN];
        status = HAL_UHF_getSCW(scw);
        if (status == U_GOOD_CONFIG) {
            scw[ser_subtype - UHF_SET_ECHO + 4] = 1;
            status = HAL_UHF_setSCW(scw);
        }
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);
        break;
    }

    case UHF_SET_FREQ: {
        uint32_t freq;
        cnv8_32(&packet->data[IN_DATA_BYTE], &freq);
        freq = csp_ntoh32(freq);
        status = HAL_UHF_setFreq(freq);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_PIPE_TIMEOUT: {
        uint32_t pipe_t;
        cnv8_32(&packet->data[IN_DATA_BYTE], &pipe_t);
        pipe_t = csp_ntoh32(pipe_t);
        status = HAL_UHF_setPipeT(pipe_t);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_BEACON_T: {
        uint32_t beacon_t;
        cnv8_32(&packet->data[IN_DATA_BYTE], &beacon_t);
        beacon_t = csp_ntoh32(beacon_t);
        status = HAL_UHF_setBeaconT(beacon_t);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_AUDIO_T: {
        uint32_t audio_t;
        cnv8_32(&packet->data[IN_DATA_BYTE], &audio_t);
        audio_t = csp_ntoh32(audio_t);
        status = HAL_UHF_setAudioT(audio_t);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_PARAMS: {
        UHF_Settings set;
        cnv8_32(&packet->data[IN_DATA_BYTE], &set.freq);
        set.freq = csp_ntoh32(set.freq);
        cnv8_32(&packet->data[IN_DATA_BYTE + 4], &set.pipe_t);
        set.pipe_t = csp_ntoh32(set.pipe_t);
        cnv8_32(&packet->data[IN_DATA_BYTE + 8], &set.beacon_t);
        set.beacon_t = csp_ntoh32(set.beacon_t);
        cnv8_32(&packet->data[IN_DATA_BYTE + 12], &set.audio_t);
        set.audio_t = csp_ntoh32(set.audio_t);
        status = HAL_UHF_setFreq(set.freq) + HAL_UHF_setPipeT(set.pipe_t) + HAL_UHF_setBeaconT(set.beacon_t) +
                 HAL_UHF_setAudioT(set.audio_t);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_RESTORE_DEFAULT:
        confirm = (uint8_t)packet->data[IN_DATA_BYTE];
        status = HAL_UHF_restore(confirm);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;

    case UHF_LOW_PWR:
        confirm = (uint8_t)packet->data[IN_DATA_BYTE];
        status = HAL_UHF_lowPwr(confirm);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;

    case UHF_SET_DEST: {
        UHF_configStruct dest;
        uhf_struct_len = CALLSIGN_LEN;
        dest.len = uhf_struct_len;
        for (i = 0; i < uhf_struct_len && (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i] != 0;
             i++) {
            dest.message[i] = (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i];
        }
        status = HAL_UHF_setDestination(dest);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_SRC: {
        UHF_configStruct src;
        uhf_struct_len = CALLSIGN_LEN;
        src.len = uhf_struct_len;
        for (i = 0; i < uhf_struct_len && (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i] != 0;
             i++) {
            src.message[i] = (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i];
        }
        status = HAL_UHF_setSource(src);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_MORSE: {
        UHF_configStruct morse;
        uhf_struct_len = MORSE_BEACON_MSG_LEN_MAX;
        for (i = 0; i < uhf_struct_len && packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i] != 0; i++) {
            morse.message[i] = (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i];
            if (morse.message[i] == '|') {
                morse.message[i] = ' ';
            }
        }
        morse.len = i;
        status = HAL_UHF_setMorse(morse);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_MIDI: {
        UHF_configStruct MIDI;
        uhf_struct_len = BEACON_MSG_LEN_MAX;
        uint8_t M_char = (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1)];
        if (M_char != 'M') { // To get around the parser, force a letter in the start
            status = U_BAD_PARAM;
        } else { // Now parse it
            for (i = 0; i < uhf_struct_len &&
                        packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * (i + 1)] != 0; // +1 for M_char
                 i++) {
                MIDI.message[i] =
                    (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * (i + 1)]; // +1 for M_char
            }
            if (i % SINGLE_NOTE_LEN != 0) {
                status = U_BAD_PARAM;
            } else {
                MIDI.len = i / SINGLE_NOTE_LEN;
                status = HAL_UHF_setMIDI(MIDI);
            }
        }

        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_BEACON_MSG: {
        UHF_configStruct beacon;
        uhf_struct_len = BEACON_MSG_LEN_MAX;
        for (i = 0; i < uhf_struct_len && packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i] != 0; i++) {
            beacon.message[i] = (uint8_t)packet->data[IN_DATA_BYTE + (CHAR_LEN - 1) + CHAR_LEN * i];
        }
        beacon.len = i;
        status = HAL_UHF_setBeaconMsg(beacon);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_SET_I2C: {
        uint8_t I2C_address = (uint8_t)packet->data[IN_DATA_BYTE] + 12; // Hex to Dec (22 -> 0x22)
        I2C_address = csp_ntoh32((uint32_t)I2C_address);
        status = HAL_UHF_setI2C(I2C_address);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;
    }

    case UHF_WRITE_FRAM: {
        UHF_framStruct U_FRAM;
        cnv8_32(&packet->data[IN_DATA_BYTE], &U_FRAM.addr);
        for (i = 0; i < FRAM_SIZE; i++) {
            U_FRAM.data[i] =
                (uint8_t)packet->data[IN_DATA_BYTE + sizeof(U_FRAM.addr) + (CHAR_LEN - 1) + CHAR_LEN * i];
        }
        status = HAL_UHF_setFRAM(U_FRAM);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);
        break;
    }

    case UHF_SECURE:
        confirm = (uint8_t)packet->data[IN_DATA_BYTE];
        status = HAL_UHF_secure(confirm);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);

        break;

    case UHF_GET_FULL_STAT: {
        UHF_Status U_stat;
        status = HAL_UHF_getSCW(U_stat.scw) + HAL_UHF_getFreq(&U_stat.set.freq) +
                 HAL_UHF_getUptime(&U_stat.uptime) + HAL_UHF_getPcktsOut(&U_stat.pckts_out) +
                 HAL_UHF_getPcktsIn(&U_stat.pckts_in) + HAL_UHF_getPcktsInCRC16(&U_stat.pckts_in_crc16) +
                 HAL_UHF_getPipeT(&U_stat.set.pipe_t) + HAL_UHF_getBeaconT(&U_stat.set.beacon_t) +
                 HAL_UHF_getAudioT(&U_stat.set.audio_t) + HAL_UHF_getTemp(&U_stat.temperature);

        if (sizeof(U_stat) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        U_stat.set.freq = csp_hton32(U_stat.set.freq);
        U_stat.uptime = csp_hton32(U_stat.uptime);
        U_stat.pckts_out = csp_hton32(U_stat.pckts_out);
        U_stat.pckts_in = csp_hton32(U_stat.pckts_in);
        U_stat.pckts_in_crc16 = csp_hton32(U_stat.pckts_in_crc16);
        U_stat.set.pipe_t = csp_hton32(U_stat.set.pipe_t);
        U_stat.set.beacon_t = csp_hton32(U_stat.set.beacon_t);
        U_stat.set.audio_t = csp_hton32(U_stat.set.audio_t);
        U_stat.temperature = csp_htonflt(U_stat.temperature);

        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &U_stat, sizeof(U_stat));
        set_packet_length(packet, sizeof(int8_t) + sizeof(U_stat) + 1);
        break;
    }

    case UHF_GET_CALL_SIGN: {
        UHF_Call_Sign U_callsign;
        status = HAL_UHF_getDestination(&U_callsign.dest) + HAL_UHF_getSource(&U_callsign.src);
        uhf_struct_len = CALLSIGN_LEN;
        // dst, src are used for parsing in case of using unicode strings
        uint8_t dst[CALLSIGN_LEN * CHAR_LEN];
        memset(dst, 0, CALLSIGN_LEN * CHAR_LEN);
        uint8_t src[CALLSIGN_LEN * CHAR_LEN];
        memset(src, 0, CALLSIGN_LEN * CHAR_LEN);
        if (sizeof(dst) + sizeof(src) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        for (i = 0; i < uhf_struct_len; i++) {
            dst[(CHAR_LEN - 1) + CHAR_LEN * i] = U_callsign.dest.message[i];
            src[(CHAR_LEN - 1) + CHAR_LEN * i] = U_callsign.src.message[i];
        }

        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &dst, sizeof(dst));
        memcpy(&packet->data[OUT_DATA_BYTE + sizeof(dst)], &src, sizeof(src));
        set_packet_length(packet, sizeof(int8_t) + sizeof(dst) + sizeof(src) + 1);
        break;
    }

    case UHF_GET_MORSE: {
        UHF_configStruct morse;
        status = HAL_UHF_getMorse(&morse);
        uhf_struct_len = morse.len;
        // mrs used in case of using unicode strings
        uint8_t mrs[MORSE_BEACON_MSG_LEN_MAX * CHAR_LEN];
        memset(mrs, 0, MORSE_BEACON_MSG_LEN_MAX * CHAR_LEN);
        if (sizeof(mrs) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        for (i = 0; i < uhf_struct_len; i++) {
            mrs[(CHAR_LEN - 1) + CHAR_LEN * i] = morse.message[i];
        }

        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &mrs, sizeof(mrs));
        set_packet_length(packet, sizeof(int8_t) + sizeof(mrs) + 1);

        break;
    }

    case UHF_GET_MIDI: {
        UHF_configStruct MIDI_bcn;
        status = HAL_UHF_getMIDI(&MIDI_bcn);
        uhf_struct_len = MIDI_bcn.len * SINGLE_NOTE_LEN;
        uint8_t midi[BEACON_MSG_LEN_MAX * CHAR_LEN];
        memset(midi, 0, BEACON_MSG_LEN_MAX * CHAR_LEN);
        if (sizeof(midi) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        for (i = 0; i < uhf_struct_len; i++) {
            midi[(CHAR_LEN - 1) + CHAR_LEN * i] = MIDI_bcn.message[i];
        }

        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &midi, sizeof(midi));
        set_packet_length(packet, sizeof(int8_t) + sizeof(midi) + 1);

        break;
    }

    case UHF_GET_BEACON_MSG: {
        UHF_configStruct beacon_msg;
        status = HAL_UHF_getBeaconMsg(&beacon_msg);
        uhf_struct_len = beacon_msg.len;
        // Switch BEACON_MSG_LEN_MAX to MAX_W_CMDLEN after packet configuration
        uint8_t beacon[BEACON_MSG_LEN_MAX * CHAR_LEN];
        memset(beacon, 0, BEACON_MSG_LEN_MAX * CHAR_LEN);
        if (sizeof(beacon) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        for (i = 0; i < uhf_struct_len; i++) {
            beacon[(CHAR_LEN - 1) + CHAR_LEN * i] = beacon_msg.message[i];
        }

        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &beacon, sizeof(beacon));
        set_packet_length(packet, sizeof(int8_t) + sizeof(beacon) + 1);

        break;
    }

    case UHF_GET_FRAM: {
        UHF_framStruct U_FRAM;
        cnv8_32(&packet->data[IN_DATA_BYTE], &U_FRAM.addr);
        status = HAL_UHF_getFRAM(&U_FRAM);
        uint8_t fram[FRAM_SIZE * CHAR_LEN];
        memset(fram, 0, FRAM_SIZE * CHAR_LEN);
        if (sizeof(fram) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        for (i = 0; i < FRAM_SIZE; i++) {
            fram[(CHAR_LEN - 1) + CHAR_LEN * i] = U_FRAM.data[i];
        }
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &fram, sizeof(fram));
        set_packet_length(packet, sizeof(int8_t) + sizeof(fram) + 1);
        break;
    }

    case UHF_GET_SECURE_KEY: {
        uint32_t key;
        status = HAL_UHF_getSecureKey(&key);
        key = csp_hton32(key);
        if (sizeof(key) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &key, sizeof(key));
        set_packet_length(packet, sizeof(int8_t) + sizeof(key) + 1);

        break;
    }
    case UHF_GET_SWVER: {
        uint16_t version;
        status = HAL_UHF_getSWVersion(&version);
        if (sizeof(version) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &version, sizeof(version));
        set_packet_length(packet, sizeof(int8_t) + sizeof(version) + 1);

        break;
    }
    case UHF_GET_PLDSZ: {
        uint16_t payload_size;
        status = HAL_UHF_getPayload(&payload_size);
        if (sizeof(payload_size) + 1 > csp_buffer_data_size()) {
            return_state = SATR_ERROR;
        }
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        memcpy(&packet->data[OUT_DATA_BYTE], &payload_size, sizeof(payload_size));
        set_packet_length(packet, sizeof(int8_t) + sizeof(payload_size) + 1);

        break;
    }
    case UHF_SET_CRC16_ENABLE: {
        bool enabled = (uint8_t)packet->data[IN_DATA_BYTE];
        status = HAL_UHF_setCRC16Enable(enabled);
        memcpy(&packet->data[STATUS_BYTE], &status, sizeof(int8_t));
        set_packet_length(packet, sizeof(int8_t) + 1);
        break;
    }

    default:
        ex2_log("No such subservice\n");
        return_state = SATR_PKT_ILLEGAL_SUBSERVICE;
    }

    return return_state;
}
