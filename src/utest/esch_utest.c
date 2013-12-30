#include "esch.h"
#include "esch_utest.h"
#include <stdio.h>

esch_log* g_testLog = NULL;

int main(int argc, char* argv[])
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_config* config = NULL;
    esch_log* testLog = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_object* config_obj = NULL;

    ret = esch_log_new_printf(NULL, &testLog);

    g_testLog = testLog;
    ret = esch_alloc_new_c_default(NULL, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "main:Can't create alloc", ret);
    ret = esch_config_new(testLog, alloc, &config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "main:Can't create config", ret);

    ret = esch_object_cast_to_object(alloc, &alloc_obj);
    ret = esch_object_cast_to_object(testLog, &log_obj);
    ret = esch_object_cast_to_object(config, &config_obj);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, alloc_obj);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, log_obj);

    /* Really run test */
    ret = test_AllocCreateDeleteCDefault(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_AllocCreateDeleteCDefault() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_AllocCreateDeleteCDefault()");

    /*
    ret = test_config();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_config() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_config()");

    ret = test_vectorBase();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_vectorBase() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_vectorBase()");

    ret = test_vectorElementType();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_vectorElementType() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_vectorElementType()");

    ret = test_vectorDeleteElement();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_vectorDeleteElement() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_vectorDeleteElement()");

    ret = test_integer();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_integer() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_integer()");

    ret = test_string();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_string() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_string()");

    ret = test_identifier();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_identifier() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_identifier()");
    */

    esch_log_info(g_testLog, "All passed.");
Exit:
    (void)esch_object_delete(config_obj);
    (void)esch_object_delete(alloc_obj);
    (void)esch_object_delete(log_obj);
    return ret;
}
