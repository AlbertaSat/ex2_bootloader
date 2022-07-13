/*
 * config.h
 *
 *  Created on: Jul. 2, 2022
 *      Author: Robert
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define SYSTEM_APP_ID _OBC_APP_ID_

#define IS_ATHENA 0
#define IS_ATHENA_V2 0

#define IS_SATELLITE 0
#if IS_SATELLITE == 1
#define IS_FLATSAT 0
#define IS_EXALTA2 1
#define IS_YUKONSAT 0
#define IS_AURORASAT 0
#endif

#define ATHENA_IS_STUBBED 1
#define UHF_IS_STUBBED 1
#define ADCS_IS_STUBBED 1
#define SBAND_IS_STUBBED 1
#define EPS_IS_STUBBED 1
#define HYPERION_IS_STUBBED 1
#define CHARON_IS_STUBBED 1
#define DFGM_IS_STUBBED 1
#define WATCHDOG_IS_STUBBED 1
#define PAYLOAD_IS_STUBBED 1

#define HYPERION_PANEL_3U 0
#define HYPERION_PANEL_2U 0
#define HYPERION_PANEL_2U_LIMITED 0

#define IS_SN0072_EPS 0

#define CSP_FREERTOS 1
#define CSP_USE_SDR 1
#define CSP_USE_KISS 0

/* Define SDR_NO_CSP==0 to use CSP for SDR */
#define SDR_NO_CSP 0
#define OS_FREERTOS

#define FLATSAT_TEST 0

#endif /* MAIN_CONFIG_H_ */
