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
    ESCH_OK                               = 0,
    ESCH_ERROR_OUT_OF_MEMORY              = 1,
    ESCH_ERROR_INVALID_PARAMETER          = 2,
    ESCH_ERROR_INVALID_STATE              = 3,
} esch_error;

typedef enum {
    ESCH_TYPE_UNKNOWN          = 0,
    /* Generic types */
    ESCH_TYPE_CUSTOMIZED       = 0x10,
    ESCH_TYPE_ALLOC            = 0x20,
    ESCH_TYPE_LOG              = 0x30,
    ESCH_TYPE_PARSER           = 0x40,
    ESCH_TYPE_VM               = 0xf0,

    ESCH_TYPE_ALLOC_C_DEFAULT  = 0x21,
    ESCH_TYPE_LOG_PRINTF       = 0x31,
} esch_type;

typedef struct esch_object         esch_object;
typedef struct esch_alloc          esch_alloc;
typedef struct esch_log            esch_log;

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

#define ESCH_OBJECT(obj)             ((esch_object*)obj)
#define ESCH_OBJECT_GET_TYPE(obj)    (((esch_object*)obj)->type)
#define ESCH_OBJECT_GET_LOG(obj)     (((esch_object*)obj)->log)
#define ESCH_OBJECT_GET_ALLOC(obj)   (((esch_object*)obj)->alloc)

/* --- Memory allocator --- */
esch_error esch_alloc_new_c_default(esch_alloc** ret_alloc);
esch_error esch_alloc_delete(esch_alloc* alloc);
esch_error esch_alloc_malloc(esch_alloc* alloc, size_t size, void** ret);
esch_error esch_alloc_free(esch_alloc* alloc, void* ptr);

/* --- Logger objects -- */
esch_error esch_log_config_new_printf(esch_log** log);
esch_error esch_log_config_delete(esch_log* log);
esch_error esch_log_error(esch_log* log, char* message, esch_error code);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_ESCH_H_ */
