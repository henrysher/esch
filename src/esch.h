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

#define ESCH_FALSE 0
#define ESCH_TRUE !(ESCH_FALSE)

typedef enum {
    ESCH_OK = 0,
    ESCH_ERROR_NOT_IMPLEMENTED,
    ESCH_ERROR_OUT_OF_MEMORY,
    ESCH_ERROR_INVALID_PARAMETER,
    ESCH_ERROR_INVALID_STATE,
} esch_error;

typedef enum {
    ESCH_TYPE_UNKNOWN          = 0,
    ESCH_TYPE_MAGIC            = 0xEC00,
    /* Generic types */
    ESCH_TYPE_NO_DELETE        = ESCH_TYPE_MAGIC | 0x10,
    ESCH_TYPE_PRIMITIVE        = ESCH_TYPE_MAGIC | 0x20,
    ESCH_TYPE_CONTAINER        = ESCH_TYPE_MAGIC | 0x30,

    /* Concret types */
    ESCH_TYPE_CHAR_AS_STRING   = ESCH_TYPE_MAGIC | ESCH_TYPE_PRIMITIVE | 1,
    ESCH_TYPE_STRING           = ESCH_TYPE_MAGIC | ESCH_TYPE_PRIMITIVE | 2,
    ESCH_TYPE_SYMBOL           = ESCH_TYPE_MAGIC | ESCH_TYPE_PRIMITIVE | 3,
    ESCH_TYPE_NUMBER           = ESCH_TYPE_MAGIC | ESCH_TYPE_PRIMITIVE | 4,
    ESCH_TYPE_LIST             = ESCH_TYPE_MAGIC | ESCH_TYPE_CONTAINER | 1,

    ESCH_TYPE_ALLOC_DUMMY      = ESCH_TYPE_MAGIC | ESCH_TYPE_NO_DELETE | 1,
    ESCH_TYPE_ALLOC_C_DEFAULT  = ESCH_TYPE_MAGIC | ESCH_TYPE_NO_DELETE | 2,
    ESCH_TYPE_LOG_PRINTF       = ESCH_TYPE_MAGIC | ESCH_TYPE_NO_DELETE | 3,
    ESCH_TYPE_LOG_DO_NOTHING   = ESCH_TYPE_MAGIC | ESCH_TYPE_NO_DELETE | 4,

    /* Will be deleted */
    ESCH_TYPE_PARSER           = ESCH_TYPE_MAGIC | 0x60,
    ESCH_TYPE_VM               = ESCH_TYPE_MAGIC | 0xf0,
} esch_type;

typedef struct esch_object          esch_object;
typedef struct esch_object          esch_config; /* Alias only */
typedef struct esch_alloc           esch_alloc;
typedef struct esch_log             esch_log;
typedef struct esch_parser          esch_parser;
typedef struct esch_parser_callback esch_parser_callback;
typedef struct esch_ast             esch_ast;
typedef struct esch_string          esch_string;
typedef struct esch_list            esch_list;
typedef struct esch_list_node       esch_list_node;

typedef char                        esch_utf8_char;
typedef wchar_t                     esch_unicode;

/**
 * The public type info structure. This is the header of all structures
 * used in Esch. All objects can be casted to esch_object to get type
 * information.
 */
struct esch_object
{
    esch_type   type;       /**< Registered type ID */
    esch_alloc* alloc;      /**< Allocator object to manage memory. */
    esch_log*   log;        /**< Log object to write trace/errors.*/
};

#define ESCH_COMMON_HEADER    esch_object header;

#define ESCH_GET_CONFIG(obj)         ((esch_object*)obj)
#define ESCH_GET_TYPE(obj)           (((esch_object*)obj)->type)
#define ESCH_GET_LOG(obj)            (((esch_object*)obj)->log)
#define ESCH_GET_ALLOC(obj)          (((esch_object*)obj)->alloc)

#define ESCH_IS_VALID_OBJECT(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_MAGIC) == ESCH_TYPE_MAGIC && \
     ESCH_GET_LOG(obj) != NULL && \
     ESCH_GET_ALLOC(obj) != NULL)

#define ESCH_IS_VALID_CONFIG(obj) \
     (ESCH_GET_LOG(obj) != NULL && \
      ESCH_GET_ALLOC(obj) != NULL)

#define ESCH_IS_NO_DELETE(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_NO_DELETE) == ESCH_TYPE_NO_DELETE)
#define ESCH_IS_PRIMITIVE(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_PRIMITIVE) == ESCH_TYPE_PRIMITIVE)
#define ESCH_IS_CONTAINER(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_CONTAINER) == ESCH_TYPE_CONTAINER)

/* --- Memory allocator --- */
esch_error esch_alloc_new_c_default(esch_object* config,
                                    esch_alloc** ret_alloc);
esch_error esch_alloc_delete(esch_alloc* alloc);
esch_error esch_alloc_malloc(esch_alloc* alloc, size_t size, void** ret);
esch_error esch_alloc_free(esch_alloc* alloc, void* ptr);

/* --- Logger objects -- */
extern esch_log* esch_global_log;
esch_error esch_log_new_do_nothing(esch_log** log);
esch_error esch_log_new_printf(esch_log** log);
esch_error esch_log_delete(esch_log* log);
esch_error esch_log_error(esch_log* log, const char* fmt, ...);
esch_error esch_log_info(esch_log* log, const char* fmt, ...);

/* --- String objects --- */
esch_error esch_string_new_from_utf8(esch_object* config, char* utf8,
                                     int begin, int end,
                                     esch_string** str);
esch_error esch_string_delete(esch_string* str);
char* esch_string_get_utf8_ref(esch_string* str);
esch_unicode* esch_string_get_unicode_ref(esch_string* str);
size_t esch_string_get_utf8_length(esch_string* str);
size_t esch_string_get_unicode_length(esch_string* str);

int esch_unicode_string_is_valid_identifier(const esch_unicode* unicode);

#define esch_unicode_is_ascii(ch) \
    (((esch_unicode)(ch)) >= 0 && ((esch_unicode)(ch)) < 127 )
#define esch_unicode_is_digit(ch) ((ch) >= '0' && (ch) <= '9')
#define esch_unicode_is_alpha(ch) \
    (((ch) >= 'A' && (ch) <= 'Z') || ((ch) >= 'a' && (ch) <= 'z'))
#define esch_unicode_is_extended_alphabetic(ch) \
    ((ch) == '!' || (ch) == '$' || (ch) == '%' || (ch) == '&' || \
     (ch) == '*' || (ch) == '+' || (ch) == '-' || (ch) == '.' || \
     (ch) == '/' || (ch) == ':' || (ch) == '<' || (ch) == '=' || \
     (ch) == '>' || (ch) == '?' || (ch) == '@' || (ch) == '^' || \
     (ch) == '_' || (ch) == '~')
int esch_unicode_is_range_lu(esch_unicode ch);
int esch_unicode_is_range_ll(esch_unicode ch);
int esch_unicode_is_range_lt(esch_unicode ch);
int esch_unicode_is_range_lm(esch_unicode ch);
int esch_unicode_is_range_lo(esch_unicode ch);
int esch_unicode_is_range_mn(esch_unicode ch);
int esch_unicode_is_range_mc(esch_unicode ch);
int esch_unicode_is_range_nd(esch_unicode ch);
int esch_unicode_is_range_nl(esch_unicode ch);
int esch_unicode_is_range_no(esch_unicode ch);
int esch_unicode_is_range_pd(esch_unicode ch);
int esch_unicode_is_range_pc(esch_unicode ch);
int esch_unicode_is_range_po(esch_unicode ch);
int esch_unicode_is_range_sc(esch_unicode ch);
int esch_unicode_is_range_sm(esch_unicode ch);
int esch_unicode_is_range_sk(esch_unicode ch);
int esch_unicode_is_range_so(esch_unicode ch);
int esch_unicode_is_range_co(esch_unicode ch);

/* --- List --- */
/* XXX
 * This is a list used by both Scheme list and array. So it supports
 * accessors for both. However, do not try to access same list object in
 * both ways.
 *
 * The list accepts only valid esch_ objects.
 *
 */
esch_error esch_list_new(esch_config* config, size_t initial_length,
                         esch_list** lst);
esch_error esch_list_delete(esch_list* list, int delete_data);
esch_error esch_list_get_length(esch_list* list, size_t* length);
esch_error esch_list_get_by_index(esch_list* list,
                                  int index,
                                  esch_list_node** node);
esch_error esch_list_get_first(esch_list* list, esch_list_node** node);
esch_error esch_list_get_next(esch_list_node* node,
                              esch_list_node* next);
esch_error esch_list_get_prev(esch_list_node* item, 
                              esch_list_node* node);
esch_error esch_list_prepend(esch_list* list, esch_object* data);
esch_error esch_list_append(esch_list* list, esch_object* data);

/* I will not allow we create a new externally. */
esch_error esch_list_node_get_data(esch_list_node* node, void** data);
esch_error esch_list_node_delete(esch_list_node* node);

/* --- TODO Number -- */
/* --- TODO Complex -- */
/* --- TODO Real -- */

/* --- Parser --- */
esch_error esch_parser_new(esch_config* config, esch_parser** parser);
esch_error esch_parser_delete(esch_parser* parser);
esch_error esch_parser_register_callback(esch_parser* parser,
                                         esch_parser_callback* cb);
esch_error esch_parser_remove_callback(esch_parser* parser,
                                       esch_parser_callback* cb);
esch_error esch_parser_read_line(esch_parser* parser, char* input);
esch_error esch_parser_read_file(esch_parser* parser, char* file);

/* --- 
 * TODO Add more functions as part of configurations.
 * 1. Message callback function to keep error information.
 * 2. Standard library
 * 3. Search path
 * --- */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_ESCH_H_ */
