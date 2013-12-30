#include "esch.h"
#include "esch_config.h"
#include "esch_alloc.h"
#include "esch_type.h"
#include "esch_object.h"
#include "esch_gc.h"
#include "esch_debug.h"
#include <assert.h>

esch_error
esch_object_new(esch_config* config, esch_type* type, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_TYPE(type));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_CONFIG_GET_ALLOC(config) != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_CONFIG_GET_LOG(config) != NULL);
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
    /* 
     * The second parameter must be set to ESCH_FALSE. So the behavior
     * of object lifetime management is fully delegated to GC when it's
     * registered, or directly get deleted when not managed by GC.
     *
     * We enforce this rule, because this is a public interface for
     * third-party develoeprs. We don't want developer bypass GC
     * management, even if they really know what they are doing. Make
     * sure they depend on our esch_object interfaces to get a
     * consistent behavior.
     */
    ret = esch_object_delete_i(obj, ESCH_FALSE);
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

/* ----------------------------------------------------------------- */
/*                       Internal functions                          */
/* ----------------------------------------------------------------- */
esch_error
esch_object_new_i(esch_config* config, esch_type* type, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    size_t obj_size = 0;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_gc*  gc = NULL;
    esch_object* new_object = NULL;
    void* new_gc_id = NULL;

    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_LOG,
                              (esch_object**)&log);
    if (log == NULL)
    {
        log = esch_global_log;
    }
    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_ALLOC,
                              (esch_object**)&alloc);
    ESCH_CHECK_1(ret == ESCH_OK, log, "No alloc. type: 0x%x", type, ret);
    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_GC,
                              (esch_object**)&gc);
    /* Always check binary compatibilities. */
    ESCH_CHECK_3(type->version == ESCH_VERSION, log,
            "Bad version: expected: %d, got: %d, type: 0x%x",
            ESCH_VERSION, type->version, type, ret);
    if (ret != ESCH_OK)
    {
        if (ret == ESCH_ERROR_NOT_FOUND)
        {
            (void)esch_log_info(esch_global_log, "GC not specified.");
            assert(gc == NULL);
        }
        else
        {
            ESCH_CHECK_1(ret == ESCH_OK,
                         log, "Bad GC. type: 0x%x", type, ret);
        }
    }
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
                "Can't malloc (without GC). type: 0x%x", type,
                ret);
        if (ret == ESCH_ERROR_OUT_OF_MEMORY)
        {
            ret = esch_gc_perform_gc(gc, alloc);
            ESCH_CHECK_1(ret == ESCH_OK,
                         log, "Can't trigger GC. obj: 0x%x", gc, ret);
            /* Now try allcoate memory again. Be sure that we don't run
             * all the time. */
            ret = esch_alloc_realloc(alloc, NULL,
                                     obj_size, (void**)&new_object);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                         "Can't malloc after GC. type: 0x%x", type, ret);
            ret = esch_gc_register(gc, new_object);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                    "Can't register to GC. type: 0x%x", type, ret);
        }
    }
    else
    {
        ESCH_CHECK_1(ret == ESCH_OK, log,
                "Can't malloc (without GC). type: 0x%x", type, ret);
    }

    /*
     * Final assignment. Make sure the object is initialized without any
     * risks of partial initialization.
     */
    ESCH_OBJECT_GET_TYPE(new_object)    = type;
    ESCH_OBJECT_GET_ALLOC(new_object)   = alloc;
    ESCH_OBJECT_GET_LOG(new_object)     = log;
    /* ESCH_OBJECT_GET_GC(new_object) is already assigned. */
    /* ESCH_OBJECT_GET_GC_ID(new_object) is already assigned. */

    (*obj) = new_object;
    new_object = NULL;
Exit:
    if (new_object != NULL)
    {
        (void)esch_object_delete_i(new_object, ESCH_TRUE);
    }
    return ret;
}

static esch_error
do_delete(esch_object* obj, esch_bool force_delete)
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

    assert(type != NULL);
    assert(log != NULL);
    assert(((gc && gc_id) || (!gc && !gc_id)));

    if (gc != NULL)
    {
        (void)esch_log_info(log,
                "Mananged object: obj: 0x%x, gc: 0x%x, force: %d",
                obj, gc, force_delete);
        if (force_delete)
        {
            (void)esch_log_info(log, "Force delete.");
            ret = esch_gc_release(obj);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                         "Can't release from GC: obj: 0x%x", obj, ret);
            ret = type->object_delete(obj);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                         "Can't delete: obj: 0x%x", obj, ret);
            if (alloc != NULL)
            {
                ret = esch_alloc_free(alloc, obj);
                ESCH_CHECK_1(ret == ESCH_OK, log,
                             "Can't free: obj: 0x%x", obj, ret);
            }
        }
        else
        {
            /* 
             * Inform GC to release this object. It may or may not call
             * type->object_delete(), depending on the logic of GC.
             */
            ret = esch_gc_release(obj);
            ESCH_CHECK_1(ret == ESCH_OK, log,
                         "Can't release: obj: 0x%x", obj, ret);
        }
    }
    else
    {
        (void)esch_log_info(log, "Unmananged object. delete.");
        ret = type->object_delete(obj);
        ESCH_CHECK_1(ret == ESCH_OK,
                     log, "Can't delete: obj: 0x%x", obj, ret);
        if (alloc != NULL)
        {
            ret = esch_alloc_free(alloc, obj);
            ESCH_CHECK_1(ret == ESCH_OK,
                         log, "Can't free: obj: 0x%x", obj, ret);
        }
    }
Exit:
    return ret;
}

esch_error
esch_object_delete_i(esch_object* obj, esch_bool force_delete)
{
    esch_error ret = ESCH_OK;
    esch_type* type = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_gc* gc = NULL;
    void* gc_id = NULL;
    esch_iterator iter = {0};
    esch_object* element = NULL;
    size_t count = 0;

    type  = ESCH_OBJECT_GET_TYPE(obj);
    gc_id = ESCH_OBJECT_GET_GC_ID(obj);
    alloc = ESCH_OBJECT_GET_ALLOC(obj);
    log   = ESCH_OBJECT_GET_LOG(obj);
    gc    = ESCH_OBJECT_GET_GC(obj);

    assert(type != NULL);
    assert(log != NULL);
    assert(((gc && gc_id) || (!gc && !gc_id)));

    if (ESCH_TYPE_IS_CONTAINER(type))
    {
        /* Container objects should never delete element object
         * themselves.
         */
        ret = esch_object_get_iterator_i(obj, &iter);
        ESCH_CHECK_1(ret == ESCH_OK, log,
                "Can't get iterator: obj: 0x%x", obj, ret);
        for (count = 0; iter.iterator != NULL; ++count)
        {
            ret = iter.get_value(&iter, &element);
            ESCH_CHECK_2(ret == ESCH_OK, log,
                    "Can't enumerate iterator: container: 0x%x, count:%d",
                    obj, count, ret);
            ret = do_delete(element, force_delete);
            iter.get_next(&iter);
        }
    }
    ret = do_delete(obj, force_delete);
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
        (void)esch_log_info(log, "get_iterator(): primitive type");
        ret = ESCH_ERROR_NOT_SUPPORTED;
    }
    else
    {
        ret = (ESCH_TYPE_GET_OBJECT_GET_ITERATOR(type))(obj, iter);
    }
    return ret;
}
