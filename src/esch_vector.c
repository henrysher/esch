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

const size_t ESCH_VECTOR_MINIMAL_INITIAL_LENGTH = 32;
const size_t ESCH_VECTOR_MAX_LENGTH = (INT_MAX / sizeof(esch_vector*));

size_t
esch_adjust_length_exp(size_t length)
{
    size_t bit = 0;
    size_t shift = 0;
    for (bit = length; bit != 0; (bit >>= 1), ++shift);
    return (size_t)(1 << shift);
}

/**
 * Create a new vector.
 * @param config Configuration. Give element type and initial length.
 * @param vec Returned vector object.
 * @return Return code. ESCH_OK if success.
 */
esch_error
esch_vector_new(esch_config* config, esch_vector** vec)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_vector* new_vec = NULL;
    int initial_length = 0;
    int delete_element = ESCH_FALSE;
    esch_object** array = NULL;
    esch_type element_type = ESCH_TYPE_UNKNOWN;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));

    alloc = ESCH_INTERNAL_CONFIG_GET_ALLOC(config);
    log = ESCH_INTERNAL_CONFIG_GET_LOG(config);
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);

    ret = esch_alloc_malloc(alloc, sizeof(esch_vector), (void**)&new_vec);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log, "Failed to malloc vector", ret);

    ret = esch_config_get_int(config,
                              ESCH_CONFIG_KEY_VECTOR_ELEMENT_TYPE,
                              (int*)&element_type);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to get element type", ret);

    initial_length = ESCH_INTERNAL_CONFIG_GET_VECOTR_INITIAL_LENGTH(config);
    ESCH_CHECK_PARAM_PUBLIC(initial_length >= 0 &&
                            initial_length <= ESCH_VECTOR_MAX_LENGTH);
    /* Adjust initial_length to a proper length for allocation. */
    initial_length = (initial_length <= ESCH_VECTOR_MINIMAL_INITIAL_LENGTH?
                      ESCH_VECTOR_MINIMAL_INITIAL_LENGTH:
                      esch_adjust_length_exp((size_t)initial_length));

    delete_element = ESCH_INTERNAL_CONFIG_GET_VECOTR_DELETE_ELEMENT(config);

    ret = esch_alloc_malloc(alloc, sizeof(esch_object*) * (initial_length + 1),
                            (void**)&array);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to get initial length", ret);

    (void)esch_log_info(log,
            "Vector element type = %d, initial length = %d",
            element_type, initial_length);

    ESCH_GET_VERSION(new_vec) = ESCH_VERSION;
    ESCH_GET_TYPE(new_vec) = ESCH_TYPE_VECTOR;
    ESCH_GET_ALLOC(new_vec) = alloc;
    ESCH_GET_LOG(new_vec) = log;
    new_vec->slots = (size_t)initial_length;
    new_vec->element_type = element_type;
    new_vec->begin = array;
    new_vec->next = &(new_vec->begin[0]);
    new_vec->end = (new_vec->begin + new_vec->slots);
    new_vec->delete_element = delete_element;
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
        (void)esch_vector_delete(new_vec);
    }
    return ret;
}

/**
 * Delete vector object. May also delete element if specified by config.
 * @param vec Given vector object.
 * @return Return code. ESCH_OK if success.
 */
esch_error
esch_vector_delete(esch_vector* vec)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    if (vec == NULL)
    {
        goto Exit;
    }
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));
    if (vec->delete_element)
    {
        esch_object** element = vec->begin;

        esch_log_info(ESCH_GET_LOG(vec),
                "vector = 0x%x, delete_element = true", vec);
        for (; element != vec->end; ++element)
        {
            ret = esch_object_delete((*element));
        }
    }
    alloc = ESCH_GET_ALLOC(vec);
    log = ESCH_GET_LOG(vec);
    assert(alloc != NULL);
    assert(log != NULL);

    ret = esch_alloc_free(alloc, vec->begin);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to free vector array", ret);
    ret = esch_alloc_free(alloc, vec);
    ESCH_CHECK(ret == ESCH_OK, log, "Failed to free vector", ret);
Exit:
    return ret;
}

/**
 * Append an object at the end of vector.
 * @param vec Given vector object.
 * @param data A new object. Can't be NULL.
 * @return Return code. ESCH_OK if success.
 */
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

    if (vec->element_type != ESCH_TYPE_UNKNOWN)
    {
        if (vec->element_type != ESCH_GET_TYPE(data))
        {
            esch_log_info(ESCH_GET_LOG(vec),
                    "Data type mismatch: given 0x%x, expect 0x%x",
                    ESCH_GET_TYPE(data), vec->element_type);
            ret = ESCH_ERROR_BAD_VALUE_TYPE;
            goto Exit;
        }
    }

    if (vec->next == vec->end) /* vector buffer is full */
    {
        esch_object** existing = NULL;
        esch_log_info(ESCH_GET_LOG(vec), "Enlarge vector 0x%x", vec);
        new_slots = vec->slots * 2;
        alloc = ESCH_GET_ALLOC(vec);
        assert(alloc != NULL);
        ret = esch_alloc_malloc(alloc,
                                sizeof(esch_object*) * (new_slots + 1),
                                (void**)&new_array);
        ESCH_CHECK(ret == ESCH_OK, vec, "Failed to reallocate vec", ret);
        memcpy(new_array, vec->begin, sizeof(esch_object*) * vec->slots);
        existing = vec->begin;

        vec->begin = new_array;
        vec->next = vec->begin + vec->slots;
        vec->end = vec->begin + new_slots;
        vec->slots = new_slots;

        ret = esch_alloc_free(alloc, existing);
        ESCH_CHECK(ret == ESCH_OK, vec, "Failed to free vec array", ret);
    }
    slot = vec->next;
    ++vec->next;
    (*slot) = data;
Exit:
    return ret;
}

/**
 * Get length of given vector.
 * @param vec Given vector object.
 * @param length Length of given vector.
 * @return Return code. ESCH_OK if success.
 */
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

/**
 * Get element by array.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param obj Returned object. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
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

/**
 * Get element type of given vector.
 * @param vec Given vector object.
 * @param element_type Returned type of given vector.
 * @return Return code. ESCH_OK if success.
 */
esch_error
esch_vector_get_element_type(esch_vector* vec, esch_type* element_type)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(vec != NULL);
    ESCH_CHECK_PARAM_PUBLIC(element_type != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_VECTOR(vec));
    (*element_type) = vec->element_type;
Exit:
    return ret;
}
