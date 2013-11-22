/* vim:ft=c expandtab tw=72 sw=4
 */
#include <esch.h>
#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"

esch_error test_config()
{
    esch_error ret = ESCH_OK;
    esch_config* config = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_alloc* alloc_get = NULL;
    esch_log* log_get = NULL;
    esch_log* do_nothing = NULL;
    esch_object* unknown = NULL;

    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif
    
    ret = esch_config_new(&config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create config", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG,
                              (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set log", ret);

    ret = esch_alloc_new_c_default(config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC,
                              (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set alloc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG,
                              (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set log", ret);

    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_ALLOC,
                              (esch_object**)&alloc_get);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get alloc", ret);
    ESCH_TEST_CHECK(alloc == alloc_get, "Received bad alloc", ret);

    ret = esch_config_get_obj(config, ESCH_CONFIG_KEY_LOG,
                              (esch_object**)&log_get);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to get log", ret);
    ESCH_TEST_CHECK(log == log_get, "Received bad alloc", ret);

    ret = esch_config_set_obj(config, "nothing:unknown", NULL);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_NOT_FOUND, "Can't set unknown", ret);
    ret = esch_config_get_obj(config, "nothing:unknown", &unknown);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_NOT_FOUND, "Can't get unknown", ret);


    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete log", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config", ret);
Exit:
    return ret;
}
