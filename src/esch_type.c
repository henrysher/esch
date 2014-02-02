#include "esch.h"
#include "esch_type.h"
#include "esch_object.h"
#include "esch_config.h"
#include "esch_alloc.h"
#include "esch_debug.h"
#include <assert.h>

static esch_error
esch_type_new_s(esch_config* config, esch_object** obj);
static esch_error
esch_type_delete_s(esch_object* obj);

/* ----------------------------------------------------------------- */
/*                         Public functions                          */
/* ----------------------------------------------------------------- */
struct esch_builtin_type esch_meta_type =
{
    {
        &(esch_meta_type.type),
        NULL,
        &(esch_log_do_nothing.log),
        NULL,
        NULL,
    },
    {
        sizeof(esch_type),
        ESCH_VERSION,
        esch_type_new_s,
        esch_type_delete_s,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    }
};

esch_error
esch_type_new(esch_config* config, esch_type** type)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);

    ret = esch_type_new_i(config, type);
Exit:
    return ret;
}

esch_error
esch_type_set_object_size(esch_type* type, size_t size)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(size >= sizeof(esch_object));

    ESCH_TYPE_GET_OBJECT_SIZE(type) = size;
Exit:
    return ret;
}

esch_error
esch_type_set_object_new(esch_type* type, esch_object_new_f object_new)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(object_new != NULL);

    ESCH_TYPE_GET_OBJECT_NEW(type) = object_new;
Exit:
    return ret;
}

esch_error
esch_type_set_object_delete(esch_type* type,
                            esch_object_destructor_f object_destructor)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(object_destructor != NULL);

    ESCH_TYPE_GET_OBJECT_DESTRUCTOR(type) = object_destructor;
Exit:
    return ret;
}

esch_error
esch_type_set_object_copy(esch_type* type,
                          esch_object_copy_f object_copy)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(object_copy != NULL);

    ESCH_TYPE_GET_OBJECT_COPY(type) = object_copy;
Exit:
    return ret;
}

esch_error
esch_type_set_object_to_string(esch_type* type,
                               esch_object_to_string_f object_to_string)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(object_to_string != NULL);

    ESCH_TYPE_GET_OBJECT_TO_STRING(type) = object_to_string;
Exit:
    return ret;
}

esch_error
esch_type_set_object_get_doc(esch_type* type,
                             esch_object_get_doc_f object_get_doc)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(object_get_doc != NULL);

    ESCH_TYPE_GET_OBJECT_GET_DOC(type) = object_get_doc;
Exit:
    return ret;
}

esch_error
esch_type_set_object_get_iterator(esch_type* type,
                      esch_object_get_iterator_f object_get_iterator)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(object_get_iterator != NULL);

    ESCH_TYPE_GET_OBJECT_GET_ITERATOR(type) = object_get_iterator;
Exit:
    return ret;
}

esch_error
esch_type_is_valid_type(esch_type* type, esch_bool* valid)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(valid != NULL);

    (*valid) = ESCH_IS_VALID_TYPE(type);
Exit:
    return ret;
}

/* ----------------------------------------------------------------- */
/*                       Internal functions                          */
/* ----------------------------------------------------------------- */
esch_error
esch_type_new_i(esch_config* config, esch_type** type)
{
    esch_error ret = ESCH_OK;
    esch_object* new_obj = NULL;
    esch_type* new_type = NULL;
    esch_log* log = NULL;
    esch_object* log_obj = NULL;

    assert(type != NULL);
    assert(config != NULL);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    assert(log_obj != NULL);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);

    ret = esch_object_new_i(config, &(esch_meta_type.type), &new_obj);
    ESCH_CHECK(new_type != NULL, log, "Can't alloc new type.", ret);
    new_type = ESCH_CAST_FROM_OBJECT(new_obj, esch_type);

    ESCH_TYPE_GET_OBJECT_SIZE(new_type)          = sizeof(esch_type);
    ESCH_TYPE_GET_VERSION(new_type)              = ESCH_VERSION;
    ESCH_TYPE_GET_OBJECT_NEW(new_type)           = NULL;
    ESCH_TYPE_GET_OBJECT_DESTRUCTOR(new_type)    = NULL;
    ESCH_TYPE_GET_OBJECT_COPY(new_type) =
                                      esch_type_default_non_copiable;
    ESCH_TYPE_GET_OBJECT_TO_STRING(new_type) =
                                      esch_type_default_no_string_form;
    ESCH_TYPE_GET_OBJECT_GET_DOC(new_type) =
                                      esch_type_default_no_doc;
    ESCH_TYPE_GET_OBJECT_GET_ITERATOR(new_type) =
                                      esch_type_default_no_iterator;

    (*type) = new_type;
    new_type = NULL;
Exit:
    if (new_type)
    {
        esch_object_delete_i(new_obj, ESCH_FALSE);
    }
    return ret;
}

esch_error
esch_type_delete_i(esch_type* type)
{
    (void)type;
    return ESCH_OK;
}

static esch_error
esch_type_new_s(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_type* type = NULL;

    ret = esch_type_new(config, &type);
    if (ret != ESCH_OK)
    {
        (*obj) = ESCH_CAST_TO_OBJECT(type);
    }
Exit:
    return ret;
}

static esch_error
esch_type_delete_s(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_type* type = NULL;
    if (type == NULL)
    {
        return ret;
    }
    obj = ESCH_CAST_TO_OBJECT(type);
    type = ESCH_CAST_FROM_OBJECT(obj, esch_type);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_TYPE(type));

    ret = esch_type_delete_i(type);
Exit:
    return ret;
}

esch_error
esch_type_default_non_copiable(esch_object* from, esch_object** to)
{
    (void)from;
    (void)to;
    return ESCH_ERROR_NOT_SUPPORTED;
}
esch_error
esch_type_default_no_string_form(esch_object* obj, esch_string** str)
{
    (void)obj;
    (void)str;
    return ESCH_ERROR_NOT_SUPPORTED;
}
esch_error
esch_type_default_no_doc(esch_object* obj, esch_string** str)
{
    (void)obj;
    (void)str;
    return ESCH_ERROR_NOT_SUPPORTED;
}
esch_error
esch_type_default_no_iterator(esch_object* obj, esch_iterator* iter)
{
    (void)obj;
    (void)iter;
    return ESCH_ERROR_NOT_SUPPORTED;
}
