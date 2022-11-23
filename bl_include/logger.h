/*
 * logger.h
 *
 *  Created on: Oct. 27, 2022
 *      Author: Robert
 */

#ifndef BL_INCLUDE_LOGGER_H_
#define BL_INCLUDE_LOGGER_H_

typedef enum {
    PANIC,    // Emergency - system is going down
    ALERT,    // Should be corrected immediately
    CRITICAL, // Primary system failure
    ERROR,    // Non-urgent failures
    WARN,     // Not a failure, but error could follow
    NOTICE,   // Unusual but not error condition
    INFO,     // Normal operational message
    DEBUG,
} SysLog_Level;

void sys_log(SysLog_Level level, const char *format, ...);

#define ex2_log(_args_...) sys_log(INFO, _args_)

#endif /* BL_INCLUDE_LOGGER_H_ */
