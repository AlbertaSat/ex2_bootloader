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
 * updater.h
 *
 *  Created on: Jan. 24, 2022
 *      Author: Robert Taylor
 */

#ifndef UPDATER_H_
#define UPDATER_H_

#include "services.h"

#define UPDATE_BLOCK_SIZE 512

SAT_returnState start_updater_service(void);

typedef enum {
    INITIALIZE_UPDATE,
    PROGRAM_BLOCK,
    GET_PROGRESS,
    ERASE_APP,
    VERIFY_APPLICATION_IMAGE,
    VERIFY_GOLDEN_IMAGE
} updater_subtype; // shared with EPS!

typedef enum {
    UPDATE_NOFAIL = 0,
    UPDATE_GENERICFAILURE = 1,
    UPDATE_INVALIDADDR = 2,
    UPDATE_NOINIT = 3,
    UPDATE_ERASEFAILED = 4,
    UPDATE_WRITEFAILED = 5,
    UPDATE_OUTOFORDER = 6,
    UPDATE_CRCMISMATCH = 7,
    UPDATE_VERIFYFAILED = 8,
    UPDATE_NOSUBSERVICE = 9
} update_failuretype;

#endif /* UPDATER_H_ */
