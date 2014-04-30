/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_LOG_H_
#define _ESCH_LOG_H_

#include "esch.h"
#include "esch_object.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef esch_error (*error_log_f)(esch_log*, const char*, va_list);

struct esch_log
{
    error_log_f log_error;
    error_log_f log_warn;
    error_log_f log_info;
};

struct esch_log_builtin_static
{
    esch_object   header;
    esch_log      log;
};
extern struct esch_log_builtin_static esch_log_do_nothing;

#define ESCH_IS_VALID_LOG(log) \
    ((log) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(log)) && \
     (log)->log_error != NULL && \
     (log)->log_info != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_LOG_H_ */

