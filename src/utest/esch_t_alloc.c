/* vim:ft=c expandtab tw=72 sw=4
 */
#include <esch.h>
#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"

int test_AllocCreateDeleteCDefault()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_log* do_nothing = NULL;
    esch_config config = { ESCH_TYPE_CONFIG, NULL, NULL };
    char* str = NULL;

    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    config.log = log;
    ret = esch_alloc_new_c_default(&config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_alloc_malloc(alloc, sizeof(char) * 100, (void**)(&str));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to allocate memory", ret);

    /* Try raise an error when memory is leaked*/
    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret != ESCH_OK, "Expect memory leak detected but not happening", ret);

    (void)esch_log_info(g_testLog, "Memory leak detect. Now do right thing to free buffer.");

    ret = esch_alloc_free(alloc, str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to free memory", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

Exit:
    return ret;
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

