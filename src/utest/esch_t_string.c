#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_string.h"
#include <wchar.h>
#include <string.h>

esch_error test_string()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;
    esch_config* config = NULL;
    esch_string* str = NULL;
    esch_log* log = NULL;
    esch_log* do_nothing = NULL;
    /* Chinese: hello, UTF-8 and Unicode */
    char input[] = { 0xe4, 0xbd, 0xa0, 0xe5, 0xa5, 0xbd, 0x0 };
    esch_unicode output[] = { 0x4F60, 0x597D, 0 };

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


    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && str == NULL,
            "Failed to create string - no log", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_ALLOC, NULL);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:NULL", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_LOG, log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && str == NULL,
            "Failed to create string - no alloc", ret);

    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_ALLOC, alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_data(config, ESCH_CONFIG_KEY_LOG, log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    /* Check if a full string can be parsed */
    str = NULL;
    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(wcslen(str->unicode) == 2,
            "Unicode conversion error: length != 2 - begin = 3, end = 6", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(wcscmp(str->unicode, output) == 0,
            "Unicode conversion error - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(wcscmp(str->unicode, esch_string_get_unicode_ref(str)) == 0,
            "Internal Unicode different - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(strcmp(str->utf8, esch_string_get_utf8_ref(str)) == 0,
            "Internal UTF-8 different - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(str->unicode == esch_string_get_unicode_ref(str),
            "Internal Unicode ref different - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(str->utf8 == esch_string_get_utf8_ref(str),
            "Internal UTF-8 ref different - begin = 0, end = -1", ret);

    ESCH_TEST_CHECK(esch_string_get_utf8_length(str) == strlen(input),
            "Internal UTF-8 bad length - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(esch_string_get_unicode_length(str) == wcslen(output),
            "Internal UTF-8 bad length - begin = 0, end = -1", ret);

    ret = esch_string_delete(str);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    esch_log_info(g_testLog, "[PASSED] Full length test.");

    /* Check if a sub string can be parsed */
    str = NULL;
    ret = esch_string_new_from_utf8(config, input, 3, 6, &str);
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
    ret = esch_string_new_from_utf8(config, input, 1, 5, &str);
    ESCH_TEST_CHECK(ret != ESCH_OK && str == NULL,
            "Unexpected: create bad string - begin = 1, end = 5", ret);
    esch_log_info(g_testLog, "[PASSED] Bad string test.");

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);
Exit:
    return ret;
}

esch_error test_identifier()
{
    esch_error ret = ESCH_OK;
    int val = 0;
    esch_log* log = NULL;
    /* Chinese: hello, Unicode */
    esch_unicode hello[] = { 0x4F60, 0x597D, 0 };
    /* Chinese: CJK Compat Ideograph, 'you' in Chinese */
    esch_unicode lo_str[] = { 0x2F804, 0xF9DC, 0 };
    /* Chinese: CJK Compat Ideograph, 'you' in Chinese, then a number */
    esch_unicode nl_str[] = { 0x2F804, 0x303A, 0 };

    val = esch_unicode_is_ascii('A');
    ESCH_TEST_CHECK(val, "'A' should be digit", ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_is_ascii(128);
    ESCH_TEST_CHECK(!val, "123 should not be a digit", ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_is_digit('0');
    ESCH_TEST_CHECK(val, "0 should be a digit", ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_is_digit('9');
    ESCH_TEST_CHECK(val, "9 should be a digit", ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_is_digit('A');
    ESCH_TEST_CHECK(!val, "A should not be a digit", ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_string_is_valid_identifier(hello);
    ESCH_TEST_CHECK(!val, "'Hello(Chinese)' Should not be identifier",
            ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_string_is_valid_identifier(lo_str);
    ESCH_TEST_CHECK(val, "Lo string should be identifier",
            ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_string_is_valid_identifier(L"1ABC");
    ESCH_TEST_CHECK(!val, "'1ABC' should not be identifier",
            ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_string_is_valid_identifier(nl_str);
    ESCH_TEST_CHECK(val, "Nl string should be identifier",
            ESCH_ERROR_INVALID_PARAMETER);
Exit:
    return ret;
}
