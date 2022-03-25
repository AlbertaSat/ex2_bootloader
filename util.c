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
 * util.c
 *
 *  Created on: Feb. 15, 2022
 *      Author: Robert Taylor
 */

/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */

#include <stdint.h>

uint32_t hex2int(const char *hex) {
    uint32_t val = 0;
    if (*hex == 0 && (*(hex+1) == 'x' || *(hex+1) == 'X')) {
        hex += 2;
    }
    while (*hex && *hex != ' ') {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}


