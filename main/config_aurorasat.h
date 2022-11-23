/*
 * config.h
 *
 *  Created on: Jul. 2, 2022
 *      Author: Robert
 */

#ifndef CONFIG_AURORASAT_H_
#define CONFIG_AURORASAT_H_

#define IS_ATHENA 1
#define IS_ATHENA_V2 1

#define IS_SATELLITE 1
#if IS_SATELLITE == 1
#define IS_FLATSAT 0
#define IS_EXALTA2 1
#define IS_YUKONSAT 0
#define IS_AURORASAT 0
#endif

#define HAS_SD_CARD 1
#if HAS_SD_CARD == 1
#define SD_CARD_REFORMAT 0
#endif

#define ATHENA_IS_STUBBED 0
#define UHF_IS_STUBBED 0
#define ADCS_IS_STUBBED 1
#define SBAND_IS_STUBBED 1
#define EPS_IS_STUBBED 1
#define HYPERION_IS_STUBBED 1
#define CHARON_IS_STUBBED 1
#define DFGM_IS_STUBBED 1
#define WATCHDOG_IS_STUBBED 0
#define PAYLOAD_IS_STUBBED 1

#define IS_SN0072_EPS 0

#define USE_RADIO_ERROR_CORRECTION 1

/* Only defined for final flight build of software */
#define FLIGHT_CONFIGURATION 1

#define CSP_FREERTOS 1
#define CSP_USE_SDR 1
#define CSP_USE_KISS 0

#define CSP_ADDRESS 3
#define CSP_HOSTNAME "AuroraSat_BL"
#define EPS_ADDRESS 5

/* Define SDR_NO_CSP==0 to use CSP for SDR */
#define SDR_NO_CSP 0
#define OS_FREERTOS

#endif /* MAIN_CONFIG_AURORASAT_H_ */
