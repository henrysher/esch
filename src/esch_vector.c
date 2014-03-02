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

const size_t ESCH_VECTOR_MINIMAL_INITIAL_LENGTH = 31;
const size_t ESCH_VECTOR_MAX_LENGTH = (INT_MAX / sizeof(esch_vector*));

static size_t
esch_adjust_length_exp(size_t length);
static esch_error
esch_vector_new_as_object(esch_config* config, esch_object** obj);
static esch_error
esch_vector_destructor(esch_object* obj);
static esch_error
esch_vector_copy_object(esch_object* input, esch_object* output);
static esch_error
esch_vector_get_iterator(esch_object* obj, esch_iterator* iter);

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
        esch_vecotr_new_as_object,
        esch_vector_destructor,
        esch_vector_copy_object, /* vector.copy */
        esch_type_default_no_string_form, /* String.toString() */
        esch_type_default_no_doc,
        esch_vector_get_iterator
    },
};

esch_error
esch_vector_new(esch_config* config, esch_vector** vec)
{
    esch_error ret = ESCH_OK;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* vec_obj = NULL;
    esch_vector* new_vec = NULL;
    int initial_length = ESCH_VECTOR_MINIMAL_INITIAL_LENGTH;
    int delete_element = ESCH_FALSE;
    esch_object** array = NULL;
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

    ret = esch_alloc_realloc(alloc, NULL,
                             sizeof(esch_object*) * (initial_length + 1),
                             (void**)&array);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to allocate array", ret);

    ret = esch_object_new_i(config, &(esch_vector_type.type), &vec_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to new vector object", ret);
    new_vec = ESCH_CAST_TO_OBJECT(vec_obj);

    new_vec->slots = (size_t)initial_length;
    new_vec->begin = array;
    new_vec->next = &(new_vec->begin[0]);
    new_vec->end = (new_vec->begin + new_vec->slots);
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
esch_vector_append(esch_vector* vec, esch_object* data)
{
    esch_error ret = ESCH_OK;
    size_t new_slots = 0;
    esch_alloc* alloc = NULL;
    esch_object** new_array = NULL;
    esch_object** slot = NULL;
    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(data != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));


    if (vec->next == vec->end) /* vector buffer is full */
    {
        esch_object** existing = NULL;
        esch_log_info(ESCH_GET_LOG(vec), "Enlarge vector 0x%x", vec);
        new_slots = vec->slots * 2;

        alloc = ESCH_OBJECT_GET_ALLOC(ESCH_CAST_TO_OBJECT(vect));
        ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
        ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_ALLOC(alloc));

        ret = esch_alloc_realloc(alloc, vec->begin,
                                sizeof(esch_object*) * (new_slots + 1),
                                (void**)&new_array);
        ESCH_CHECK(ret == ESCH_OK, vec, "Failed to reallocate vec", ret);

        vec->begin = new_array;
        vec->next = vec->begin + vec->slots;
        vec->end = vec->begin + new_slots;
        vec->slots = new_slots;
    }
    slot = vec->next;
    ++vec->next;
    (*slot) = data;
Exit:
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

esch_error
esch_vector_get_data(esch_vector* vec, int index, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    int real_index = 0;
    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));
    real_index = index;
    if (real_index < 0)
    {
        real_index += (vec->next - vec->begin);
    }
    if (real_index >= 0 && vec->next - vec->begin > real_index)
    {
        (*obj) = vec->begin[real_index];
    }
    else
    {
        esch_log_info(ESCH_GET_LOG(vec),
                "vector = 0x%x, invalid index = %d", vec, index);
        ret = ESCH_ERROR_OUT_OF_BOUND;
    }
Exit:
    return ret;
}

/*
 * -----------------------------------------------------------------
 * Internal functions. Used only within vector
 * -----------------------------------------------------------------
 */
static size_t
esch_adjust_length_exp(size_t length)
{
    size_t bit = 0;
    size_t shift = 0;
    for (bit = length; bit != 0; (bit >>= 1), ++shift);
    return (size_t)(1 << shift);
}

static esch_error
esch_vector_new_as_object(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_vector* vec = NULL;

    ret = esch_vector_new(config, &vec);
    if (ret == ESCH_OK)
    {
        (*obj) = ESCH_CAST_TO_OBJECT(vec);
    }
    return ret;
}

static esch_error
esch_vector_destructor(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_vector* vec = NULL;
    esch_alloc* alloc = NULL;

    ESCH_CHECK_PARAM_INTERNAL(obj != NULL);
    alloc = ESCH_OBJECT_GET_ALLOC(obj);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
    vec = ESCH_CAST_FROM_OBJECT(obj, esch_vector);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));

    ret = esch_alloc_free(alloc, vec->begin);
    vec->begin = NULL;
    vec->end = NULL;
    vec->next = NULL;
    vec->slots = 0;
Exit:
    return ret;
}

static esch_error
esch_vector_copy_object(esch_object* input, esch_object** output)
{
    esch_error ret = ESCH_OK;
    esch_object* vec_obj = NULL;
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
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));

    alloc = ESCH_OBJECT_GET_ALLOC(input);
    log = ESCH_OBJECT_GET_LOG(input);
    gc = ESCH_OBJECT_GET_GC(input);

    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_ALLOC(alloc));
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_LOG(log));
    ESCH_CHECK_PARAM_INTERNAL((gc == NULL || ESCH_IS_VALID_GC(gc)));

    ret = esch_config_new(log, alloc, &config);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't create config", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC,
                              ESCH_CAST_TO_OBJECT(alloc));
    ESCH_CHECK(ret == ESCH_OK, log, "Can't insert alloc", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG,
                              ESCH_CAST_TO_OBJECT(log));
    ESCH_CHECK(ret == ESCH_OK, log, "Can't insert log", ret);
    if (gc != NULL)
    {
        ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                                  ESCH_CAST_TO_OBJECT(gc));
        ESCH_CHECK(ret == ESCH_OK, log, "Can't insert gc", ret);
    }

    ret = esch_object_new_i(config, &(esch_vector_type.type), &vec_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't create new vector", ret);
    vec_obj = ESCH_CAST_FROM_OBJECT(vec_obj, esch_vector);

    if (vec->slots <= new_vec->slots)
    {
        memcpy(new_vec->begin, vec->begin,
               sizeof(esch_object*) * (vec->slots));
        new_vec->slots = vec->slots;
        new_vec->end = (new_vec->begin + new_vec->slots);
    }
Exit:
    return ret;
}
static esch_error
esch_vector_get_iterator(esch_object* obj, esch_iterator* iter);

