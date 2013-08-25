/* vim:ft=c expandtab tw=72 sw=4
 */
#ifndef _ESCH_DEBUG_H_
#define _ESCH_DEBUG_H_

#include "esch.h"
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
        esch_log* log = ESCH_OBJECT_LOG(obj); \
        (void)esch_log_error(log, "[%s:%d] " msg, __FILE__, __LINE__); \
        ret = errorcode; \
        goto Exit; \
    } \
}

#define ESCH_CHECK_1(cond, obj, fmt, val1, errorcode) { \
    if (!(cond)) { \
        esch_log* log = ESCH_OBJECT_LOG(obj); \
        (void)esch_log_error(log, "[%s:%d] " fmt, __FILE__, __LINE__, val1); \
        ret = errorcode; \
        goto Exit; \
    } \
}

#define ESCH_CHECK_2(cond, obj, fmt, val1, val2, errorcode) { \
    if (!(cond)) { \
        esch_log* log = ESCH_OBJECT_LOG(obj); \
        (void)esch_log_error(log, "[%s:%d] " fmt, __FILE__, __LINE__, val1, val2); \
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_DEBUG_H_ */
/*
 * +=================================================================+
 *
 * From: http://opensource.org/licenses/BSD-3-Clause 
 *
 * Copyright (c) 2013, Fuzhou Chen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Fuzhou Chen nor other names of the contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * +=================================================================+
 */
