#include <stdio.h>
#include "esch.h"
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_vector.h"
#include "esch_string.h"
#include <wchar.h>
#include <string.h>

esch_error test_vectorBase()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;
    esch_config* config = NULL;
    esch_vector* vec = NULL;
    esch_string* str = NULL;
    esch_log* log = NULL;
    esch_log* do_nothing = NULL;
    const char* input = "hello";
    size_t length = 0;
    int i = 0;
    esch_object* obj = NULL;

    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    ret = esch_config_new(&config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create config", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:log", ret);

    ret = esch_alloc_new_c_default(config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:NULL", ret);

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && vec == NULL,
            "Failed to create vector - no log", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:NULL", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:log", ret);

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && vec == NULL,
            "Failed to create vector - no alloc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:NULL", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:log", ret);

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
        ret = esch_vector_append(vec, (esch_object*)str);
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
        ret = esch_vector_append(vec, (esch_object*)config);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append object", ret);

        ret = esch_vector_get_length(vec, &length);
        ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
        ret = ESCH_ERROR_INVALID_STATE;
        ESCH_TEST_CHECK(length == i + 1, "vector length not match", ret);
        ret = ESCH_OK;
    }
    /* Index */
    ret = esch_vector_get_data(vec, -1, (esch_object**)&obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to index element -1", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(obj == (esch_object*)config, "Object not match -1", ret);
    ret = ESCH_OK;

    ret = esch_vector_get_data(vec, 0, (esch_object**)&obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to index element 0", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(obj == (esch_object*)str, "Object not match 0", ret);
    ret = ESCH_OK;

    ret = esch_vector_get_data(vec,
            -ESCH_VECTOR_MINIMAL_INITIAL_LENGTH - 1, (esch_object**)&obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to index element -Mid", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(obj == (esch_object*)str, "Object not match -Mid", ret);
    ret = ESCH_OK;

    /* Clean up */
    ret = esch_vector_delete(vec, ESCH_FALSE);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector object.", ret);

    ret = esch_string_delete(str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete str object.", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);


Exit:
    return ret;
}

esch_error test_vectorElementType()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;
    esch_config* config = NULL;
    esch_vector* vec = NULL;
    esch_string* str = NULL;
    esch_log* log = NULL;
    esch_log* do_nothing = NULL;
    const char* input = "hello";
    size_t length = 0;

    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    ret = esch_config_new(&config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create config", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:log", ret);

    ret = esch_alloc_new_c_default(config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:NULL", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:log", ret);
    ret = esch_config_set_int(config,
            ESCH_CONFIG_KEY_VECTOR_ELEMENT_TYPE, ESCH_TYPE_STRING);
    ESCH_TEST_CHECK(ret == ESCH_OK,
                    "Failed to set config:vector:type", ret);

    ret = esch_vector_new(config, &vec);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create vector", ret);
    ESCH_TEST_CHECK(vec->slots == ESCH_VECTOR_MINIMAL_INITIAL_LENGTH,
                    "Not default vector length", ret);

    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);

    esch_log_info(log, "Append object: str");
    ret = esch_vector_append(vec, (esch_object*)str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append object", ret);
    ret = esch_vector_get_length(vec, &length);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(length == 1, "Initial vector length not 0", ret);
    ret = ESCH_OK;

    esch_log_info(log, "Append object: config");
    ret = esch_vector_append(vec, (esch_object*)config);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_BAD_VALUE_TYPE,
                    "Supposed fail to append object: Value type", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ret = esch_vector_get_length(vec, &length);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get vector length", ret);
    ESCH_TEST_CHECK(length == 1, "Initial vector length not 0", ret);
    ret = ESCH_OK;

    /* Clean up */
    ret = esch_vector_delete(vec, ESCH_FALSE);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector object.", ret);

    ret = esch_string_delete(str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete str object.", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);


Exit:
    return ret;
}
