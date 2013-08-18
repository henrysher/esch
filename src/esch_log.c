/* vim:ft=c expandtab tw=72 sw=4
 */
#include <stdio.h>
#include <assert.h>
#include "esch.h"
#include "esch_alloc.h"
#include "esch_log.h"
#include "esch_debug.h"

static esch_alloc g_dummy_alloc = {
    { ESCH_TYPE_ALLOC, &g_esch_log_do_nothing, &g_dummy_alloc },
};
esch_log g_esch_log_printf = {
    { ESCH_TYPE_LOG_PRINTF, &g_esch_log_printf, &g_dummy_alloc },
    esch_log_error_printf
};
esch_log g_esch_log_do_nothing = {
    { ESCH_TYPE_LOG_DO_NOTHING, &g_esch_log_do_nothing, &g_dummy_alloc },
    esch_log_error_do_nothing
};

/**
 * Create a log that print string to use printf().
 * @param log Returned created log object.
 * @return Error code.
 */
esch_error
esch_log_new_printf(esch_log** log)
{
    esch_error ret = ESCH_OK;
    esch_log* new_obj = NULL;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);

    (*log) = &g_esch_log_printf;
    assert(ESCH_IS_VALID_OBJECT(*log));
Exit:
    return ret;
}

/**
 * Create a log that does nothing.
 * @param log Returned created log object.
 * @return Error code.
 */
esch_error
esch_log_new_do_nothing(esch_log** log)
{
    esch_error ret = ESCH_OK;
    esch_log* new_obj = NULL;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);

    (*log) = &g_esch_log_do_nothing;
    assert(ESCH_IS_VALID_OBJECT(*log));
Exit:
    return ret;
}

esch_error
esch_log_error(esch_log* log, char* fmt, ...)
{
    esch_error ret = ESCH_OK;
    va_list ap;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(fmt != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_OBJECT(log), ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(log == &g_esch_log_do_nothing ||
                      log == &g_esch_log_printf,
                     ESCH_ERROR_INVALID_PARAMETER);

    va_start(ap, fmt);
    ret = log->log_error(log, fmt, ap);
    va_end(ap);
Exit:
    return ret;
}

esch_error
esch_log_delete(esch_log* log)
{
    esch_error ret = ESCH_OK;
    if (log == NULL) {
        return ret;
    }
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_OBJECT(log), ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(log == &g_esch_log_do_nothing ||
                      log == &g_esch_log_printf,
                     ESCH_ERROR_INVALID_PARAMETER);
    /* Just do nothing */
Exit:
    return ret;
}

esch_error
esch_log_error_printf(esch_log* log, char* fmt, va_list args)
{
    (void)log;
    vprintf(fmt, args);
    printf("\n");
    return ESCH_OK;
}
esch_error
esch_log_error_do_nothing(esch_log* log, char* fmt, va_list args)
{
    (void)log;
    (void)fmt;
    (void)args;
    return ESCH_OK;
}

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
