/*
 * logger.c
 *
 *  Created on: Oct. 27, 2022
 *      Author: robert
 */

#include "logger.h"
#include <stdarg.h>
#include "printf.h"

void sys_log(SysLog_Level level, const char *format, ...) {
    (void)level;
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}
