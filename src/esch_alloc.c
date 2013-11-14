/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "esch_alloc.h"
#include "esch_debug.h"

/**
 * Create a new esch_alloc object. With underlying allocator 
 * @param config Config object.
 * @param alloc Returned allocator object.
 * @return Error code.
 */
esch_error
esch_alloc_new_c_default(esch_config* config, esch_alloc** alloc)
{
    esch_error ret = ESCH_OK;
    esch_alloc* new_obj = NULL;
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config->log != NULL);

    new_obj = (esch_alloc*)malloc(sizeof(esch_alloc));
    ESCH_CHECK_NO_LOG(new_obj != NULL, ESCH_ERROR_OUT_OF_MEMORY);
    ESCH_GET_TYPE(new_obj) = ESCH_TYPE_ALLOC_C_DEFAULT;
    ESCH_GET_ALLOC(new_obj) = new_obj; /* No use */
    ESCH_GET_LOG(new_obj) = config->log;
    new_obj->allocate_count = 0;
    new_obj->deallocate_count = 0;
    (*alloc) = new_obj;
    new_obj = NULL;
    assert(ESCH_IS_VALID_ALLOC(*alloc));
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
    esch_config* obj = NULL;
    esch_alloc* self_alloc = NULL;
    esch_type self_type = ESCH_TYPE_UNKNOWN;

    if (alloc == NULL) {
        return ret;
    }
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));
    self_alloc = ESCH_GET_ALLOC(alloc);
    self_type = ESCH_GET_TYPE(alloc);
    ESCH_CHECK(self_alloc == alloc,
               alloc,
               "base.alloc != self on C default alloc",
               ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK(self_type == ESCH_TYPE_ALLOC_C_DEFAULT,
               alloc,
               "Not ESCH_TYPE_ALLOC_C_DEFAULT",
               ESCH_ERROR_INVALID_PARAMETER);

    ESCH_CHECK_2(alloc->allocate_count == alloc->deallocate_count,
               alloc,
               "Memory leak detected. Allocated = %d, deallocated = %d",
               alloc->allocate_count, alloc->deallocate_count,
               ESCH_ERROR_INVALID_STATE);
    free(alloc);
Exit:
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
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(size > 0);
    ESCH_CHECK_PARAM_PUBLIC(ptr != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));

    new_buffer = malloc(size);
    ESCH_CHECK(new_buffer != NULL, alloc,
            "esch_alloc_malloc(): Fail to allocate",
            ESCH_ERROR_OUT_OF_MEMORY);
    alloc->allocate_count += 1;
    bzero(new_buffer, size);
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
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));

    if (ptr != NULL) {
        alloc->deallocate_count += 1;
    }
Exit:
    free(ptr);
    return ret;
}

