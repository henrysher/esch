/* vim:ft=c expandtab tw=72 sw=4
 */
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
#ifndef _ESCH_ESCH_H_
#define _ESCH_ESCH_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stddef.h>
/*
 * This is the only public include file for Esch interpreter. 
 */

typedef enum {
    ESCH_OK = 0,
    ESCH_ERROR_NOT_IMPLEMENTED,
    ESCH_ERROR_OUT_OF_MEMORY,
    ESCH_ERROR_INVALID_PARAMETER,
    ESCH_ERROR_INVALID_STATE,
} esch_error;

typedef enum {
    ESCH_TYPE_UNKNOWN          = 0,
    ESCH_TYPE_MAGIC            = 0xEC,
    /* Generic types */
    ESCH_TYPE_CUSTOMIZED       = ESCH_TYPE_MAGIC | 0x10,
    ESCH_TYPE_ALLOC            = ESCH_TYPE_MAGIC | 0x20,
    ESCH_TYPE_LOG              = ESCH_TYPE_MAGIC | 0x30,
    ESCH_TYPE_PARSER           = ESCH_TYPE_MAGIC | 0x40,
    ESCH_TYPE_AST              = ESCH_TYPE_MAGIC | 0x42,
    ESCH_TYPE_CONFIG           = ESCH_TYPE_MAGIC | 0x50,
    ESCH_TYPE_STRING           = ESCH_TYPE_MAGIC | 0x60,
    ESCH_TYPE_VM               = ESCH_TYPE_MAGIC | 0xf0,

    ESCH_TYPE_ALLOC_C_DEFAULT  = ESCH_TYPE_MAGIC | ESCH_TYPE_ALLOC | 1,
    ESCH_TYPE_LOG_PRINTF       = ESCH_TYPE_MAGIC | ESCH_TYPE_LOG   | 1,
    ESCH_TYPE_LOG_DO_NOTHING   = ESCH_TYPE_MAGIC | ESCH_TYPE_LOG   | 2,
} esch_type;

typedef struct esch_object         esch_object;
typedef struct esch_alloc          esch_alloc;
typedef struct esch_log            esch_log;
typedef struct esch_parser_config  esch_parser_config;
typedef struct esch_parser         esch_parser;
typedef struct esch_ast            esch_ast;
typedef struct esch_object         esch_config;
typedef struct esch_string         esch_string;

typedef char                       esch_utf8_char;
typedef wchar_t                    esch_unicode;

/**
 * The public type info structure. This is the header of all structures
 * used in Esch. All objects can be casted to esch_object to get type
 * information.
 */
struct esch_object
{
    esch_type   type;       /**< Registered type ID */
    esch_log*   log;        /**< Log object to write trace/errors.*/
    esch_alloc* alloc;      /**< Allocator object to manage memory. */
};

#define ESCH_IS_VALID_OBJECT(obj)    ((((esch_object*)obj)->type & ESCH_TYPE_MAGIC) == ESCH_TYPE_MAGIC && \
                                      ((esch_object*)obj)->log != NULL && \
                                      ((esch_object*)obj)->alloc != NULL)
#define ESCH_OBJECT(obj)             ((esch_object*)obj)
#define ESCH_OBJECT_TYPE(obj)    (((esch_object*)obj)->type)
#define ESCH_OBJECT_LOG(obj)     (((esch_object*)obj)->log)
#define ESCH_OBJECT_ALLOC(obj)   (((esch_object*)obj)->alloc)
#define ESCH_IS_VALID_CONFIG(obj)    ((((esch_object*)obj)->type == ESCH_TYPE_CONFIG) && \
                                      ((esch_object*)obj)->log != NULL && \
                                      ((esch_object*)obj)->alloc != NULL)

/* --- Memory allocator --- */
esch_error esch_alloc_new_c_default(esch_alloc** ret_alloc);
esch_error esch_alloc_delete(esch_alloc* alloc);
esch_error esch_alloc_malloc(esch_alloc* alloc, size_t size, void** ret);
esch_error esch_alloc_free(esch_alloc* alloc, void* ptr);

/* --- Logger objects -- */
extern esch_log* esch_global_log;
esch_error esch_log_new_do_nothing(esch_log** log);
esch_error esch_log_new_printf(esch_log** log);
esch_error esch_log_delete(esch_log* log);
esch_error esch_log_error(esch_log* log, char* fmt, ...);
esch_error esch_log_info(esch_log* log, char* fmt, ...);

/* --- String objects --- */
esch_error esch_string_new_from_utf8(esch_config* config, char* utf8,
                                     int begin, int end, esch_string** str);
esch_error esch_string_delete(esch_string* str);
esch_error esch_string_get_utf8(esch_string* str, char** utf8);
esch_error esch_string_get_unicode(esch_string* str, esch_unicode** unicode);
esch_error esch_unicode_is_lu(esch_unicode ch);
esch_error esch_unicode_is_ll(esch_unicode ch);
esch_error esch_unicode_is_lt(esch_unicode ch);
esch_error esch_unicode_is_lm(esch_unicode ch);
esch_error esch_unicode_is_lo(esch_unicode ch);
esch_error esch_unicode_is_mn(esch_unicode ch);
esch_error esch_unicode_is_mc(esch_unicode ch);
esch_error esch_unicode_is_nd(esch_unicode ch);
esch_error esch_unicode_is_nl(esch_unicode ch);
esch_error esch_unicode_is_no(esch_unicode ch);
esch_error esch_unicode_is_pd(esch_unicode ch);
esch_error esch_unicode_is_pc(esch_unicode ch);
esch_error esch_unicode_is_po(esch_unicode ch);
esch_error esch_unicode_is_sc(esch_unicode ch);
esch_error esch_unicode_is_sm(esch_unicode ch);
esch_error esch_unicode_is_sk(esch_unicode ch);
esch_error esch_unicode_is_so(esch_unicode ch);
esch_error esch_unicode_is_co(esch_unicode ch);
esch_error esch_unicode_is_ascii(esch_unicode ch);
esch_error esch_unicode_is_extended_alphabetic(esch_unicode ch);

/* --- Parser --- */
/* --- 
 * TODO Add more functions as part of configurations.
 * 1. Message callback function to keep error information.
 * 2. Standard library
 * 3. Search path
 * --- */

esch_error esch_parser_new(esch_config* config, esch_parser** parser);
esch_error esch_parser_delete(esch_parser* parser);
esch_error esch_parser_read_line(esch_parser* parser, char* input);
esch_error esch_parser_read_file(esch_parser* parser, char* file);
esch_error esch_parser_get_ast(esch_parser* parser, esch_ast** ast);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_ESCH_H_ */
