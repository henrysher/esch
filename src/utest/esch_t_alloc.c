/* vim:ft=c expandtab tw=72 sw=4
 */
#include "esch.h"
#include "esch_utest.h"
#include "esch_debug.h"
#include <stdio.h>

esch_error test_AllocCreateDeleteCDefault(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_alloc* alloc_with_config = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* alloc_obj2 = NULL;
    char* str = NULL;
    char* str2 = NULL;

    esch_log_info(g_testLog, "Case 1: Create alloc with or without config");
    ret = esch_alloc_new_c_default(NULL, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);
    ret = esch_alloc_new_c_default(config, &alloc_with_config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_object_cast_to_object(alloc, &alloc_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to cast alloc object.", ret);
    ret = esch_object_cast_to_object(alloc_with_config, &alloc_obj2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to cast alloc2 object.", ret);

    esch_log_info(g_testLog, "Case 2: Malloc buffer.");
    ret = esch_alloc_realloc(alloc, NULL, sizeof(char) * 100, (void**)(&str));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to allocate memory", ret);
    ESCH_TEST_CHECK(str[0] == '\0', "Memory is not cleared", ret);
    str[0] = '1';
    str[1] = '2';

    esch_log_info(g_testLog, "Case 3: Realloc buffer.");
    ret = esch_alloc_realloc(alloc, str, sizeof(char) * 300, (void**)(&str));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to realloc memory", ret);
    ESCH_TEST_CHECK(str[0] == '1' && str[1] == '2',
                    "Buffer is cleared unexpected.", ret);

    esch_log_info(g_testLog, "Case 3: Free with wrong alloc.");
    ret = esch_alloc_realloc(alloc_with_config, NULL, sizeof(char) * 200,
                            (void**)(&str2));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to allocate memory 2", ret);
    ret = esch_alloc_free(alloc, str2);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_STATE, "Failed to free memory", ret);

    esch_log_info(g_testLog, "Case 4: Memory leak detection.");
    ret = esch_object_delete(alloc_obj);
    ESCH_TEST_CHECK(ret != ESCH_OK,
            "Expect memory leak detected but not happening",
            ESCH_ERROR_INVALID_STATE);
    (void)esch_log_info(g_testLog,
        "[PASSED] Memory leak detect. Now do right thing to free buffer.");

    esch_log_info(g_testLog, "Case 5: Free buffer and delete object.");
    ret = esch_alloc_free(alloc_with_config, str2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to free memory str2", ret);
    ret = esch_alloc_free(alloc, str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to free memory str", ret);

    ret = esch_object_delete(alloc_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc_obj.", ret);

    ret = esch_object_delete(alloc_obj2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc_obj2.", ret);
Exit:
    return ret;
}

