#include "esch_utest.h"

esch_error
test_list()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_parser* parser = NULL;
    esch_config* config = NULL;
    esch_list* lst = NULL;
    esch_log* do_nothing = NULL;
    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    ret = esch_alloc_new_c_default(g_testLog, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_config_new(alloc, &config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create config", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_ALLOC_KEY, alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_LOG_KEY, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:NULL", ret);

    ret = esch_list_new(config, 0, &lst);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && lst == NULL,
                    "Failed to create log - no log", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_ALLOC_KEY, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:NULL", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_LOG_KEY, log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    ret = esch_list_new(config, 0, &lst);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && lst == NULL,
                    "Failed to create log - no alloc", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_ALLOC_KEY, alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_LOG_KEY, log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    ret = esch_list_new(config, 0, &lst);
    ESCH_TEST_CHECK(ret == ESCH_OK && lst != NULL,
                    "Failed to create log - no alloc", ret);

    ret = esch_list_delete(lst, ESCH_FALSE);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete list.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

Exit:
    return ret;
}
