/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch_alloc.h"
#include "esch_debug.h"
#include "esch_config.h"
#include "esch_object.h"
#include "esch_type.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ================================================================= */
/*                      Definitions for esch_alloc                   */
/* ================================================================= */
esch_error
esch_alloc_realloc(esch_alloc* alloc, void* in, size_t size, void** out)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(out != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));
    ret = esch_alloc_realloc_i(alloc, in, size, out);
Exit:
    return ret;
}

esch_error
esch_alloc_free(esch_alloc* alloc, void* ptr)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ptr != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));
    ret = esch_alloc_free_i(alloc, ptr);
Exit:
    return ret;
}

/* ================================================================= */
/*                       Internal functions                          */
/* ================================================================= */

esch_error
esch_alloc_realloc_i(esch_alloc* alloc, void* in, size_t size, void** out)
{
    return (alloc->realloc)(alloc, in, size, out);
}

esch_error
esch_alloc_free_i(esch_alloc* alloc, void* ptr)
{
    return (alloc->free)(alloc, ptr);
}


/* ================================================================= */
/*             Definitions for esch_alloc_c_default                  */
/* ================================================================= */

static esch_error
esch_alloc_new_c_default_s(esch_config* config, esch_object** obj);

static esch_error
esch_alloc_delete_c_default_s(esch_object* obj);

static esch_error
esch_alloc_realloc_c_default(esch_alloc* alloc,
                             void* in, size_t size, void** out);
static esch_error
esch_alloc_free_c_default(esch_alloc* alloc, void* ptr);

struct esch_builtin_type esch_alloc_c_default_type =
{
    {
        &(esch_alloc_c_default_type.type),
        NULL, /* No log */
        &(esch_log_do_nothing.log),
        NULL,
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_alloc_c_default),
        esch_alloc_new_c_default_s,
        esch_alloc_delete_c_default_s,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    }
};

esch_error
esch_alloc_new_c_default(esch_config* config, esch_alloc** alloc)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    esch_object* log_obj = NULL;
    esch_object* new_obj = NULL;
    esch_alloc_c_default* new_alloc = NULL;
    void* buffer = NULL;
    size_t size = 0;
    esch_alloc** cookie = NULL;
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);

    if (config != NULL)
    {
        log_obj = ESCH_CONFIG_GET_LOG(config);
        ESCH_CHECK_NO_LOG(log_obj != NULL, ESCH_ERROR_INVALID_PARAMETER);
        log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    }
    else
    {
        log = esch_global_log;
    }
    /*
     * NOTE: By default we provide c_default alloc object. It follows
     * basic layout but indeed NOT a real managed esch_object!
     */

    size = sizeof(esch_alloc*) +
           sizeof(esch_object) + sizeof(esch_alloc_c_default);
    buffer = malloc(size);
    ret = ESCH_ERROR_OUT_OF_MEMORY;
    ESCH_CHECK(buffer != NULL, log, "Can't malloc() default alloc", ret);
    ret = ESCH_OK;

    new_obj = (esch_object*)(buffer + sizeof(esch_alloc*));
    new_alloc = ESCH_CAST_FROM_OBJECT(new_obj, esch_alloc_c_default);
    new_alloc->base.realloc = esch_alloc_realloc_c_default;
    new_alloc->base.free = esch_alloc_free_c_default;
    new_alloc->allocate_count = 0;
    new_alloc->deallocate_count = 0;
    cookie = (esch_alloc**)buffer;
    (*cookie) = &(new_alloc->base);

    ESCH_OBJECT_GET_TYPE(new_obj) = &(esch_alloc_c_default_type.type);
    ESCH_OBJECT_GET_ALLOC(new_obj) = &(new_alloc->base);
    ESCH_OBJECT_GET_LOG(new_obj) = log;
    ESCH_OBJECT_GET_GC(new_obj) = NULL; /* Alloc can't be managed! */
    ESCH_OBJECT_GET_GC_ID(new_obj) = NULL;
    assert(ESCH_IS_VALID_C_DEFAULT_ALLOC(new_alloc));

    (*alloc) = &(new_alloc->base);
    new_alloc = NULL;
    new_obj = NULL;
Exit:
    if (new_alloc != NULL) {
        free(buffer);
    }
    return ret;
}

/* ================================================================= */
/*                       Internal functions                          */
/* ================================================================= */
static esch_error
esch_alloc_realloc_c_default(esch_alloc* alloc,
                             void* in, size_t size, void** out)
{
    esch_error ret = ESCH_OK;
    esch_alloc_c_default* alloc_c = NULL;
    esch_log* log = NULL;
    void* new_buffer = NULL;
    size_t size_with_cookie = 0;
    void* old_buffer = NULL;
    esch_alloc** cookie = NULL;

    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(size > 0);
    ESCH_CHECK_PARAM_PUBLIC(out != NULL);
    alloc_c = (esch_alloc_c_default*)alloc;
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_C_DEFAULT_ALLOC(alloc_c));

    log = ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(alloc));
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);

    old_buffer = in;
    if (in != NULL)
    {
        esch_alloc** obj = (esch_alloc**)(in - sizeof(esch_alloc*));
        ESCH_CHECK_1((*obj) == alloc, log,
                     "malloc: Bad alloc cookie, cookie = 0x%x",
                     (*obj), ESCH_ERROR_INVALID_STATE);
        old_buffer = in - sizeof(esch_alloc*);
    }

    size_with_cookie = size + sizeof(esch_alloc*);
    new_buffer = (void*)realloc(old_buffer, size_with_cookie);
    ESCH_CHECK(new_buffer != NULL, log,
            "esch_alloc_malloc(): Fail to allocate",
            ESCH_ERROR_OUT_OF_MEMORY);
    if (in == NULL)
    {
        memset(new_buffer, 0, size_with_cookie);
    }
    if (in != NULL)
    {
        alloc_c->deallocate_count += 1;
    }
    alloc_c->allocate_count += 1;
    cookie = (esch_alloc**)new_buffer;
    (*cookie) = alloc;
    (*out) = new_buffer + sizeof(esch_alloc*);
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
static esch_error
esch_alloc_free_c_default(esch_alloc* alloc, void* ptr)
{
    esch_error ret = ESCH_OK;
    esch_alloc_c_default* alloc_c = NULL;
    esch_object* alloc_obj = NULL;
    esch_log* log = NULL;

    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    alloc_c = (esch_alloc_c_default*)alloc;
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_C_DEFAULT_ALLOC(alloc_c));
    alloc_obj = ESCH_CAST_TO_OBJECT(alloc);
    log = ESCH_OBJECT_GET_LOG(alloc_obj);
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);
    if (ptr != NULL)
    {
        esch_alloc** cookie = (esch_alloc**)(ptr - sizeof(esch_alloc*));
        ESCH_CHECK_1((*cookie) == alloc, log,
                     "malloc: Bad alloc cookie, cookie = 0x%x",
                     (*cookie), ESCH_ERROR_INVALID_STATE);
        alloc_c->deallocate_count += 1;
        /* Must be the last step, 'cause it can be used to free itself. */
        free(cookie);
    }
Exit:
    return ret;
}

/**
 * Delete esch_alloc object.
 * @param alloc The esch_alloc object to be deleted.
 * @return Error code.
 */
static esch_error
esch_alloc_delete_c_default_s(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    esch_alloc_c_default* alloc_c = NULL;

    if (obj == NULL)
    {
        return ret;
    }
    alloc_c = ESCH_CAST_FROM_OBJECT(obj, esch_alloc_c_default);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_C_DEFAULT_ALLOC(alloc_c));
    log = ESCH_OBJECT_GET_LOG(obj);
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);

    ESCH_CHECK_2(alloc_c->allocate_count == alloc_c->deallocate_count,
               log,
               "Memory leak detected. Allocated = %d, deallocated = %d",
               alloc_c->allocate_count, alloc_c->deallocate_count,
               ESCH_ERROR_INVALID_STATE);
    /* Don't free here. It will be called by esch_object_delete() by
     * esch_alloc_free(). */
Exit:
    return ret;
}

static esch_error
esch_alloc_new_c_default_s(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;

    ret = esch_alloc_new_c_default(config, &alloc);
    if (ret == ESCH_OK)
    {
        (*obj) = ESCH_CAST_TO_OBJECT(alloc);
    }
Exit:
    return ret;
}
