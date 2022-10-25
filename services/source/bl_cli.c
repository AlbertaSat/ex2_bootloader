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
 * cli.c
 *
 *  Created on: Dec. 21, 2021
 *      Author: Robert Taylor
 */

#include "bl_cli.h"
#include "bl_eeprom.h"
#include "printf.h"
#include "services.h"
#include "system.h"
#include <FreeRTOS.h>
#include <FreeRTOS-Plus-CLI/FreeRTOS_CLI.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <os_task.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "leop_eeprom.h"

/*
 * Command Implementations
 *
 * These functions are the implementations of all the commands registered with
 * the FreeRTOS+CLI controller
 */

static BaseType_t prvHostNameCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    snprintf(pcWriteBuffer, xWriteBufferLen, "%s\r\n", CSP_HOSTNAME);
    return pdFALSE;
}

static BaseType_t prvImageTypeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    snprintf(pcWriteBuffer, xWriteBufferLen, "Bootloader\r\n");
    return pdFALSE;
}

static BaseType_t prvHelloCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    snprintf(pcWriteBuffer, xWriteBufferLen, "Hello\r\n");
    return pdFALSE;
}

static BaseType_t prvEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *parameter;
    BaseType_t parameter_len, xReturn;

    /* Note that the use of the static parameter means this function is not
     * reentrant. */
    static BaseType_t parameter_num = 0;

    if (parameter_num == 0) {
        /* parameter_number is 0, so this is the first time the function has been
        called since the command was entered. */

        /* Next time the function is called the first parameter will be echoed
        back. */
        parameter_num = 1;

        /* There is more data to be returned as no parameters have been echoed
        back yet, so set xReturn to pdPASS so the function will be called again. */
        xReturn = pdPASS;
    } else {
        /* lParameter is not 0, so holds the number of the parameter that should
        be returned.  Obtain the complete parameter string. */
        parameter = FreeRTOS_CLIGetParameter(
            /* The command string itself. */
            pcCommandString,
            /* Return the next parameter. */
            parameter_num,
            /* Store the parameter string length. */
            &parameter_len);

        if (parameter != NULL) {
            /* There was another parameter to return.  Copy it into pcWriteBuffer. */
            snprintf(pcWriteBuffer, parameter_len + 2, "%s ", parameter);

            /* There might be more parameters to return after this one, so again
            set xReturn to pdTRUE. */
            xReturn = pdTRUE;
            parameter_num++;
        } else {
            /* No more parameters were found.  Make sure the write buffer does
            not contain a valid string to prevent junk being printed out. */
            /* now add CRLF to the last line, for formating*/
            snprintf(pcWriteBuffer, 3, "\r\n");

            /* There is no more data to return, so this time set xReturn to
            pdFALSE. */
            xReturn = pdFALSE;

            /* Start over the next time this command is executed. */
            parameter_num = 0;
        }
    }

    return xReturn;
}

static BaseType_t prvBootInfoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    boot_info inf = {0};
    eeprom_get_boot_info(&inf);
    char *reset_source_str;
    switch (inf.reason.rstsrc) {
    case POWERON_RESET:
        reset_source_str = "POWERON_RESET";
        break;
    case OSC_FAILURE_RESET:
        reset_source_str = "OSC_FAILURE_RESET";
        break;
    case WATCHDOG_RESET:
        reset_source_str = "WATCHDOG_RESET";
        break;
    case WATCHDOG2_RESET:
        reset_source_str = "WATCHDOG2_RESET";
        break;
    case DEBUG_RESET:
        reset_source_str = "DEBUG_RESET";
        break;
    case INTERCONNECT_RESET:
        reset_source_str = "INTERCONNECT_RESET";
        break;
    case CPU0_RESET:
        reset_source_str = "CPU0_RESET";
        break;
    case SW_RESET:
        reset_source_str = "SW_RESET";
        break;
    case EXT_RESET:
        reset_source_str = "EXT_RESET";
        break;
    case NO_RESET:
        reset_source_str = "NO_RESET";
        break;
    default:
        reset_source_str = "UNDEFINED";
        break;
    }

    char *reason_str;
    switch (inf.reason.swr_reason) {
    case NONE:
        reason_str = "NONE";
        break;
    case UNDEF:
        reason_str = "UNDEF";
        break;
    case DABORT:
        reason_str = "DABORT";
        break;
    case PREFETCH:
        reason_str = "PREFETCH";
        break;
    case REQUESTED:
        reason_str = "REQUESTED";
        break;
    default:
        reason_str = "UNDEFINED";
        break;
    }
    snprintf(pcWriteBuffer, xWriteBufferLen, "Count: %d, Attempts: %d, Reset: %s, Reason: %s\n", inf.count,
             inf.attempts, reset_source_str, reason_str);
    return pdFALSE;
}

static BaseType_t prvLeopCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    BaseType_t parameter_len;
    const char *parameter = FreeRTOS_CLIGetParameter(
        /* The command string itself. */
        pcCommandString,
        /* Return the first parameter. */
        1,
        /* Store the parameter string length. */
        &parameter_len);
    if (strncmp(parameter, "set", parameter_len) == 0) {
        eeprom_set_leop_status();
    } else if (strncmp(parameter, "reset", parameter_len) == 0) {
        eeprom_reset_leop_status();
    } else if (strncmp(parameter, "get", parameter_len) == 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "%d", eeprom_get_leop_status());
    }
    return pdFALSE;
}

static BaseType_t prvSetCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    int parameter_len;
    const char *parameter = FreeRTOS_CLIGetParameter(
        /* The command string itself. */
        pcCommandString,
        /* Return the next parameter. */
        1,
        /* Store the parameter string length. */
        &parameter_len);
    if (parameter_len > 1) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid boot Type\n");
    } else {
        switch (*parameter) {
        case 'A':
            if (verify_application() == false) {
                snprintf(pcWriteBuffer, xWriteBufferLen, "Application invalid\n");
                return pdFALSE;
            }
            break;
        case 'G':
            if (verify_golden() == false) {
                snprintf(pcWriteBuffer, xWriteBufferLen, "Golden Image invalid\n");
                return pdFALSE;
            }
            break;
        case 'B':
            break;
        default:
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid boot Type\n");
            return pdFALSE;
        }
        snprintf(pcWriteBuffer, xWriteBufferLen, "Setting boot type to %c\n", *parameter);
        boot_info info = {0};

        eeprom_get_boot_info(&info);
        info.type = *parameter;
        eeprom_set_boot_info(&info);
    }
    return pdFALSE;
}

// Small task that reboots the system after 3 second delay
void vRebootHandler(void *pvParameters) {
    vTaskDelay(3000);
    char reboot_type = (char)pvParameters;
    sw_reset(reboot_type, REQUESTED);
    vTaskDelete(0); // Delete self just in case the reset fails
}

static BaseType_t prvRebootCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    int parameter_len;
    const char *parameter = FreeRTOS_CLIGetParameter(
        /* The command string itself. */
        pcCommandString,
        /* Return the next parameter. */
        1,
        /* Store the parameter string length. */
        &parameter_len);
    if (parameter_len > 1) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid Reboot Type\n");
    } else {
        switch (*parameter) {
        case 'A':
            if (verify_application() == false) {
                snprintf(pcWriteBuffer, xWriteBufferLen, "Application invalid\n");
                return pdFALSE;
            }
            break;
        case 'G':
            if (verify_golden() == false) {
                snprintf(pcWriteBuffer, xWriteBufferLen, "Golden Image invalid\n");
                return pdFALSE;
            }
            break;
        case 'B':
            break;
        default:
            snprintf(pcWriteBuffer, xWriteBufferLen, "Invalid Reboot Type\n");
            return pdFALSE;
        }
        snprintf(pcWriteBuffer, xWriteBufferLen, "Rebooting in 3 seconds\n");
        xTaskCreate(vRebootHandler, "rebooter", 128, *parameter, 4, NULL);
    }
    return pdFALSE;
}

static BaseType_t prvUptimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    BaseType_t uptime = (xTaskGetTickCount() / 1000);
    snprintf(pcWriteBuffer, xWriteBufferLen, "%d Seconds\n", uptime);
    return pdFALSE;
}

static BaseType_t prvHexdumpCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // TODO: implement this

    return pdFALSE;
}

/*
 * Command Struct Definitions
 *
 * These structs store the definitions for each command implemented in the CLI
 *  to be registered to the FreeRTOS+CLI controller
 */

static const CLI_Command_Definition_t xEchoCommand = {"echo", "echo:\n\tEchoes all parameters back\n",
                                                      prvEchoCommand, -1};
static const CLI_Command_Definition_t xHelloCommand = {"hello", "hello:\n\tSays hello :)\n", prvHelloCommand, 0};
static const CLI_Command_Definition_t xBootInfoCommand = {
    "bootinfo", "bootinfo:\n\tGives a breakdown of the boot info\n", prvBootInfoCommand, 0};
static const CLI_Command_Definition_t xRebootCommand = {
    "reboot", "reboot:\n\tReboot to a mode. Can be B, G, or A\n", prvRebootCommand, 1};
static const CLI_Command_Definition_t xUptimeCommand = {"uptime", "uptime:\n\tGet uptime in seconds\n",
                                                        prvUptimeCommand, 0};
static const CLI_Command_Definition_t xhexdumpCommand = {
    "hexdump", "hexdump:\n\tDump data(as hex). Param 1: Address, Param 2: size", prvHexdumpCommand, 2};
static const CLI_Command_Definition_t xImageTypeCommand = {"imagetype", "imagetype:\n\tGet type of image booted\n",
                                                           prvImageTypeCommand, 0};
static const CLI_Command_Definition_t xSetCommand = {
    "set", "set:\n\tSet the next boot without rebooting. Can be B, G, or A\n", prvSetCommand, 1};
static const CLI_Command_Definition_t xHostNameCommand = {"hostname", "hostname\n\tReturns hostname\n",
                                                          prvHostNameCommand, 0};
static const CLI_Command_Definition_t xLeopCommand = {"leop", "leop\n\tGet, set, or reset leop status\n",
                                                      prvLeopCommand, 1};

/**
 * @brief
 *      Handle incoming csp_packet_t
 * @details
 *      Takes a csp packet destined for the cli service,
 *              and returns its string return value.
 * @param csp_packet_t *packet
 *              Incoming CSP packet - we can be sure that this packet is
 *              valid and destined for this service.
 * @return SAT_returnState
 *      success report
 */
SAT_returnState cli_app(csp_packet_t *packet, csp_conn_t *conn) {
    uint8_t size = (uint8_t)packet->data[IN_DATA_BYTE];
    bool xMoreDataToFollow;
    char pcOutputString[MAX_OUTPUT_SIZE];
    char pcInputString[MAX_INPUT_SIZE] = {0};
    memcpy(&pcInputString, (char *)&packet->data[IN_DATA_BYTE + 1], size);

    // store the original packet so we can reuse it later
    csp_packet_t *original = csp_buffer_clone(packet);

    do {
        memset(pcOutputString, 0x00, MAX_OUTPUT_SIZE);
        /* Send the command string to the command interpreter.  Any
        output generated by the command interpreter will be placed in the
        pcOutputString buffer. */
        xMoreDataToFollow = FreeRTOS_CLIProcessCommand((char *)&pcInputString,  /* The command string.*/
                                                       (char *)&pcOutputString, /* The output buffer. */
                                                       MAX_OUTPUT_SIZE /* The size of the output buffer. */
        );
        memcpy(&packet->data[IN_DATA_BYTE + 1], &pcOutputString, MAX_OUTPUT_SIZE);
        memcpy(&packet->data[STATUS_BYTE], &xMoreDataToFollow, sizeof(int8_t));
        if (!csp_send(conn, packet, 50)) {
            while (xMoreDataToFollow != pdFALSE) { // Keep running the command so it can return itself to
                                                   // its initial state
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand((char *)&pcInputString,  /* The command string.*/
                                                               (char *)&pcOutputString, /* The output buffer. */
                                                               MAX_OUTPUT_SIZE /* The size of the output buffer. */
                );
            }
            csp_buffer_free(original);
            csp_buffer_free(packet);
            return SATR_ERROR;
        }
        if (xMoreDataToFollow == pdTRUE) {
            // csp_send frees the packet for some reason. get a new one
            packet = csp_buffer_clone(original);
        }

    } while (xMoreDataToFollow != pdFALSE);

    csp_buffer_free(original);
    return SATR_OK;
}

/**
 * @brief
 *      FreeRTOS cli server task
 * @details
 *      Accepts incoming cli service packets and executes the application
 * @param void* param
 * @return None
 */
void cli_service(void *param) {
    csp_socket_t *sock;
    sock = csp_socket(CSP_SO_HMACREQ); // require RDP connection
    csp_bind(sock, TC_CLI_SERVICE);
    csp_listen(sock, SERVICE_BACKLOG_LEN);
    for (;;) {
        csp_conn_t *conn;
        csp_packet_t *packet;
        if ((conn = csp_accept(sock, DELAY_WAIT_TIMEOUT)) == NULL) {
            /* timeout */
            continue;
        }
        while ((packet = csp_read(conn, 50)) != NULL) {
            SAT_returnState status = cli_app(packet, conn);
            if (status != SATR_OK) {
                csp_buffer_free(packet);
            }
        }
        csp_close(conn);
    }
}

void register_commands() {
    FreeRTOS_CLIRegisterCommand(&xEchoCommand);
    FreeRTOS_CLIRegisterCommand(&xHelloCommand);
    FreeRTOS_CLIRegisterCommand(&xBootInfoCommand);
    FreeRTOS_CLIRegisterCommand(&xRebootCommand);
    FreeRTOS_CLIRegisterCommand(&xUptimeCommand);
    FreeRTOS_CLIRegisterCommand(&xImageTypeCommand);
    FreeRTOS_CLIRegisterCommand(&xSetCommand);
    FreeRTOS_CLIRegisterCommand(&xHostNameCommand);
    FreeRTOS_CLIRegisterCommand(&xLeopCommand);
}

/**
 * @brief
 *      Start the cli server task
 * @details
 *      Starts the FreeRTOS task responsible for accepting incoming
 *      cli packets and registers cli commands
 * @param None
 * @return SAT_returnState
 *      success report
 */
SAT_returnState start_cli_service(void) {
    if (xTaskCreate((TaskFunction_t)cli_service, "cli_svc", CLI_SVC_SIZE, NULL, NORMAL_SERVICE_PRIO, NULL) !=
        pdPASS) {
        return SATR_ERROR;
    }
    register_commands();
    return SATR_OK;
}
