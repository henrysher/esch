/* vim:ft=c expandtab tw=72 sw=4
 */
#ifndef _ESCH_LOG_H_
#define _ESCH_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "esch.h"
#include <stdarg.h>

typedef esch_error (*esch_log_error_func)(esch_log* log, char* fmt, va_list args);

esch_error esch_log_error_printf(esch_log* log, char* fmt, va_list args);
esch_error esch_log_error_do_nothing(esch_log* log, char* fmt, va_list args);

struct esch_log
{
    esch_object base;
    esch_log_error_func log_error;
};

extern esch_log g_esch_log_printf;
extern esch_log g_esch_log_do_nothing;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_LOG_H_ */
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