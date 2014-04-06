#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_string.h"
#include <wchar.h>
#include <string.h>
#include "esch_config.h"
#include "esch_alloc.h"

static size_t unicode_len_i(esch_unicode* ustr)
{
    size_t i = 0;
    while((*ustr) != '\0')
    {
        ++ustr;
        ++i;
    }
    return i;
}

static int
unicode_compare_i(esch_unicode* left, esch_unicode* right)
{
    int ret = 0;
    while((*left) != '\0' && (*right) != '\0')
    {
        if ((*left) == (*right))
        {
            ++left;
            ++right;
            continue;
        }
        else
        {
            break;
        }
    }
    if ((*left) == '\0' && (*right) == '\0')
    {
        return 0;
    }
    else if ((*left) < (*right))
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

esch_error test_string(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_string* str = NULL;
    esch_string* str_dup = NULL;
    esch_object* str_obj = NULL;
    /* Chinese: hello, UTF-8 and Unicode */
    char input[] = { 0xe4, 0xbd, 0xa0, 0xe5, 0xa5, 0xbd, 0x0 };
    esch_unicode output[] = { 0x4F60, 0x597D, 0 };
    size_t input_len = 6;
    size_t output_len = 2;

    esch_log_info(g_testLog, "Case 1: Create a string from UTF-8 input.");
    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(unicode_len_i(str->unicode) == 2,
            "Unicode conversion error: length != 2 - begin = 3, end = 6", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(unicode_compare_i(str->unicode, output) == 0,
            "Unicode conversion error - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(unicode_compare_i(str->unicode,
                        esch_string_get_unicode_ref(str)) == 0,
            "Internal Unicode different - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(strcmp(str->utf8, esch_string_get_utf8_ref(str)) == 0,
            "Internal UTF-8 different - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(str->unicode == esch_string_get_unicode_ref(str),
            "Internal Unicode ref different - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(str->utf8 == esch_string_get_utf8_ref(str),
            "Internal UTF-8 ref different - begin = 0, end = -1", ret);

    ESCH_TEST_CHECK(esch_string_get_utf8_length(str) == input_len,
            "Internal UTF-8 bad length - begin = 0, end = -1", ret);
    ESCH_TEST_CHECK(esch_string_get_unicode_length(str) == output_len,
            "Internal UTF-8 bad length - begin = 0, end = -1", ret);

    ret = esch_object_cast_to_object(str, &str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "failed to cast to object", ret);
    ret = esch_object_delete(str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    esch_log_info(g_testLog, "[PASSED] Full length test.");

    esch_log_info(g_testLog, "Case 2: Substring test.");
    str = NULL;
    ret = esch_string_new_from_utf8(config, input, 3, 6, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 3, end = 6", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(str->unicode[0] == output[1],
            "Unicode conversion error: bad content - begin = 3, end = 6", ret);
    ESCH_TEST_CHECK(str->unicode[1] == 0,
            "Unicode conversion error: bad content - longer string", ret);

    ret = esch_object_cast_to_object(str, &str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "failed to cast to object", ret);
    ret = esch_object_delete(str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    esch_log_info(g_testLog, "[PASSED] Substring test.");

    esch_log_info(g_testLog, "Case 3: Bad string test.");
    str = NULL;
    ret = esch_string_new_from_utf8(config, input, 1, 5, &str);
    ESCH_TEST_CHECK(ret != ESCH_OK && str == NULL,
            "Unexpected: create bad string - begin = 1, end = 5", ret);
    esch_log_info(g_testLog,
                  "Error (as expected) is detected. Nothing is allocated.");
    ret = ESCH_OK;
    esch_log_info(g_testLog, "[PASSED] Bad string test.");

    esch_log_info(g_testLog, "Case 4: Duplicate existing string");
    ret = esch_string_new_from_utf8(config, input, 0, -1, &str);
    ESCH_TEST_CHECK(ret == ESCH_OK && str != NULL,
            "Failed to create string - begin = 0, end = -1", ret);
    ret = esch_string_duplicate(str, &str_dup);
    ESCH_TEST_CHECK(ret == ESCH_OK && str_dup != NULL,
            "Failed to create string - begin = 0, end = -1", ret);
    ret = esch_object_cast_to_object(str, &str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "failed to cast to object", ret);
    ret = esch_object_delete(str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    ret = esch_object_cast_to_object(str_dup, &str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "failed to cast to object", ret);
    ret = esch_object_delete(str_obj);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete string", ret);
    esch_log_info(g_testLog, "[PASSED] Duplicate existing string");

Exit:
    return ret;
}

esch_error test_identifier()
{
    esch_error ret = ESCH_OK;
    int val = 0;
    /* Chinese: hello, Unicode */
    esch_unicode hello[] = { 0x4F60, 0x597D, 0 };
    /* Chinese: CJK Compat Ideograph, 'you' in Chinese */
    esch_unicode lo_str[] = { 0x2F804, 0xF9DC, 0 };
    /* Chinese: CJK Compat Ideograph, 'you' in Chinese, then a number */
    esch_unicode nl_str[] = { 0x2F804, 0x303A, 0 };

    esch_unicode identifier[] = { '1', 'A', 'B', 'C', '0' };

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
    val = esch_unicode_string_is_valid_identifier(identifier);
    ESCH_TEST_CHECK(!val, "'1ABC' should not be identifier",
            ESCH_ERROR_INVALID_PARAMETER);
    val = esch_unicode_string_is_valid_identifier(nl_str);
    ESCH_TEST_CHECK(val, "Nl string should be identifier",
            ESCH_ERROR_INVALID_PARAMETER);
Exit:
    return ret;
}
