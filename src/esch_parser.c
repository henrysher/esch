/* vim:ft=c expandtab tw=72 sw=4
 */
#include "esch.h"
#include "esch_parser.h"
#include "esch_alloc.h"
#include "esch_log.h"
#include "esch_debug.h"

/**
 * Create a new esch_parser object.
 * @param alloc  Alloc object used by parser object.
 * @param log    Log object used by parser object.
 * @param parser Returned parameter of newly created parser object.
 * @return Error code.
 */
esch_error
esch_parser_new(esch_alloc* alloc, esch_log* log, esch_parser** parser)
{
    esch_error ret = ESCH_OK;
    esch_parser* new_parser = NULL;
    ESCH_CHECK_PARAM_PUBLIC(parser != NULL);
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);

    ret = esch_alloc_malloc(alloc, sizeof(esch_parser), (void**)&new_parser);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log, "Can't malloc for parser", ret);

    new_parser->base.type = ESCH_TYPE_PARSER;
    new_parser->base.log = log;
    new_parser->base.alloc = alloc;
    (*parser) = new_parser;
    new_parser = NULL;
    assert(ESCH_IS_VALID_OBJECT(*parser));
Exit:
    if (new_parser != NULL)
    {
        (void)esch_parser_delete(new_parser);
    }
    return ret;
}

/**
 * Delete esch_parser_config object.
 * @param parser Given parser object.
 * @return Error code.
 */
esch_error
esch_parser_delete(esch_parser* parser)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    ESCH_CHECK_PARAM_PUBLIC(parser != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(parser));

    alloc = parser->base.alloc;
    esch_alloc_free(alloc, parser);
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

