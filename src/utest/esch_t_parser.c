#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"

esch_error test_parser()
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    esch_log* do_nothing = NULL;
    esch_alloc* alloc = NULL;
    esch_config* config = NULL;
    esch_parser* parser = NULL;

    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    ret = esch_config_new(&config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create config", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_ALLOC, alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_LOG, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:NULL", ret);

    ret = esch_alloc_new_c_default(config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_parser_new(config, &parser);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER, "Failed to create parser - no log", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_ALLOC, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:NULL", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_LOG, log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    ret = esch_parser_new(config, &parser);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER, "Failed to create parser - no alloc", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_ALLOC, alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_LOG, log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    ret = esch_parser_new(config, &parser);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create parser", ret);

    ret = esch_parser_delete(parser);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete parser", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);
Exit:
    return ret;
}
