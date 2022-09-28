#include <FreeRTOS.h>
#include <os_task.h>
#include <system.h>
#include "services.h"
#include "HL_system.h"
#include "HL_rti.h"
#include "HL_gio.h"
#include "bl_eeprom.h"
#include "bl_launch.h"
#include "HL_sci.h"
#include "HL_can.h"
#include "HL_sys_common.h"
#include "printf.h"
#include "bl_launch.h"
#include "bl_eeprom.h"
#include "HL_reg_system.h"
#include "ti_fee.h"
#include "csp/csp.h"
#include <csp/drivers/usart.h>
#include <csp/interfaces/csp_if_can.h>
#include <csp/drivers/can.h>
#include "privileged_functions.h"
#include "crypto.h"
#include <csp/interfaces/csp_if_sdr.h>
#include "csp/crypto/csp_hmac.h"
#include "csp/crypto/csp_xtea.h"
#include "sw_wdt.h"
#define INIT_PRIO configMAX_PRIORITIES - 1
#define INIT_STACK_SIZE 1500

uint32_t JumpAddress;
void get_software_Version(void);
void get_hardware_Info(void);
void bl_main(resetSource_t rstsrc);

/**
 * Initialize CSP interfaces
 * @details
 *      start the localhost zmq server and add it to the default route
 * with no VIA address
 */
static inline bool init_csp_interface() {
    int error;

#if EPS_IS_STUBBED == 0
    csp_iface_t *can_iface = NULL;
    error = csp_can_open_and_add_interface("CAN", &can_iface);
    if (error != CSP_ERR_NONE) {
        return SATR_ERROR;
    }
#endif /* EPS_IS_STUBBED */

#if CSP_USE_KISS == 1
    csp_usart_conf_t conf = {.device = "UART",
                             .baudrate = 115200, /* supported on all platforms */
                             .databits = 8,
                             .stopbits = 2,
                             .paritysetting = 0,
                             .checkparity = 0};

    csp_iface_t *uart_iface = NULL;
    error = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME, &uart_iface);
    if (error != CSP_ERR_NONE) {
        return SATR_ERROR;
    }

    char *gs_if_name = CSP_IF_KISS_DEFAULT_NAME;
    int gs_if_addr = 16;

#endif /* defined(CSP_USE_KISS) */

#if CSP_USE_SDR == 1

#ifdef SDR_TEST
    char *gs_if_name = "LOOPBACK";
    int gs_if_addr = 23;
#else
    char *gs_if_name = "UHF";
    int gs_if_addr = 16;
#endif /* SDR_TEST */

    sdr_conf_t sdr_conf = {0};
    sdr_conf.use_fec = USE_RADIO_ERROR_CORRECTION;
    sdr_conf.uhf_conf.uhf_baudrate = SDR_UHF_9600_BAUD;
    sdr_conf.uhf_conf.uart_baudrate = 115200;
    error = csp_sdr_open_and_add_interface(&sdr_conf, gs_if_name, NULL);
    if (error != CSP_ERR_NONE) {
        return SATR_ERROR;
    }

#endif /* defined(CSP_USE_SDR) */

    char rtable[128] = {0};
    snprintf(rtable, 128, "%d %s", gs_if_addr, gs_if_name);

#if EPS_IS_STUBBED == 0
    snprintf(rtable, 128, "%s, %d CAN", rtable, EPS_ADDRESS);
#endif /* EPS_IS_STUBBED */

    csp_rtable_load(rtable);

    return SATR_OK;
}

/**
 * Initialize CSP network
 */
static void init_csp() {
    /* Init CSP with address and default settings */
    csp_conf_t csp_conf;
    csp_conf.address = CSP_ADDRESS;
    csp_conf.model = "Athena_Bl";
    csp_conf.hostname = CSP_HOSTNAME;
    csp_conf.revision = "2";
    csp_conf.conn_max = 20;
    csp_conf.conn_queue_length = 10;
    csp_conf.fifo_length = 25;
    csp_conf.port_max_bind = 254;
    csp_conf.rdp_max_window = 20;
    csp_conf.buffers = 10;
    csp_conf.buffer_data_size = 1024;
    csp_conf.conn_dfl_so = CSP_O_NONE;

    csp_init(&csp_conf);
    /* Set default route and start router & server */
    csp_route_start_task(1000, 2);
    init_csp_interface();
    char *hmac_key;
    int hmac_len;
    get_crypto_key(HMAC_KEY, &hmac_key, &hmac_len);
    csp_hmac_set_key(hmac_key, hmac_len);
    char *xtea_key;
    int xtea_len;
    get_crypto_key(ENCRYPT_KEY, &xtea_key, &xtea_len);
    csp_xtea_set_key(xtea_key, xtea_len);
    return;
    return;
}

void bl_init(void *pvParameters) {
    start_sw_watchdog();
    init_csp();
    start_service_server();
    vTaskDelete(0);
}

char get_boot_type(resetSource_t rstsrc, boot_info *b_inf) {

    char stored_boot_type = b_inf->type;

    switch (rstsrc) {
    case POWERON_RESET:
        return stored_boot_type;
    case DEBUG_RESET:
    case EXT_RESET:
        return 'B';
    case SW_RESET:
    case WATCHDOG_RESET:
        if (b_inf->attempts >= 5) {
            b_inf->attempts = 0;
            if (stored_boot_type == 'A') {
                return 'G';
            } else {
                return 'B';
            }
        } else {
            b_inf->attempts += 1;
            return stored_boot_type;
        }
    default:
        return 'B';
    }
}

void bl_main(resetSource_t rstsrc) {
    char bootType;
    boot_info b_inf = {0};
    eeprom_get_boot_info(&b_inf);
    bootType = get_boot_type(rstsrc, &b_inf);

    b_inf.count += 1;
    b_inf.reason.rstsrc = rstsrc;
    if (rstsrc != SW_RESET) {
        b_inf.reason.swr_reason = NONE;
    }
    eeprom_set_boot_info(&b_inf);

    switch (bootType) {
    case 'A':
        start_application(); // no break to automatically attempt start golden on failure
    /* no break */
    case 'G':
        start_golden();
        break;
    case 'B':
    default:
        break;
    }

    // if we make it here the golden image didn't work

    /* Initialize SCI Routines to receive Command and transmit data */
    sciInit();
    canInit();
    xTaskCreate(bl_init, "init", INIT_STACK_SIZE, NULL, INIT_PRIO | portPRIVILEGE_BIT, NULL);

    vTaskStartScheduler();
}

void vAssertCalled(unsigned long ulLine, const char *const pcFileName) {
    /* Called if an assertion passed to configASSERT() fails.  See
    http://www.freertos.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    (void)ulLine;
    (void)pcFileName;

    printf("ASSERT! Line %d, file %s\r\n", ulLine, pcFileName);
    RAISE_PRIVILEGE;
    systemREG1->SYSECR = (0x10) << 14;
    RESET_PRIVILEGE;
}

void initializeProfiler() {
    /* Enable PMU Cycle Counter for Profiling */
    RAISE_PRIVILEGE;
    _pmuInit_();
    _pmuEnableCountersGlobal_();
    _pmuResetCycleCounter_();
    _pmuStartCounters_(pmuCYCLE_COUNTER);
    RESET_PRIVILEGE;
}

// TODO: This might need to be put in application_defined_privileged_functions.h
uint32 getProfilerTimerCount() {
    RAISE_PRIVILEGE;
    uint32_t ret = _pmuGetCycleCount_() / GCLK_FREQ;
    RESET_PRIVILEGE;
    return ret;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    for (;;)
        sw_reset('B', DABORT);
}

void vApplicationMallocFailedHook(void) {
    for (;;)
        sw_reset('B', DABORT); // This function is disabled in FreeRTOS.h, but this is here anyway just in case
}

void vApplicationDaemonTaskStartupHook(void) {}

void ex2_log(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}
