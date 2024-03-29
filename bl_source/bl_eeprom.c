/*
 * bl_eeprom.c
 *
 *  Created on: May 17, 2021
 *      Author: Robert Taylor
 */

#include "bl_eeprom.h"
#include "leop_eeprom.h"
#include "FreeRTOSConfig.h"
#include "bl_flash.h"
#include "flash_defines.h"
#include "privileged_functions.h"
#include "ti_fee.h"
#include "util.h"

unsigned short crc16();

// @param reboot_type: if 0, preserve boot type
void sw_reset(char reboot_type, SW_RESET_REASON reason) {

    boot_info info = {0};
    eeprom_get_boot_info(&info);
    info.reason.swr_reason = reason;
    if (reason == REQUESTED) {
        info.attempts = 0; // Reset counter because this is a request
    }
    if (reboot_type != 0) {
        info.type = reboot_type;
    }
    eeprom_set_boot_info(&info);
    raise_privilege();
    systemREG1->SYSECR = (0x10) << 14;
    reset_privilege();
}

const SECTORS *eeprom_get_sector_by_block(uint8_t block) {
    const SECTORS *sector = 0;
    for (int i = 0; i < NUMBEROFSECTORS; i++) {
        if (flash_sector[i].bankNumber != 7) {
            continue;
        }
        if (flash_sector[i].sectorNumber == block) {
            sector = &flash_sector[i];
            break;
        }
    }
    return sector;
}

static Fapi_StatusType eeprom_write(void *dat, uint8_t block, uint32_t size) {
    // find address of block
    const SECTORS *sector = eeprom_get_sector_by_block(block);
    if (sector == 0) {
        return Fapi_Error_InvalidAddress;
    }

    void *addr = sector->start;
    uint32_t sector_size = sector->length;

    if (size > sector_size) {
        return Fapi_Error_AsyncIncorrectDataBufferLength;
    }
    raise_privilege();
    uint32_t status = Fapi_BlockErase((uint32_t)addr, size);
    status = Fapi_BlockProgram(7, (uint32_t)addr, (uint32_t)dat, size);
    reset_privilege();
    return status;
}

static Fapi_StatusType eeprom_read(void *dat, uint8_t block, uint32_t size) {
    const SECTORS *sector = eeprom_get_sector_by_block(block);
    if (sector == 0) {
        return Fapi_Error_InvalidAddress;
    }
    void *addr = sector->start;
    uint32_t sector_size = sector->length;

    if (sector == 0) {
        return Fapi_Error_InvalidAddress;
    }

    if (size > sector_size) {
        return Fapi_Error_AsyncIncorrectDataBufferLength;
    }
    memcpy(dat, addr, size);
    return Fapi_Status_Success;
}

Fapi_StatusType eeprom_set_app_info(image_info *i) {
    Fapi_StatusType status = eeprom_write((void *)i, APP_STATUS_BLOCKNUMBER, sizeof(image_info));
    return status;
}

Fapi_StatusType eeprom_get_app_info(image_info *i) {
    Fapi_StatusType status = eeprom_read((void *)i, APP_STATUS_BLOCKNUMBER, sizeof(image_info));
    return status;
}

Fapi_StatusType eeprom_set_golden_info(image_info *i) {
    Fapi_StatusType status = eeprom_write((void *)i, GOLD_STATUS_BLOCKNUMBER, sizeof(image_info));
    return status;
}

Fapi_StatusType eeprom_get_golden_info(image_info *i) {
    Fapi_StatusType status = eeprom_read((void *)i, GOLD_STATUS_BLOCKNUMBER, sizeof(image_info));
    return status;
}

Fapi_StatusType eeprom_get_boot_info(boot_info *b) {
    Fapi_StatusType status = eeprom_read((void *)b, BOOT_INFO_BLOCKNUMBER, sizeof(boot_info));
    return status;
}

Fapi_StatusType eeprom_set_boot_info(boot_info *b) {
    Fapi_StatusType status = eeprom_write((void *)b, BOOT_INFO_BLOCKNUMBER, sizeof(boot_info));
    return status;
}

Fapi_StatusType eeprom_set_update_info(update_info *u) {
    Fapi_StatusType status = eeprom_write((void *)u, UPDATE_INFO_BLOCKNUMBER, UPDATE_INFO_LEN);
    return status;
}

Fapi_StatusType eeprom_get_update_info(update_info *u) {
    Fapi_StatusType status = eeprom_read((void *)u, UPDATE_INFO_BLOCKNUMBER, UPDATE_INFO_LEN);
    return status;
}

Fapi_StatusType eeprom_get_key_store(key_store *k) {
    Fapi_StatusType status = eeprom_read((void *)k, KEY_STORE_BLOCKNUMBER, KEY_STORE_LEN);
    return status;
}

Fapi_StatusType eeprom_set_key_store(key_store *k) {
    Fapi_StatusType status = eeprom_write((void *)k, KEY_STORE_BLOCKNUMBER, KEY_STORE_LEN);
    return status;
}

bool eeprom_get_leop_status() {
    leop_status_t state = {0};
    if (eeprom_read(&state, LEOP_INFO_BLOCKNUMBER, sizeof(state)) == Fapi_Status_Success) {
        if (state.exists_flag == EXISTS_FLAG) {
            return state.status;
        } else {
            memset(&state, 0, sizeof(state));
            state.exists_flag = EXISTS_FLAG;
            eeprom_write(&state, LEOP_INFO_BLOCKNUMBER, sizeof(state));
            return state.status;
        }
    } else {
        return false;
    }
}

bool eeprom_set_leop_status() {
    leop_status_t state = {0};
    state.exists_flag = EXISTS_FLAG;
    state.status = true;
    eeprom_write(&state, LEOP_INFO_BLOCKNUMBER, sizeof(state));
    return true;
}

/**
 * DANGEROUS FUNCTION
 * Used for flatsat testing, not to be used for normal code
 */
void eeprom_reset_leop_status() {
    leop_status_t state = {0};
    state.exists_flag = EXISTS_FLAG;
    state.status = false;
    eeprom_write(&state, LEOP_INFO_BLOCKNUMBER, sizeof(state));
    return true;
}

bool verify_application() {
    image_info app_info = {0};
    eeprom_get_app_info(&app_info);
    if (app_info.exists == EXISTS_FLAG) {
        if (crc16((char *)app_info.addr, app_info.size) == app_info.crc) {
            return true;
        } else
            return false;
    } else
        return false;
}

bool verify_golden() {
    image_info app_info = {0};
    eeprom_get_golden_info(&app_info);
    if (app_info.exists == EXISTS_FLAG) {
        if (crc16((char *)app_info.addr, app_info.size) == app_info.crc) {
            return true;
        } else
            return false;
    } else
        return false;
}
