/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include <limits.h>
#include <assert.h>
#include <string.h>
#include "esch_vector.h"
#include "esch_debug.h"
#include "esch_config.h"
#include "esch_object.h"
#include "esch_gc.h"
#include "esch_value.h"

const size_t ESCH_VECTOR_MINIMAL_INITIAL_LENGTH = 31;
const size_t ESCH_VECTOR_MAX_LENGTH = (INT_MAX / sizeof(esch_value));

static esch_error
esch_vector_new_i(esch_config* config, esch_vector** vec);
static esch_error
esch_vector_destructor_i(esch_object* obj);
static esch_error
esch_vector_copy_object_i(esch_object* input, esch_object** output);
static esch_error
esch_vector_get_iterator_i(esch_object* obj, esch_iterator* iter);
esch_error
esch_vector_new_default_as_object_i(esch_config* config, esch_object** vec);

struct esch_builtin_type esch_vector_type = 
{
    {
        &(esch_meta_type.type),
        NULL, /* No alloc */
        &(esch_log_do_nothing.log),
        NULL, /* Non-GC object */
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_vector),
        esch_vector_new_default_as_object_i,
        esch_vector_destructor_i,
        esch_vector_copy_object_i,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_vector_get_iterator_i
    },
};

esch_error
esch_vector_new(esch_config* config, esch_vector** vec)
{
    esch_error ret = ESCH_OK;
    esch_object* alloc_obj = NULL;
    esch_alloc* alloc = NULL;
    esch_object* log_obj = NULL;
    esch_log* log = NULL;

    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));

    alloc_obj = ESCH_CONFIG_GET_ALLOC(config);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    ESCH_CHECK_PARAM_PUBLIC(alloc_obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(log_obj != NULL);

    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_ALLOC(alloc));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LOG(log));

    ret = esch_vector_new_i(config, vec);
Exit:
    return ret;
}
esch_error
esch_vector_new_default_as_object_i(esch_config* config, esch_object** vec)
{
    esch_error ret = ESCH_OK;
    esch_vector* new_vec = NULL;

    ret = esch_vector_new_i(config, &new_vec);
    if (ret != ESCH_OK)
    {
        (*vec) = ESCH_CAST_TO_OBJECT(new_vec);
    }

    return ret;
}

esch_error
esch_vector_new_i(esch_config* config, esch_vector** vec)
{
    esch_error ret = ESCH_OK;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* vec_obj = NULL;
    esch_vector* new_vec = NULL;
    int initial_length = 0;
    esch_value* array = NULL;

    ESCH_CHECK_PARAM_INTERNAL(config != NULL);
    ESCH_CHECK_PARAM_INTERNAL(vec != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_CONFIG(config));

    alloc_obj = ESCH_CONFIG_GET_ALLOC(config);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    ESCH_CHECK_PARAM_INTERNAL(alloc_obj != NULL);
    ESCH_CHECK_PARAM_INTERNAL(log_obj != NULL);

    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_ALLOC(alloc));
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_LOG(log));

    initial_length = ESCH_CONFIG_GET_VECOTR_LENGTH(config);
    if (initial_length < ESCH_VECTOR_MINIMAL_INITIAL_LENGTH) {
        initial_length = ESCH_VECTOR_MINIMAL_INITIAL_LENGTH;
    }

    ret = esch_alloc_realloc(alloc, NULL,
                             sizeof(esch_value) * (initial_length + 1),
                             (void**)&array);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to allocate array", ret);

    ret = esch_object_new_i(config, &(esch_vector_type.type), &vec_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to new vector object", ret);
    new_vec = ESCH_CAST_FROM_OBJECT(vec_obj, esch_vector);
    ESCH_ASSERT(ESCH_IS_VALID_TYPE(&(esch_vector_type.type)));

    new_vec->enlarge = (ESCH_CONFIG_GET_VECTOR_ENLARGE(config)?
                        ESCH_TRUE: ESCH_FALSE);
    new_vec->slots = (size_t)initial_length;
    new_vec->begin = array;
    new_vec->next = &(new_vec->begin[0]);
    array = NULL;
    (*vec) = new_vec;
    new_vec = NULL;
Exit:
    if (array != NULL)
    {
        (void)esch_alloc_free(alloc, array);
    }
    if (new_vec != NULL)
    {
        (void)esch_object_delete(vec_obj);
    }
    return ret;
}

esch_error
esch_vector_get_length(esch_vector* vec, size_t* length)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(length != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));
    (*length) = (vec->next - vec->begin);
Exit:
    return ret;
}

/*
 * -----------------------------------------------------------------
 * Internal functions. Used only within vector
 * -----------------------------------------------------------------
 */
static esch_error
esch_vector_destructor_i(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_vector* vec = NULL;
    esch_alloc* alloc = NULL;

    ESCH_CHECK_PARAM_INTERNAL(obj != NULL);
    alloc = ESCH_OBJECT_GET_ALLOC(obj);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
    vec = ESCH_CAST_FROM_OBJECT(obj, esch_vector);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(vec));

    ret = esch_alloc_free(alloc, vec->begin);
    vec->begin = NULL;
    vec->next = NULL;
    vec->slots = 0;
    /* Object ref is deleted. */
Exit:
    return ret;
}

static esch_error
esch_vector_copy_object_i(esch_object* input, esch_object** output)
{
    /* TODO
     * I left here for a question here: Because we didn't really keep
     * all config here, it's difficult to keep exactly the same
     * configuration when creating a new object.
     */
    esch_error ret = ESCH_OK;
    esch_vector* new_vec = NULL;
    esch_vector* vec = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_gc* gc = NULL;
    esch_config* config = NULL;

    ESCH_CHECK_PARAM_INTERNAL(input != NULL);
    ESCH_CHECK_PARAM_INTERNAL(output != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_OBJECT(input));
    vec = ESCH_CAST_FROM_OBJECT(input, esch_vector);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(vec));

    alloc = ESCH_OBJECT_GET_ALLOC(input);
    log = ESCH_OBJECT_GET_LOG(input);
    gc = ESCH_OBJECT_GET_GC(input);

    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_ALLOC(alloc));
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_LOG(log));
    ESCH_CHECK_PARAM_INTERNAL((gc == NULL || ESCH_IS_VALID_GC(gc)));

    ret = esch_config_new(log, alloc, &config);
    ESCH_CHECK(ret == ESCH_OK, log, "vec:Can't create config", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC,
                              ESCH_CAST_TO_OBJECT(alloc));
    ESCH_CHECK(ret == ESCH_OK, log, "vec:Can't insert alloc", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG,
                              ESCH_CAST_TO_OBJECT(log));
    ESCH_CHECK(ret == ESCH_OK, log, "vec:Can't insert log", ret);
    if (gc != NULL)
    {
        ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                                  ESCH_CAST_TO_OBJECT(gc));
        ESCH_CHECK(ret == ESCH_OK, log, "vec:Can't insert gc", ret);
    }
    ret = esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH,
                              vec->slots);
    ESCH_CHECK(ret == ESCH_OK, log, "vec:Can't set initial length", ret);

    ret = esch_vector_new_i(config, &new_vec);
    ESCH_CHECK(ret == ESCH_OK, log, "vec:Can't create new vector", ret);
    /*
     * Copy connects so two vectors contains same objects.
     * NOTE: We don't do real deep copy.
     */
    memcpy(new_vec->begin, vec->begin, sizeof(esch_value) * (vec->slots));
    new_vec->next = new_vec->begin + (vec->next - vec->begin);

    (*output) = ESCH_CAST_TO_OBJECT(new_vec);

Exit:
    esch_object_delete_i(ESCH_CAST_TO_OBJECT(config));
    return ret;
}
static esch_error
esch_vector_iterator_get_value_i(esch_iterator* iter, esch_value* value)
{
    esch_error ret = ESCH_OK;
    size_t offset = 0;
    esch_vector* vec = NULL;
    
    ESCH_CHECK_PARAM_PUBLIC(iter != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value != NULL);
    ESCH_CHECK_PARAM_PUBLIC(iter->container != NULL);
    vec = ESCH_CAST_FROM_OBJECT(iter->container, esch_vector);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));

    offset = (size_t)(iter->iterator);
    if (offset >= vec->next - vec->begin) {
        value->type = ESCH_VALUE_TYPE_END;
        value->val.o = 0;
    } else {
        (*value) = vec->begin[offset];
    }
Exit:
    return ret;
}

static esch_error
esch_vector_get_next_i(esch_iterator* iter)
{
    esch_error ret = ESCH_OK;
    size_t offset = 0;
    ESCH_CHECK_PARAM_PUBLIC(iter != NULL);
    ESCH_CHECK_PARAM_INTERNAL(iter->container != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(
                ESCH_CAST_FROM_OBJECT(iter->container, esch_vector)));
    iter->iterator = (void*)(((size_t)iter->iterator) + 1);
Exit:
    return ret;
}

static esch_error
esch_vector_get_iterator_i(esch_object* obj, esch_iterator* iter)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(iter != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);

    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_OBJECT(obj));
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(
                ESCH_CAST_FROM_OBJECT(obj, esch_vector)));

    log = ESCH_OBJECT_GET_LOG(obj);
    ESCH_CHECK_PARAM_INTERNAL(log != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_LOG(log));

    iter->container = obj;
    iter->iterator = (void*)0;
    iter->get_value = esch_vector_iterator_get_value_i;
    iter->get_next = esch_vector_get_next_i;
Exit:
    return ret;
}

/*
 * =================================================================
 * Getter & setter
 * =================================================================
 */
esch_error
esch_vector_append_value_i(esch_vector* vec, esch_value* value)
{
    esch_error ret = ESCH_OK;
    size_t new_slots = 0;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_value* new_array = NULL;
    esch_value* slot = NULL;
    ESCH_CHECK_PARAM_INTERNAL(vec != NULL);
    ESCH_CHECK_PARAM_INTERNAL(value != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(vec));

    ESCH_CHECK_PARAM_INTERNAL(value->type > ESCH_VALUE_TYPE_UNKNOWN);
    ESCH_CHECK_PARAM_INTERNAL(value->type < ESCH_VALUE_TYPE_END);

    alloc = ESCH_OBJECT_GET_ALLOC(ESCH_CAST_TO_OBJECT(vec));
    log = ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(vec));

    if (vec->next == vec->begin + vec->slots) {
        /* vector buffer is full */
        if (vec->enlarge) {
            new_slots = vec->slots * 2;
            ESCH_ASSERT(alloc != NULL);
            ESCH_ASSERT(ESCH_IS_VALID_ALLOC(alloc));

            ret = esch_alloc_realloc(alloc, vec->begin,
                             sizeof(esch_value) * (new_slots + 1),
                             (void**)&new_array);
            ESCH_CHECK(ret == ESCH_OK, log,
                       "vec:append:Failed to reallocate vec", ret);

            vec->begin = new_array;
            vec->next = vec->begin + vec->slots;
            vec->slots = new_slots;
        } else {
            /* Enlarge is by default not allowed. */
            esch_log_error(log, "vec:append:Enlarge is disabled.");
            ret = ESCH_ERROR_CONTAINER_FULL;
            goto Exit;
        }
    }
    slot = vec->next;
    vec->next += 1;
    ret = esch_value_check[value->type](value);
    esch_value_assign[ret == ESCH_OK? 0: 1](slot, value);
Exit:
    return ret;
}

static esch_error
esch_vector_get_value_i(esch_vector* vec, int index,
                        esch_value_type expected_type,
                        esch_value* value)
{
    esch_error ret = ESCH_OK;
    int real_index = 0;
    esch_log* log = NULL;

    ESCH_CHECK_PARAM_INTERNAL(vec != NULL);
    ESCH_CHECK_PARAM_INTERNAL(value != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(vec));
    ESCH_CHECK_PARAM_INTERNAL(expected_type > ESCH_VALUE_TYPE_UNKNOWN);
    ESCH_CHECK_PARAM_INTERNAL(expected_type <= ESCH_VALUE_TYPE_END);
    log = ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(vec));
    real_index = index;
    if (real_index < 0) {
        if (real_index >= -((int)(vec->next - vec->begin))) {
            real_index += (vec->next - vec->begin);
            ESCH_ASSERT(real_index >= 0);
        } else {
            esch_log_info(log, "vec:obj = 0x%x, idx = %d", vec, index);
            ret = ESCH_ERROR_OUT_OF_BOUND;
            goto Exit;
        }
    }
    if (real_index >= 0 && vec->next - vec->begin > real_index) {
        esch_value_type real_type = vec->begin[real_index].type;
        ret = esch_value_type_check[expected_type][real_type];
        esch_value_assign[ret == ESCH_OK? 0: 1](value,
                                             &(vec->begin[real_index]));
    } else {
        esch_log_info(log, "vec:obj = 0x%x, idx = %d", vec, index);
        ret = ESCH_ERROR_OUT_OF_BOUND;
    }
Exit:
    return ret;
}
static esch_error
esch_vector_set_value_i(esch_vector* vec, int index, esch_value* value)
{
    esch_error ret = ESCH_OK;
    int real_index = 0;
    esch_log* log = NULL;

    ESCH_CHECK_PARAM_INTERNAL(vec != NULL);
    ESCH_CHECK_PARAM_INTERNAL(value != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_VECTOR(vec));
    log = ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(vec));
    real_index = index;
    if (real_index < 0) {
        if (real_index >= -((int)(vec->next - vec->begin))) {
            real_index += (vec->next - vec->begin);
            ESCH_ASSERT(real_index >= 0);
        } else {
            esch_log_info(log, "vec:obj = 0x%x, idx = %d", vec, index);
            ret = ESCH_ERROR_OUT_OF_BOUND;
            goto Exit;
        }
    }
    if (real_index >= 0 && vec->next - vec->begin > real_index) {
        esch_value_type real_type = vec->begin[real_index].type;
        ESCH_CHECK_PARAM_INTERNAL(real_type > ESCH_VALUE_TYPE_UNKNOWN);
        ESCH_CHECK_PARAM_INTERNAL(real_type < ESCH_VALUE_TYPE_END);
        ret = esch_value_check[real_type](value);
        esch_value_assign[ret == ESCH_OK? 0: 1](&(vec->begin[real_index]),
                                             value);
    } else {
        esch_log_info(log, "vec:obj = 0x%x, idx = %d", vec, index);
        ret = ESCH_ERROR_OUT_OF_BOUND;
    }
Exit:
    return ret;
}

esch_error
esch_vector_append_value(esch_vector* vec, esch_value* value)
{
    esch_error ret = ESCH_OK;

    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value->type > ESCH_VALUE_TYPE_UNKNOWN);
    ESCH_CHECK_PARAM_PUBLIC(value->type < ESCH_VALUE_TYPE_END);

    ret = esch_vector_append_value_i(vec, value);
Exit:
    return ret;
}

esch_error
esch_vector_get_value(esch_vector* vec, int index,
                      esch_value_type expected_type, esch_value* value)
{
    esch_error ret = ESCH_OK;

    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value != NULL);
    ESCH_CHECK_PARAM_PUBLIC(expected_type > ESCH_VALUE_TYPE_UNKNOWN);
    ESCH_CHECK_PARAM_PUBLIC(expected_type <= ESCH_VALUE_TYPE_END);

    ret = esch_vector_get_value_i(vec, index, expected_type, value);
Exit:
    return ret;
}
esch_error
esch_vector_set_value(esch_vector* vec, int index, esch_value* value)
{
    esch_error ret = ESCH_OK;

    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value->type > ESCH_VALUE_TYPE_UNKNOWN);
    ESCH_CHECK_PARAM_PUBLIC(value->type < ESCH_VALUE_TYPE_END);

    ret = esch_vector_set_value_i(vec, index, value);
Exit:
    return ret;
}

/*
 * Auto generated code to define getter and setter.
 */
#define ESCH_VECTOR_DEF_GET(suffix, vt, ot) \
esch_error \
esch_vector_get_##suffix(esch_vector* vec, int index, ot* value) \
{ \
    esch_error ret = ESCH_OK; \
    esch_value slot_value; \
    ret = esch_vector_get_value(vec, index, vt, &slot_value); \
    esch_value_get_##suffix((void*)value, &slot_value); \
    return ret; \
}

#define ESCH_VECTOR_DEF_SET(suffix, vt, ot) \
esch_error \
esch_vector_set_##suffix(esch_vector* vec, int index, ot value) \
{ \
    esch_error ret = ESCH_OK; \
    esch_value new_value; \
    new_value.type = vt; \
    esch_value_set_##suffix(&new_value, (void*)(&value)); \
    ret = esch_vector_set_value(vec, index, &new_value); \
    return ret; \
}

#define ESCH_VECTOR_DEF_APPEND(suffix, ot, vt, field) \
esch_error \
esch_vector_append_##suffix(esch_vector* vec, ot value) \
{ \
    esch_error ret = ESCH_OK; \
    esch_value new_value; \
    new_value.type = vt; \
    new_value.val. field = value; \
    return esch_vector_append_value(vec, &new_value); \
}

ESCH_VECTOR_DEF_GET(byte, ESCH_VALUE_TYPE_BYTE, esch_byte);
ESCH_VECTOR_DEF_GET(unicode, ESCH_VALUE_TYPE_UNICODE, esch_unicode);
ESCH_VECTOR_DEF_GET(integer, ESCH_VALUE_TYPE_INTEGER, int);
ESCH_VECTOR_DEF_GET(float, ESCH_VALUE_TYPE_FLOAT, double);
ESCH_VECTOR_DEF_GET(object, ESCH_VALUE_TYPE_OBJECT, esch_object*);

ESCH_VECTOR_DEF_SET(byte, ESCH_VALUE_TYPE_BYTE, esch_byte);
ESCH_VECTOR_DEF_SET(unicode, ESCH_VALUE_TYPE_UNICODE, esch_unicode);
ESCH_VECTOR_DEF_SET(integer, ESCH_VALUE_TYPE_INTEGER, int);
ESCH_VECTOR_DEF_SET(float, ESCH_VALUE_TYPE_FLOAT, double);

ESCH_VECTOR_DEF_APPEND(byte, esch_byte, ESCH_VALUE_TYPE_BYTE, b);
ESCH_VECTOR_DEF_APPEND(unicode, esch_unicode, ESCH_VALUE_TYPE_UNICODE, u);
ESCH_VECTOR_DEF_APPEND(integer, int, ESCH_VALUE_TYPE_INTEGER, i);
ESCH_VECTOR_DEF_APPEND(float, double, ESCH_VALUE_TYPE_FLOAT, f);

/* NOTE: object setter/appender can't be automatically generated,
 * because we want to add an additional check to make sure the value
 * won't be set as NULL. */

esch_error
esch_vector_set_object(esch_vector* vec, int index, esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_value new_value;
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    new_value.type = ESCH_VALUE_TYPE_OBJECT;
    new_value.val.o = obj;
    ret = esch_vector_set_value(vec, index, &new_value);
Exit:
    return ret;
}
esch_error
esch_vector_append_object(esch_vector* vec, esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_value new_value;
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    new_value.type = ESCH_VALUE_TYPE_OBJECT;
    new_value.val.o = obj;
    ret = esch_vector_append_value(vec, &new_value);
Exit:
    return ret;
}

