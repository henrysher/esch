#include "esch.h"
#include "esch_config.h"
#include "esch_alloc.h"
#include "esch_type.h"
#include "esch_object.h"
#include "esch_gc.h"
#include "esch_debug.h"

/*
 * -----------------------------------------------------------------
 * Public interface. Used by esch.h.
 * -----------------------------------------------------------------
 */
esch_error
esch_object_new(esch_config* config, esch_type* type, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_object* gc_obj = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_gc* gc = NULL;

    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_TYPE(type));

    alloc_obj = ESCH_CONFIG_GET_ALLOC(config);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    gc_obj = ESCH_CONFIG_GET_GC(config);
    ESCH_CHECK_PARAM_PUBLIC(alloc_obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(log_obj != NULL);

    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LOG(log));

    if (gc_obj != NULL)
    {
        gc = ESCH_CAST_FROM_OBJECT(gc_obj, esch_gc);
        ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_GC(gc));
    }
    ret = esch_object_new_i(config, type, obj);
Exit:
    return ret;
}

esch_error
esch_object_delete(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    if (obj == NULL)
    {
        return ret;
    }
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(obj));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LOG(obj->log));
    /* 
     * For public interface we prevent user delete a managed object.
     */
    ESCH_CHECK_1((obj->gc == NULL && obj->gc_id == NULL), obj->log,
            "object:delete: Can't delete an GC-awared object: %x", obj,
            ESCH_ERROR_DELETE_MANAGED_OBJECT);
    ret = esch_object_delete_i(obj);
Exit:
    return ret;
}

esch_error
esch_object_get_type(esch_object* obj, esch_type** type)
{
    esch_error ret = ESCH_OK;

    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(obj));
    (*type) = ESCH_OBJECT_GET_TYPE(obj);
Exit:
    return ret;
}

esch_error
esch_object_is_primitive(esch_object* obj, esch_bool* primitive)
{
    esch_error ret = ESCH_OK;

    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(primitive != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(obj));
    (*primitive) = ESCH_TYPE_IS_PRIMITIVE(ESCH_OBJECT_GET_TYPE(obj));
Exit:
    return ret;
}

esch_error
esch_object_is_container(esch_object* obj, esch_bool* container)
{
    esch_error ret = ESCH_OK;

    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(container != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(obj));
    (*container) = ESCH_TYPE_IS_CONTAINER(ESCH_OBJECT_GET_TYPE(obj));
Exit:
    return ret;

}
esch_error
esch_object_get_iterator(esch_object* obj, esch_iterator* iter)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(iter != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(obj));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_TYPE(ESCH_OBJECT_GET_TYPE(obj)));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_OBJECT_GET_LOG(obj) != NULL);
    ret = esch_object_get_iterator_i(obj, iter);
Exit:
    return ret;
}

esch_error
esch_object_cast_to_object(void* data, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_object* to_obj = NULL;
    ESCH_CHECK_PARAM_PUBLIC(data != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);

    to_obj = ESCH_CAST_TO_OBJECT(data);
    if (ESCH_IS_VALID_OBJECT(to_obj))
    {
        (*obj) = to_obj;
        ret = ESCH_OK;
    }
    else
    {
        ret = ESCH_ERROR_INVALID_PARAMETER;
    }
Exit:
    return ret;
}
esch_error
esch_object_cast_from_object(esch_object* obj, void** data)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(data != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);

    (*data) = ESCH_CAST_FROM_OBJECT(obj, void);
Exit:
    return ret;
}

/*
 * -----------------------------------------------------------------
 * Internal functions. Used only within internal esch function.
 * -----------------------------------------------------------------
 */
esch_error
esch_object_new_i(esch_config* config, esch_type* type, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    size_t obj_size = 0;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_object* gc_obj = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_gc*  gc = NULL;
    esch_object* new_object = NULL;
    void* new_gc_id = NULL;

    ESCH_CHECK_PARAM_INTERNAL(type != NULL);

    alloc_obj = ESCH_CONFIG_GET_ALLOC(config);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    gc_obj = ESCH_CONFIG_GET_GC(config);
    ESCH_CHECK_PARAM_INTERNAL(alloc_obj != NULL);
    ESCH_CHECK_PARAM_INTERNAL(log_obj != NULL);

    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    gc = (gc_obj == NULL? NULL: ESCH_CAST_FROM_OBJECT(gc_obj, esch_gc));

    /* Always check binary compatibilities. */
    ESCH_CHECK_3(type->version == ESCH_VERSION, log,
            "Bad version: expected: %d, got: %d, type: 0x%x",
            ESCH_VERSION, type->version, type, ret);
    ESCH_CHECK_1(ESCH_TYPE_GET_OBJECT_SIZE(type) >= 0,
            log,
            "Object size smaller than 0. type: 0x%x", type, ret);

    /*
     * Alloc object may have running out of memory. GC logic should
     * be triggered here:
     */
    obj_size = sizeof(esch_object) + ESCH_TYPE_GET_OBJECT_SIZE(type);
    ret = esch_alloc_realloc(alloc, NULL, obj_size, (void**)&new_object);

    if (gc != NULL)
    {
        ESCH_CHECK_1((ret == ESCH_OK || ret == ESCH_ERROR_OUT_OF_MEMORY),
                log,
                "Can't malloc (with GC). type: 0x%x", type,
                ret);
        if (ret == ESCH_ERROR_OUT_OF_MEMORY)
        {
            (void)esch_log_info(log, "object:new: OOM. Trigger GC.");
            ret = esch_gc_recycle_i(gc);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                    "object:new: Can't trigger GC. obj: 0x%x", gc, ret);
            /* Now try allcoate memory again. Be sure that we don't run
             * all the time. */
            ret = esch_alloc_realloc(alloc, NULL,
                                     obj_size, (void**)&new_object);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                         "FATAL: Can't malloc after GC. type: 0x%x",
                         type, ret);
        }
        ESCH_OBJECT_GET_TYPE(new_object)  = type;
        ESCH_OBJECT_GET_ALLOC(new_object) = alloc;
        ESCH_OBJECT_GET_LOG(new_object)   = log;
        (void)esch_log_info(log, "object:new: Try attach GC.");
        ret = esch_gc_attach_i(gc, new_object);
        ESCH_CHECK_1(ret == ESCH_OK, log,
                "object:new: Can't attach to GC. type: 0x%x", type, ret);
    }
    else
    {
        ESCH_CHECK_1(ret == ESCH_OK, log,
                "object:new: Can't malloc (without GC). type: 0x%x",
                type, ret);
        ESCH_OBJECT_GET_TYPE(new_object)  = type;
        ESCH_OBJECT_GET_ALLOC(new_object) = alloc;
        ESCH_OBJECT_GET_LOG(new_object)   = log;
    }

    /*
     * Final assignment. Make sure the object is initialized without any
     * risks of partial initialization.
     */
    /* ESCH_OBJECT_GET_GC(new_object) is already assigned. */
    /* ESCH_OBJECT_GET_GC_ID(new_object) is already assigned. */
    (*obj) = new_object;
    new_object = NULL;
Exit:
    if (new_object != NULL)
    {
        /* In this case, the object is not fully created. The only thing
         * we can do is to free the memory. */
        (void)esch_alloc_free(alloc, (void*)new_object);
    }
    return ret;
}

esch_error
esch_object_delete_i(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    esch_gc* gc = NULL;
    esch_type* type = NULL;
    esch_alloc *alloc = NULL;
    void* gc_id = NULL;

    type  = ESCH_OBJECT_GET_TYPE(obj);
    alloc = ESCH_OBJECT_GET_ALLOC(obj);
    log   = ESCH_OBJECT_GET_LOG(obj);
    gc    = ESCH_OBJECT_GET_GC(obj);
    gc_id = ESCH_OBJECT_GET_GC_ID(obj);

    ESCH_ASSERT(type != NULL);
    ESCH_ASSERT(log != NULL);
    ESCH_ASSERT(!gc && !gc_id);

    ret = type->object_destructor(obj);
    ESCH_CHECK_1(ret == ESCH_OK, log, "Dtor fails: obj: 0x%x", obj, ret);
    if (alloc != NULL)
    {
        ret = esch_alloc_free(alloc, obj);
        ESCH_CHECK_1(ret == ESCH_OK,
                log, "Can't free: obj: 0x%x", obj, ret);
    }
    else
    {
        esch_log_info(log, "object:delete: No alloc. Do nothing");
    }
Exit:
    return ret;
}

esch_error
esch_object_get_iterator_i(esch_object* obj, esch_iterator* iter)
{
    esch_error ret = ESCH_OK;
    esch_type* type = NULL;
    esch_log* log = NULL;

    type = ESCH_OBJECT_GET_TYPE(obj);
    log = ESCH_OBJECT_GET_LOG(obj);
    if (ESCH_TYPE_IS_PRIMITIVE(type))
    {
        /* Primitive does not have iterator. Change nothing. */
        (void)esch_log_error(log, "get_iterator: primitive type.");
        ret = ESCH_ERROR_NOT_SUPPORTED;
    }
    else
    {
        (void)esch_log_info(log, "get_iterator(): container type.");
        ret = (ESCH_TYPE_GET_OBJECT_GET_ITERATOR(type))(obj, iter);
    }
    return ret;
}
