#include <stdio.h>
#include "esch.h"
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_vector.h"
#include "esch_string.h"
#include <wchar.h>
#include <string.h>

esch_error test_vector()
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
    ESCH_TEST_CHECK(vec->length == ESCH_VECTOR_MINIMAL_INITIAL_LENGTH,
                    "Not default vector length", ret);

    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);

    ret = esch_string_delete(str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);

    ret = esch_vector_delete(vec, ESCH_TRUE);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete vector object.", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);


Exit:
    return ret;
}
