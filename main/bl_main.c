#include <FreeRTOS.h>
#include <os_task.h>
#include <system.h>
#include "services.h"
#include "HL_system.h"
#include "HL_rti.h"
#include "HL_gio.h"
#include "ti_fee.h"
#include "bl_eeprom.h"
#include "bl_launch.h"
#include "HL_sci.h"
#include "HL_sys_common.h"
#include "printf.h"
#include "bl_launch.h"
#include "bl_eeprom.h"
#include "HL_reg_system.h"
#include "ti_fee.h"
#include "csp/csp.h"
#include <csp/drivers/usart.h>
#include <csp/interfaces/csp_if_can.h>
#include "privileged_functions.h"
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
    csp_iface_t *uart_iface = NULL;
    csp_iface_t *can_iface = NULL;
    csp_usart_conf_t conf = {.device = "UART",
                             .baudrate = 115200, /* supported on all platforms */
                             .databits = 8,
                             .stopbits = 2,
                             .paritysetting = 0,
                             .checkparity = 0};

    int error = csp_can_open_and_add_interface("CAN", &can_iface);
    if (error != CSP_ERR_NONE) {
        return false;
    }

    error = csp_usart_open_and_add_kiss_interface(&conf, CSP_IF_KISS_DEFAULT_NAME, &uart_iface);
    if (error != CSP_ERR_NONE) {
        return false;
    }

#ifndef EPS_IS_STUBBED
    csp_rtable_load("16 KISS, 4 CAN, 10 KISS");
#else
    csp_rtable_load("16 KISS, 10 KISS");
#endif

    return true;
}

void eeprom_spin(void *pvParameters) {
    for (;;) {
        do {
            eeprom_mainfunction();
            vTaskDelay(2);
        } while (TI_Fee_GetStatus(0) != IDLE);
    }
}

/**
 * Initialize CSP network
 */
static void init_csp() {
    TC_TM_app_id my_address = OBC_APP_ID;

    /* Init CSP with address and default settings */
    csp_conf_t csp_conf;
    csp_conf_get_defaults(&csp_conf);
    csp_conf.address = my_address;
    csp_init(&csp_conf);
    /* Set default route and start router & server */
    csp_route_start_task(1000, 2);
    init_csp_interface();
    return;
}

void bl_init(void *pvParameters) {
    printf("Hello world!\n");
    //xTaskCreate(eeprom_spin, "eprm_spin", 128, NULL, INIT_PRIO | portPRIVILEGE_BIT, NULL);
    init_csp();
    start_service_server();
    vTaskDelete(0);
}

char get_boot_type(int rstsrc, boot_info *b_inf) {

    char stored_boot_type = eeprom_get_boot_type();

    switch(rstsrc) {
    case POWERON_RESET:
        return stored_boot_type;
    case DEBUG_RESET:
    case EXT_RESET:
        return 'B';
    case SW_RESET:
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
    RAISE_PRIVILEGE;
    RESET_PRIVILEGE;
    char bootType;
    bool fee_init = eeprom_init();
    boot_info b_inf;

    if (!fee_init) {
        bootType = 'B'; // EEPROM didn't work, emergency mode
    } else {
        b_inf = eeprom_get_boot_info();
        bootType = get_boot_type(rstsrc, &b_inf);
    }

    b_inf.count += 1;
    b_inf.reason.rstsrc = rstsrc;
    if (rstsrc != SW_RESET) {
        b_inf.reason.swr_reason = NONE;
    }
    eeprom_set_boot_info(b_inf);

    switch(bootType) {
    case 'A': start_application(); // no break to automatically attempt start golden on failure
    /* no break */
    case 'G': start_golden(); break;
    case 'B':
    default: break;
    }

    image_info app;
    app.addr = 0x00200000;
    app.exists = EXISTS_FLAG;
    app.crc=0x3019;
    app.size=354560;
    priv_eeprom_set_app_info(app);

    // if we make it here the golden image didn't work

    /* Initialize SCI Routines to receive Command and transmit data */
	sciInit();
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
    for (;;)
        ;
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
        ;
}

void vApplicationMallocFailedHook(void) {
    for (;;)
        ;
}

void vApplicationDaemonTaskStartupHook(void) {}

void ex2_log(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

