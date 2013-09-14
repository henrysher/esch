/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch.h"
#include "esch_string.h"
#include "esch_log.h"
#include "esch_debug.h"
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
decode_utf8(char* utf8, int begin, int end, esch_config* config, esch_unicode** str)
{
    esch_error ret = ESCH_OK;
    int len = 0;
    int bad_index;
    int buf_size = 0;
    unsigned char* ptr = (unsigned char*)utf8;
    esch_unicode* new_str = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;

    alloc = config->alloc;
    log = config->log;

    len = utf8_get_unicode_len(utf8, begin, end, &bad_index);
    ESCH_CHECK_1(len >= 0, log, "Bad Unicode at index %d", bad_index,
                 ESCH_ERROR_INVALID_PARAMETER);

    buf_size = sizeof(esch_unicode) * (len + 1);
    ret = esch_alloc_malloc(alloc, buf_size, (void**)&new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc new buffer", ret);
    write_unicode_from_utf8(utf8, begin, end, new_str);
    (*str) = new_str;
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
esch_string_new_from_utf8(esch_config* config, char* utf8,
                          int begin, int end, esch_string** str)
{
    esch_error ret = ESCH_OK;
    esch_string* new_str = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    char* new_utf8 = NULL;
    size_t len = 0;
    esch_unicode* new_unicode = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    ESCH_CHECK_PARAM_PUBLIC(utf8 != NULL);
    ESCH_CHECK_PARAM_PUBLIC(str != NULL);

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
        ESCH_CHECK(ret, log, "Invalid length", ret);
    }

    alloc = config->alloc;
    log = config->log;

    ret = esch_alloc_malloc(alloc, sizeof(esch_string), (void**)&new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc for string", ret);

    ret = esch_alloc_malloc(alloc, sizeof(char) * (len + 1), (void**)&new_utf8);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc UTF-8", ret);
    (void)strncpy(new_utf8, (utf8 + begin), len);

    ret = decode_utf8(new_utf8, 0, len, config, &new_unicode);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't decode UTF-8", ret);

    new_str->base.type = ESCH_TYPE_STRING;
    new_str->base.alloc = alloc;
    new_str->base.log = log;
    new_str->utf8 = new_utf8;
    new_str->unicode = new_unicode;
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
        return ret;

    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(str));
    esch_alloc_free(str->base.alloc, str->utf8);
    esch_alloc_free(str->base.alloc, str->unicode);
    esch_alloc_free(str->base.alloc, str);
Exit:
    return ret;
}
esch_error esch_string_to_utf8(esch_string* str, char** utf8);
esch_error esch_string_to_unicode(esch_string* str, esch_unicode** unicode);


esch_error esch_unicode_is_lu(esch_unicode ch);
esch_error esch_unicode_is_ll(esch_unicode ch);
esch_error esch_unicode_is_lt(esch_unicode ch);
esch_error esch_unicode_is_lm(esch_unicode ch);
esch_error esch_unicode_is_lo(esch_unicode ch);
esch_error esch_unicode_is_mn(esch_unicode ch);
esch_error esch_unicode_is_mc(esch_unicode ch);
esch_error esch_unicode_is_nd(esch_unicode ch);
esch_error esch_unicode_is_nl(esch_unicode ch);
esch_error esch_unicode_is_no(esch_unicode ch);
esch_error esch_unicode_is_pd(esch_unicode ch);
esch_error esch_unicode_is_pc(esch_unicode ch);
esch_error esch_unicode_is_po(esch_unicode ch);
esch_error esch_unicode_is_sc(esch_unicode ch);
esch_error esch_unicode_is_sm(esch_unicode ch);
esch_error esch_unicode_is_sk(esch_unicode ch);
esch_error esch_unicode_is_so(esch_unicode ch);
esch_error esch_unicode_is_co(esch_unicode ch);
esch_error esch_unicode_is_ascii(esch_unicode ch);
esch_error esch_unicode_is_extended_alphabetic(esch_unicode ch);

