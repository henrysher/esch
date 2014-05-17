#include <stdio.h>
#include "esch.h"
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_vector.h"
#include "esch_string.h"
#include <wchar.h>
#include <string.h>

esch_error test_vectorBase(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_vector* vec = NULL;
    esch_string* str = NULL;
    const char* input = "hello";
    size_t length = 0;
    int i = 0;
    esch_object* obj = NULL;

    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_LOG, &log_obj);
    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_ALLOC, &alloc_obj);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);

    ret = esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 1);

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create vector", ret);
    ESCH_TEST_CHECK(vec->slots == ESCH_VECTOR_MINIMAL_INITIAL_LENGTH,
                    "Not default vector length", ret);

    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);

    /* Test length calculation */
    ret = esch_vector_get_length(vec, &length);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
    ESCH_TEST_CHECK(length == 0, "Initial vector length not 0", ret);

    for (i = 0; i < ESCH_VECTOR_MINIMAL_INITIAL_LENGTH; ++i)
    {
        esch_log_info(log, "Append object: %d", i);
        ret = esch_vector_append_object(vec, ESCH_CAST_TO_OBJECT(str));
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append object", ret);

        ret = esch_vector_get_length(vec, &length);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
        ret = ESCH_ERROR_INVALID_STATE;
        ESCH_TEST_CHECK(length == i + 1, "vector length not match", ret);
        ret = ESCH_OK;
    }
    for (; i < 2 * ESCH_VECTOR_MINIMAL_INITIAL_LENGTH; ++i)
    {
        esch_log_info(log, "Append object 2: %d", i);
        ret = esch_vector_append_object(vec, ESCH_CAST_TO_OBJECT(config));
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append object", ret);

        ret = esch_vector_get_length(vec, &length);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
        ret = ESCH_ERROR_INVALID_STATE;
        ESCH_TEST_CHECK(length == i + 1, "vector length not match", ret);
        ret = ESCH_OK;
    }
    /* Index */
    ret = esch_vector_get_object(vec, -1, (esch_object**)&obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to index element -1", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(obj == ESCH_CAST_TO_OBJECT(config),
                    "Object not match -1", ret);
    ret = ESCH_OK;

    ret = esch_vector_get_object(vec, 0, (esch_object**)&obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to index element 0", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(obj == ESCH_CAST_TO_OBJECT(str),
                    "Object not match 0", ret);
    ret = ESCH_OK;

    ret = esch_vector_get_object(vec,
            -ESCH_VECTOR_MINIMAL_INITIAL_LENGTH - 1, (esch_object**)&obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to index element -Mid", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(obj == ESCH_CAST_TO_OBJECT(str),
                    "Object not match -Mid", ret);
    ret = ESCH_OK;

    /* Clean up */
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(vec));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector object.", ret);

    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(str));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete str object.", ret);
Exit:
    /* Set it back so other objects can initialize with default value. */
    (void)esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 0);
    return ret;
}

esch_error test_vectorElementType(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_vector* vec = NULL;
    esch_string* str = NULL;
    const char* input = "hello";
    size_t length = 0;

    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_LOG, &log_obj);
    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_ALLOC, &alloc_obj);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create vector", ret);
    ESCH_TEST_CHECK(vec->slots == ESCH_VECTOR_MINIMAL_INITIAL_LENGTH,
                    "Not default vector length", ret);

    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);

    esch_log_info(log, "Append object: str");
    ret = esch_vector_append_object(vec, ESCH_CAST_TO_OBJECT(str));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append object", ret);
    ret = esch_vector_get_length(vec, &length);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(length == 1, "Initial vector length not 0", ret);
    ret = ESCH_OK;

    /* Clean up */
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(vec));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector object.", ret);

    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(str));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete str object.", ret);
Exit:
    return ret;
}

static esch_error
EnumerateAndCompare(esch_log* log, esch_vector* vec,
                    esch_string* str[], size_t len)
{
    esch_iterator iter;
    esch_value element;
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ret = esch_object_get_iterator(ESCH_CAST_TO_OBJECT(vec), &iter);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get iterator", ret);
    while (ESCH_TRUE) {
        ret = iter.get_value(&iter, &element);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element",
                        ESCH_ERROR_BAD_VALUE_TYPE);
        if (element.type == ESCH_VALUE_TYPE_END) {
            break;
        }
        ESCH_TEST_CHECK(element.type == ESCH_VALUE_TYPE_OBJECT,
                "Element type is wrong", ESCH_ERROR_BAD_VALUE_TYPE);
        ESCH_TEST_CHECK(element.val.o != NULL,
                "Element is not object", ESCH_ERROR_BAD_VALUE_TYPE);
        ESCH_TEST_CHECK(element.val.o == ESCH_CAST_TO_OBJECT(str[i]),
                "Element object is wrong", ESCH_ERROR_BAD_VALUE_TYPE);
        ret = iter.get_next(&iter);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get next element",
                        ESCH_ERROR_BAD_VALUE_TYPE);
        ++i;
    };
    ESCH_TEST_CHECK(i == len,
            "Element count is wrong", ESCH_ERROR_BAD_VALUE_TYPE);
Exit:
    return ret;
}

esch_error test_vectorIteration(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_vector* vec = NULL;
    esch_string* str[100] = {0};
    esch_vector* vec_copy = NULL;
    esch_object* vec_copy_obj = NULL;
    const char* input = "hello";
    size_t length = 0;
    size_t i = 0;

    ret = esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 1);
    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_LOG, &log_obj);
    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_ALLOC, &alloc_obj);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create vector", ret);
    ESCH_TEST_CHECK(vec->slots == ESCH_VECTOR_MINIMAL_INITIAL_LENGTH,
                    "Not default vector length", ret);

    esch_log_info(log, "Enumerate empty vector: Should do nothing.");
    ret = EnumerateAndCompare(log, vec, NULL, 0);
    ESCH_TEST_CHECK(ret == ESCH_OK,
            "Failed: enumerating 0 length", ESCH_ERROR_BAD_VALUE_TYPE);


    esch_log_info(log, "Enumerate string vector: 100 elements.");
    for (i = 0; i < 100; ++i) {
        ret = esch_string_new_from_utf8(config, input, 0, -1, &str[i]);
        ESCH_TEST_CHECK(ret == ESCH_OK && str[i] != NULL,
                "Failed to create string - begin = 0, end = -1", ret);
        ret = esch_vector_append_object(vec, ESCH_CAST_TO_OBJECT(str[i]));
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append object", ret);
        ret = esch_vector_get_length(vec, &length);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
        ret = ESCH_ERROR_INVALID_STATE;
        ESCH_TEST_CHECK(length == 1 + i, "Initial vector length", ret);
        ret = ESCH_OK;
    }
    ret = EnumerateAndCompare(log, vec, str, 100);
    ESCH_TEST_CHECK(ret == ESCH_OK,
            "Failed: enumerating 100 elements", ESCH_ERROR_BAD_VALUE_TYPE);

    esch_log_info(log, "Enumerate copied vector: 100 elements.");
    ret = esch_vector_type.type.object_copy(ESCH_CAST_TO_OBJECT(vec),
                                            &vec_copy_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Fail to copy object", ret);
    vec_copy = ESCH_CAST_FROM_OBJECT(vec_copy_obj, esch_vector);

    ESCH_TEST_CHECK(vec->begin != vec_copy->begin,
            "Copy: begin should be different", ESCH_ERROR_BAD_VALUE_TYPE);
    ESCH_TEST_CHECK(vec->next != vec_copy->next,
            "Copy: next should be different", ESCH_ERROR_BAD_VALUE_TYPE);
    ESCH_TEST_CHECK(vec->slots == vec_copy->slots,
            "Copy: elements should be same", ESCH_ERROR_BAD_VALUE_TYPE);
    ESCH_TEST_CHECK(vec->next - vec->begin == 
                    vec_copy->next - vec_copy->begin,
            "Copy: elements should be same", ESCH_ERROR_BAD_VALUE_TYPE);
    ret = EnumerateAndCompare(log, vec_copy, str, 100);
    ESCH_TEST_CHECK(ret == ESCH_OK,
            "Failed: enumerating copy", ESCH_ERROR_BAD_VALUE_TYPE);


    /* Clean up */
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(vec));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector object.", ret);

    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(vec_copy));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector copy.", ret);

    for (i = 0; i < 100; ++i) {
        ret = esch_object_delete(ESCH_CAST_TO_OBJECT(str[i]));
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete str.", ret);
    }
Exit:
    (void)esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 0);
    return ret;
}

esch_error test_vectorResizeFlag(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_vector* vec_fixed = NULL;
    esch_vector* vec_dyn = NULL;
    size_t elements = ESCH_VECTOR_MINIMAL_INITIAL_LENGTH * 3;
    size_t i = 0;
    size_t length = 0;

    ret = esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH,
                              elements);
    ret = esch_vector_new(config, &vec_fixed);
    ESCH_TEST_CHECK(ret == ESCH_OK, "fixed:Failed to create vector", ret);
    ESCH_TEST_CHECK(vec_fixed->slots == elements, "fixed:Not new size", ret);

    ret = esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 1);
    ret = esch_vector_new(config, &vec_dyn);
    ESCH_TEST_CHECK(ret == ESCH_OK, "dyn:Failed to create vector", ret);
    ESCH_TEST_CHECK(vec_dyn->slots == elements, "dyn:Not new size", ret);

    for (i = 0; i < elements; ++i) {
        ret = esch_vector_append_object(vec_fixed,
                                        ESCH_CAST_TO_OBJECT(vec_fixed));
        ESCH_TEST_CHECK(ret == ESCH_OK, "fixed:Can't append element", ret);
        ret = esch_vector_get_length(vec_fixed, &length);
        ESCH_TEST_CHECK(length == i + 1, "fixed:Can't append element",
                        ESCH_ERROR_OUT_OF_BOUND);
    }

    ret = esch_vector_append_object(vec_fixed,
                                    ESCH_CAST_TO_OBJECT(vec_fixed));
    ESCH_TEST_CHECK(ret == ESCH_ERROR_CONTAINER_FULL,
                    "fixed:(error as expected) Can't append element", ret);
    ret = esch_vector_get_length(vec_fixed, &length);
    ESCH_TEST_CHECK(length == elements, "fixed:Can't append element",
                    ESCH_ERROR_INVALID_STATE);

    for (i = 0; i < elements; ++i) {
        ret = esch_vector_append_object(vec_dyn,
                                        ESCH_CAST_TO_OBJECT(vec_dyn));
        ESCH_TEST_CHECK(ret == ESCH_OK, "dyn:Can't append element", ret);
        ret = esch_vector_get_length(vec_dyn, &length);
        ESCH_TEST_CHECK(length == i + 1, "dyn:Can't append element",
                        ESCH_ERROR_OUT_OF_BOUND);
    }
    ret = esch_vector_append_object(vec_dyn,
                                    ESCH_CAST_TO_OBJECT(vec_dyn));
    ESCH_TEST_CHECK(ret == ESCH_OK,
                    "dyn:Supposed length should be enlarged", ret);
    ret = esch_vector_get_length(vec_dyn, &length);
    ESCH_TEST_CHECK(length == elements + 1, "dyn:Suppose should append",
                    ESCH_ERROR_INVALID_STATE);

Exit:
    if (vec_fixed != NULL) {
        (void)esch_object_delete(ESCH_CAST_TO_OBJECT(vec_fixed));
    }
    if (vec_dyn != NULL) {
        (void)esch_object_delete(ESCH_CAST_TO_OBJECT(vec_dyn));
    }
    /* Set default value. */
    (void)esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 0);
    (void)esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH, 1);
    return ret;
}

esch_error test_vectorDifferentValues(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_vector* vec = NULL;
    esch_object* vec_obj = NULL;
    size_t i = 0;
    size_t len = 0;
    esch_value value;
    esch_value_type vt[5] = {
        ESCH_VALUE_TYPE_OBJECT,
        ESCH_VALUE_TYPE_INTEGER,
        ESCH_VALUE_TYPE_FLOAT,
        ESCH_VALUE_TYPE_BYTE,
        ESCH_VALUE_TYPE_UNICODE
    };
    int ival = 0;
    double fval = 0.0;
    esch_byte bval = '\0';
    esch_unicode uval = 0;
    esch_value value2;

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create vector", ret);

    esch_log_info(g_testLog, "Case 1: Append different values.");
    ret = esch_vector_append_object(vec, ESCH_CAST_TO_OBJECT(vec));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't append object", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 1, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_append_integer(vec, 100);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't append integer", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 2, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_append_float(vec, 100.20);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't append float", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 3, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_append_byte(vec, 'a');
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't append byte", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 4, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_append_unicode(vec, (esch_unicode)0x4f60);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't append unicode", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 5, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    value.type = ESCH_VALUE_TYPE_UNKNOWN;
    value.val.o = NULL;
    ret = esch_vector_append_value(vec, &value);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER,
                    "UNKNOWN should be rejected", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 5, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    value.type = ESCH_VALUE_TYPE_END;
    value.val.o = NULL;
    ret = esch_vector_append_value(vec, &value);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER,
                    "END should be rejected", ret);
    ret = esch_vector_get_length(vec, &len);
    ESCH_TEST_CHECK(ret == ESCH_OK && len == 5, "Length not expand",
                    ESCH_ERROR_INVALID_STATE);

    esch_log_info(g_testLog, "Case 2: Get values");
    ret = esch_vector_get_object(vec, 0, &vec_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 0",
                    ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(vec_obj == ESCH_CAST_TO_OBJECT(vec),
                    "Can't get element 0",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_integer(vec, 1, &ival);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 1",
                    ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(ival == 100, "Bad value: element 1",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_float(vec, 2, &fval);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 2",
                    ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK((fval - 100.20 < 0.00001 || 100.20 - fval < 0.0001),
                    "Bad value: element 2", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_byte(vec, 3, &bval);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 3",
                    ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(bval == 'a', "Bad value: element 3",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_unicode(vec, 4, &uval);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 4",
                    ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(uval == (esch_unicode)0x4f60,
                    "Bad value: element 4", ESCH_ERROR_INVALID_STATE);

    esch_log_info(g_testLog, "Case 3: Enumerate via iterator");
    esch_iterator iter;
    ret = esch_object_get_iterator(ESCH_CAST_TO_OBJECT(vec), &iter);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get iterator", ret);
    for (i = 0; i < 5; ++i) {
        ret = iter.get_value(&iter, &value);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get value", ret);
        ESCH_TEST_CHECK(value.type == vt[i],
                        "Value type is not correct",
                        ESCH_ERROR_INVALID_STATE);
        ret = iter.get_next(&iter);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get next value", ret);
    }
    ret = iter.get_value(&iter, &value);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get last value", ret);
    ESCH_TEST_CHECK(value.type == ESCH_VALUE_TYPE_END,
                    "Last value is not END tag", ESCH_ERROR_INVALID_STATE);
    ret = iter.get_value(&iter, &value);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get last value 2", ret);
    ESCH_TEST_CHECK(value.type == ESCH_VALUE_TYPE_END,
                    "Last value is not END tag 2",
                    ESCH_ERROR_INVALID_STATE);
    esch_log_info(g_testLog, "Case 4: Get generic values.");
    for (i = 1; i < 5; ++i) {
        ret = esch_vector_get_value(vec, i,
                                    ESCH_VALUE_TYPE_OBJECT, &value);
        ESCH_TEST_CHECK(ret == ESCH_ERROR_BAD_VALUE_TYPE,
                        "Query with bad type should be rejected", ret);
    }
    for (i = 0; i < 5; ++i) {
        ret = esch_vector_get_value(vec, i, vt[i], &value);
        ESCH_TEST_CHECK(ret == ESCH_OK,
                        "Query with correct type should accepted", ret);
    }
    for (i = 0; i < 5; ++i) {
        ret = esch_vector_get_value(vec, i, ESCH_VALUE_TYPE_END, &value);
        ESCH_TEST_CHECK(ret == ESCH_OK,
                        "Query with END type should accepted",
                        ESCH_ERROR_INVALID_STATE);
        ESCH_TEST_CHECK(value.type == vt[i],
                        "Query with END type and get correct type",
                        ESCH_ERROR_INVALID_STATE);
    }
    esch_log_info(g_testLog, "Case 5: Set generic values.");
    for (i = 0; i < 5; ++i) {
        value.type = ESCH_VALUE_TYPE_INTEGER;
        value.val.i = i + 1000;
        ret = esch_vector_set_value(vec, i, &value);
        ESCH_TEST_CHECK(ret == ESCH_OK,
                        "Set to a new value should be accepted", ret);
        value.type = ESCH_VALUE_TYPE_UNKNOWN;
        ret = esch_vector_get_value(vec, i,
                                    ESCH_VALUE_TYPE_INTEGER, &value);
        ESCH_TEST_CHECK(ret == ESCH_OK,
                        "Query with new type should accept", ret);
        ESCH_TEST_CHECK(value.type == ESCH_VALUE_TYPE_INTEGER,
                        "Query with new type should get correct value",
                        ESCH_ERROR_INVALID_STATE);
    }
    esch_log_info(g_testLog, "Case 6: Set specific values.");
    ret = esch_vector_set_object(vec, 0, vec_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't set element 0",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_integer(vec, 1, ival);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 1",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_float(vec, 2, fval);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 2",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_byte(vec, 3, bval);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 3",
                    ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_unicode(vec, 4, uval);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Can't get element 4",
                    ESCH_ERROR_INVALID_STATE);
    for (i = 0; i < 5; ++i) {
        ret = esch_vector_get_value(vec, i, vt[i], &value);
        ESCH_TEST_CHECK(ret == ESCH_OK,
                        "Query again with specific type (set back)",
                        ret);
    }
    esch_log_info(g_testLog, "Case 7: Out of bound (set)");
    ret = esch_vector_set_object(vec, 1000, vec_obj);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't set element 0", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_integer(vec, 1000, ival);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't set element 1", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_float(vec, -10001, fval);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't set element 2", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_byte(vec, 1234, bval);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't set element 3", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_set_unicode(vec, 5432, uval);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't set element 4", ESCH_ERROR_INVALID_STATE);

    value2.type = ESCH_VALUE_TYPE_INTEGER;
    value2.val.i = 12345;
    ret = esch_vector_set_value(vec, 54321, &value2);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't set element 5", ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(value2.type == ESCH_VALUE_TYPE_INTEGER,
                    "Can't set element 5", ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(value2.val.i == 12345,
                    "Can't set element 5", ESCH_ERROR_INVALID_STATE);


    esch_log_info(g_testLog, "Case 8: Out of bound (get)");
    ret = esch_vector_get_object(vec, 1009, &vec_obj);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't get element 0", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_integer(vec, 1000, &ival);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't get element 1", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_float(vec, -10001, &fval);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't get element 2", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_byte(vec, 1234, &bval);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't get element 3", ESCH_ERROR_INVALID_STATE);

    ret = esch_vector_get_unicode(vec, 5432, &uval);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't get element 4", ESCH_ERROR_INVALID_STATE);

    value2.type = ESCH_VALUE_TYPE_INTEGER;
    value2.val.i = 12345;
    ret = esch_vector_get_value(vec, 54321, ESCH_VALUE_TYPE_END, &value2);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OUT_OF_BOUND,
                    "Can't get element 5", ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(value2.type == ESCH_VALUE_TYPE_INTEGER,
                    "Can't get element 5", ESCH_ERROR_INVALID_STATE);
    ESCH_TEST_CHECK(value2.val.i == 12345,
                    "Can't get element 5", ESCH_ERROR_INVALID_STATE);
    ret = ESCH_OK;

Exit:
    if (vec != NULL) {
        esch_object_delete(ESCH_CAST_TO_OBJECT(vec));
    }
    return ret;
}
