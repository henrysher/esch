/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch_string.h"
#include "esch_log.h"
#include "esch_debug.h"
#include "esch_config.h"
#include <assert.h>
#include <string.h>

enum esch_unicode_parse_state
{
    ESCH_WRONG_BYTE = 0,
    ESCH_BEGIN_BYTE,
    ESCH_ONE_BYTE_FIRST_BYTE,
    ESCH_TWO_BYTES_FIRST_BYTE,
    ESCH_TWO_BYTES_SECOND_BYTE,
    ESCH_THREE_BYTES_FIRST_BYTE,
    ESCH_THREE_BYTES_SECOND_BYTE,
    ESCH_THREE_BYTES_THIRD_BYTE,
    ESCH_FOUR_BYTES_FIRST_BYTE,
    ESCH_FOUR_BYTES_SECOND_BYTE,
    ESCH_FOUR_BYTES_THIRD_BYTE,
    ESCH_FOUR_BYTES_FOURTH_BYTE,
};

static int
utf8_get_unicode_len(char* utf8, int begin, int end, int* bad_index)
{
    int len = 0;
    int idx = begin;
    unsigned char ch = '\0';
    enum esch_unicode_parse_state state = ESCH_BEGIN_BYTE;
    /*
     * UTF-8 format: http://www.fileformat.info/info/unicode/utf8.htm
     * 0xxxxxxx
     * 110xxxxx 10xxxxxx
     * 1110xxxx 10xxxxxx 10xxxxxx
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     */
    for (idx = begin; idx < end && state != ESCH_WRONG_BYTE; ++idx)
    {
        ch = (unsigned char)utf8[idx];
        if ((ch >> 7) == 0)
        {
            state = ESCH_ONE_BYTE_FIRST_BYTE;
            len += 1;
        }
        else if ((ch >> 5) == 0x6)
        {
            state = ESCH_TWO_BYTES_FIRST_BYTE;
            len += 1;
        }
        else if ((ch >> 4) == 0xE)
        {
            state = ESCH_THREE_BYTES_FIRST_BYTE;
            len += 1;
        }
        else if ((ch >> 3) == 0x1E)
        {
            state = ESCH_FOUR_BYTES_FIRST_BYTE;
            len += 1;
        }
        else if ((ch >> 6) == 0x2)
        {
            switch (state)
            {
                case ESCH_TWO_BYTES_FIRST_BYTE:
                    state = ESCH_TWO_BYTES_SECOND_BYTE;
                    break;
                case ESCH_THREE_BYTES_FIRST_BYTE:
                    state = ESCH_THREE_BYTES_SECOND_BYTE;
                    break;
                case ESCH_THREE_BYTES_SECOND_BYTE:
                    state = ESCH_THREE_BYTES_THIRD_BYTE;
                    break;
                case ESCH_FOUR_BYTES_FIRST_BYTE:
                    state = ESCH_FOUR_BYTES_SECOND_BYTE;
                    break;
                case ESCH_FOUR_BYTES_THIRD_BYTE:
                    state = ESCH_FOUR_BYTES_FOURTH_BYTE;
                    break;
                case ESCH_TWO_BYTES_SECOND_BYTE:
                case ESCH_THREE_BYTES_THIRD_BYTE:
                case ESCH_FOUR_BYTES_FOURTH_BYTE:
                default:
                    state = ESCH_WRONG_BYTE;
                    (*bad_index) = idx;
                    break;
            }
        }
    }
    return (state == ESCH_WRONG_BYTE? -1: len);
}
static void
write_unicode_from_utf8(char* utf8, int begin, int end, esch_unicode* unicode)
{
    int utf8_idx = begin;
    int unicode_idx = 0;
    unsigned char utf8_ch = '\0';
    enum esch_unicode_parse_state state = ESCH_BEGIN_BYTE;
    /*
     * UTF-8 format: http://www.fileformat.info/info/unicode/utf8.htm
     * 0xxxxxxx
     * 110xxxxx 10xxxxxx
     * 1110xxxx 10xxxxxx 10xxxxxx
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     */
    for (utf8_idx = begin, unicode_idx = 0; utf8_idx < end; ++utf8_idx)
    {
        utf8_ch = (unsigned char)utf8[utf8_idx];
        if ((utf8_ch >> 7) == 0)
        {
            state = ESCH_ONE_BYTE_FIRST_BYTE;
            unicode[unicode_idx] = (utf8_ch & 0x7F);
            ++unicode_idx;
        }
        else if ((utf8_ch >> 5) == 0x6)
        {
            state = ESCH_TWO_BYTES_FIRST_BYTE;
            unicode[unicode_idx] = (utf8_ch & 0x1F);
        }
        else if ((utf8_ch >> 4) == 0xE)
        {
            state = ESCH_THREE_BYTES_FIRST_BYTE;
            unicode[unicode_idx] = (utf8_ch & 0xF);
        }
        else if ((utf8_ch >> 3) == 0x1E)
        {
            state = ESCH_FOUR_BYTES_FIRST_BYTE;
            unicode[unicode_idx] = (utf8_ch & 0x7);
        }
        else if ((utf8_ch >> 6) == 0x2)
        {
            switch (state)
            {
                case ESCH_TWO_BYTES_FIRST_BYTE:
                    state = ESCH_TWO_BYTES_SECOND_BYTE;
                    unicode[unicode_idx] <<= 6;
                    unicode[unicode_idx] += (utf8_ch & 0x3F);
                    ++unicode_idx;
                    break;
                case ESCH_THREE_BYTES_FIRST_BYTE:
                    state = ESCH_THREE_BYTES_SECOND_BYTE;
                    unicode[unicode_idx] <<= 6;
                    unicode[unicode_idx] += (utf8_ch & 0x3F);
                    break;
                case ESCH_THREE_BYTES_SECOND_BYTE:
                    state = ESCH_THREE_BYTES_THIRD_BYTE;
                    unicode[unicode_idx] <<= 6;
                    unicode[unicode_idx] += (utf8_ch & 0x3F);
                    ++unicode_idx;
                    break;
                case ESCH_FOUR_BYTES_FIRST_BYTE:
                    state = ESCH_FOUR_BYTES_SECOND_BYTE;
                    unicode[unicode_idx] <<= 6;
                    unicode[unicode_idx] += (utf8_ch & 0x3F);
                    break;
                case ESCH_FOUR_BYTES_THIRD_BYTE:
                    state = ESCH_FOUR_BYTES_FOURTH_BYTE;
                    unicode[unicode_idx] <<= 6;
                    unicode[unicode_idx] += (utf8_ch & 0x3F);
                    ++unicode_idx;
                    break;
                case ESCH_TWO_BYTES_SECOND_BYTE:
                case ESCH_THREE_BYTES_THIRD_BYTE:
                case ESCH_FOUR_BYTES_FOURTH_BYTE:
                default:
                    /* Impossible to happen: we've got correct
                     * calcaulation. */
                    assert(0);
                    state = ESCH_WRONG_BYTE;
                    break;
            }
        }
    }
    assert(state != ESCH_WRONG_BYTE);
    unicode[unicode_idx] = L'\0';
    return;
}

static esch_error
decode_utf8(char* utf8, int begin, int end, 
            esch_config* config, esch_unicode** str, size_t* unicode_len)
{
    esch_error ret = ESCH_OK;
    int len = 0;
    int bad_index;
    int buf_size = 0;
    unsigned char* ptr = (unsigned char*)utf8;
    esch_unicode* new_str = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;

    alloc = ESCH_INTERNAL_CONFIG_GET_ALLOC(config);
    log = ESCH_INTERNAL_CONFIG_GET_LOG(config);

    len = utf8_get_unicode_len(utf8, begin, end, &bad_index);
    ESCH_CHECK_1(len >= 0, log, "Bad Unicode at index %d", bad_index,
                 ESCH_ERROR_INVALID_PARAMETER);

    buf_size = sizeof(esch_unicode) * (len + 1);
    ret = esch_alloc_malloc(alloc, buf_size, (void**)&new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc new buffer", ret);
    write_unicode_from_utf8(utf8, begin, end, new_str);
    (*str) = new_str;
    (*unicode_len) = (size_t)len;
    new_str = NULL;
Exit:
    esch_alloc_free(alloc, new_str);
    return ret;
}
static esch_error
encode_utf8(esch_unicode* unicode, int begin, int end, esch_alloc* alloc, char** str)
{
    esch_error ret = ESCH_ERROR_NOT_IMPLEMENTED;
    (void)unicode;
    (void)begin;
    (void)end;
    (void)alloc;
    (void)str;
    return ret;
}

/**
 * Create a new string from UTF-8 string. It also converts
 * @param config Config object to set alloc and log.
 * @param utf8 Input UTF-8 string.
 * @param str Returned string object.
 * @return Error code.
 */
esch_error
esch_string_new_from_utf8(esch_config* config, const char* utf8,
                          int begin, int end, esch_string** str)
{
    esch_error ret = ESCH_OK;
    esch_string* new_str = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    char* new_utf8 = NULL;
    size_t len = 0;
    size_t unicode_len = 0;
    esch_unicode* new_unicode = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    ESCH_CHECK_PARAM_PUBLIC(utf8 != NULL);
    ESCH_CHECK_PARAM_PUBLIC(str != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_INTERNAL_CONFIG_GET_ALLOC(config) != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_INTERNAL_CONFIG_GET_LOG(config) != NULL);

    if (begin == 0 && end < 0)
    {
        len = (size_t)strlen(utf8);
    }
    else if (begin >= 0 && end >= 0 && end >= begin)
    {
        len = end - begin;
    }
    else
    {
        ret = ESCH_ERROR_INVALID_PARAMETER;
        ESCH_CHECK(ret, esch_global_log, "Invalid length", ret);
    }

    alloc = ESCH_INTERNAL_CONFIG_GET_ALLOC(config);
    log = ESCH_INTERNAL_CONFIG_GET_LOG(config);

    ret = esch_alloc_malloc(alloc, sizeof(esch_string), (void**)&new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc for string", ret);

    ret = esch_alloc_malloc(alloc, sizeof(char) * (len + 1), (void**)&new_utf8);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc UTF-8", ret);
    (void)strncpy(new_utf8, (utf8 + begin), len);

    ret = decode_utf8(new_utf8, 0, len, config, &new_unicode, &unicode_len);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't decode UTF-8", ret);

    ESCH_GET_VERSION(new_str) = ESCH_VERSION;
    ESCH_GET_TYPE(new_str) = ESCH_TYPE_STRING;
    ESCH_GET_ALLOC(new_str) = alloc;
    ESCH_GET_LOG(new_str) = log;
    new_str->utf8 = new_utf8;
    new_str->unicode = new_unicode;
    new_str->utf8_len = len;
    new_str->unicode_len = unicode_len;
    (*str) = new_str;

    new_str = NULL;
    new_utf8 = NULL;
    new_unicode = NULL;
    assert(ESCH_IS_VALID_STRING(*str));
Exit:
    esch_alloc_free(alloc, new_utf8);
    esch_alloc_free(alloc, new_unicode);
    esch_alloc_free(alloc, new_str);
    return ret;
}
/**
 * Delete a string object.
 * @param str Given string object.
 * @return Errot code.
 */
esch_error
esch_string_delete(esch_string* str)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    if (str == NULL)
    {
        return ret;
    }

    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(str));
    esch_alloc_free(ESCH_GET_ALLOC(str), str->utf8);
    esch_alloc_free(ESCH_GET_ALLOC(str), str->unicode);
    esch_alloc_free(ESCH_GET_ALLOC(str), str);
Exit:
    return ret;
}

/**
 * Get a reference to UTF-8 string of given string.
 * @param str Give esch_string object
 * @return The internal UTF-8 c-string, ending with '\0'.
 */
char*
esch_string_get_utf8_ref(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    assert(str != NULL && ESCH_IS_VALID_STRING(str));
    return str->utf8;
}

/**
 * Get a reference to UTF-8 string of given string.
 * @param str Give esch_string object
 * @return The internal Unicode c-string, ending with '\0'.
 */
esch_unicode*
esch_string_get_unicode_ref(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    assert(str != NULL && ESCH_IS_VALID_STRING(str));
    return str->unicode;
}

/**
 * Get length of given string in UTF-8 representation.
 * @param str Give esch_string object
 * @return Length of string, in UTF-8 representation, '\0' not included.
 */
size_t
esch_string_get_utf8_length(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    assert(str != NULL && ESCH_IS_VALID_STRING(str));
    return str->utf8_len;
}

/**
 * Get length of given string in Unicode representation.
 * @param str Give esch_string object
 * @return Length of string, in Unicode representation, '\0' not included.
 */
size_t
esch_string_get_unicode_length(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    assert(str != NULL && ESCH_IS_VALID_STRING(str));
    return str->unicode_len;
}

int
esch_unicode_string_is_valid_identifier(const esch_unicode* unicode)
{
    const esch_unicode* ptr = NULL;
    if (unicode == NULL)
        return 0;

    /* TODO: Need to convert escape characters. */
    /* TODO: A known failure on unicode point > 65536 from Windows.
     * Windows uses 16-bit wchar_t, which does not represent all Unicode
     * points. It causes failures in unicode string tests. */

    ptr = unicode;
    /* Always start from non-digital */
    if (esch_unicode_is_ascii(*ptr))
    {
        if (esch_unicode_is_digit(*ptr))
        {
            return 0;
        }
    }
    else if (esch_unicode_is_range_nd(*ptr) ||
                esch_unicode_is_range_no(*ptr) ||
                esch_unicode_is_range_nl(*ptr))
    {
        return 0;
    }
    for (++ptr; *ptr != '\0'; ++ptr)
    {
        if (esch_unicode_is_ascii(*ptr))
        {
            if (!(esch_unicode_is_digit(*ptr) ||
                    esch_unicode_is_alpha(*ptr) ||
                    esch_unicode_is_extended_alphabetic(*ptr)))
            {
                return 0;
            }
        }
        else if (!(esch_unicode_is_range_lu(*ptr) ||
                   esch_unicode_is_range_ll(*ptr) ||
                   esch_unicode_is_range_lt(*ptr) ||
                   esch_unicode_is_range_lm(*ptr) ||
                   esch_unicode_is_range_lo(*ptr) ||
                   esch_unicode_is_range_mn(*ptr) ||
                   esch_unicode_is_range_mc(*ptr) ||
                   esch_unicode_is_range_nd(*ptr) ||
                   esch_unicode_is_range_nl(*ptr) ||
                   esch_unicode_is_range_no(*ptr) ||
                   esch_unicode_is_range_pd(*ptr) ||
                   esch_unicode_is_range_pc(*ptr) ||
                   esch_unicode_is_range_po(*ptr) ||
                   esch_unicode_is_range_sc(*ptr) ||
                   esch_unicode_is_range_sm(*ptr) ||
                   esch_unicode_is_range_sk(*ptr) ||
                   esch_unicode_is_range_so(*ptr) ||
                   esch_unicode_is_range_co(*ptr)))

        {
            return 0;
        }
    }
    return 1;
}

