/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_DEBUG_H_
#define _ESCH_DEBUG_H_

#include "esch_log.h"
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ESCH_CHECK_NO_LOG(cond, errorcode) { \
    if (!(cond)) { \
        ret = errorcode; \
        goto Exit; \
    } \
}

#define ESCH_CHECK(cond, obj, msg, errorcode) { \
    if (!(cond)) { \
        (void)esch_log_error(ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(obj)), \
            "[%s:%d] %s, ret = %d", __FILE__, __LINE__, msg, errorcode); \
        ret = errorcode; \
        goto Exit; \
    } \
}

#define ESCH_CHECK_1(cond, obj, fmt, val1, errorcode) { \
    if (!(cond)) { \
        (void)esch_log_error(ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(obj)), \
                "[%s:%d] " fmt ", ret = %d", \
                __FILE__, __LINE__, val1, errorcode); \
        ret = errorcode; \
        goto Exit; \
    } \
}

#define ESCH_CHECK_2(cond, obj, fmt, val1, val2, errorcode) { \
    if (!(cond)) { \
        (void)esch_log_error(ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(obj)), \
                "[%s:%d] " fmt ", ret = %d", \
                __FILE__, __LINE__, val1, val2, errorcode); \
        ret = errorcode; \
        goto Exit; \
    } \
}

#define ESCH_CHECK_3(cond, obj, fmt, val1, val2, val3, errorcode) { \
    if (!(cond)) { \
        (void)esch_log_error(ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(obj)), \
                "[%s:%d] " fmt ", ret = %d", \
                __FILE__, __LINE__, val1, val2, val3, errorcode); \
        ret = errorcode; \
        goto Exit; \
    } \
}

#ifdef NDEBUG
#    define ESCH_CHECK_PARAM_PUBLIC(cond) \
        ESCH_CHECK_NO_LOG(cond, ESCH_ERROR_INVALID_PARAMETER)
#else
#    define ESCH_CHECK_PARAM_PUBLIC(cond) \
        ESCH_CHECK(cond, esch_global_log, "Bad parameter: "#cond, ESCH_ERROR_INVALID_PARAMETER) 
#endif /* NDEBUG */

#define ESCH_CHECK_PARAM_INTERNAL(cond) assert(cond);
#define ESCH_ASSERT(cond) assert(cond);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_DEBUG_H_ */

