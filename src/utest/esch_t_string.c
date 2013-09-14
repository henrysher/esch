#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_string.h"
#include <wchar.h>
#include <string.h>

int test_string()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;
    esch_config config = { ESCH_TYPE_CONFIG, NULL, NULL };
    esch_string* str = NULL;
    /* Chinese: hello, UTF-8 and Unicode */
    char input[] = { 0xe4, 0xbd, 0xa0, 0xe5, 0xa5, 0xbd, 0x0 };
    esch_unicode output[] = { 0x4F60, 0x597D, 0 };

    config.log = g_testLog;
    ret = esch_alloc_new_c_default(&config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    config.alloc = alloc;
    config.log = NULL;

    ret = esch_string_new_from_utf8(&config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && str == NULL,
            "Failed to create string - no log", ret);

    config.alloc = NULL;
    config.log = g_testLog;
    ret = esch_string_new_from_utf8(&config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && str == NULL,
            "Failed to create string - no alloc", ret);

    config.alloc = alloc;
    config.log = g_testLog;
    /* Check if a full string can be parsed */
    str = NULL;
    ret = esch_string_new_from_utf8(&config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(wcslen(str->unicode) == 2,
            "Unicode conversion error: length != 2 - begin = 3, end = 6", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(wcscmp(str->unicode, output) == 0,
            "Unicode conversion error - begin = 0, end = -1", ret);
    ret = esch_string_delete(str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    esch_log_info(g_testLog, "[PASSED] Full length test.");

    /* Check if a sub string can be parsed */
    str = NULL;
    ret = esch_string_new_from_utf8(&config, input, 3, 6, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 3, end = 6", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(wcslen(str->unicode) == 1,
            "Unicode conversion error: length != 1 - begin = 3, end = 6", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(str->unicode[0] == output[1],
            "Unicode conversion error: bad content - begin = 3, end = 6", ret);
    ret = esch_string_delete(str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    esch_log_info(g_testLog, "[PASSED] Substring test.");

    /* Check if a bad string can be detected. */
    str = NULL;
    ret = esch_string_new_from_utf8(&config, input, 1, 5, &str);
    ESCH_TEST_CHECK(ret != ESCH_OK && str == NULL,
            "Unexpected: create bad string - begin = 1, end = 5", ret);
    esch_log_info(g_testLog, "[PASSED] Bad string test.");

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);
Exit:
    return ret;
}
