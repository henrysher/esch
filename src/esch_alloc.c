/* vim:ft=c expandtab tw=72 sw=4
 */
#include "esch.h"
#include "esch_alloc.h"
#include "esch_debug.h"
#include <stdlib.h>

/**
 * Create a new esch_alloc object. With underlying allocator 
 * @param alloc Returned allocator object.
 * @return Error code.
 */
esch_error
esch_alloc_new_c_default(esch_alloc** alloc)
{
    esch_error ret = ESCH_OK;
    esch_alloc* new_obj = NULL;

    ESCH_CHECK_NO_LOG(alloc != NULL, ESCH_ERROR_INVALID_PARAMETER);
    new_obj = (esch_alloc*)malloc(sizeof(esch_alloc));
    ESCH_CHECK_NO_LOG(alloc != NULL, ESCH_ERROR_OUT_OF_MEMORY);
    new_obj->base.type = ESCH_TYPE_ALLOC_C_DEFAULT;
    new_obj->base.log = NULL;
    new_obj->base.alloc = NULL;
    new_obj->allocate_count = 0;
    new_obj->deallocate_count = 0;
    (*alloc) = new_obj;
    new_obj = NULL;
Exit:
    if (new_obj != NULL) {
        free(new_obj);
    }
    return ret;
}

/**
 * Delete esch_alloc object.
 * @param alloc The esch_alloc object to be deleted.
 * @return Error code.
 */
esch_error
esch_alloc_delete(esch_alloc* alloc)
{
    esch_error ret = ESCH_OK;

    esch_object* obj = NULL;
    if (alloc != NULL) {
        ESCH_CHECK(alloc->allocate_count == alloc->deallocate_count,
                   alloc,
                   "Memory leak detected when detroying allocator.",
                   ESCH_ERROR_INVALID_STATE);
    }
Exit:
    free(alloc);
    return ret;
}

/**
 * Allocate a new buffer.
 * @param alloc The esch_alloc object to allocate buffer.
 * @param size  Size of wanted buffer.
 * @param ptr   Returned pointer to allocated buffer.
 * @return      Error code.
 */
esch_error
esch_alloc_malloc(esch_alloc* alloc, size_t size, void** ptr)
{
    esch_error ret = ESCH_OK;
    void* new_buffer = NULL;
    ESCH_CHECK_NO_LOG(alloc != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(size > 0, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(ptr != NULL, ESCH_ERROR_INVALID_PARAMETER);

    new_buffer = malloc(size);
    ESCH_CHECK(new_buffer != NULL, alloc,
            "esch_alloc_malloc(): Fail to allocate",
            ESCH_ERROR_OUT_OF_MEMORY);
    alloc->allocate_count += 1;
    (*ptr) = new_buffer;
    new_buffer = NULL;
Exit:
    if (new_buffer != NULL) {
        free(new_buffer);
    }
    return ret;
}

/**
 * Deallocate a buffer.
 * @param alloc The esch_alloc object to reclaim the buffer.
 * @param ptr   Buffer to be deleted.
 * @return      Error code.
 */
esch_error
esch_alloc_free(struct esch_alloc* alloc, void* ptr)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_NO_LOG(alloc != NULL, ESCH_ERROR_INVALID_PARAMETER);

    if (ptr != NULL) {
        alloc->allocate_count -= 1;
    }
Exit:
    free(ptr);
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

