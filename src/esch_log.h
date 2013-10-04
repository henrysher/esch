/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_LOG_H_
#define _ESCH_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "esch.h"
#include <stdarg.h>

typedef esch_error (*error_log_func)(esch_log* log, char* fmt, va_list args);

esch_error esch_log_error_printf(esch_log* log, char* fmt, va_list args);
esch_error esch_log_info_printf(esch_log* log, char* fmt, va_list args);
esch_error esch_log_message_do_nothing(esch_log* log, char* fmt, va_list args);

struct esch_log
{
    esch_config base;
    error_log_func log_error;
    error_log_func log_info;
};

extern esch_log g_esch_log_printf;
extern esch_log g_esch_log_do_nothing;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_LOG_H_ */

