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
    {
        return ret;
    }

    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(str));
    esch_alloc_free(str->base.alloc, str->utf8);
    esch_alloc_free(str->base.alloc, str->unicode);
    esch_alloc_free(str->base.alloc, str);
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


/* DON'T MODIFY: The code below is automatically generated. */
/* Data source: http://www.fileformat.info/info/unicode/category/index.htm */
int
esch_unicode_is_range_co(esch_unicode ch)
{
    if (ch >= 0xe000 && ch <= 0xe000) { return 1; }
    else if (ch >= 0xf8ff && ch <= 0xf8ff) { return 1; }
    else if (ch >= 0xf0000 && ch <= 0xf0000) { return 1; }
    else if (ch >= 0xffffd && ch <= 0xffffd) { return 1; }
    else if (ch >= 0x100000 && ch <= 0x100000) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_ll(esch_unicode ch)
{
    if (ch >= 0x61 && ch <= 0x7a) { return 1; }
    else if (ch >= 0xb5 && ch <= 0xb5) { return 1; }
    else if (ch >= 0xdf && ch <= 0xf6) { return 1; }
    else if (ch >= 0xf8 && ch <= 0xff) { return 1; }
    else if (ch >= 0x101 && ch <= 0x101) { return 1; }
    else if (ch >= 0x103 && ch <= 0x103) { return 1; }
    else if (ch >= 0x105 && ch <= 0x105) { return 1; }
    else if (ch >= 0x107 && ch <= 0x107) { return 1; }
    else if (ch >= 0x109 && ch <= 0x109) { return 1; }
    else if (ch >= 0x10b && ch <= 0x10b) { return 1; }
    else if (ch >= 0x10d && ch <= 0x10d) { return 1; }
    else if (ch >= 0x10f && ch <= 0x10f) { return 1; }
    else if (ch >= 0x111 && ch <= 0x111) { return 1; }
    else if (ch >= 0x113 && ch <= 0x113) { return 1; }
    else if (ch >= 0x115 && ch <= 0x115) { return 1; }
    else if (ch >= 0x117 && ch <= 0x117) { return 1; }
    else if (ch >= 0x119 && ch <= 0x119) { return 1; }
    else if (ch >= 0x11b && ch <= 0x11b) { return 1; }
    else if (ch >= 0x11d && ch <= 0x11d) { return 1; }
    else if (ch >= 0x11f && ch <= 0x11f) { return 1; }
    else if (ch >= 0x121 && ch <= 0x121) { return 1; }
    else if (ch >= 0x123 && ch <= 0x123) { return 1; }
    else if (ch >= 0x125 && ch <= 0x125) { return 1; }
    else if (ch >= 0x127 && ch <= 0x127) { return 1; }
    else if (ch >= 0x129 && ch <= 0x129) { return 1; }
    else if (ch >= 0x12b && ch <= 0x12b) { return 1; }
    else if (ch >= 0x12d && ch <= 0x12d) { return 1; }
    else if (ch >= 0x12f && ch <= 0x12f) { return 1; }
    else if (ch >= 0x131 && ch <= 0x131) { return 1; }
    else if (ch >= 0x133 && ch <= 0x133) { return 1; }
    else if (ch >= 0x135 && ch <= 0x135) { return 1; }
    else if (ch >= 0x137 && ch <= 0x138) { return 1; }
    else if (ch >= 0x13a && ch <= 0x13a) { return 1; }
    else if (ch >= 0x13c && ch <= 0x13c) { return 1; }
    else if (ch >= 0x13e && ch <= 0x13e) { return 1; }
    else if (ch >= 0x140 && ch <= 0x140) { return 1; }
    else if (ch >= 0x142 && ch <= 0x142) { return 1; }
    else if (ch >= 0x144 && ch <= 0x144) { return 1; }
    else if (ch >= 0x146 && ch <= 0x146) { return 1; }
    else if (ch >= 0x148 && ch <= 0x149) { return 1; }
    else if (ch >= 0x14b && ch <= 0x14b) { return 1; }
    else if (ch >= 0x14d && ch <= 0x14d) { return 1; }
    else if (ch >= 0x14f && ch <= 0x14f) { return 1; }
    else if (ch >= 0x151 && ch <= 0x151) { return 1; }
    else if (ch >= 0x153 && ch <= 0x153) { return 1; }
    else if (ch >= 0x155 && ch <= 0x155) { return 1; }
    else if (ch >= 0x157 && ch <= 0x157) { return 1; }
    else if (ch >= 0x159 && ch <= 0x159) { return 1; }
    else if (ch >= 0x15b && ch <= 0x15b) { return 1; }
    else if (ch >= 0x15d && ch <= 0x15d) { return 1; }
    else if (ch >= 0x15f && ch <= 0x15f) { return 1; }
    else if (ch >= 0x161 && ch <= 0x161) { return 1; }
    else if (ch >= 0x163 && ch <= 0x163) { return 1; }
    else if (ch >= 0x165 && ch <= 0x165) { return 1; }
    else if (ch >= 0x167 && ch <= 0x167) { return 1; }
    else if (ch >= 0x169 && ch <= 0x169) { return 1; }
    else if (ch >= 0x16b && ch <= 0x16b) { return 1; }
    else if (ch >= 0x16d && ch <= 0x16d) { return 1; }
    else if (ch >= 0x16f && ch <= 0x16f) { return 1; }
    else if (ch >= 0x171 && ch <= 0x171) { return 1; }
    else if (ch >= 0x173 && ch <= 0x173) { return 1; }
    else if (ch >= 0x175 && ch <= 0x175) { return 1; }
    else if (ch >= 0x177 && ch <= 0x177) { return 1; }
    else if (ch >= 0x17a && ch <= 0x17a) { return 1; }
    else if (ch >= 0x17c && ch <= 0x17c) { return 1; }
    else if (ch >= 0x17e && ch <= 0x180) { return 1; }
    else if (ch >= 0x183 && ch <= 0x183) { return 1; }
    else if (ch >= 0x185 && ch <= 0x185) { return 1; }
    else if (ch >= 0x188 && ch <= 0x188) { return 1; }
    else if (ch >= 0x18c && ch <= 0x18d) { return 1; }
    else if (ch >= 0x192 && ch <= 0x192) { return 1; }
    else if (ch >= 0x195 && ch <= 0x195) { return 1; }
    else if (ch >= 0x199 && ch <= 0x19b) { return 1; }
    else if (ch >= 0x19e && ch <= 0x19e) { return 1; }
    else if (ch >= 0x1a1 && ch <= 0x1a1) { return 1; }
    else if (ch >= 0x1a3 && ch <= 0x1a3) { return 1; }
    else if (ch >= 0x1a5 && ch <= 0x1a5) { return 1; }
    else if (ch >= 0x1a8 && ch <= 0x1a8) { return 1; }
    else if (ch >= 0x1aa && ch <= 0x1ab) { return 1; }
    else if (ch >= 0x1ad && ch <= 0x1ad) { return 1; }
    else if (ch >= 0x1b0 && ch <= 0x1b0) { return 1; }
    else if (ch >= 0x1b4 && ch <= 0x1b4) { return 1; }
    else if (ch >= 0x1b6 && ch <= 0x1b6) { return 1; }
    else if (ch >= 0x1b9 && ch <= 0x1ba) { return 1; }
    else if (ch >= 0x1bd && ch <= 0x1bf) { return 1; }
    else if (ch >= 0x1c6 && ch <= 0x1c6) { return 1; }
    else if (ch >= 0x1c9 && ch <= 0x1c9) { return 1; }
    else if (ch >= 0x1cc && ch <= 0x1cc) { return 1; }
    else if (ch >= 0x1ce && ch <= 0x1ce) { return 1; }
    else if (ch >= 0x1d0 && ch <= 0x1d0) { return 1; }
    else if (ch >= 0x1d2 && ch <= 0x1d2) { return 1; }
    else if (ch >= 0x1d4 && ch <= 0x1d4) { return 1; }
    else if (ch >= 0x1d6 && ch <= 0x1d6) { return 1; }
    else if (ch >= 0x1d8 && ch <= 0x1d8) { return 1; }
    else if (ch >= 0x1da && ch <= 0x1da) { return 1; }
    else if (ch >= 0x1dc && ch <= 0x1dd) { return 1; }
    else if (ch >= 0x1df && ch <= 0x1df) { return 1; }
    else if (ch >= 0x1e1 && ch <= 0x1e1) { return 1; }
    else if (ch >= 0x1e3 && ch <= 0x1e3) { return 1; }
    else if (ch >= 0x1e5 && ch <= 0x1e5) { return 1; }
    else if (ch >= 0x1e7 && ch <= 0x1e7) { return 1; }
    else if (ch >= 0x1e9 && ch <= 0x1e9) { return 1; }
    else if (ch >= 0x1eb && ch <= 0x1eb) { return 1; }
    else if (ch >= 0x1ed && ch <= 0x1ed) { return 1; }
    else if (ch >= 0x1ef && ch <= 0x1f0) { return 1; }
    else if (ch >= 0x1f3 && ch <= 0x1f3) { return 1; }
    else if (ch >= 0x1f5 && ch <= 0x1f5) { return 1; }
    else if (ch >= 0x1f9 && ch <= 0x1f9) { return 1; }
    else if (ch >= 0x1fb && ch <= 0x1fb) { return 1; }
    else if (ch >= 0x1fd && ch <= 0x1fd) { return 1; }
    else if (ch >= 0x1ff && ch <= 0x1ff) { return 1; }
    else if (ch >= 0x201 && ch <= 0x201) { return 1; }
    else if (ch >= 0x203 && ch <= 0x203) { return 1; }
    else if (ch >= 0x205 && ch <= 0x205) { return 1; }
    else if (ch >= 0x207 && ch <= 0x207) { return 1; }
    else if (ch >= 0x209 && ch <= 0x209) { return 1; }
    else if (ch >= 0x20b && ch <= 0x20b) { return 1; }
    else if (ch >= 0x20d && ch <= 0x20d) { return 1; }
    else if (ch >= 0x20f && ch <= 0x20f) { return 1; }
    else if (ch >= 0x211 && ch <= 0x211) { return 1; }
    else if (ch >= 0x213 && ch <= 0x213) { return 1; }
    else if (ch >= 0x215 && ch <= 0x215) { return 1; }
    else if (ch >= 0x217 && ch <= 0x217) { return 1; }
    else if (ch >= 0x219 && ch <= 0x219) { return 1; }
    else if (ch >= 0x21b && ch <= 0x21b) { return 1; }
    else if (ch >= 0x21d && ch <= 0x21d) { return 1; }
    else if (ch >= 0x21f && ch <= 0x21f) { return 1; }
    else if (ch >= 0x221 && ch <= 0x221) { return 1; }
    else if (ch >= 0x223 && ch <= 0x223) { return 1; }
    else if (ch >= 0x225 && ch <= 0x225) { return 1; }
    else if (ch >= 0x227 && ch <= 0x227) { return 1; }
    else if (ch >= 0x229 && ch <= 0x229) { return 1; }
    else if (ch >= 0x22b && ch <= 0x22b) { return 1; }
    else if (ch >= 0x22d && ch <= 0x22d) { return 1; }
    else if (ch >= 0x22f && ch <= 0x22f) { return 1; }
    else if (ch >= 0x231 && ch <= 0x231) { return 1; }
    else if (ch >= 0x233 && ch <= 0x239) { return 1; }
    else if (ch >= 0x23c && ch <= 0x23c) { return 1; }
    else if (ch >= 0x23f && ch <= 0x240) { return 1; }
    else if (ch >= 0x242 && ch <= 0x242) { return 1; }
    else if (ch >= 0x247 && ch <= 0x247) { return 1; }
    else if (ch >= 0x249 && ch <= 0x249) { return 1; }
    else if (ch >= 0x24b && ch <= 0x24b) { return 1; }
    else if (ch >= 0x24d && ch <= 0x24d) { return 1; }
    else if (ch >= 0x24f && ch <= 0x293) { return 1; }
    else if (ch >= 0x295 && ch <= 0x2af) { return 1; }
    else if (ch >= 0x371 && ch <= 0x371) { return 1; }
    else if (ch >= 0x373 && ch <= 0x373) { return 1; }
    else if (ch >= 0x377 && ch <= 0x377) { return 1; }
    else if (ch >= 0x37b && ch <= 0x37d) { return 1; }
    else if (ch >= 0x390 && ch <= 0x390) { return 1; }
    else if (ch >= 0x3ac && ch <= 0x3ce) { return 1; }
    else if (ch >= 0x3d0 && ch <= 0x3d1) { return 1; }
    else if (ch >= 0x3d5 && ch <= 0x3d7) { return 1; }
    else if (ch >= 0x3d9 && ch <= 0x3d9) { return 1; }
    else if (ch >= 0x3db && ch <= 0x3db) { return 1; }
    else if (ch >= 0x3dd && ch <= 0x3dd) { return 1; }
    else if (ch >= 0x3df && ch <= 0x3df) { return 1; }
    else if (ch >= 0x3e1 && ch <= 0x3e1) { return 1; }
    else if (ch >= 0x3e3 && ch <= 0x3e3) { return 1; }
    else if (ch >= 0x3e5 && ch <= 0x3e5) { return 1; }
    else if (ch >= 0x3e7 && ch <= 0x3e7) { return 1; }
    else if (ch >= 0x3e9 && ch <= 0x3e9) { return 1; }
    else if (ch >= 0x3eb && ch <= 0x3eb) { return 1; }
    else if (ch >= 0x3ed && ch <= 0x3ed) { return 1; }
    else if (ch >= 0x3ef && ch <= 0x3f3) { return 1; }
    else if (ch >= 0x3f5 && ch <= 0x3f5) { return 1; }
    else if (ch >= 0x3f8 && ch <= 0x3f8) { return 1; }
    else if (ch >= 0x3fb && ch <= 0x3fc) { return 1; }
    else if (ch >= 0x430 && ch <= 0x45f) { return 1; }
    else if (ch >= 0x461 && ch <= 0x461) { return 1; }
    else if (ch >= 0x463 && ch <= 0x463) { return 1; }
    else if (ch >= 0x465 && ch <= 0x465) { return 1; }
    else if (ch >= 0x467 && ch <= 0x467) { return 1; }
    else if (ch >= 0x469 && ch <= 0x469) { return 1; }
    else if (ch >= 0x46b && ch <= 0x46b) { return 1; }
    else if (ch >= 0x46d && ch <= 0x46d) { return 1; }
    else if (ch >= 0x46f && ch <= 0x46f) { return 1; }
    else if (ch >= 0x471 && ch <= 0x471) { return 1; }
    else if (ch >= 0x473 && ch <= 0x473) { return 1; }
    else if (ch >= 0x475 && ch <= 0x475) { return 1; }
    else if (ch >= 0x477 && ch <= 0x477) { return 1; }
    else if (ch >= 0x479 && ch <= 0x479) { return 1; }
    else if (ch >= 0x47b && ch <= 0x47b) { return 1; }
    else if (ch >= 0x47d && ch <= 0x47d) { return 1; }
    else if (ch >= 0x47f && ch <= 0x47f) { return 1; }
    else if (ch >= 0x481 && ch <= 0x481) { return 1; }
    else if (ch >= 0x48b && ch <= 0x48b) { return 1; }
    else if (ch >= 0x48d && ch <= 0x48d) { return 1; }
    else if (ch >= 0x48f && ch <= 0x48f) { return 1; }
    else if (ch >= 0x491 && ch <= 0x491) { return 1; }
    else if (ch >= 0x493 && ch <= 0x493) { return 1; }
    else if (ch >= 0x495 && ch <= 0x495) { return 1; }
    else if (ch >= 0x497 && ch <= 0x497) { return 1; }
    else if (ch >= 0x499 && ch <= 0x499) { return 1; }
    else if (ch >= 0x49b && ch <= 0x49b) { return 1; }
    else if (ch >= 0x49d && ch <= 0x49d) { return 1; }
    else if (ch >= 0x49f && ch <= 0x49f) { return 1; }
    else if (ch >= 0x4a1 && ch <= 0x4a1) { return 1; }
    else if (ch >= 0x4a3 && ch <= 0x4a3) { return 1; }
    else if (ch >= 0x4a5 && ch <= 0x4a5) { return 1; }
    else if (ch >= 0x4a7 && ch <= 0x4a7) { return 1; }
    else if (ch >= 0x4a9 && ch <= 0x4a9) { return 1; }
    else if (ch >= 0x4ab && ch <= 0x4ab) { return 1; }
    else if (ch >= 0x4ad && ch <= 0x4ad) { return 1; }
    else if (ch >= 0x4af && ch <= 0x4af) { return 1; }
    else if (ch >= 0x4b1 && ch <= 0x4b1) { return 1; }
    else if (ch >= 0x4b3 && ch <= 0x4b3) { return 1; }
    else if (ch >= 0x4b5 && ch <= 0x4b5) { return 1; }
    else if (ch >= 0x4b7 && ch <= 0x4b7) { return 1; }
    else if (ch >= 0x4b9 && ch <= 0x4b9) { return 1; }
    else if (ch >= 0x4bb && ch <= 0x4bb) { return 1; }
    else if (ch >= 0x4bd && ch <= 0x4bd) { return 1; }
    else if (ch >= 0x4bf && ch <= 0x4bf) { return 1; }
    else if (ch >= 0x4c2 && ch <= 0x4c2) { return 1; }
    else if (ch >= 0x4c4 && ch <= 0x4c4) { return 1; }
    else if (ch >= 0x4c6 && ch <= 0x4c6) { return 1; }
    else if (ch >= 0x4c8 && ch <= 0x4c8) { return 1; }
    else if (ch >= 0x4ca && ch <= 0x4ca) { return 1; }
    else if (ch >= 0x4cc && ch <= 0x4cc) { return 1; }
    else if (ch >= 0x4ce && ch <= 0x4cf) { return 1; }
    else if (ch >= 0x4d1 && ch <= 0x4d1) { return 1; }
    else if (ch >= 0x4d3 && ch <= 0x4d3) { return 1; }
    else if (ch >= 0x4d5 && ch <= 0x4d5) { return 1; }
    else if (ch >= 0x4d7 && ch <= 0x4d7) { return 1; }
    else if (ch >= 0x4d9 && ch <= 0x4d9) { return 1; }
    else if (ch >= 0x4db && ch <= 0x4db) { return 1; }
    else if (ch >= 0x4dd && ch <= 0x4dd) { return 1; }
    else if (ch >= 0x4df && ch <= 0x4df) { return 1; }
    else if (ch >= 0x4e1 && ch <= 0x4e1) { return 1; }
    else if (ch >= 0x4e3 && ch <= 0x4e3) { return 1; }
    else if (ch >= 0x4e5 && ch <= 0x4e5) { return 1; }
    else if (ch >= 0x4e7 && ch <= 0x4e7) { return 1; }
    else if (ch >= 0x4e9 && ch <= 0x4e9) { return 1; }
    else if (ch >= 0x4eb && ch <= 0x4eb) { return 1; }
    else if (ch >= 0x4ed && ch <= 0x4ed) { return 1; }
    else if (ch >= 0x4ef && ch <= 0x4ef) { return 1; }
    else if (ch >= 0x4f1 && ch <= 0x4f1) { return 1; }
    else if (ch >= 0x4f3 && ch <= 0x4f3) { return 1; }
    else if (ch >= 0x4f5 && ch <= 0x4f5) { return 1; }
    else if (ch >= 0x4f7 && ch <= 0x4f7) { return 1; }
    else if (ch >= 0x4f9 && ch <= 0x4f9) { return 1; }
    else if (ch >= 0x4fb && ch <= 0x4fb) { return 1; }
    else if (ch >= 0x4fd && ch <= 0x4fd) { return 1; }
    else if (ch >= 0x4ff && ch <= 0x4ff) { return 1; }
    else if (ch >= 0x501 && ch <= 0x501) { return 1; }
    else if (ch >= 0x503 && ch <= 0x503) { return 1; }
    else if (ch >= 0x505 && ch <= 0x505) { return 1; }
    else if (ch >= 0x507 && ch <= 0x507) { return 1; }
    else if (ch >= 0x509 && ch <= 0x509) { return 1; }
    else if (ch >= 0x50b && ch <= 0x50b) { return 1; }
    else if (ch >= 0x50d && ch <= 0x50d) { return 1; }
    else if (ch >= 0x50f && ch <= 0x50f) { return 1; }
    else if (ch >= 0x511 && ch <= 0x511) { return 1; }
    else if (ch >= 0x513 && ch <= 0x513) { return 1; }
    else if (ch >= 0x515 && ch <= 0x515) { return 1; }
    else if (ch >= 0x517 && ch <= 0x517) { return 1; }
    else if (ch >= 0x519 && ch <= 0x519) { return 1; }
    else if (ch >= 0x51b && ch <= 0x51b) { return 1; }
    else if (ch >= 0x51d && ch <= 0x51d) { return 1; }
    else if (ch >= 0x51f && ch <= 0x51f) { return 1; }
    else if (ch >= 0x521 && ch <= 0x521) { return 1; }
    else if (ch >= 0x523 && ch <= 0x523) { return 1; }
    else if (ch >= 0x525 && ch <= 0x525) { return 1; }
    else if (ch >= 0x527 && ch <= 0x527) { return 1; }
    else if (ch >= 0x561 && ch <= 0x587) { return 1; }
    else if (ch >= 0x1d00 && ch <= 0x1d2b) { return 1; }
    else if (ch >= 0x1d6b && ch <= 0x1d77) { return 1; }
    else if (ch >= 0x1d79 && ch <= 0x1d9a) { return 1; }
    else if (ch >= 0x1e01 && ch <= 0x1e01) { return 1; }
    else if (ch >= 0x1e03 && ch <= 0x1e03) { return 1; }
    else if (ch >= 0x1e05 && ch <= 0x1e05) { return 1; }
    else if (ch >= 0x1e07 && ch <= 0x1e07) { return 1; }
    else if (ch >= 0x1e09 && ch <= 0x1e09) { return 1; }
    else if (ch >= 0x1e0b && ch <= 0x1e0b) { return 1; }
    else if (ch >= 0x1e0d && ch <= 0x1e0d) { return 1; }
    else if (ch >= 0x1e0f && ch <= 0x1e0f) { return 1; }
    else if (ch >= 0x1e11 && ch <= 0x1e11) { return 1; }
    else if (ch >= 0x1e13 && ch <= 0x1e13) { return 1; }
    else if (ch >= 0x1e15 && ch <= 0x1e15) { return 1; }
    else if (ch >= 0x1e17 && ch <= 0x1e17) { return 1; }
    else if (ch >= 0x1e19 && ch <= 0x1e19) { return 1; }
    else if (ch >= 0x1e1b && ch <= 0x1e1b) { return 1; }
    else if (ch >= 0x1e1d && ch <= 0x1e1d) { return 1; }
    else if (ch >= 0x1e1f && ch <= 0x1e1f) { return 1; }
    else if (ch >= 0x1e21 && ch <= 0x1e21) { return 1; }
    else if (ch >= 0x1e23 && ch <= 0x1e23) { return 1; }
    else if (ch >= 0x1e25 && ch <= 0x1e25) { return 1; }
    else if (ch >= 0x1e27 && ch <= 0x1e27) { return 1; }
    else if (ch >= 0x1e29 && ch <= 0x1e29) { return 1; }
    else if (ch >= 0x1e2b && ch <= 0x1e2b) { return 1; }
    else if (ch >= 0x1e2d && ch <= 0x1e2d) { return 1; }
    else if (ch >= 0x1e2f && ch <= 0x1e2f) { return 1; }
    else if (ch >= 0x1e31 && ch <= 0x1e31) { return 1; }
    else if (ch >= 0x1e33 && ch <= 0x1e33) { return 1; }
    else if (ch >= 0x1e35 && ch <= 0x1e35) { return 1; }
    else if (ch >= 0x1e37 && ch <= 0x1e37) { return 1; }
    else if (ch >= 0x1e39 && ch <= 0x1e39) { return 1; }
    else if (ch >= 0x1e3b && ch <= 0x1e3b) { return 1; }
    else if (ch >= 0x1e3d && ch <= 0x1e3d) { return 1; }
    else if (ch >= 0x1e3f && ch <= 0x1e3f) { return 1; }
    else if (ch >= 0x1e41 && ch <= 0x1e41) { return 1; }
    else if (ch >= 0x1e43 && ch <= 0x1e43) { return 1; }
    else if (ch >= 0x1e45 && ch <= 0x1e45) { return 1; }
    else if (ch >= 0x1e47 && ch <= 0x1e47) { return 1; }
    else if (ch >= 0x1e49 && ch <= 0x1e49) { return 1; }
    else if (ch >= 0x1e4b && ch <= 0x1e4b) { return 1; }
    else if (ch >= 0x1e4d && ch <= 0x1e4d) { return 1; }
    else if (ch >= 0x1e4f && ch <= 0x1e4f) { return 1; }
    else if (ch >= 0x1e51 && ch <= 0x1e51) { return 1; }
    else if (ch >= 0x1e53 && ch <= 0x1e53) { return 1; }
    else if (ch >= 0x1e55 && ch <= 0x1e55) { return 1; }
    else if (ch >= 0x1e57 && ch <= 0x1e57) { return 1; }
    else if (ch >= 0x1e59 && ch <= 0x1e59) { return 1; }
    else if (ch >= 0x1e5b && ch <= 0x1e5b) { return 1; }
    else if (ch >= 0x1e5d && ch <= 0x1e5d) { return 1; }
    else if (ch >= 0x1e5f && ch <= 0x1e5f) { return 1; }
    else if (ch >= 0x1e61 && ch <= 0x1e61) { return 1; }
    else if (ch >= 0x1e63 && ch <= 0x1e63) { return 1; }
    else if (ch >= 0x1e65 && ch <= 0x1e65) { return 1; }
    else if (ch >= 0x1e67 && ch <= 0x1e67) { return 1; }
    else if (ch >= 0x1e69 && ch <= 0x1e69) { return 1; }
    else if (ch >= 0x1e6b && ch <= 0x1e6b) { return 1; }
    else if (ch >= 0x1e6d && ch <= 0x1e6d) { return 1; }
    else if (ch >= 0x1e6f && ch <= 0x1e6f) { return 1; }
    else if (ch >= 0x1e71 && ch <= 0x1e71) { return 1; }
    else if (ch >= 0x1e73 && ch <= 0x1e73) { return 1; }
    else if (ch >= 0x1e75 && ch <= 0x1e75) { return 1; }
    else if (ch >= 0x1e77 && ch <= 0x1e77) { return 1; }
    else if (ch >= 0x1e79 && ch <= 0x1e79) { return 1; }
    else if (ch >= 0x1e7b && ch <= 0x1e7b) { return 1; }
    else if (ch >= 0x1e7d && ch <= 0x1e7d) { return 1; }
    else if (ch >= 0x1e7f && ch <= 0x1e7f) { return 1; }
    else if (ch >= 0x1e81 && ch <= 0x1e81) { return 1; }
    else if (ch >= 0x1e83 && ch <= 0x1e83) { return 1; }
    else if (ch >= 0x1e85 && ch <= 0x1e85) { return 1; }
    else if (ch >= 0x1e87 && ch <= 0x1e87) { return 1; }
    else if (ch >= 0x1e89 && ch <= 0x1e89) { return 1; }
    else if (ch >= 0x1e8b && ch <= 0x1e8b) { return 1; }
    else if (ch >= 0x1e8d && ch <= 0x1e8d) { return 1; }
    else if (ch >= 0x1e8f && ch <= 0x1e8f) { return 1; }
    else if (ch >= 0x1e91 && ch <= 0x1e91) { return 1; }
    else if (ch >= 0x1e93 && ch <= 0x1e93) { return 1; }
    else if (ch >= 0x1e95 && ch <= 0x1e9d) { return 1; }
    else if (ch >= 0x1e9f && ch <= 0x1e9f) { return 1; }
    else if (ch >= 0x1ea1 && ch <= 0x1ea1) { return 1; }
    else if (ch >= 0x1ea3 && ch <= 0x1ea3) { return 1; }
    else if (ch >= 0x1ea5 && ch <= 0x1ea5) { return 1; }
    else if (ch >= 0x1ea7 && ch <= 0x1ea7) { return 1; }
    else if (ch >= 0x1ea9 && ch <= 0x1ea9) { return 1; }
    else if (ch >= 0x1eab && ch <= 0x1eab) { return 1; }
    else if (ch >= 0x1ead && ch <= 0x1ead) { return 1; }
    else if (ch >= 0x1eaf && ch <= 0x1eaf) { return 1; }
    else if (ch >= 0x1eb1 && ch <= 0x1eb1) { return 1; }
    else if (ch >= 0x1eb3 && ch <= 0x1eb3) { return 1; }
    else if (ch >= 0x1eb5 && ch <= 0x1eb5) { return 1; }
    else if (ch >= 0x1eb7 && ch <= 0x1eb7) { return 1; }
    else if (ch >= 0x1eb9 && ch <= 0x1eb9) { return 1; }
    else if (ch >= 0x1ebb && ch <= 0x1ebb) { return 1; }
    else if (ch >= 0x1ebd && ch <= 0x1ebd) { return 1; }
    else if (ch >= 0x1ebf && ch <= 0x1ebf) { return 1; }
    else if (ch >= 0x1ec1 && ch <= 0x1ec1) { return 1; }
    else if (ch >= 0x1ec3 && ch <= 0x1ec3) { return 1; }
    else if (ch >= 0x1ec5 && ch <= 0x1ec5) { return 1; }
    else if (ch >= 0x1ec7 && ch <= 0x1ec7) { return 1; }
    else if (ch >= 0x1ec9 && ch <= 0x1ec9) { return 1; }
    else if (ch >= 0x1ecb && ch <= 0x1ecb) { return 1; }
    else if (ch >= 0x1ecd && ch <= 0x1ecd) { return 1; }
    else if (ch >= 0x1ecf && ch <= 0x1ecf) { return 1; }
    else if (ch >= 0x1ed1 && ch <= 0x1ed1) { return 1; }
    else if (ch >= 0x1ed3 && ch <= 0x1ed3) { return 1; }
    else if (ch >= 0x1ed5 && ch <= 0x1ed5) { return 1; }
    else if (ch >= 0x1ed7 && ch <= 0x1ed7) { return 1; }
    else if (ch >= 0x1ed9 && ch <= 0x1ed9) { return 1; }
    else if (ch >= 0x1edb && ch <= 0x1edb) { return 1; }
    else if (ch >= 0x1edd && ch <= 0x1edd) { return 1; }
    else if (ch >= 0x1edf && ch <= 0x1edf) { return 1; }
    else if (ch >= 0x1ee1 && ch <= 0x1ee1) { return 1; }
    else if (ch >= 0x1ee3 && ch <= 0x1ee3) { return 1; }
    else if (ch >= 0x1ee5 && ch <= 0x1ee5) { return 1; }
    else if (ch >= 0x1ee7 && ch <= 0x1ee7) { return 1; }
    else if (ch >= 0x1ee9 && ch <= 0x1ee9) { return 1; }
    else if (ch >= 0x1eeb && ch <= 0x1eeb) { return 1; }
    else if (ch >= 0x1eed && ch <= 0x1eed) { return 1; }
    else if (ch >= 0x1eef && ch <= 0x1eef) { return 1; }
    else if (ch >= 0x1ef1 && ch <= 0x1ef1) { return 1; }
    else if (ch >= 0x1ef3 && ch <= 0x1ef3) { return 1; }
    else if (ch >= 0x1ef5 && ch <= 0x1ef5) { return 1; }
    else if (ch >= 0x1ef7 && ch <= 0x1ef7) { return 1; }
    else if (ch >= 0x1ef9 && ch <= 0x1ef9) { return 1; }
    else if (ch >= 0x1efb && ch <= 0x1efb) { return 1; }
    else if (ch >= 0x1efd && ch <= 0x1efd) { return 1; }
    else if (ch >= 0x1eff && ch <= 0x1f07) { return 1; }
    else if (ch >= 0x1f10 && ch <= 0x1f15) { return 1; }
    else if (ch >= 0x1f20 && ch <= 0x1f27) { return 1; }
    else if (ch >= 0x1f30 && ch <= 0x1f37) { return 1; }
    else if (ch >= 0x1f40 && ch <= 0x1f45) { return 1; }
    else if (ch >= 0x1f50 && ch <= 0x1f57) { return 1; }
    else if (ch >= 0x1f60 && ch <= 0x1f67) { return 1; }
    else if (ch >= 0x1f70 && ch <= 0x1f7d) { return 1; }
    else if (ch >= 0x1f80 && ch <= 0x1f87) { return 1; }
    else if (ch >= 0x1f90 && ch <= 0x1f97) { return 1; }
    else if (ch >= 0x1fa0 && ch <= 0x1fa7) { return 1; }
    else if (ch >= 0x1fb0 && ch <= 0x1fb4) { return 1; }
    else if (ch >= 0x1fb6 && ch <= 0x1fb7) { return 1; }
    else if (ch >= 0x1fbe && ch <= 0x1fbe) { return 1; }
    else if (ch >= 0x1fc2 && ch <= 0x1fc4) { return 1; }
    else if (ch >= 0x1fc6 && ch <= 0x1fc7) { return 1; }
    else if (ch >= 0x1fd0 && ch <= 0x1fd3) { return 1; }
    else if (ch >= 0x1fd6 && ch <= 0x1fd7) { return 1; }
    else if (ch >= 0x1fe0 && ch <= 0x1fe7) { return 1; }
    else if (ch >= 0x1ff2 && ch <= 0x1ff4) { return 1; }
    else if (ch >= 0x1ff6 && ch <= 0x1ff7) { return 1; }
    else if (ch >= 0x210a && ch <= 0x210a) { return 1; }
    else if (ch >= 0x210e && ch <= 0x210f) { return 1; }
    else if (ch >= 0x2113 && ch <= 0x2113) { return 1; }
    else if (ch >= 0x212f && ch <= 0x212f) { return 1; }
    else if (ch >= 0x2134 && ch <= 0x2134) { return 1; }
    else if (ch >= 0x2139 && ch <= 0x2139) { return 1; }
    else if (ch >= 0x213c && ch <= 0x213d) { return 1; }
    else if (ch >= 0x2146 && ch <= 0x2149) { return 1; }
    else if (ch >= 0x214e && ch <= 0x214e) { return 1; }
    else if (ch >= 0x2184 && ch <= 0x2184) { return 1; }
    else if (ch >= 0x2c30 && ch <= 0x2c5e) { return 1; }
    else if (ch >= 0x2c61 && ch <= 0x2c61) { return 1; }
    else if (ch >= 0x2c65 && ch <= 0x2c66) { return 1; }
    else if (ch >= 0x2c68 && ch <= 0x2c68) { return 1; }
    else if (ch >= 0x2c6a && ch <= 0x2c6a) { return 1; }
    else if (ch >= 0x2c6c && ch <= 0x2c6c) { return 1; }
    else if (ch >= 0x2c71 && ch <= 0x2c71) { return 1; }
    else if (ch >= 0x2c73 && ch <= 0x2c74) { return 1; }
    else if (ch >= 0x2c76 && ch <= 0x2c7b) { return 1; }
    else if (ch >= 0x2c81 && ch <= 0x2c81) { return 1; }
    else if (ch >= 0x2c83 && ch <= 0x2c83) { return 1; }
    else if (ch >= 0x2c85 && ch <= 0x2c85) { return 1; }
    else if (ch >= 0x2c87 && ch <= 0x2c87) { return 1; }
    else if (ch >= 0x2c89 && ch <= 0x2c89) { return 1; }
    else if (ch >= 0x2c8b && ch <= 0x2c8b) { return 1; }
    else if (ch >= 0x2c8d && ch <= 0x2c8d) { return 1; }
    else if (ch >= 0x2c8f && ch <= 0x2c8f) { return 1; }
    else if (ch >= 0x2c91 && ch <= 0x2c91) { return 1; }
    else if (ch >= 0x2c93 && ch <= 0x2c93) { return 1; }
    else if (ch >= 0x2c95 && ch <= 0x2c95) { return 1; }
    else if (ch >= 0x2c97 && ch <= 0x2c97) { return 1; }
    else if (ch >= 0x2c99 && ch <= 0x2c99) { return 1; }
    else if (ch >= 0x2c9b && ch <= 0x2c9b) { return 1; }
    else if (ch >= 0x2c9d && ch <= 0x2c9d) { return 1; }
    else if (ch >= 0x2c9f && ch <= 0x2c9f) { return 1; }
    else if (ch >= 0x2ca1 && ch <= 0x2ca1) { return 1; }
    else if (ch >= 0x2ca3 && ch <= 0x2ca3) { return 1; }
    else if (ch >= 0x2ca5 && ch <= 0x2ca5) { return 1; }
    else if (ch >= 0x2ca7 && ch <= 0x2ca7) { return 1; }
    else if (ch >= 0x2ca9 && ch <= 0x2ca9) { return 1; }
    else if (ch >= 0x2cab && ch <= 0x2cab) { return 1; }
    else if (ch >= 0x2cad && ch <= 0x2cad) { return 1; }
    else if (ch >= 0x2caf && ch <= 0x2caf) { return 1; }
    else if (ch >= 0x2cb1 && ch <= 0x2cb1) { return 1; }
    else if (ch >= 0x2cb3 && ch <= 0x2cb3) { return 1; }
    else if (ch >= 0x2cb5 && ch <= 0x2cb5) { return 1; }
    else if (ch >= 0x2cb7 && ch <= 0x2cb7) { return 1; }
    else if (ch >= 0x2cb9 && ch <= 0x2cb9) { return 1; }
    else if (ch >= 0x2cbb && ch <= 0x2cbb) { return 1; }
    else if (ch >= 0x2cbd && ch <= 0x2cbd) { return 1; }
    else if (ch >= 0x2cbf && ch <= 0x2cbf) { return 1; }
    else if (ch >= 0x2cc1 && ch <= 0x2cc1) { return 1; }
    else if (ch >= 0x2cc3 && ch <= 0x2cc3) { return 1; }
    else if (ch >= 0x2cc5 && ch <= 0x2cc5) { return 1; }
    else if (ch >= 0x2cc7 && ch <= 0x2cc7) { return 1; }
    else if (ch >= 0x2cc9 && ch <= 0x2cc9) { return 1; }
    else if (ch >= 0x2ccb && ch <= 0x2ccb) { return 1; }
    else if (ch >= 0x2ccd && ch <= 0x2ccd) { return 1; }
    else if (ch >= 0x2ccf && ch <= 0x2ccf) { return 1; }
    else if (ch >= 0x2cd1 && ch <= 0x2cd1) { return 1; }
    else if (ch >= 0x2cd3 && ch <= 0x2cd3) { return 1; }
    else if (ch >= 0x2cd5 && ch <= 0x2cd5) { return 1; }
    else if (ch >= 0x2cd7 && ch <= 0x2cd7) { return 1; }
    else if (ch >= 0x2cd9 && ch <= 0x2cd9) { return 1; }
    else if (ch >= 0x2cdb && ch <= 0x2cdb) { return 1; }
    else if (ch >= 0x2cdd && ch <= 0x2cdd) { return 1; }
    else if (ch >= 0x2cdf && ch <= 0x2cdf) { return 1; }
    else if (ch >= 0x2ce1 && ch <= 0x2ce1) { return 1; }
    else if (ch >= 0x2ce3 && ch <= 0x2ce4) { return 1; }
    else if (ch >= 0x2cec && ch <= 0x2cec) { return 1; }
    else if (ch >= 0x2cee && ch <= 0x2cee) { return 1; }
    else if (ch >= 0x2cf3 && ch <= 0x2cf3) { return 1; }
    else if (ch >= 0x2d00 && ch <= 0x2d25) { return 1; }
    else if (ch >= 0x2d27 && ch <= 0x2d27) { return 1; }
    else if (ch >= 0x2d2d && ch <= 0x2d2d) { return 1; }
    else if (ch >= 0xa641 && ch <= 0xa641) { return 1; }
    else if (ch >= 0xa643 && ch <= 0xa643) { return 1; }
    else if (ch >= 0xa645 && ch <= 0xa645) { return 1; }
    else if (ch >= 0xa647 && ch <= 0xa647) { return 1; }
    else if (ch >= 0xa649 && ch <= 0xa649) { return 1; }
    else if (ch >= 0xa64b && ch <= 0xa64b) { return 1; }
    else if (ch >= 0xa64d && ch <= 0xa64d) { return 1; }
    else if (ch >= 0xa64f && ch <= 0xa64f) { return 1; }
    else if (ch >= 0xa651 && ch <= 0xa651) { return 1; }
    else if (ch >= 0xa653 && ch <= 0xa653) { return 1; }
    else if (ch >= 0xa655 && ch <= 0xa655) { return 1; }
    else if (ch >= 0xa657 && ch <= 0xa657) { return 1; }
    else if (ch >= 0xa659 && ch <= 0xa659) { return 1; }
    else if (ch >= 0xa65b && ch <= 0xa65b) { return 1; }
    else if (ch >= 0xa65d && ch <= 0xa65d) { return 1; }
    else if (ch >= 0xa65f && ch <= 0xa65f) { return 1; }
    else if (ch >= 0xa661 && ch <= 0xa661) { return 1; }
    else if (ch >= 0xa663 && ch <= 0xa663) { return 1; }
    else if (ch >= 0xa665 && ch <= 0xa665) { return 1; }
    else if (ch >= 0xa667 && ch <= 0xa667) { return 1; }
    else if (ch >= 0xa669 && ch <= 0xa669) { return 1; }
    else if (ch >= 0xa66b && ch <= 0xa66b) { return 1; }
    else if (ch >= 0xa66d && ch <= 0xa66d) { return 1; }
    else if (ch >= 0xa681 && ch <= 0xa681) { return 1; }
    else if (ch >= 0xa683 && ch <= 0xa683) { return 1; }
    else if (ch >= 0xa685 && ch <= 0xa685) { return 1; }
    else if (ch >= 0xa687 && ch <= 0xa687) { return 1; }
    else if (ch >= 0xa689 && ch <= 0xa689) { return 1; }
    else if (ch >= 0xa68b && ch <= 0xa68b) { return 1; }
    else if (ch >= 0xa68d && ch <= 0xa68d) { return 1; }
    else if (ch >= 0xa68f && ch <= 0xa68f) { return 1; }
    else if (ch >= 0xa691 && ch <= 0xa691) { return 1; }
    else if (ch >= 0xa693 && ch <= 0xa693) { return 1; }
    else if (ch >= 0xa695 && ch <= 0xa695) { return 1; }
    else if (ch >= 0xa697 && ch <= 0xa697) { return 1; }
    else if (ch >= 0xa723 && ch <= 0xa723) { return 1; }
    else if (ch >= 0xa725 && ch <= 0xa725) { return 1; }
    else if (ch >= 0xa727 && ch <= 0xa727) { return 1; }
    else if (ch >= 0xa729 && ch <= 0xa729) { return 1; }
    else if (ch >= 0xa72b && ch <= 0xa72b) { return 1; }
    else if (ch >= 0xa72d && ch <= 0xa72d) { return 1; }
    else if (ch >= 0xa72f && ch <= 0xa731) { return 1; }
    else if (ch >= 0xa733 && ch <= 0xa733) { return 1; }
    else if (ch >= 0xa735 && ch <= 0xa735) { return 1; }
    else if (ch >= 0xa737 && ch <= 0xa737) { return 1; }
    else if (ch >= 0xa739 && ch <= 0xa739) { return 1; }
    else if (ch >= 0xa73b && ch <= 0xa73b) { return 1; }
    else if (ch >= 0xa73d && ch <= 0xa73d) { return 1; }
    else if (ch >= 0xa73f && ch <= 0xa73f) { return 1; }
    else if (ch >= 0xa741 && ch <= 0xa741) { return 1; }
    else if (ch >= 0xa743 && ch <= 0xa743) { return 1; }
    else if (ch >= 0xa745 && ch <= 0xa745) { return 1; }
    else if (ch >= 0xa747 && ch <= 0xa747) { return 1; }
    else if (ch >= 0xa749 && ch <= 0xa749) { return 1; }
    else if (ch >= 0xa74b && ch <= 0xa74b) { return 1; }
    else if (ch >= 0xa74d && ch <= 0xa74d) { return 1; }
    else if (ch >= 0xa74f && ch <= 0xa74f) { return 1; }
    else if (ch >= 0xa751 && ch <= 0xa751) { return 1; }
    else if (ch >= 0xa753 && ch <= 0xa753) { return 1; }
    else if (ch >= 0xa755 && ch <= 0xa755) { return 1; }
    else if (ch >= 0xa757 && ch <= 0xa757) { return 1; }
    else if (ch >= 0xa759 && ch <= 0xa759) { return 1; }
    else if (ch >= 0xa75b && ch <= 0xa75b) { return 1; }
    else if (ch >= 0xa75d && ch <= 0xa75d) { return 1; }
    else if (ch >= 0xa75f && ch <= 0xa75f) { return 1; }
    else if (ch >= 0xa761 && ch <= 0xa761) { return 1; }
    else if (ch >= 0xa763 && ch <= 0xa763) { return 1; }
    else if (ch >= 0xa765 && ch <= 0xa765) { return 1; }
    else if (ch >= 0xa767 && ch <= 0xa767) { return 1; }
    else if (ch >= 0xa769 && ch <= 0xa769) { return 1; }
    else if (ch >= 0xa76b && ch <= 0xa76b) { return 1; }
    else if (ch >= 0xa76d && ch <= 0xa76d) { return 1; }
    else if (ch >= 0xa76f && ch <= 0xa76f) { return 1; }
    else if (ch >= 0xa771 && ch <= 0xa778) { return 1; }
    else if (ch >= 0xa77a && ch <= 0xa77a) { return 1; }
    else if (ch >= 0xa77c && ch <= 0xa77c) { return 1; }
    else if (ch >= 0xa77f && ch <= 0xa77f) { return 1; }
    else if (ch >= 0xa781 && ch <= 0xa781) { return 1; }
    else if (ch >= 0xa783 && ch <= 0xa783) { return 1; }
    else if (ch >= 0xa785 && ch <= 0xa785) { return 1; }
    else if (ch >= 0xa787 && ch <= 0xa787) { return 1; }
    else if (ch >= 0xa78c && ch <= 0xa78c) { return 1; }
    else if (ch >= 0xa78e && ch <= 0xa78e) { return 1; }
    else if (ch >= 0xa791 && ch <= 0xa791) { return 1; }
    else if (ch >= 0xa793 && ch <= 0xa793) { return 1; }
    else if (ch >= 0xa7a1 && ch <= 0xa7a1) { return 1; }
    else if (ch >= 0xa7a3 && ch <= 0xa7a3) { return 1; }
    else if (ch >= 0xa7a5 && ch <= 0xa7a5) { return 1; }
    else if (ch >= 0xa7a7 && ch <= 0xa7a7) { return 1; }
    else if (ch >= 0xa7a9 && ch <= 0xa7a9) { return 1; }
    else if (ch >= 0xa7fa && ch <= 0xa7fa) { return 1; }
    else if (ch >= 0xfb00 && ch <= 0xfb06) { return 1; }
    else if (ch >= 0xfb13 && ch <= 0xfb17) { return 1; }
    else if (ch >= 0xff41 && ch <= 0xff5a) { return 1; }
    else if (ch >= 0x10428 && ch <= 0x1044f) { return 1; }
    else if (ch >= 0x1d41a && ch <= 0x1d433) { return 1; }
    else if (ch >= 0x1d44e && ch <= 0x1d454) { return 1; }
    else if (ch >= 0x1d456 && ch <= 0x1d467) { return 1; }
    else if (ch >= 0x1d482 && ch <= 0x1d49b) { return 1; }
    else if (ch >= 0x1d4b6 && ch <= 0x1d4b9) { return 1; }
    else if (ch >= 0x1d4bb && ch <= 0x1d4bb) { return 1; }
    else if (ch >= 0x1d4bd && ch <= 0x1d4c3) { return 1; }
    else if (ch >= 0x1d4c5 && ch <= 0x1d4cf) { return 1; }
    else if (ch >= 0x1d4ea && ch <= 0x1d503) { return 1; }
    else if (ch >= 0x1d51e && ch <= 0x1d537) { return 1; }
    else if (ch >= 0x1d552 && ch <= 0x1d56b) { return 1; }
    else if (ch >= 0x1d586 && ch <= 0x1d59f) { return 1; }
    else if (ch >= 0x1d5ba && ch <= 0x1d5d3) { return 1; }
    else if (ch >= 0x1d5ee && ch <= 0x1d607) { return 1; }
    else if (ch >= 0x1d622 && ch <= 0x1d63b) { return 1; }
    else if (ch >= 0x1d656 && ch <= 0x1d66f) { return 1; }
    else if (ch >= 0x1d68a && ch <= 0x1d6a5) { return 1; }
    else if (ch >= 0x1d6c2 && ch <= 0x1d6da) { return 1; }
    else if (ch >= 0x1d6dc && ch <= 0x1d6e1) { return 1; }
    else if (ch >= 0x1d6fc && ch <= 0x1d714) { return 1; }
    else if (ch >= 0x1d716 && ch <= 0x1d71b) { return 1; }
    else if (ch >= 0x1d736 && ch <= 0x1d74e) { return 1; }
    else if (ch >= 0x1d750 && ch <= 0x1d755) { return 1; }
    else if (ch >= 0x1d770 && ch <= 0x1d788) { return 1; }
    else if (ch >= 0x1d78a && ch <= 0x1d78f) { return 1; }
    else if (ch >= 0x1d7aa && ch <= 0x1d7c2) { return 1; }
    else if (ch >= 0x1d7c4 && ch <= 0x1d7c9) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_lm(esch_unicode ch)
{
    if (ch >= 0x2b0 && ch <= 0x2c1) { return 1; }
    else if (ch >= 0x2c6 && ch <= 0x2d1) { return 1; }
    else if (ch >= 0x2e0 && ch <= 0x2e4) { return 1; }
    else if (ch >= 0x2ec && ch <= 0x2ec) { return 1; }
    else if (ch >= 0x2ee && ch <= 0x2ee) { return 1; }
    else if (ch >= 0x374 && ch <= 0x374) { return 1; }
    else if (ch >= 0x37a && ch <= 0x37a) { return 1; }
    else if (ch >= 0x559 && ch <= 0x559) { return 1; }
    else if (ch >= 0x640 && ch <= 0x640) { return 1; }
    else if (ch >= 0x6e5 && ch <= 0x6e6) { return 1; }
    else if (ch >= 0x7f4 && ch <= 0x7f5) { return 1; }
    else if (ch >= 0x7fa && ch <= 0x7fa) { return 1; }
    else if (ch >= 0x81a && ch <= 0x81a) { return 1; }
    else if (ch >= 0x824 && ch <= 0x824) { return 1; }
    else if (ch >= 0x828 && ch <= 0x828) { return 1; }
    else if (ch >= 0x971 && ch <= 0x971) { return 1; }
    else if (ch >= 0xe46 && ch <= 0xe46) { return 1; }
    else if (ch >= 0xec6 && ch <= 0xec6) { return 1; }
    else if (ch >= 0x10fc && ch <= 0x10fc) { return 1; }
    else if (ch >= 0x17d7 && ch <= 0x17d7) { return 1; }
    else if (ch >= 0x1843 && ch <= 0x1843) { return 1; }
    else if (ch >= 0x1aa7 && ch <= 0x1aa7) { return 1; }
    else if (ch >= 0x1c78 && ch <= 0x1c7d) { return 1; }
    else if (ch >= 0x1d2c && ch <= 0x1d6a) { return 1; }
    else if (ch >= 0x1d78 && ch <= 0x1d78) { return 1; }
    else if (ch >= 0x1d9b && ch <= 0x1dbf) { return 1; }
    else if (ch >= 0x2071 && ch <= 0x2071) { return 1; }
    else if (ch >= 0x207f && ch <= 0x207f) { return 1; }
    else if (ch >= 0x2090 && ch <= 0x209c) { return 1; }
    else if (ch >= 0x2c7c && ch <= 0x2c7d) { return 1; }
    else if (ch >= 0x2d6f && ch <= 0x2d6f) { return 1; }
    else if (ch >= 0x2e2f && ch <= 0x2e2f) { return 1; }
    else if (ch >= 0x3005 && ch <= 0x3005) { return 1; }
    else if (ch >= 0x3031 && ch <= 0x3035) { return 1; }
    else if (ch >= 0x303b && ch <= 0x303b) { return 1; }
    else if (ch >= 0x309d && ch <= 0x309e) { return 1; }
    else if (ch >= 0x30fc && ch <= 0x30fe) { return 1; }
    else if (ch >= 0xa015 && ch <= 0xa015) { return 1; }
    else if (ch >= 0xa4f8 && ch <= 0xa4fd) { return 1; }
    else if (ch >= 0xa60c && ch <= 0xa60c) { return 1; }
    else if (ch >= 0xa67f && ch <= 0xa67f) { return 1; }
    else if (ch >= 0xa717 && ch <= 0xa71f) { return 1; }
    else if (ch >= 0xa770 && ch <= 0xa770) { return 1; }
    else if (ch >= 0xa788 && ch <= 0xa788) { return 1; }
    else if (ch >= 0xa7f8 && ch <= 0xa7f9) { return 1; }
    else if (ch >= 0xa9cf && ch <= 0xa9cf) { return 1; }
    else if (ch >= 0xaa70 && ch <= 0xaa70) { return 1; }
    else if (ch >= 0xaadd && ch <= 0xaadd) { return 1; }
    else if (ch >= 0xaaf3 && ch <= 0xaaf4) { return 1; }
    else if (ch >= 0xff70 && ch <= 0xff70) { return 1; }
    else if (ch >= 0xff9e && ch <= 0xff9f) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_lo(esch_unicode ch)
{
    if (ch >= 0xaa && ch <= 0xaa) { return 1; }
    else if (ch >= 0xba && ch <= 0xba) { return 1; }
    else if (ch >= 0x1bb && ch <= 0x1bb) { return 1; }
    else if (ch >= 0x1c0 && ch <= 0x1c3) { return 1; }
    else if (ch >= 0x294 && ch <= 0x294) { return 1; }
    else if (ch >= 0x5d0 && ch <= 0x5ea) { return 1; }
    else if (ch >= 0x5f0 && ch <= 0x5f2) { return 1; }
    else if (ch >= 0x620 && ch <= 0x63f) { return 1; }
    else if (ch >= 0x641 && ch <= 0x64a) { return 1; }
    else if (ch >= 0x66e && ch <= 0x66f) { return 1; }
    else if (ch >= 0x671 && ch <= 0x6d3) { return 1; }
    else if (ch >= 0x6d5 && ch <= 0x6d5) { return 1; }
    else if (ch >= 0x6ee && ch <= 0x6ef) { return 1; }
    else if (ch >= 0x6fa && ch <= 0x6fc) { return 1; }
    else if (ch >= 0x6ff && ch <= 0x6ff) { return 1; }
    else if (ch >= 0x710 && ch <= 0x710) { return 1; }
    else if (ch >= 0x712 && ch <= 0x72f) { return 1; }
    else if (ch >= 0x74d && ch <= 0x7a5) { return 1; }
    else if (ch >= 0x7b1 && ch <= 0x7b1) { return 1; }
    else if (ch >= 0x7ca && ch <= 0x7ea) { return 1; }
    else if (ch >= 0x800 && ch <= 0x815) { return 1; }
    else if (ch >= 0x840 && ch <= 0x858) { return 1; }
    else if (ch >= 0x8a0 && ch <= 0x8a0) { return 1; }
    else if (ch >= 0x8a2 && ch <= 0x8ac) { return 1; }
    else if (ch >= 0x904 && ch <= 0x939) { return 1; }
    else if (ch >= 0x93d && ch <= 0x93d) { return 1; }
    else if (ch >= 0x950 && ch <= 0x950) { return 1; }
    else if (ch >= 0x958 && ch <= 0x961) { return 1; }
    else if (ch >= 0x972 && ch <= 0x977) { return 1; }
    else if (ch >= 0x979 && ch <= 0x97f) { return 1; }
    else if (ch >= 0x985 && ch <= 0x98c) { return 1; }
    else if (ch >= 0x98f && ch <= 0x990) { return 1; }
    else if (ch >= 0x993 && ch <= 0x9a8) { return 1; }
    else if (ch >= 0x9aa && ch <= 0x9b0) { return 1; }
    else if (ch >= 0x9b2 && ch <= 0x9b2) { return 1; }
    else if (ch >= 0x9b6 && ch <= 0x9b9) { return 1; }
    else if (ch >= 0x9bd && ch <= 0x9bd) { return 1; }
    else if (ch >= 0x9ce && ch <= 0x9ce) { return 1; }
    else if (ch >= 0x9dc && ch <= 0x9dd) { return 1; }
    else if (ch >= 0x9df && ch <= 0x9e1) { return 1; }
    else if (ch >= 0x9f0 && ch <= 0x9f1) { return 1; }
    else if (ch >= 0xa05 && ch <= 0xa0a) { return 1; }
    else if (ch >= 0xa0f && ch <= 0xa10) { return 1; }
    else if (ch >= 0xa13 && ch <= 0xa28) { return 1; }
    else if (ch >= 0xa2a && ch <= 0xa30) { return 1; }
    else if (ch >= 0xa32 && ch <= 0xa33) { return 1; }
    else if (ch >= 0xa35 && ch <= 0xa36) { return 1; }
    else if (ch >= 0xa38 && ch <= 0xa39) { return 1; }
    else if (ch >= 0xa59 && ch <= 0xa5c) { return 1; }
    else if (ch >= 0xa5e && ch <= 0xa5e) { return 1; }
    else if (ch >= 0xa72 && ch <= 0xa74) { return 1; }
    else if (ch >= 0xa85 && ch <= 0xa8d) { return 1; }
    else if (ch >= 0xa8f && ch <= 0xa91) { return 1; }
    else if (ch >= 0xa93 && ch <= 0xaa8) { return 1; }
    else if (ch >= 0xaaa && ch <= 0xab0) { return 1; }
    else if (ch >= 0xab2 && ch <= 0xab3) { return 1; }
    else if (ch >= 0xab5 && ch <= 0xab9) { return 1; }
    else if (ch >= 0xabd && ch <= 0xabd) { return 1; }
    else if (ch >= 0xad0 && ch <= 0xad0) { return 1; }
    else if (ch >= 0xae0 && ch <= 0xae1) { return 1; }
    else if (ch >= 0xb05 && ch <= 0xb0c) { return 1; }
    else if (ch >= 0xb0f && ch <= 0xb10) { return 1; }
    else if (ch >= 0xb13 && ch <= 0xb28) { return 1; }
    else if (ch >= 0xb2a && ch <= 0xb30) { return 1; }
    else if (ch >= 0xb32 && ch <= 0xb33) { return 1; }
    else if (ch >= 0xb35 && ch <= 0xb39) { return 1; }
    else if (ch >= 0xb3d && ch <= 0xb3d) { return 1; }
    else if (ch >= 0xb5c && ch <= 0xb5d) { return 1; }
    else if (ch >= 0xb5f && ch <= 0xb61) { return 1; }
    else if (ch >= 0xb71 && ch <= 0xb71) { return 1; }
    else if (ch >= 0xb83 && ch <= 0xb83) { return 1; }
    else if (ch >= 0xb85 && ch <= 0xb8a) { return 1; }
    else if (ch >= 0xb8e && ch <= 0xb90) { return 1; }
    else if (ch >= 0xb92 && ch <= 0xb95) { return 1; }
    else if (ch >= 0xb99 && ch <= 0xb9a) { return 1; }
    else if (ch >= 0xb9c && ch <= 0xb9c) { return 1; }
    else if (ch >= 0xb9e && ch <= 0xb9f) { return 1; }
    else if (ch >= 0xba3 && ch <= 0xba4) { return 1; }
    else if (ch >= 0xba8 && ch <= 0xbaa) { return 1; }
    else if (ch >= 0xbae && ch <= 0xbb9) { return 1; }
    else if (ch >= 0xbd0 && ch <= 0xbd0) { return 1; }
    else if (ch >= 0xc05 && ch <= 0xc0c) { return 1; }
    else if (ch >= 0xc0e && ch <= 0xc10) { return 1; }
    else if (ch >= 0xc12 && ch <= 0xc28) { return 1; }
    else if (ch >= 0xc2a && ch <= 0xc33) { return 1; }
    else if (ch >= 0xc35 && ch <= 0xc39) { return 1; }
    else if (ch >= 0xc3d && ch <= 0xc3d) { return 1; }
    else if (ch >= 0xc58 && ch <= 0xc59) { return 1; }
    else if (ch >= 0xc60 && ch <= 0xc61) { return 1; }
    else if (ch >= 0xc85 && ch <= 0xc8c) { return 1; }
    else if (ch >= 0xc8e && ch <= 0xc90) { return 1; }
    else if (ch >= 0xc92 && ch <= 0xca8) { return 1; }
    else if (ch >= 0xcaa && ch <= 0xcb3) { return 1; }
    else if (ch >= 0xcb5 && ch <= 0xcb9) { return 1; }
    else if (ch >= 0xcbd && ch <= 0xcbd) { return 1; }
    else if (ch >= 0xcde && ch <= 0xcde) { return 1; }
    else if (ch >= 0xce0 && ch <= 0xce1) { return 1; }
    else if (ch >= 0xcf1 && ch <= 0xcf2) { return 1; }
    else if (ch >= 0xd05 && ch <= 0xd0c) { return 1; }
    else if (ch >= 0xd0e && ch <= 0xd10) { return 1; }
    else if (ch >= 0xd12 && ch <= 0xd3a) { return 1; }
    else if (ch >= 0xd3d && ch <= 0xd3d) { return 1; }
    else if (ch >= 0xd4e && ch <= 0xd4e) { return 1; }
    else if (ch >= 0xd60 && ch <= 0xd61) { return 1; }
    else if (ch >= 0xd7a && ch <= 0xd7f) { return 1; }
    else if (ch >= 0xd85 && ch <= 0xd96) { return 1; }
    else if (ch >= 0xd9a && ch <= 0xdb1) { return 1; }
    else if (ch >= 0xdb3 && ch <= 0xdbb) { return 1; }
    else if (ch >= 0xdbd && ch <= 0xdbd) { return 1; }
    else if (ch >= 0xdc0 && ch <= 0xdc6) { return 1; }
    else if (ch >= 0xe01 && ch <= 0xe30) { return 1; }
    else if (ch >= 0xe32 && ch <= 0xe33) { return 1; }
    else if (ch >= 0xe40 && ch <= 0xe45) { return 1; }
    else if (ch >= 0xe81 && ch <= 0xe82) { return 1; }
    else if (ch >= 0xe84 && ch <= 0xe84) { return 1; }
    else if (ch >= 0xe87 && ch <= 0xe88) { return 1; }
    else if (ch >= 0xe8a && ch <= 0xe8a) { return 1; }
    else if (ch >= 0xe8d && ch <= 0xe8d) { return 1; }
    else if (ch >= 0xe94 && ch <= 0xe97) { return 1; }
    else if (ch >= 0xe99 && ch <= 0xe9f) { return 1; }
    else if (ch >= 0xea1 && ch <= 0xea3) { return 1; }
    else if (ch >= 0xea5 && ch <= 0xea5) { return 1; }
    else if (ch >= 0xea7 && ch <= 0xea7) { return 1; }
    else if (ch >= 0xeaa && ch <= 0xeab) { return 1; }
    else if (ch >= 0xead && ch <= 0xeb0) { return 1; }
    else if (ch >= 0xeb2 && ch <= 0xeb3) { return 1; }
    else if (ch >= 0xebd && ch <= 0xebd) { return 1; }
    else if (ch >= 0xec0 && ch <= 0xec4) { return 1; }
    else if (ch >= 0xedc && ch <= 0xedf) { return 1; }
    else if (ch >= 0xf00 && ch <= 0xf00) { return 1; }
    else if (ch >= 0xf40 && ch <= 0xf47) { return 1; }
    else if (ch >= 0xf49 && ch <= 0xf6c) { return 1; }
    else if (ch >= 0xf88 && ch <= 0xf8c) { return 1; }
    else if (ch >= 0x1000 && ch <= 0x102a) { return 1; }
    else if (ch >= 0x103f && ch <= 0x103f) { return 1; }
    else if (ch >= 0x1050 && ch <= 0x1055) { return 1; }
    else if (ch >= 0x105a && ch <= 0x105d) { return 1; }
    else if (ch >= 0x1061 && ch <= 0x1061) { return 1; }
    else if (ch >= 0x1065 && ch <= 0x1066) { return 1; }
    else if (ch >= 0x106e && ch <= 0x1070) { return 1; }
    else if (ch >= 0x1075 && ch <= 0x1081) { return 1; }
    else if (ch >= 0x108e && ch <= 0x108e) { return 1; }
    else if (ch >= 0x10d0 && ch <= 0x10fa) { return 1; }
    else if (ch >= 0x10fd && ch <= 0x1248) { return 1; }
    else if (ch >= 0x124a && ch <= 0x124d) { return 1; }
    else if (ch >= 0x1250 && ch <= 0x1256) { return 1; }
    else if (ch >= 0x1258 && ch <= 0x1258) { return 1; }
    else if (ch >= 0x125a && ch <= 0x125d) { return 1; }
    else if (ch >= 0x1260 && ch <= 0x1288) { return 1; }
    else if (ch >= 0x128a && ch <= 0x128d) { return 1; }
    else if (ch >= 0x1290 && ch <= 0x12b0) { return 1; }
    else if (ch >= 0x12b2 && ch <= 0x12b5) { return 1; }
    else if (ch >= 0x12b8 && ch <= 0x12be) { return 1; }
    else if (ch >= 0x12c0 && ch <= 0x12c0) { return 1; }
    else if (ch >= 0x12c2 && ch <= 0x12c5) { return 1; }
    else if (ch >= 0x12c8 && ch <= 0x12d6) { return 1; }
    else if (ch >= 0x12d8 && ch <= 0x1310) { return 1; }
    else if (ch >= 0x1312 && ch <= 0x1315) { return 1; }
    else if (ch >= 0x1318 && ch <= 0x135a) { return 1; }
    else if (ch >= 0x1380 && ch <= 0x138f) { return 1; }
    else if (ch >= 0x13a0 && ch <= 0x13f4) { return 1; }
    else if (ch >= 0x1401 && ch <= 0x166c) { return 1; }
    else if (ch >= 0x166f && ch <= 0x167f) { return 1; }
    else if (ch >= 0x1681 && ch <= 0x169a) { return 1; }
    else if (ch >= 0x16a0 && ch <= 0x16ea) { return 1; }
    else if (ch >= 0x1700 && ch <= 0x170c) { return 1; }
    else if (ch >= 0x170e && ch <= 0x1711) { return 1; }
    else if (ch >= 0x1720 && ch <= 0x1731) { return 1; }
    else if (ch >= 0x1740 && ch <= 0x1751) { return 1; }
    else if (ch >= 0x1760 && ch <= 0x176c) { return 1; }
    else if (ch >= 0x176e && ch <= 0x1770) { return 1; }
    else if (ch >= 0x1780 && ch <= 0x17b3) { return 1; }
    else if (ch >= 0x17dc && ch <= 0x17dc) { return 1; }
    else if (ch >= 0x1820 && ch <= 0x1842) { return 1; }
    else if (ch >= 0x1844 && ch <= 0x1877) { return 1; }
    else if (ch >= 0x1880 && ch <= 0x18a8) { return 1; }
    else if (ch >= 0x18aa && ch <= 0x18aa) { return 1; }
    else if (ch >= 0x18b0 && ch <= 0x18f5) { return 1; }
    else if (ch >= 0x1900 && ch <= 0x191c) { return 1; }
    else if (ch >= 0x1950 && ch <= 0x196d) { return 1; }
    else if (ch >= 0x1970 && ch <= 0x1974) { return 1; }
    else if (ch >= 0x1980 && ch <= 0x19ab) { return 1; }
    else if (ch >= 0x19c1 && ch <= 0x19c7) { return 1; }
    else if (ch >= 0x1a00 && ch <= 0x1a16) { return 1; }
    else if (ch >= 0x1a20 && ch <= 0x1a54) { return 1; }
    else if (ch >= 0x1b05 && ch <= 0x1b33) { return 1; }
    else if (ch >= 0x1b45 && ch <= 0x1b4b) { return 1; }
    else if (ch >= 0x1b83 && ch <= 0x1ba0) { return 1; }
    else if (ch >= 0x1bae && ch <= 0x1baf) { return 1; }
    else if (ch >= 0x1bba && ch <= 0x1be5) { return 1; }
    else if (ch >= 0x1c00 && ch <= 0x1c23) { return 1; }
    else if (ch >= 0x1c4d && ch <= 0x1c4f) { return 1; }
    else if (ch >= 0x1c5a && ch <= 0x1c77) { return 1; }
    else if (ch >= 0x1ce9 && ch <= 0x1cec) { return 1; }
    else if (ch >= 0x1cee && ch <= 0x1cf1) { return 1; }
    else if (ch >= 0x1cf5 && ch <= 0x1cf6) { return 1; }
    else if (ch >= 0x2135 && ch <= 0x2138) { return 1; }
    else if (ch >= 0x2d30 && ch <= 0x2d67) { return 1; }
    else if (ch >= 0x2d80 && ch <= 0x2d96) { return 1; }
    else if (ch >= 0x2da0 && ch <= 0x2da6) { return 1; }
    else if (ch >= 0x2da8 && ch <= 0x2dae) { return 1; }
    else if (ch >= 0x2db0 && ch <= 0x2db6) { return 1; }
    else if (ch >= 0x2db8 && ch <= 0x2dbe) { return 1; }
    else if (ch >= 0x2dc0 && ch <= 0x2dc6) { return 1; }
    else if (ch >= 0x2dc8 && ch <= 0x2dce) { return 1; }
    else if (ch >= 0x2dd0 && ch <= 0x2dd6) { return 1; }
    else if (ch >= 0x2dd8 && ch <= 0x2dde) { return 1; }
    else if (ch >= 0x3006 && ch <= 0x3006) { return 1; }
    else if (ch >= 0x303c && ch <= 0x303c) { return 1; }
    else if (ch >= 0x3041 && ch <= 0x3096) { return 1; }
    else if (ch >= 0x309f && ch <= 0x309f) { return 1; }
    else if (ch >= 0x30a1 && ch <= 0x30fa) { return 1; }
    else if (ch >= 0x30ff && ch <= 0x30ff) { return 1; }
    else if (ch >= 0x3105 && ch <= 0x312d) { return 1; }
    else if (ch >= 0x3131 && ch <= 0x318e) { return 1; }
    else if (ch >= 0x31a0 && ch <= 0x31ba) { return 1; }
    else if (ch >= 0x31f0 && ch <= 0x31ff) { return 1; }
    else if (ch >= 0x3400 && ch <= 0x3400) { return 1; }
    else if (ch >= 0x4db5 && ch <= 0x4db5) { return 1; }
    else if (ch >= 0x4e00 && ch <= 0x4e00) { return 1; }
    else if (ch >= 0x9fcc && ch <= 0x9fcc) { return 1; }
    else if (ch >= 0xa000 && ch <= 0xa014) { return 1; }
    else if (ch >= 0xa016 && ch <= 0xa48c) { return 1; }
    else if (ch >= 0xa4d0 && ch <= 0xa4f7) { return 1; }
    else if (ch >= 0xa500 && ch <= 0xa60b) { return 1; }
    else if (ch >= 0xa610 && ch <= 0xa61f) { return 1; }
    else if (ch >= 0xa62a && ch <= 0xa62b) { return 1; }
    else if (ch >= 0xa66e && ch <= 0xa66e) { return 1; }
    else if (ch >= 0xa6a0 && ch <= 0xa6e5) { return 1; }
    else if (ch >= 0xa7fb && ch <= 0xa801) { return 1; }
    else if (ch >= 0xa803 && ch <= 0xa805) { return 1; }
    else if (ch >= 0xa807 && ch <= 0xa80a) { return 1; }
    else if (ch >= 0xa80c && ch <= 0xa822) { return 1; }
    else if (ch >= 0xa840 && ch <= 0xa873) { return 1; }
    else if (ch >= 0xa882 && ch <= 0xa8b3) { return 1; }
    else if (ch >= 0xa8f2 && ch <= 0xa8f7) { return 1; }
    else if (ch >= 0xa8fb && ch <= 0xa8fb) { return 1; }
    else if (ch >= 0xa90a && ch <= 0xa925) { return 1; }
    else if (ch >= 0xa930 && ch <= 0xa946) { return 1; }
    else if (ch >= 0xa960 && ch <= 0xa97c) { return 1; }
    else if (ch >= 0xa984 && ch <= 0xa9b2) { return 1; }
    else if (ch >= 0xaa00 && ch <= 0xaa28) { return 1; }
    else if (ch >= 0xaa40 && ch <= 0xaa42) { return 1; }
    else if (ch >= 0xaa44 && ch <= 0xaa4b) { return 1; }
    else if (ch >= 0xaa60 && ch <= 0xaa6f) { return 1; }
    else if (ch >= 0xaa71 && ch <= 0xaa76) { return 1; }
    else if (ch >= 0xaa7a && ch <= 0xaa7a) { return 1; }
    else if (ch >= 0xaa80 && ch <= 0xaaaf) { return 1; }
    else if (ch >= 0xaab1 && ch <= 0xaab1) { return 1; }
    else if (ch >= 0xaab5 && ch <= 0xaab6) { return 1; }
    else if (ch >= 0xaab9 && ch <= 0xaabd) { return 1; }
    else if (ch >= 0xaac0 && ch <= 0xaac0) { return 1; }
    else if (ch >= 0xaac2 && ch <= 0xaac2) { return 1; }
    else if (ch >= 0xaadb && ch <= 0xaadc) { return 1; }
    else if (ch >= 0xaae0 && ch <= 0xaaea) { return 1; }
    else if (ch >= 0xaaf2 && ch <= 0xaaf2) { return 1; }
    else if (ch >= 0xab01 && ch <= 0xab06) { return 1; }
    else if (ch >= 0xab09 && ch <= 0xab0e) { return 1; }
    else if (ch >= 0xab11 && ch <= 0xab16) { return 1; }
    else if (ch >= 0xab20 && ch <= 0xab26) { return 1; }
    else if (ch >= 0xab28 && ch <= 0xab2e) { return 1; }
    else if (ch >= 0xabc0 && ch <= 0xabe2) { return 1; }
    else if (ch >= 0xac00 && ch <= 0xac00) { return 1; }
    else if (ch >= 0xd7a3 && ch <= 0xd7a3) { return 1; }
    else if (ch >= 0xd7b0 && ch <= 0xd7c6) { return 1; }
    else if (ch >= 0xd7cb && ch <= 0xd7fb) { return 1; }
    else if (ch >= 0xf900 && ch <= 0xfa6d) { return 1; }
    else if (ch >= 0xfa70 && ch <= 0xfad9) { return 1; }
    else if (ch >= 0xfb1d && ch <= 0xfb1d) { return 1; }
    else if (ch >= 0xfb1f && ch <= 0xfb28) { return 1; }
    else if (ch >= 0xfb2a && ch <= 0xfb36) { return 1; }
    else if (ch >= 0xfb38 && ch <= 0xfb3c) { return 1; }
    else if (ch >= 0xfb3e && ch <= 0xfb3e) { return 1; }
    else if (ch >= 0xfb40 && ch <= 0xfb41) { return 1; }
    else if (ch >= 0xfb43 && ch <= 0xfb44) { return 1; }
    else if (ch >= 0xfb46 && ch <= 0xfbb1) { return 1; }
    else if (ch >= 0xfbd3 && ch <= 0xfd3d) { return 1; }
    else if (ch >= 0xfd50 && ch <= 0xfd8f) { return 1; }
    else if (ch >= 0xfd92 && ch <= 0xfdc7) { return 1; }
    else if (ch >= 0xfdf0 && ch <= 0xfdfb) { return 1; }
    else if (ch >= 0xfe70 && ch <= 0xfe74) { return 1; }
    else if (ch >= 0xfe76 && ch <= 0xfefc) { return 1; }
    else if (ch >= 0xff66 && ch <= 0xff6f) { return 1; }
    else if (ch >= 0xff71 && ch <= 0xff9d) { return 1; }
    else if (ch >= 0xffa0 && ch <= 0xffbe) { return 1; }
    else if (ch >= 0xffc2 && ch <= 0xffc7) { return 1; }
    else if (ch >= 0xffca && ch <= 0xffcf) { return 1; }
    else if (ch >= 0xffd2 && ch <= 0xffd7) { return 1; }
    else if (ch >= 0xffda && ch <= 0xffdc) { return 1; }
    else if (ch >= 0x10000 && ch <= 0x1000b) { return 1; }
    else if (ch >= 0x1000d && ch <= 0x10026) { return 1; }
    else if (ch >= 0x10028 && ch <= 0x1003a) { return 1; }
    else if (ch >= 0x1003c && ch <= 0x1003d) { return 1; }
    else if (ch >= 0x1003f && ch <= 0x1004d) { return 1; }
    else if (ch >= 0x10050 && ch <= 0x1005d) { return 1; }
    else if (ch >= 0x10080 && ch <= 0x100fa) { return 1; }
    else if (ch >= 0x10280 && ch <= 0x1029c) { return 1; }
    else if (ch >= 0x102a0 && ch <= 0x102d0) { return 1; }
    else if (ch >= 0x10300 && ch <= 0x1031e) { return 1; }
    else if (ch >= 0x10330 && ch <= 0x10340) { return 1; }
    else if (ch >= 0x10342 && ch <= 0x10349) { return 1; }
    else if (ch >= 0x10380 && ch <= 0x1039d) { return 1; }
    else if (ch >= 0x103a0 && ch <= 0x103c3) { return 1; }
    else if (ch >= 0x103c8 && ch <= 0x103cf) { return 1; }
    else if (ch >= 0x10450 && ch <= 0x1049d) { return 1; }
    else if (ch >= 0x10800 && ch <= 0x10805) { return 1; }
    else if (ch >= 0x10808 && ch <= 0x10808) { return 1; }
    else if (ch >= 0x1080a && ch <= 0x10835) { return 1; }
    else if (ch >= 0x10837 && ch <= 0x10838) { return 1; }
    else if (ch >= 0x1083c && ch <= 0x1083c) { return 1; }
    else if (ch >= 0x1083f && ch <= 0x10855) { return 1; }
    else if (ch >= 0x10900 && ch <= 0x10915) { return 1; }
    else if (ch >= 0x10920 && ch <= 0x10939) { return 1; }
    else if (ch >= 0x10980 && ch <= 0x109b7) { return 1; }
    else if (ch >= 0x109be && ch <= 0x109bf) { return 1; }
    else if (ch >= 0x10a00 && ch <= 0x10a00) { return 1; }
    else if (ch >= 0x10a10 && ch <= 0x10a13) { return 1; }
    else if (ch >= 0x10a15 && ch <= 0x10a17) { return 1; }
    else if (ch >= 0x10a19 && ch <= 0x10a33) { return 1; }
    else if (ch >= 0x10a60 && ch <= 0x10a7c) { return 1; }
    else if (ch >= 0x10b00 && ch <= 0x10b35) { return 1; }
    else if (ch >= 0x10b40 && ch <= 0x10b55) { return 1; }
    else if (ch >= 0x10b60 && ch <= 0x10b72) { return 1; }
    else if (ch >= 0x10c00 && ch <= 0x10c48) { return 1; }
    else if (ch >= 0x11003 && ch <= 0x11037) { return 1; }
    else if (ch >= 0x11083 && ch <= 0x110af) { return 1; }
    else if (ch >= 0x110d0 && ch <= 0x110e8) { return 1; }
    else if (ch >= 0x11103 && ch <= 0x11126) { return 1; }
    else if (ch >= 0x11183 && ch <= 0x111b2) { return 1; }
    else if (ch >= 0x111c1 && ch <= 0x111c4) { return 1; }
    else if (ch >= 0x11680 && ch <= 0x116aa) { return 1; }
    else if (ch >= 0x12000 && ch <= 0x1236e) { return 1; }
    else if (ch >= 0x13000 && ch <= 0x1342e) { return 1; }
    else if (ch >= 0x16800 && ch <= 0x16a38) { return 1; }
    else if (ch >= 0x16f00 && ch <= 0x16f44) { return 1; }
    else if (ch >= 0x16f50 && ch <= 0x16f50) { return 1; }
    else if (ch >= 0x1b000 && ch <= 0x1b001) { return 1; }
    else if (ch >= 0x1ee00 && ch <= 0x1ee03) { return 1; }
    else if (ch >= 0x1ee05 && ch <= 0x1ee1f) { return 1; }
    else if (ch >= 0x1ee21 && ch <= 0x1ee22) { return 1; }
    else if (ch >= 0x1ee24 && ch <= 0x1ee24) { return 1; }
    else if (ch >= 0x1ee27 && ch <= 0x1ee27) { return 1; }
    else if (ch >= 0x1ee29 && ch <= 0x1ee32) { return 1; }
    else if (ch >= 0x1ee34 && ch <= 0x1ee37) { return 1; }
    else if (ch >= 0x1ee39 && ch <= 0x1ee39) { return 1; }
    else if (ch >= 0x1ee3b && ch <= 0x1ee3b) { return 1; }
    else if (ch >= 0x1ee42 && ch <= 0x1ee42) { return 1; }
    else if (ch >= 0x1ee47 && ch <= 0x1ee47) { return 1; }
    else if (ch >= 0x1ee49 && ch <= 0x1ee49) { return 1; }
    else if (ch >= 0x1ee4b && ch <= 0x1ee4b) { return 1; }
    else if (ch >= 0x1ee4d && ch <= 0x1ee4f) { return 1; }
    else if (ch >= 0x1ee51 && ch <= 0x1ee52) { return 1; }
    else if (ch >= 0x1ee54 && ch <= 0x1ee54) { return 1; }
    else if (ch >= 0x1ee57 && ch <= 0x1ee57) { return 1; }
    else if (ch >= 0x1ee59 && ch <= 0x1ee59) { return 1; }
    else if (ch >= 0x1ee5b && ch <= 0x1ee5b) { return 1; }
    else if (ch >= 0x1ee5d && ch <= 0x1ee5d) { return 1; }
    else if (ch >= 0x1ee5f && ch <= 0x1ee5f) { return 1; }
    else if (ch >= 0x1ee61 && ch <= 0x1ee62) { return 1; }
    else if (ch >= 0x1ee64 && ch <= 0x1ee64) { return 1; }
    else if (ch >= 0x1ee67 && ch <= 0x1ee6a) { return 1; }
    else if (ch >= 0x1ee6c && ch <= 0x1ee72) { return 1; }
    else if (ch >= 0x1ee74 && ch <= 0x1ee77) { return 1; }
    else if (ch >= 0x1ee79 && ch <= 0x1ee7c) { return 1; }
    else if (ch >= 0x1ee7e && ch <= 0x1ee7e) { return 1; }
    else if (ch >= 0x1ee80 && ch <= 0x1ee89) { return 1; }
    else if (ch >= 0x1ee8b && ch <= 0x1ee9b) { return 1; }
    else if (ch >= 0x1eea1 && ch <= 0x1eea3) { return 1; }
    else if (ch >= 0x1eea5 && ch <= 0x1eea9) { return 1; }
    else if (ch >= 0x1eeab && ch <= 0x1eebb) { return 1; }
    else if (ch >= 0x20000 && ch <= 0x20000) { return 1; }
    else if (ch >= 0x2a6d6 && ch <= 0x2a6d6) { return 1; }
    else if (ch >= 0x2a700 && ch <= 0x2a700) { return 1; }
    else if (ch >= 0x2b734 && ch <= 0x2b734) { return 1; }
    else if (ch >= 0x2b740 && ch <= 0x2b740) { return 1; }
    else if (ch >= 0x2b81d && ch <= 0x2b81d) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_lt(esch_unicode ch)
{
    if (ch >= 0x1c5 && ch <= 0x1c5) { return 1; }
    else if (ch >= 0x1c8 && ch <= 0x1c8) { return 1; }
    else if (ch >= 0x1cb && ch <= 0x1cb) { return 1; }
    else if (ch >= 0x1f2 && ch <= 0x1f2) { return 1; }
    else if (ch >= 0x1f88 && ch <= 0x1f8f) { return 1; }
    else if (ch >= 0x1f98 && ch <= 0x1f9f) { return 1; }
    else if (ch >= 0x1fa8 && ch <= 0x1faf) { return 1; }
    else if (ch >= 0x1fbc && ch <= 0x1fbc) { return 1; }
    else if (ch >= 0x1fcc && ch <= 0x1fcc) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_lu(esch_unicode ch)
{
    if (ch >= 0x41 && ch <= 0x5a) { return 1; }
    else if (ch >= 0xc0 && ch <= 0xd6) { return 1; }
    else if (ch >= 0xd8 && ch <= 0xde) { return 1; }
    else if (ch >= 0x100 && ch <= 0x100) { return 1; }
    else if (ch >= 0x102 && ch <= 0x102) { return 1; }
    else if (ch >= 0x104 && ch <= 0x104) { return 1; }
    else if (ch >= 0x106 && ch <= 0x106) { return 1; }
    else if (ch >= 0x108 && ch <= 0x108) { return 1; }
    else if (ch >= 0x10a && ch <= 0x10a) { return 1; }
    else if (ch >= 0x10c && ch <= 0x10c) { return 1; }
    else if (ch >= 0x10e && ch <= 0x10e) { return 1; }
    else if (ch >= 0x110 && ch <= 0x110) { return 1; }
    else if (ch >= 0x112 && ch <= 0x112) { return 1; }
    else if (ch >= 0x114 && ch <= 0x114) { return 1; }
    else if (ch >= 0x116 && ch <= 0x116) { return 1; }
    else if (ch >= 0x118 && ch <= 0x118) { return 1; }
    else if (ch >= 0x11a && ch <= 0x11a) { return 1; }
    else if (ch >= 0x11c && ch <= 0x11c) { return 1; }
    else if (ch >= 0x11e && ch <= 0x11e) { return 1; }
    else if (ch >= 0x120 && ch <= 0x120) { return 1; }
    else if (ch >= 0x122 && ch <= 0x122) { return 1; }
    else if (ch >= 0x124 && ch <= 0x124) { return 1; }
    else if (ch >= 0x126 && ch <= 0x126) { return 1; }
    else if (ch >= 0x128 && ch <= 0x128) { return 1; }
    else if (ch >= 0x12a && ch <= 0x12a) { return 1; }
    else if (ch >= 0x12c && ch <= 0x12c) { return 1; }
    else if (ch >= 0x12e && ch <= 0x12e) { return 1; }
    else if (ch >= 0x130 && ch <= 0x130) { return 1; }
    else if (ch >= 0x132 && ch <= 0x132) { return 1; }
    else if (ch >= 0x134 && ch <= 0x134) { return 1; }
    else if (ch >= 0x136 && ch <= 0x136) { return 1; }
    else if (ch >= 0x139 && ch <= 0x139) { return 1; }
    else if (ch >= 0x13b && ch <= 0x13b) { return 1; }
    else if (ch >= 0x13d && ch <= 0x13d) { return 1; }
    else if (ch >= 0x13f && ch <= 0x13f) { return 1; }
    else if (ch >= 0x141 && ch <= 0x141) { return 1; }
    else if (ch >= 0x143 && ch <= 0x143) { return 1; }
    else if (ch >= 0x145 && ch <= 0x145) { return 1; }
    else if (ch >= 0x147 && ch <= 0x147) { return 1; }
    else if (ch >= 0x14a && ch <= 0x14a) { return 1; }
    else if (ch >= 0x14c && ch <= 0x14c) { return 1; }
    else if (ch >= 0x14e && ch <= 0x14e) { return 1; }
    else if (ch >= 0x150 && ch <= 0x150) { return 1; }
    else if (ch >= 0x152 && ch <= 0x152) { return 1; }
    else if (ch >= 0x154 && ch <= 0x154) { return 1; }
    else if (ch >= 0x156 && ch <= 0x156) { return 1; }
    else if (ch >= 0x158 && ch <= 0x158) { return 1; }
    else if (ch >= 0x15a && ch <= 0x15a) { return 1; }
    else if (ch >= 0x15c && ch <= 0x15c) { return 1; }
    else if (ch >= 0x15e && ch <= 0x15e) { return 1; }
    else if (ch >= 0x160 && ch <= 0x160) { return 1; }
    else if (ch >= 0x162 && ch <= 0x162) { return 1; }
    else if (ch >= 0x164 && ch <= 0x164) { return 1; }
    else if (ch >= 0x166 && ch <= 0x166) { return 1; }
    else if (ch >= 0x168 && ch <= 0x168) { return 1; }
    else if (ch >= 0x16a && ch <= 0x16a) { return 1; }
    else if (ch >= 0x16c && ch <= 0x16c) { return 1; }
    else if (ch >= 0x16e && ch <= 0x16e) { return 1; }
    else if (ch >= 0x170 && ch <= 0x170) { return 1; }
    else if (ch >= 0x172 && ch <= 0x172) { return 1; }
    else if (ch >= 0x174 && ch <= 0x174) { return 1; }
    else if (ch >= 0x176 && ch <= 0x176) { return 1; }
    else if (ch >= 0x178 && ch <= 0x179) { return 1; }
    else if (ch >= 0x17b && ch <= 0x17b) { return 1; }
    else if (ch >= 0x17d && ch <= 0x17d) { return 1; }
    else if (ch >= 0x181 && ch <= 0x182) { return 1; }
    else if (ch >= 0x184 && ch <= 0x184) { return 1; }
    else if (ch >= 0x186 && ch <= 0x187) { return 1; }
    else if (ch >= 0x189 && ch <= 0x18b) { return 1; }
    else if (ch >= 0x18e && ch <= 0x191) { return 1; }
    else if (ch >= 0x193 && ch <= 0x194) { return 1; }
    else if (ch >= 0x196 && ch <= 0x198) { return 1; }
    else if (ch >= 0x19c && ch <= 0x19d) { return 1; }
    else if (ch >= 0x19f && ch <= 0x1a0) { return 1; }
    else if (ch >= 0x1a2 && ch <= 0x1a2) { return 1; }
    else if (ch >= 0x1a4 && ch <= 0x1a4) { return 1; }
    else if (ch >= 0x1a6 && ch <= 0x1a7) { return 1; }
    else if (ch >= 0x1a9 && ch <= 0x1a9) { return 1; }
    else if (ch >= 0x1ac && ch <= 0x1ac) { return 1; }
    else if (ch >= 0x1ae && ch <= 0x1af) { return 1; }
    else if (ch >= 0x1b1 && ch <= 0x1b3) { return 1; }
    else if (ch >= 0x1b5 && ch <= 0x1b5) { return 1; }
    else if (ch >= 0x1b7 && ch <= 0x1b8) { return 1; }
    else if (ch >= 0x1bc && ch <= 0x1bc) { return 1; }
    else if (ch >= 0x1c4 && ch <= 0x1c4) { return 1; }
    else if (ch >= 0x1c7 && ch <= 0x1c7) { return 1; }
    else if (ch >= 0x1ca && ch <= 0x1ca) { return 1; }
    else if (ch >= 0x1cd && ch <= 0x1cd) { return 1; }
    else if (ch >= 0x1cf && ch <= 0x1cf) { return 1; }
    else if (ch >= 0x1d1 && ch <= 0x1d1) { return 1; }
    else if (ch >= 0x1d3 && ch <= 0x1d3) { return 1; }
    else if (ch >= 0x1d5 && ch <= 0x1d5) { return 1; }
    else if (ch >= 0x1d7 && ch <= 0x1d7) { return 1; }
    else if (ch >= 0x1d9 && ch <= 0x1d9) { return 1; }
    else if (ch >= 0x1db && ch <= 0x1db) { return 1; }
    else if (ch >= 0x1de && ch <= 0x1de) { return 1; }
    else if (ch >= 0x1e0 && ch <= 0x1e0) { return 1; }
    else if (ch >= 0x1e2 && ch <= 0x1e2) { return 1; }
    else if (ch >= 0x1e4 && ch <= 0x1e4) { return 1; }
    else if (ch >= 0x1e6 && ch <= 0x1e6) { return 1; }
    else if (ch >= 0x1e8 && ch <= 0x1e8) { return 1; }
    else if (ch >= 0x1ea && ch <= 0x1ea) { return 1; }
    else if (ch >= 0x1ec && ch <= 0x1ec) { return 1; }
    else if (ch >= 0x1ee && ch <= 0x1ee) { return 1; }
    else if (ch >= 0x1f1 && ch <= 0x1f1) { return 1; }
    else if (ch >= 0x1f4 && ch <= 0x1f4) { return 1; }
    else if (ch >= 0x1f6 && ch <= 0x1f8) { return 1; }
    else if (ch >= 0x1fa && ch <= 0x1fa) { return 1; }
    else if (ch >= 0x1fc && ch <= 0x1fc) { return 1; }
    else if (ch >= 0x1fe && ch <= 0x1fe) { return 1; }
    else if (ch >= 0x200 && ch <= 0x200) { return 1; }
    else if (ch >= 0x202 && ch <= 0x202) { return 1; }
    else if (ch >= 0x204 && ch <= 0x204) { return 1; }
    else if (ch >= 0x206 && ch <= 0x206) { return 1; }
    else if (ch >= 0x208 && ch <= 0x208) { return 1; }
    else if (ch >= 0x20a && ch <= 0x20a) { return 1; }
    else if (ch >= 0x20c && ch <= 0x20c) { return 1; }
    else if (ch >= 0x20e && ch <= 0x20e) { return 1; }
    else if (ch >= 0x210 && ch <= 0x210) { return 1; }
    else if (ch >= 0x212 && ch <= 0x212) { return 1; }
    else if (ch >= 0x214 && ch <= 0x214) { return 1; }
    else if (ch >= 0x216 && ch <= 0x216) { return 1; }
    else if (ch >= 0x218 && ch <= 0x218) { return 1; }
    else if (ch >= 0x21a && ch <= 0x21a) { return 1; }
    else if (ch >= 0x21c && ch <= 0x21c) { return 1; }
    else if (ch >= 0x21e && ch <= 0x21e) { return 1; }
    else if (ch >= 0x220 && ch <= 0x220) { return 1; }
    else if (ch >= 0x222 && ch <= 0x222) { return 1; }
    else if (ch >= 0x224 && ch <= 0x224) { return 1; }
    else if (ch >= 0x226 && ch <= 0x226) { return 1; }
    else if (ch >= 0x228 && ch <= 0x228) { return 1; }
    else if (ch >= 0x22a && ch <= 0x22a) { return 1; }
    else if (ch >= 0x22c && ch <= 0x22c) { return 1; }
    else if (ch >= 0x22e && ch <= 0x22e) { return 1; }
    else if (ch >= 0x230 && ch <= 0x230) { return 1; }
    else if (ch >= 0x232 && ch <= 0x232) { return 1; }
    else if (ch >= 0x23a && ch <= 0x23b) { return 1; }
    else if (ch >= 0x23d && ch <= 0x23e) { return 1; }
    else if (ch >= 0x241 && ch <= 0x241) { return 1; }
    else if (ch >= 0x243 && ch <= 0x246) { return 1; }
    else if (ch >= 0x248 && ch <= 0x248) { return 1; }
    else if (ch >= 0x24a && ch <= 0x24a) { return 1; }
    else if (ch >= 0x24c && ch <= 0x24c) { return 1; }
    else if (ch >= 0x24e && ch <= 0x24e) { return 1; }
    else if (ch >= 0x370 && ch <= 0x370) { return 1; }
    else if (ch >= 0x372 && ch <= 0x372) { return 1; }
    else if (ch >= 0x376 && ch <= 0x376) { return 1; }
    else if (ch >= 0x386 && ch <= 0x386) { return 1; }
    else if (ch >= 0x388 && ch <= 0x38a) { return 1; }
    else if (ch >= 0x38c && ch <= 0x38c) { return 1; }
    else if (ch >= 0x38e && ch <= 0x38f) { return 1; }
    else if (ch >= 0x391 && ch <= 0x3a1) { return 1; }
    else if (ch >= 0x3a3 && ch <= 0x3ab) { return 1; }
    else if (ch >= 0x3cf && ch <= 0x3cf) { return 1; }
    else if (ch >= 0x3d2 && ch <= 0x3d4) { return 1; }
    else if (ch >= 0x3d8 && ch <= 0x3d8) { return 1; }
    else if (ch >= 0x3da && ch <= 0x3da) { return 1; }
    else if (ch >= 0x3dc && ch <= 0x3dc) { return 1; }
    else if (ch >= 0x3de && ch <= 0x3de) { return 1; }
    else if (ch >= 0x3e0 && ch <= 0x3e0) { return 1; }
    else if (ch >= 0x3e2 && ch <= 0x3e2) { return 1; }
    else if (ch >= 0x3e4 && ch <= 0x3e4) { return 1; }
    else if (ch >= 0x3e6 && ch <= 0x3e6) { return 1; }
    else if (ch >= 0x3e8 && ch <= 0x3e8) { return 1; }
    else if (ch >= 0x3ea && ch <= 0x3ea) { return 1; }
    else if (ch >= 0x3ec && ch <= 0x3ec) { return 1; }
    else if (ch >= 0x3ee && ch <= 0x3ee) { return 1; }
    else if (ch >= 0x3f4 && ch <= 0x3f4) { return 1; }
    else if (ch >= 0x3f7 && ch <= 0x3f7) { return 1; }
    else if (ch >= 0x3f9 && ch <= 0x3fa) { return 1; }
    else if (ch >= 0x3fd && ch <= 0x42f) { return 1; }
    else if (ch >= 0x460 && ch <= 0x460) { return 1; }
    else if (ch >= 0x462 && ch <= 0x462) { return 1; }
    else if (ch >= 0x464 && ch <= 0x464) { return 1; }
    else if (ch >= 0x466 && ch <= 0x466) { return 1; }
    else if (ch >= 0x468 && ch <= 0x468) { return 1; }
    else if (ch >= 0x46a && ch <= 0x46a) { return 1; }
    else if (ch >= 0x46c && ch <= 0x46c) { return 1; }
    else if (ch >= 0x46e && ch <= 0x46e) { return 1; }
    else if (ch >= 0x470 && ch <= 0x470) { return 1; }
    else if (ch >= 0x472 && ch <= 0x472) { return 1; }
    else if (ch >= 0x474 && ch <= 0x474) { return 1; }
    else if (ch >= 0x476 && ch <= 0x476) { return 1; }
    else if (ch >= 0x478 && ch <= 0x478) { return 1; }
    else if (ch >= 0x47a && ch <= 0x47a) { return 1; }
    else if (ch >= 0x47c && ch <= 0x47c) { return 1; }
    else if (ch >= 0x47e && ch <= 0x47e) { return 1; }
    else if (ch >= 0x480 && ch <= 0x480) { return 1; }
    else if (ch >= 0x48a && ch <= 0x48a) { return 1; }
    else if (ch >= 0x48c && ch <= 0x48c) { return 1; }
    else if (ch >= 0x48e && ch <= 0x48e) { return 1; }
    else if (ch >= 0x490 && ch <= 0x490) { return 1; }
    else if (ch >= 0x492 && ch <= 0x492) { return 1; }
    else if (ch >= 0x494 && ch <= 0x494) { return 1; }
    else if (ch >= 0x496 && ch <= 0x496) { return 1; }
    else if (ch >= 0x498 && ch <= 0x498) { return 1; }
    else if (ch >= 0x49a && ch <= 0x49a) { return 1; }
    else if (ch >= 0x49c && ch <= 0x49c) { return 1; }
    else if (ch >= 0x49e && ch <= 0x49e) { return 1; }
    else if (ch >= 0x4a0 && ch <= 0x4a0) { return 1; }
    else if (ch >= 0x4a2 && ch <= 0x4a2) { return 1; }
    else if (ch >= 0x4a4 && ch <= 0x4a4) { return 1; }
    else if (ch >= 0x4a6 && ch <= 0x4a6) { return 1; }
    else if (ch >= 0x4a8 && ch <= 0x4a8) { return 1; }
    else if (ch >= 0x4aa && ch <= 0x4aa) { return 1; }
    else if (ch >= 0x4ac && ch <= 0x4ac) { return 1; }
    else if (ch >= 0x4ae && ch <= 0x4ae) { return 1; }
    else if (ch >= 0x4b0 && ch <= 0x4b0) { return 1; }
    else if (ch >= 0x4b2 && ch <= 0x4b2) { return 1; }
    else if (ch >= 0x4b4 && ch <= 0x4b4) { return 1; }
    else if (ch >= 0x4b6 && ch <= 0x4b6) { return 1; }
    else if (ch >= 0x4b8 && ch <= 0x4b8) { return 1; }
    else if (ch >= 0x4ba && ch <= 0x4ba) { return 1; }
    else if (ch >= 0x4bc && ch <= 0x4bc) { return 1; }
    else if (ch >= 0x4be && ch <= 0x4be) { return 1; }
    else if (ch >= 0x4c0 && ch <= 0x4c1) { return 1; }
    else if (ch >= 0x4c3 && ch <= 0x4c3) { return 1; }
    else if (ch >= 0x4c5 && ch <= 0x4c5) { return 1; }
    else if (ch >= 0x4c7 && ch <= 0x4c7) { return 1; }
    else if (ch >= 0x4c9 && ch <= 0x4c9) { return 1; }
    else if (ch >= 0x4cb && ch <= 0x4cb) { return 1; }
    else if (ch >= 0x4cd && ch <= 0x4cd) { return 1; }
    else if (ch >= 0x4d0 && ch <= 0x4d0) { return 1; }
    else if (ch >= 0x4d2 && ch <= 0x4d2) { return 1; }
    else if (ch >= 0x4d4 && ch <= 0x4d4) { return 1; }
    else if (ch >= 0x4d6 && ch <= 0x4d6) { return 1; }
    else if (ch >= 0x4d8 && ch <= 0x4d8) { return 1; }
    else if (ch >= 0x4da && ch <= 0x4da) { return 1; }
    else if (ch >= 0x4dc && ch <= 0x4dc) { return 1; }
    else if (ch >= 0x4de && ch <= 0x4de) { return 1; }
    else if (ch >= 0x4e0 && ch <= 0x4e0) { return 1; }
    else if (ch >= 0x4e2 && ch <= 0x4e2) { return 1; }
    else if (ch >= 0x4e4 && ch <= 0x4e4) { return 1; }
    else if (ch >= 0x4e6 && ch <= 0x4e6) { return 1; }
    else if (ch >= 0x4e8 && ch <= 0x4e8) { return 1; }
    else if (ch >= 0x4ea && ch <= 0x4ea) { return 1; }
    else if (ch >= 0x4ec && ch <= 0x4ec) { return 1; }
    else if (ch >= 0x4ee && ch <= 0x4ee) { return 1; }
    else if (ch >= 0x4f0 && ch <= 0x4f0) { return 1; }
    else if (ch >= 0x4f2 && ch <= 0x4f2) { return 1; }
    else if (ch >= 0x4f4 && ch <= 0x4f4) { return 1; }
    else if (ch >= 0x4f6 && ch <= 0x4f6) { return 1; }
    else if (ch >= 0x4f8 && ch <= 0x4f8) { return 1; }
    else if (ch >= 0x4fa && ch <= 0x4fa) { return 1; }
    else if (ch >= 0x4fc && ch <= 0x4fc) { return 1; }
    else if (ch >= 0x4fe && ch <= 0x4fe) { return 1; }
    else if (ch >= 0x500 && ch <= 0x500) { return 1; }
    else if (ch >= 0x502 && ch <= 0x502) { return 1; }
    else if (ch >= 0x504 && ch <= 0x504) { return 1; }
    else if (ch >= 0x506 && ch <= 0x506) { return 1; }
    else if (ch >= 0x508 && ch <= 0x508) { return 1; }
    else if (ch >= 0x50a && ch <= 0x50a) { return 1; }
    else if (ch >= 0x50c && ch <= 0x50c) { return 1; }
    else if (ch >= 0x50e && ch <= 0x50e) { return 1; }
    else if (ch >= 0x510 && ch <= 0x510) { return 1; }
    else if (ch >= 0x512 && ch <= 0x512) { return 1; }
    else if (ch >= 0x514 && ch <= 0x514) { return 1; }
    else if (ch >= 0x516 && ch <= 0x516) { return 1; }
    else if (ch >= 0x518 && ch <= 0x518) { return 1; }
    else if (ch >= 0x51a && ch <= 0x51a) { return 1; }
    else if (ch >= 0x51c && ch <= 0x51c) { return 1; }
    else if (ch >= 0x51e && ch <= 0x51e) { return 1; }
    else if (ch >= 0x520 && ch <= 0x520) { return 1; }
    else if (ch >= 0x522 && ch <= 0x522) { return 1; }
    else if (ch >= 0x524 && ch <= 0x524) { return 1; }
    else if (ch >= 0x526 && ch <= 0x526) { return 1; }
    else if (ch >= 0x531 && ch <= 0x556) { return 1; }
    else if (ch >= 0x10a0 && ch <= 0x10c5) { return 1; }
    else if (ch >= 0x10c7 && ch <= 0x10c7) { return 1; }
    else if (ch >= 0x10cd && ch <= 0x10cd) { return 1; }
    else if (ch >= 0x1e00 && ch <= 0x1e00) { return 1; }
    else if (ch >= 0x1e02 && ch <= 0x1e02) { return 1; }
    else if (ch >= 0x1e04 && ch <= 0x1e04) { return 1; }
    else if (ch >= 0x1e06 && ch <= 0x1e06) { return 1; }
    else if (ch >= 0x1e08 && ch <= 0x1e08) { return 1; }
    else if (ch >= 0x1e0a && ch <= 0x1e0a) { return 1; }
    else if (ch >= 0x1e0c && ch <= 0x1e0c) { return 1; }
    else if (ch >= 0x1e0e && ch <= 0x1e0e) { return 1; }
    else if (ch >= 0x1e10 && ch <= 0x1e10) { return 1; }
    else if (ch >= 0x1e12 && ch <= 0x1e12) { return 1; }
    else if (ch >= 0x1e14 && ch <= 0x1e14) { return 1; }
    else if (ch >= 0x1e16 && ch <= 0x1e16) { return 1; }
    else if (ch >= 0x1e18 && ch <= 0x1e18) { return 1; }
    else if (ch >= 0x1e1a && ch <= 0x1e1a) { return 1; }
    else if (ch >= 0x1e1c && ch <= 0x1e1c) { return 1; }
    else if (ch >= 0x1e1e && ch <= 0x1e1e) { return 1; }
    else if (ch >= 0x1e20 && ch <= 0x1e20) { return 1; }
    else if (ch >= 0x1e22 && ch <= 0x1e22) { return 1; }
    else if (ch >= 0x1e24 && ch <= 0x1e24) { return 1; }
    else if (ch >= 0x1e26 && ch <= 0x1e26) { return 1; }
    else if (ch >= 0x1e28 && ch <= 0x1e28) { return 1; }
    else if (ch >= 0x1e2a && ch <= 0x1e2a) { return 1; }
    else if (ch >= 0x1e2c && ch <= 0x1e2c) { return 1; }
    else if (ch >= 0x1e2e && ch <= 0x1e2e) { return 1; }
    else if (ch >= 0x1e30 && ch <= 0x1e30) { return 1; }
    else if (ch >= 0x1e32 && ch <= 0x1e32) { return 1; }
    else if (ch >= 0x1e34 && ch <= 0x1e34) { return 1; }
    else if (ch >= 0x1e36 && ch <= 0x1e36) { return 1; }
    else if (ch >= 0x1e38 && ch <= 0x1e38) { return 1; }
    else if (ch >= 0x1e3a && ch <= 0x1e3a) { return 1; }
    else if (ch >= 0x1e3c && ch <= 0x1e3c) { return 1; }
    else if (ch >= 0x1e3e && ch <= 0x1e3e) { return 1; }
    else if (ch >= 0x1e40 && ch <= 0x1e40) { return 1; }
    else if (ch >= 0x1e42 && ch <= 0x1e42) { return 1; }
    else if (ch >= 0x1e44 && ch <= 0x1e44) { return 1; }
    else if (ch >= 0x1e46 && ch <= 0x1e46) { return 1; }
    else if (ch >= 0x1e48 && ch <= 0x1e48) { return 1; }
    else if (ch >= 0x1e4a && ch <= 0x1e4a) { return 1; }
    else if (ch >= 0x1e4c && ch <= 0x1e4c) { return 1; }
    else if (ch >= 0x1e4e && ch <= 0x1e4e) { return 1; }
    else if (ch >= 0x1e50 && ch <= 0x1e50) { return 1; }
    else if (ch >= 0x1e52 && ch <= 0x1e52) { return 1; }
    else if (ch >= 0x1e54 && ch <= 0x1e54) { return 1; }
    else if (ch >= 0x1e56 && ch <= 0x1e56) { return 1; }
    else if (ch >= 0x1e58 && ch <= 0x1e58) { return 1; }
    else if (ch >= 0x1e5a && ch <= 0x1e5a) { return 1; }
    else if (ch >= 0x1e5c && ch <= 0x1e5c) { return 1; }
    else if (ch >= 0x1e5e && ch <= 0x1e5e) { return 1; }
    else if (ch >= 0x1e60 && ch <= 0x1e60) { return 1; }
    else if (ch >= 0x1e62 && ch <= 0x1e62) { return 1; }
    else if (ch >= 0x1e64 && ch <= 0x1e64) { return 1; }
    else if (ch >= 0x1e66 && ch <= 0x1e66) { return 1; }
    else if (ch >= 0x1e68 && ch <= 0x1e68) { return 1; }
    else if (ch >= 0x1e6a && ch <= 0x1e6a) { return 1; }
    else if (ch >= 0x1e6c && ch <= 0x1e6c) { return 1; }
    else if (ch >= 0x1e6e && ch <= 0x1e6e) { return 1; }
    else if (ch >= 0x1e70 && ch <= 0x1e70) { return 1; }
    else if (ch >= 0x1e72 && ch <= 0x1e72) { return 1; }
    else if (ch >= 0x1e74 && ch <= 0x1e74) { return 1; }
    else if (ch >= 0x1e76 && ch <= 0x1e76) { return 1; }
    else if (ch >= 0x1e78 && ch <= 0x1e78) { return 1; }
    else if (ch >= 0x1e7a && ch <= 0x1e7a) { return 1; }
    else if (ch >= 0x1e7c && ch <= 0x1e7c) { return 1; }
    else if (ch >= 0x1e7e && ch <= 0x1e7e) { return 1; }
    else if (ch >= 0x1e80 && ch <= 0x1e80) { return 1; }
    else if (ch >= 0x1e82 && ch <= 0x1e82) { return 1; }
    else if (ch >= 0x1e84 && ch <= 0x1e84) { return 1; }
    else if (ch >= 0x1e86 && ch <= 0x1e86) { return 1; }
    else if (ch >= 0x1e88 && ch <= 0x1e88) { return 1; }
    else if (ch >= 0x1e8a && ch <= 0x1e8a) { return 1; }
    else if (ch >= 0x1e8c && ch <= 0x1e8c) { return 1; }
    else if (ch >= 0x1e8e && ch <= 0x1e8e) { return 1; }
    else if (ch >= 0x1e90 && ch <= 0x1e90) { return 1; }
    else if (ch >= 0x1e92 && ch <= 0x1e92) { return 1; }
    else if (ch >= 0x1e94 && ch <= 0x1e94) { return 1; }
    else if (ch >= 0x1e9e && ch <= 0x1e9e) { return 1; }
    else if (ch >= 0x1ea0 && ch <= 0x1ea0) { return 1; }
    else if (ch >= 0x1ea2 && ch <= 0x1ea2) { return 1; }
    else if (ch >= 0x1ea4 && ch <= 0x1ea4) { return 1; }
    else if (ch >= 0x1ea6 && ch <= 0x1ea6) { return 1; }
    else if (ch >= 0x1ea8 && ch <= 0x1ea8) { return 1; }
    else if (ch >= 0x1eaa && ch <= 0x1eaa) { return 1; }
    else if (ch >= 0x1eac && ch <= 0x1eac) { return 1; }
    else if (ch >= 0x1eae && ch <= 0x1eae) { return 1; }
    else if (ch >= 0x1eb0 && ch <= 0x1eb0) { return 1; }
    else if (ch >= 0x1eb2 && ch <= 0x1eb2) { return 1; }
    else if (ch >= 0x1eb4 && ch <= 0x1eb4) { return 1; }
    else if (ch >= 0x1eb6 && ch <= 0x1eb6) { return 1; }
    else if (ch >= 0x1eb8 && ch <= 0x1eb8) { return 1; }
    else if (ch >= 0x1eba && ch <= 0x1eba) { return 1; }
    else if (ch >= 0x1ebc && ch <= 0x1ebc) { return 1; }
    else if (ch >= 0x1ebe && ch <= 0x1ebe) { return 1; }
    else if (ch >= 0x1ec0 && ch <= 0x1ec0) { return 1; }
    else if (ch >= 0x1ec2 && ch <= 0x1ec2) { return 1; }
    else if (ch >= 0x1ec4 && ch <= 0x1ec4) { return 1; }
    else if (ch >= 0x1ec6 && ch <= 0x1ec6) { return 1; }
    else if (ch >= 0x1ec8 && ch <= 0x1ec8) { return 1; }
    else if (ch >= 0x1eca && ch <= 0x1eca) { return 1; }
    else if (ch >= 0x1ecc && ch <= 0x1ecc) { return 1; }
    else if (ch >= 0x1ece && ch <= 0x1ece) { return 1; }
    else if (ch >= 0x1ed0 && ch <= 0x1ed0) { return 1; }
    else if (ch >= 0x1ed2 && ch <= 0x1ed2) { return 1; }
    else if (ch >= 0x1ed4 && ch <= 0x1ed4) { return 1; }
    else if (ch >= 0x1ed6 && ch <= 0x1ed6) { return 1; }
    else if (ch >= 0x1ed8 && ch <= 0x1ed8) { return 1; }
    else if (ch >= 0x1eda && ch <= 0x1eda) { return 1; }
    else if (ch >= 0x1edc && ch <= 0x1edc) { return 1; }
    else if (ch >= 0x1ede && ch <= 0x1ede) { return 1; }
    else if (ch >= 0x1ee0 && ch <= 0x1ee0) { return 1; }
    else if (ch >= 0x1ee2 && ch <= 0x1ee2) { return 1; }
    else if (ch >= 0x1ee4 && ch <= 0x1ee4) { return 1; }
    else if (ch >= 0x1ee6 && ch <= 0x1ee6) { return 1; }
    else if (ch >= 0x1ee8 && ch <= 0x1ee8) { return 1; }
    else if (ch >= 0x1eea && ch <= 0x1eea) { return 1; }
    else if (ch >= 0x1eec && ch <= 0x1eec) { return 1; }
    else if (ch >= 0x1eee && ch <= 0x1eee) { return 1; }
    else if (ch >= 0x1ef0 && ch <= 0x1ef0) { return 1; }
    else if (ch >= 0x1ef2 && ch <= 0x1ef2) { return 1; }
    else if (ch >= 0x1ef4 && ch <= 0x1ef4) { return 1; }
    else if (ch >= 0x1ef6 && ch <= 0x1ef6) { return 1; }
    else if (ch >= 0x1ef8 && ch <= 0x1ef8) { return 1; }
    else if (ch >= 0x1efa && ch <= 0x1efa) { return 1; }
    else if (ch >= 0x1efc && ch <= 0x1efc) { return 1; }
    else if (ch >= 0x1efe && ch <= 0x1efe) { return 1; }
    else if (ch >= 0x1f08 && ch <= 0x1f0f) { return 1; }
    else if (ch >= 0x1f18 && ch <= 0x1f1d) { return 1; }
    else if (ch >= 0x1f28 && ch <= 0x1f2f) { return 1; }
    else if (ch >= 0x1f38 && ch <= 0x1f3f) { return 1; }
    else if (ch >= 0x1f48 && ch <= 0x1f4d) { return 1; }
    else if (ch >= 0x1f59 && ch <= 0x1f59) { return 1; }
    else if (ch >= 0x1f5b && ch <= 0x1f5b) { return 1; }
    else if (ch >= 0x1f5d && ch <= 0x1f5d) { return 1; }
    else if (ch >= 0x1f5f && ch <= 0x1f5f) { return 1; }
    else if (ch >= 0x1f68 && ch <= 0x1f6f) { return 1; }
    else if (ch >= 0x1fb8 && ch <= 0x1fbb) { return 1; }
    else if (ch >= 0x1fc8 && ch <= 0x1fcb) { return 1; }
    else if (ch >= 0x1fd8 && ch <= 0x1fdb) { return 1; }
    else if (ch >= 0x1fe8 && ch <= 0x1fec) { return 1; }
    else if (ch >= 0x1ff8 && ch <= 0x1ffb) { return 1; }
    else if (ch >= 0x2102 && ch <= 0x2102) { return 1; }
    else if (ch >= 0x2107 && ch <= 0x2107) { return 1; }
    else if (ch >= 0x210b && ch <= 0x210d) { return 1; }
    else if (ch >= 0x2110 && ch <= 0x2112) { return 1; }
    else if (ch >= 0x2115 && ch <= 0x2115) { return 1; }
    else if (ch >= 0x2119 && ch <= 0x211d) { return 1; }
    else if (ch >= 0x2124 && ch <= 0x2124) { return 1; }
    else if (ch >= 0x2126 && ch <= 0x2126) { return 1; }
    else if (ch >= 0x2128 && ch <= 0x2128) { return 1; }
    else if (ch >= 0x212a && ch <= 0x212d) { return 1; }
    else if (ch >= 0x2130 && ch <= 0x2133) { return 1; }
    else if (ch >= 0x213e && ch <= 0x213f) { return 1; }
    else if (ch >= 0x2145 && ch <= 0x2145) { return 1; }
    else if (ch >= 0x2183 && ch <= 0x2183) { return 1; }
    else if (ch >= 0x2c00 && ch <= 0x2c2e) { return 1; }
    else if (ch >= 0x2c60 && ch <= 0x2c60) { return 1; }
    else if (ch >= 0x2c62 && ch <= 0x2c64) { return 1; }
    else if (ch >= 0x2c67 && ch <= 0x2c67) { return 1; }
    else if (ch >= 0x2c69 && ch <= 0x2c69) { return 1; }
    else if (ch >= 0x2c6b && ch <= 0x2c6b) { return 1; }
    else if (ch >= 0x2c6d && ch <= 0x2c70) { return 1; }
    else if (ch >= 0x2c72 && ch <= 0x2c72) { return 1; }
    else if (ch >= 0x2c75 && ch <= 0x2c75) { return 1; }
    else if (ch >= 0x2c7e && ch <= 0x2c80) { return 1; }
    else if (ch >= 0x2c82 && ch <= 0x2c82) { return 1; }
    else if (ch >= 0x2c84 && ch <= 0x2c84) { return 1; }
    else if (ch >= 0x2c86 && ch <= 0x2c86) { return 1; }
    else if (ch >= 0x2c88 && ch <= 0x2c88) { return 1; }
    else if (ch >= 0x2c8a && ch <= 0x2c8a) { return 1; }
    else if (ch >= 0x2c8c && ch <= 0x2c8c) { return 1; }
    else if (ch >= 0x2c8e && ch <= 0x2c8e) { return 1; }
    else if (ch >= 0x2c90 && ch <= 0x2c90) { return 1; }
    else if (ch >= 0x2c92 && ch <= 0x2c92) { return 1; }
    else if (ch >= 0x2c94 && ch <= 0x2c94) { return 1; }
    else if (ch >= 0x2c96 && ch <= 0x2c96) { return 1; }
    else if (ch >= 0x2c98 && ch <= 0x2c98) { return 1; }
    else if (ch >= 0x2c9a && ch <= 0x2c9a) { return 1; }
    else if (ch >= 0x2c9c && ch <= 0x2c9c) { return 1; }
    else if (ch >= 0x2c9e && ch <= 0x2c9e) { return 1; }
    else if (ch >= 0x2ca0 && ch <= 0x2ca0) { return 1; }
    else if (ch >= 0x2ca2 && ch <= 0x2ca2) { return 1; }
    else if (ch >= 0x2ca4 && ch <= 0x2ca4) { return 1; }
    else if (ch >= 0x2ca6 && ch <= 0x2ca6) { return 1; }
    else if (ch >= 0x2ca8 && ch <= 0x2ca8) { return 1; }
    else if (ch >= 0x2caa && ch <= 0x2caa) { return 1; }
    else if (ch >= 0x2cac && ch <= 0x2cac) { return 1; }
    else if (ch >= 0x2cae && ch <= 0x2cae) { return 1; }
    else if (ch >= 0x2cb0 && ch <= 0x2cb0) { return 1; }
    else if (ch >= 0x2cb2 && ch <= 0x2cb2) { return 1; }
    else if (ch >= 0x2cb4 && ch <= 0x2cb4) { return 1; }
    else if (ch >= 0x2cb6 && ch <= 0x2cb6) { return 1; }
    else if (ch >= 0x2cb8 && ch <= 0x2cb8) { return 1; }
    else if (ch >= 0x2cba && ch <= 0x2cba) { return 1; }
    else if (ch >= 0x2cbc && ch <= 0x2cbc) { return 1; }
    else if (ch >= 0x2cbe && ch <= 0x2cbe) { return 1; }
    else if (ch >= 0x2cc0 && ch <= 0x2cc0) { return 1; }
    else if (ch >= 0x2cc2 && ch <= 0x2cc2) { return 1; }
    else if (ch >= 0x2cc4 && ch <= 0x2cc4) { return 1; }
    else if (ch >= 0x2cc6 && ch <= 0x2cc6) { return 1; }
    else if (ch >= 0x2cc8 && ch <= 0x2cc8) { return 1; }
    else if (ch >= 0x2cca && ch <= 0x2cca) { return 1; }
    else if (ch >= 0x2ccc && ch <= 0x2ccc) { return 1; }
    else if (ch >= 0x2cce && ch <= 0x2cce) { return 1; }
    else if (ch >= 0x2cd0 && ch <= 0x2cd0) { return 1; }
    else if (ch >= 0x2cd2 && ch <= 0x2cd2) { return 1; }
    else if (ch >= 0x2cd4 && ch <= 0x2cd4) { return 1; }
    else if (ch >= 0x2cd6 && ch <= 0x2cd6) { return 1; }
    else if (ch >= 0x2cd8 && ch <= 0x2cd8) { return 1; }
    else if (ch >= 0x2cda && ch <= 0x2cda) { return 1; }
    else if (ch >= 0x2cdc && ch <= 0x2cdc) { return 1; }
    else if (ch >= 0x2cde && ch <= 0x2cde) { return 1; }
    else if (ch >= 0x2ce0 && ch <= 0x2ce0) { return 1; }
    else if (ch >= 0x2ce2 && ch <= 0x2ce2) { return 1; }
    else if (ch >= 0x2ceb && ch <= 0x2ceb) { return 1; }
    else if (ch >= 0x2ced && ch <= 0x2ced) { return 1; }
    else if (ch >= 0x2cf2 && ch <= 0x2cf2) { return 1; }
    else if (ch >= 0xa640 && ch <= 0xa640) { return 1; }
    else if (ch >= 0xa642 && ch <= 0xa642) { return 1; }
    else if (ch >= 0xa644 && ch <= 0xa644) { return 1; }
    else if (ch >= 0xa646 && ch <= 0xa646) { return 1; }
    else if (ch >= 0xa648 && ch <= 0xa648) { return 1; }
    else if (ch >= 0xa64a && ch <= 0xa64a) { return 1; }
    else if (ch >= 0xa64c && ch <= 0xa64c) { return 1; }
    else if (ch >= 0xa64e && ch <= 0xa64e) { return 1; }
    else if (ch >= 0xa650 && ch <= 0xa650) { return 1; }
    else if (ch >= 0xa652 && ch <= 0xa652) { return 1; }
    else if (ch >= 0xa654 && ch <= 0xa654) { return 1; }
    else if (ch >= 0xa656 && ch <= 0xa656) { return 1; }
    else if (ch >= 0xa658 && ch <= 0xa658) { return 1; }
    else if (ch >= 0xa65a && ch <= 0xa65a) { return 1; }
    else if (ch >= 0xa65c && ch <= 0xa65c) { return 1; }
    else if (ch >= 0xa65e && ch <= 0xa65e) { return 1; }
    else if (ch >= 0xa660 && ch <= 0xa660) { return 1; }
    else if (ch >= 0xa662 && ch <= 0xa662) { return 1; }
    else if (ch >= 0xa664 && ch <= 0xa664) { return 1; }
    else if (ch >= 0xa666 && ch <= 0xa666) { return 1; }
    else if (ch >= 0xa668 && ch <= 0xa668) { return 1; }
    else if (ch >= 0xa66a && ch <= 0xa66a) { return 1; }
    else if (ch >= 0xa66c && ch <= 0xa66c) { return 1; }
    else if (ch >= 0xa680 && ch <= 0xa680) { return 1; }
    else if (ch >= 0xa682 && ch <= 0xa682) { return 1; }
    else if (ch >= 0xa684 && ch <= 0xa684) { return 1; }
    else if (ch >= 0xa686 && ch <= 0xa686) { return 1; }
    else if (ch >= 0xa688 && ch <= 0xa688) { return 1; }
    else if (ch >= 0xa68a && ch <= 0xa68a) { return 1; }
    else if (ch >= 0xa68c && ch <= 0xa68c) { return 1; }
    else if (ch >= 0xa68e && ch <= 0xa68e) { return 1; }
    else if (ch >= 0xa690 && ch <= 0xa690) { return 1; }
    else if (ch >= 0xa692 && ch <= 0xa692) { return 1; }
    else if (ch >= 0xa694 && ch <= 0xa694) { return 1; }
    else if (ch >= 0xa696 && ch <= 0xa696) { return 1; }
    else if (ch >= 0xa722 && ch <= 0xa722) { return 1; }
    else if (ch >= 0xa724 && ch <= 0xa724) { return 1; }
    else if (ch >= 0xa726 && ch <= 0xa726) { return 1; }
    else if (ch >= 0xa728 && ch <= 0xa728) { return 1; }
    else if (ch >= 0xa72a && ch <= 0xa72a) { return 1; }
    else if (ch >= 0xa72c && ch <= 0xa72c) { return 1; }
    else if (ch >= 0xa72e && ch <= 0xa72e) { return 1; }
    else if (ch >= 0xa732 && ch <= 0xa732) { return 1; }
    else if (ch >= 0xa734 && ch <= 0xa734) { return 1; }
    else if (ch >= 0xa736 && ch <= 0xa736) { return 1; }
    else if (ch >= 0xa738 && ch <= 0xa738) { return 1; }
    else if (ch >= 0xa73a && ch <= 0xa73a) { return 1; }
    else if (ch >= 0xa73c && ch <= 0xa73c) { return 1; }
    else if (ch >= 0xa73e && ch <= 0xa73e) { return 1; }
    else if (ch >= 0xa740 && ch <= 0xa740) { return 1; }
    else if (ch >= 0xa742 && ch <= 0xa742) { return 1; }
    else if (ch >= 0xa744 && ch <= 0xa744) { return 1; }
    else if (ch >= 0xa746 && ch <= 0xa746) { return 1; }
    else if (ch >= 0xa748 && ch <= 0xa748) { return 1; }
    else if (ch >= 0xa74a && ch <= 0xa74a) { return 1; }
    else if (ch >= 0xa74c && ch <= 0xa74c) { return 1; }
    else if (ch >= 0xa74e && ch <= 0xa74e) { return 1; }
    else if (ch >= 0xa750 && ch <= 0xa750) { return 1; }
    else if (ch >= 0xa752 && ch <= 0xa752) { return 1; }
    else if (ch >= 0xa754 && ch <= 0xa754) { return 1; }
    else if (ch >= 0xa756 && ch <= 0xa756) { return 1; }
    else if (ch >= 0xa758 && ch <= 0xa758) { return 1; }
    else if (ch >= 0xa75a && ch <= 0xa75a) { return 1; }
    else if (ch >= 0xa75c && ch <= 0xa75c) { return 1; }
    else if (ch >= 0xa75e && ch <= 0xa75e) { return 1; }
    else if (ch >= 0xa760 && ch <= 0xa760) { return 1; }
    else if (ch >= 0xa762 && ch <= 0xa762) { return 1; }
    else if (ch >= 0xa764 && ch <= 0xa764) { return 1; }
    else if (ch >= 0xa766 && ch <= 0xa766) { return 1; }
    else if (ch >= 0xa768 && ch <= 0xa768) { return 1; }
    else if (ch >= 0xa76a && ch <= 0xa76a) { return 1; }
    else if (ch >= 0xa76c && ch <= 0xa76c) { return 1; }
    else if (ch >= 0xa76e && ch <= 0xa76e) { return 1; }
    else if (ch >= 0xa779 && ch <= 0xa779) { return 1; }
    else if (ch >= 0xa77b && ch <= 0xa77b) { return 1; }
    else if (ch >= 0xa77d && ch <= 0xa77e) { return 1; }
    else if (ch >= 0xa780 && ch <= 0xa780) { return 1; }
    else if (ch >= 0xa782 && ch <= 0xa782) { return 1; }
    else if (ch >= 0xa784 && ch <= 0xa784) { return 1; }
    else if (ch >= 0xa786 && ch <= 0xa786) { return 1; }
    else if (ch >= 0xa78b && ch <= 0xa78b) { return 1; }
    else if (ch >= 0xa78d && ch <= 0xa78d) { return 1; }
    else if (ch >= 0xa790 && ch <= 0xa790) { return 1; }
    else if (ch >= 0xa792 && ch <= 0xa792) { return 1; }
    else if (ch >= 0xa7a0 && ch <= 0xa7a0) { return 1; }
    else if (ch >= 0xa7a2 && ch <= 0xa7a2) { return 1; }
    else if (ch >= 0xa7a4 && ch <= 0xa7a4) { return 1; }
    else if (ch >= 0xa7a6 && ch <= 0xa7a6) { return 1; }
    else if (ch >= 0xa7a8 && ch <= 0xa7a8) { return 1; }
    else if (ch >= 0xa7aa && ch <= 0xa7aa) { return 1; }
    else if (ch >= 0xff21 && ch <= 0xff3a) { return 1; }
    else if (ch >= 0x10400 && ch <= 0x10427) { return 1; }
    else if (ch >= 0x1d400 && ch <= 0x1d419) { return 1; }
    else if (ch >= 0x1d434 && ch <= 0x1d44d) { return 1; }
    else if (ch >= 0x1d468 && ch <= 0x1d481) { return 1; }
    else if (ch >= 0x1d49c && ch <= 0x1d49c) { return 1; }
    else if (ch >= 0x1d49e && ch <= 0x1d49f) { return 1; }
    else if (ch >= 0x1d4a2 && ch <= 0x1d4a2) { return 1; }
    else if (ch >= 0x1d4a5 && ch <= 0x1d4a6) { return 1; }
    else if (ch >= 0x1d4a9 && ch <= 0x1d4ac) { return 1; }
    else if (ch >= 0x1d4ae && ch <= 0x1d4b5) { return 1; }
    else if (ch >= 0x1d4d0 && ch <= 0x1d4e9) { return 1; }
    else if (ch >= 0x1d504 && ch <= 0x1d505) { return 1; }
    else if (ch >= 0x1d507 && ch <= 0x1d50a) { return 1; }
    else if (ch >= 0x1d50d && ch <= 0x1d514) { return 1; }
    else if (ch >= 0x1d516 && ch <= 0x1d51c) { return 1; }
    else if (ch >= 0x1d538 && ch <= 0x1d539) { return 1; }
    else if (ch >= 0x1d53b && ch <= 0x1d53e) { return 1; }
    else if (ch >= 0x1d540 && ch <= 0x1d544) { return 1; }
    else if (ch >= 0x1d546 && ch <= 0x1d546) { return 1; }
    else if (ch >= 0x1d54a && ch <= 0x1d550) { return 1; }
    else if (ch >= 0x1d56c && ch <= 0x1d585) { return 1; }
    else if (ch >= 0x1d5a0 && ch <= 0x1d5b9) { return 1; }
    else if (ch >= 0x1d5d4 && ch <= 0x1d5ed) { return 1; }
    else if (ch >= 0x1d608 && ch <= 0x1d621) { return 1; }
    else if (ch >= 0x1d63c && ch <= 0x1d655) { return 1; }
    else if (ch >= 0x1d670 && ch <= 0x1d689) { return 1; }
    else if (ch >= 0x1d6a8 && ch <= 0x1d6c0) { return 1; }
    else if (ch >= 0x1d6e2 && ch <= 0x1d6fa) { return 1; }
    else if (ch >= 0x1d71c && ch <= 0x1d734) { return 1; }
    else if (ch >= 0x1d756 && ch <= 0x1d76e) { return 1; }
    else if (ch >= 0x1d790 && ch <= 0x1d7a8) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_mc(esch_unicode ch)
{
    if (ch >= 0x903 && ch <= 0x903) { return 1; }
    else if (ch >= 0x93b && ch <= 0x93b) { return 1; }
    else if (ch >= 0x93e && ch <= 0x940) { return 1; }
    else if (ch >= 0x949 && ch <= 0x94c) { return 1; }
    else if (ch >= 0x94e && ch <= 0x94f) { return 1; }
    else if (ch >= 0x982 && ch <= 0x983) { return 1; }
    else if (ch >= 0x9be && ch <= 0x9c0) { return 1; }
    else if (ch >= 0x9c7 && ch <= 0x9c8) { return 1; }
    else if (ch >= 0x9cb && ch <= 0x9cc) { return 1; }
    else if (ch >= 0x9d7 && ch <= 0x9d7) { return 1; }
    else if (ch >= 0xa03 && ch <= 0xa03) { return 1; }
    else if (ch >= 0xa3e && ch <= 0xa40) { return 1; }
    else if (ch >= 0xa83 && ch <= 0xa83) { return 1; }
    else if (ch >= 0xabe && ch <= 0xac0) { return 1; }
    else if (ch >= 0xac9 && ch <= 0xac9) { return 1; }
    else if (ch >= 0xacb && ch <= 0xacc) { return 1; }
    else if (ch >= 0xb02 && ch <= 0xb03) { return 1; }
    else if (ch >= 0xb3e && ch <= 0xb3e) { return 1; }
    else if (ch >= 0xb40 && ch <= 0xb40) { return 1; }
    else if (ch >= 0xb47 && ch <= 0xb48) { return 1; }
    else if (ch >= 0xb4b && ch <= 0xb4c) { return 1; }
    else if (ch >= 0xb57 && ch <= 0xb57) { return 1; }
    else if (ch >= 0xbbe && ch <= 0xbbf) { return 1; }
    else if (ch >= 0xbc1 && ch <= 0xbc2) { return 1; }
    else if (ch >= 0xbc6 && ch <= 0xbc8) { return 1; }
    else if (ch >= 0xbca && ch <= 0xbcc) { return 1; }
    else if (ch >= 0xbd7 && ch <= 0xbd7) { return 1; }
    else if (ch >= 0xc01 && ch <= 0xc03) { return 1; }
    else if (ch >= 0xc41 && ch <= 0xc44) { return 1; }
    else if (ch >= 0xc82 && ch <= 0xc83) { return 1; }
    else if (ch >= 0xcbe && ch <= 0xcbe) { return 1; }
    else if (ch >= 0xcc0 && ch <= 0xcc4) { return 1; }
    else if (ch >= 0xcc7 && ch <= 0xcc8) { return 1; }
    else if (ch >= 0xcca && ch <= 0xccb) { return 1; }
    else if (ch >= 0xcd5 && ch <= 0xcd6) { return 1; }
    else if (ch >= 0xd02 && ch <= 0xd03) { return 1; }
    else if (ch >= 0xd3e && ch <= 0xd40) { return 1; }
    else if (ch >= 0xd46 && ch <= 0xd48) { return 1; }
    else if (ch >= 0xd4a && ch <= 0xd4c) { return 1; }
    else if (ch >= 0xd57 && ch <= 0xd57) { return 1; }
    else if (ch >= 0xd82 && ch <= 0xd83) { return 1; }
    else if (ch >= 0xdcf && ch <= 0xdd1) { return 1; }
    else if (ch >= 0xdd8 && ch <= 0xddf) { return 1; }
    else if (ch >= 0xdf2 && ch <= 0xdf3) { return 1; }
    else if (ch >= 0xf3e && ch <= 0xf3f) { return 1; }
    else if (ch >= 0xf7f && ch <= 0xf7f) { return 1; }
    else if (ch >= 0x102b && ch <= 0x102c) { return 1; }
    else if (ch >= 0x1031 && ch <= 0x1031) { return 1; }
    else if (ch >= 0x1038 && ch <= 0x1038) { return 1; }
    else if (ch >= 0x103b && ch <= 0x103c) { return 1; }
    else if (ch >= 0x1056 && ch <= 0x1057) { return 1; }
    else if (ch >= 0x1062 && ch <= 0x1064) { return 1; }
    else if (ch >= 0x1067 && ch <= 0x106d) { return 1; }
    else if (ch >= 0x1083 && ch <= 0x1084) { return 1; }
    else if (ch >= 0x1087 && ch <= 0x108c) { return 1; }
    else if (ch >= 0x108f && ch <= 0x108f) { return 1; }
    else if (ch >= 0x109a && ch <= 0x109c) { return 1; }
    else if (ch >= 0x17b6 && ch <= 0x17b6) { return 1; }
    else if (ch >= 0x17be && ch <= 0x17c5) { return 1; }
    else if (ch >= 0x17c7 && ch <= 0x17c8) { return 1; }
    else if (ch >= 0x1923 && ch <= 0x1926) { return 1; }
    else if (ch >= 0x1929 && ch <= 0x192b) { return 1; }
    else if (ch >= 0x1930 && ch <= 0x1931) { return 1; }
    else if (ch >= 0x1933 && ch <= 0x1938) { return 1; }
    else if (ch >= 0x19b0 && ch <= 0x19c0) { return 1; }
    else if (ch >= 0x19c8 && ch <= 0x19c9) { return 1; }
    else if (ch >= 0x1a19 && ch <= 0x1a1b) { return 1; }
    else if (ch >= 0x1a55 && ch <= 0x1a55) { return 1; }
    else if (ch >= 0x1a57 && ch <= 0x1a57) { return 1; }
    else if (ch >= 0x1a61 && ch <= 0x1a61) { return 1; }
    else if (ch >= 0x1a63 && ch <= 0x1a64) { return 1; }
    else if (ch >= 0x1a6d && ch <= 0x1a72) { return 1; }
    else if (ch >= 0x1b04 && ch <= 0x1b04) { return 1; }
    else if (ch >= 0x1b35 && ch <= 0x1b35) { return 1; }
    else if (ch >= 0x1b3b && ch <= 0x1b3b) { return 1; }
    else if (ch >= 0x1b3d && ch <= 0x1b41) { return 1; }
    else if (ch >= 0x1b43 && ch <= 0x1b44) { return 1; }
    else if (ch >= 0x1b82 && ch <= 0x1b82) { return 1; }
    else if (ch >= 0x1ba1 && ch <= 0x1ba1) { return 1; }
    else if (ch >= 0x1ba6 && ch <= 0x1ba7) { return 1; }
    else if (ch >= 0x1baa && ch <= 0x1baa) { return 1; }
    else if (ch >= 0x1bac && ch <= 0x1bad) { return 1; }
    else if (ch >= 0x1be7 && ch <= 0x1be7) { return 1; }
    else if (ch >= 0x1bea && ch <= 0x1bec) { return 1; }
    else if (ch >= 0x1bee && ch <= 0x1bee) { return 1; }
    else if (ch >= 0x1bf2 && ch <= 0x1bf3) { return 1; }
    else if (ch >= 0x1c24 && ch <= 0x1c2b) { return 1; }
    else if (ch >= 0x1c34 && ch <= 0x1c35) { return 1; }
    else if (ch >= 0x1ce1 && ch <= 0x1ce1) { return 1; }
    else if (ch >= 0x1cf2 && ch <= 0x1cf3) { return 1; }
    else if (ch >= 0x302e && ch <= 0x302f) { return 1; }
    else if (ch >= 0xa823 && ch <= 0xa824) { return 1; }
    else if (ch >= 0xa827 && ch <= 0xa827) { return 1; }
    else if (ch >= 0xa880 && ch <= 0xa881) { return 1; }
    else if (ch >= 0xa8b4 && ch <= 0xa8c3) { return 1; }
    else if (ch >= 0xa952 && ch <= 0xa953) { return 1; }
    else if (ch >= 0xa983 && ch <= 0xa983) { return 1; }
    else if (ch >= 0xa9b4 && ch <= 0xa9b5) { return 1; }
    else if (ch >= 0xa9ba && ch <= 0xa9bb) { return 1; }
    else if (ch >= 0xa9bd && ch <= 0xa9c0) { return 1; }
    else if (ch >= 0xaa2f && ch <= 0xaa30) { return 1; }
    else if (ch >= 0xaa33 && ch <= 0xaa34) { return 1; }
    else if (ch >= 0xaa4d && ch <= 0xaa4d) { return 1; }
    else if (ch >= 0xaa7b && ch <= 0xaa7b) { return 1; }
    else if (ch >= 0xaaeb && ch <= 0xaaeb) { return 1; }
    else if (ch >= 0xaaee && ch <= 0xaaef) { return 1; }
    else if (ch >= 0xaaf5 && ch <= 0xaaf5) { return 1; }
    else if (ch >= 0xabe3 && ch <= 0xabe4) { return 1; }
    else if (ch >= 0xabe6 && ch <= 0xabe7) { return 1; }
    else if (ch >= 0xabe9 && ch <= 0xabea) { return 1; }
    else if (ch >= 0xabec && ch <= 0xabec) { return 1; }
    else if (ch >= 0x11000 && ch <= 0x11000) { return 1; }
    else if (ch >= 0x11002 && ch <= 0x11002) { return 1; }
    else if (ch >= 0x11082 && ch <= 0x11082) { return 1; }
    else if (ch >= 0x110b0 && ch <= 0x110b2) { return 1; }
    else if (ch >= 0x110b7 && ch <= 0x110b8) { return 1; }
    else if (ch >= 0x1112c && ch <= 0x1112c) { return 1; }
    else if (ch >= 0x11182 && ch <= 0x11182) { return 1; }
    else if (ch >= 0x111b3 && ch <= 0x111b5) { return 1; }
    else if (ch >= 0x111bf && ch <= 0x111c0) { return 1; }
    else if (ch >= 0x116ac && ch <= 0x116ac) { return 1; }
    else if (ch >= 0x116ae && ch <= 0x116af) { return 1; }
    else if (ch >= 0x116b6 && ch <= 0x116b6) { return 1; }
    else if (ch >= 0x16f51 && ch <= 0x16f7e) { return 1; }
    else if (ch >= 0x1d165 && ch <= 0x1d166) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_mn(esch_unicode ch)
{
    if (ch >= 0x300 && ch <= 0x36f) { return 1; }
    else if (ch >= 0x483 && ch <= 0x487) { return 1; }
    else if (ch >= 0x591 && ch <= 0x5bd) { return 1; }
    else if (ch >= 0x5bf && ch <= 0x5bf) { return 1; }
    else if (ch >= 0x5c1 && ch <= 0x5c2) { return 1; }
    else if (ch >= 0x5c4 && ch <= 0x5c5) { return 1; }
    else if (ch >= 0x5c7 && ch <= 0x5c7) { return 1; }
    else if (ch >= 0x610 && ch <= 0x61a) { return 1; }
    else if (ch >= 0x64b && ch <= 0x65f) { return 1; }
    else if (ch >= 0x670 && ch <= 0x670) { return 1; }
    else if (ch >= 0x6d6 && ch <= 0x6dc) { return 1; }
    else if (ch >= 0x6df && ch <= 0x6e4) { return 1; }
    else if (ch >= 0x6e7 && ch <= 0x6e8) { return 1; }
    else if (ch >= 0x6ea && ch <= 0x6ed) { return 1; }
    else if (ch >= 0x711 && ch <= 0x711) { return 1; }
    else if (ch >= 0x730 && ch <= 0x74a) { return 1; }
    else if (ch >= 0x7a6 && ch <= 0x7b0) { return 1; }
    else if (ch >= 0x7eb && ch <= 0x7f3) { return 1; }
    else if (ch >= 0x816 && ch <= 0x819) { return 1; }
    else if (ch >= 0x81b && ch <= 0x823) { return 1; }
    else if (ch >= 0x825 && ch <= 0x827) { return 1; }
    else if (ch >= 0x829 && ch <= 0x82d) { return 1; }
    else if (ch >= 0x859 && ch <= 0x85b) { return 1; }
    else if (ch >= 0x8e4 && ch <= 0x8fe) { return 1; }
    else if (ch >= 0x900 && ch <= 0x902) { return 1; }
    else if (ch >= 0x93a && ch <= 0x93a) { return 1; }
    else if (ch >= 0x93c && ch <= 0x93c) { return 1; }
    else if (ch >= 0x941 && ch <= 0x948) { return 1; }
    else if (ch >= 0x94d && ch <= 0x94d) { return 1; }
    else if (ch >= 0x951 && ch <= 0x957) { return 1; }
    else if (ch >= 0x962 && ch <= 0x963) { return 1; }
    else if (ch >= 0x981 && ch <= 0x981) { return 1; }
    else if (ch >= 0x9bc && ch <= 0x9bc) { return 1; }
    else if (ch >= 0x9c1 && ch <= 0x9c4) { return 1; }
    else if (ch >= 0x9cd && ch <= 0x9cd) { return 1; }
    else if (ch >= 0x9e2 && ch <= 0x9e3) { return 1; }
    else if (ch >= 0xa01 && ch <= 0xa02) { return 1; }
    else if (ch >= 0xa3c && ch <= 0xa3c) { return 1; }
    else if (ch >= 0xa41 && ch <= 0xa42) { return 1; }
    else if (ch >= 0xa47 && ch <= 0xa48) { return 1; }
    else if (ch >= 0xa4b && ch <= 0xa4d) { return 1; }
    else if (ch >= 0xa51 && ch <= 0xa51) { return 1; }
    else if (ch >= 0xa70 && ch <= 0xa71) { return 1; }
    else if (ch >= 0xa75 && ch <= 0xa75) { return 1; }
    else if (ch >= 0xa81 && ch <= 0xa82) { return 1; }
    else if (ch >= 0xabc && ch <= 0xabc) { return 1; }
    else if (ch >= 0xac1 && ch <= 0xac5) { return 1; }
    else if (ch >= 0xac7 && ch <= 0xac8) { return 1; }
    else if (ch >= 0xacd && ch <= 0xacd) { return 1; }
    else if (ch >= 0xae2 && ch <= 0xae3) { return 1; }
    else if (ch >= 0xb01 && ch <= 0xb01) { return 1; }
    else if (ch >= 0xb3c && ch <= 0xb3c) { return 1; }
    else if (ch >= 0xb3f && ch <= 0xb3f) { return 1; }
    else if (ch >= 0xb41 && ch <= 0xb44) { return 1; }
    else if (ch >= 0xb4d && ch <= 0xb4d) { return 1; }
    else if (ch >= 0xb56 && ch <= 0xb56) { return 1; }
    else if (ch >= 0xb62 && ch <= 0xb63) { return 1; }
    else if (ch >= 0xb82 && ch <= 0xb82) { return 1; }
    else if (ch >= 0xbc0 && ch <= 0xbc0) { return 1; }
    else if (ch >= 0xbcd && ch <= 0xbcd) { return 1; }
    else if (ch >= 0xc3e && ch <= 0xc40) { return 1; }
    else if (ch >= 0xc46 && ch <= 0xc48) { return 1; }
    else if (ch >= 0xc4a && ch <= 0xc4d) { return 1; }
    else if (ch >= 0xc55 && ch <= 0xc56) { return 1; }
    else if (ch >= 0xc62 && ch <= 0xc63) { return 1; }
    else if (ch >= 0xcbc && ch <= 0xcbc) { return 1; }
    else if (ch >= 0xcbf && ch <= 0xcbf) { return 1; }
    else if (ch >= 0xcc6 && ch <= 0xcc6) { return 1; }
    else if (ch >= 0xccc && ch <= 0xccd) { return 1; }
    else if (ch >= 0xce2 && ch <= 0xce3) { return 1; }
    else if (ch >= 0xd41 && ch <= 0xd44) { return 1; }
    else if (ch >= 0xd4d && ch <= 0xd4d) { return 1; }
    else if (ch >= 0xd62 && ch <= 0xd63) { return 1; }
    else if (ch >= 0xdca && ch <= 0xdca) { return 1; }
    else if (ch >= 0xdd2 && ch <= 0xdd4) { return 1; }
    else if (ch >= 0xdd6 && ch <= 0xdd6) { return 1; }
    else if (ch >= 0xe31 && ch <= 0xe31) { return 1; }
    else if (ch >= 0xe34 && ch <= 0xe3a) { return 1; }
    else if (ch >= 0xe47 && ch <= 0xe4e) { return 1; }
    else if (ch >= 0xeb1 && ch <= 0xeb1) { return 1; }
    else if (ch >= 0xeb4 && ch <= 0xeb9) { return 1; }
    else if (ch >= 0xebb && ch <= 0xebc) { return 1; }
    else if (ch >= 0xec8 && ch <= 0xecd) { return 1; }
    else if (ch >= 0xf18 && ch <= 0xf19) { return 1; }
    else if (ch >= 0xf35 && ch <= 0xf35) { return 1; }
    else if (ch >= 0xf37 && ch <= 0xf37) { return 1; }
    else if (ch >= 0xf39 && ch <= 0xf39) { return 1; }
    else if (ch >= 0xf71 && ch <= 0xf7e) { return 1; }
    else if (ch >= 0xf80 && ch <= 0xf84) { return 1; }
    else if (ch >= 0xf86 && ch <= 0xf87) { return 1; }
    else if (ch >= 0xf8d && ch <= 0xf97) { return 1; }
    else if (ch >= 0xf99 && ch <= 0xfbc) { return 1; }
    else if (ch >= 0xfc6 && ch <= 0xfc6) { return 1; }
    else if (ch >= 0x102d && ch <= 0x1030) { return 1; }
    else if (ch >= 0x1032 && ch <= 0x1037) { return 1; }
    else if (ch >= 0x1039 && ch <= 0x103a) { return 1; }
    else if (ch >= 0x103d && ch <= 0x103e) { return 1; }
    else if (ch >= 0x1058 && ch <= 0x1059) { return 1; }
    else if (ch >= 0x105e && ch <= 0x1060) { return 1; }
    else if (ch >= 0x1071 && ch <= 0x1074) { return 1; }
    else if (ch >= 0x1082 && ch <= 0x1082) { return 1; }
    else if (ch >= 0x1085 && ch <= 0x1086) { return 1; }
    else if (ch >= 0x108d && ch <= 0x108d) { return 1; }
    else if (ch >= 0x109d && ch <= 0x109d) { return 1; }
    else if (ch >= 0x135d && ch <= 0x135f) { return 1; }
    else if (ch >= 0x1712 && ch <= 0x1714) { return 1; }
    else if (ch >= 0x1732 && ch <= 0x1734) { return 1; }
    else if (ch >= 0x1752 && ch <= 0x1753) { return 1; }
    else if (ch >= 0x1772 && ch <= 0x1773) { return 1; }
    else if (ch >= 0x17b4 && ch <= 0x17b5) { return 1; }
    else if (ch >= 0x17b7 && ch <= 0x17bd) { return 1; }
    else if (ch >= 0x17c6 && ch <= 0x17c6) { return 1; }
    else if (ch >= 0x17c9 && ch <= 0x17d3) { return 1; }
    else if (ch >= 0x17dd && ch <= 0x17dd) { return 1; }
    else if (ch >= 0x180b && ch <= 0x180d) { return 1; }
    else if (ch >= 0x18a9 && ch <= 0x18a9) { return 1; }
    else if (ch >= 0x1920 && ch <= 0x1922) { return 1; }
    else if (ch >= 0x1927 && ch <= 0x1928) { return 1; }
    else if (ch >= 0x1932 && ch <= 0x1932) { return 1; }
    else if (ch >= 0x1939 && ch <= 0x193b) { return 1; }
    else if (ch >= 0x1a17 && ch <= 0x1a18) { return 1; }
    else if (ch >= 0x1a56 && ch <= 0x1a56) { return 1; }
    else if (ch >= 0x1a58 && ch <= 0x1a5e) { return 1; }
    else if (ch >= 0x1a60 && ch <= 0x1a60) { return 1; }
    else if (ch >= 0x1a62 && ch <= 0x1a62) { return 1; }
    else if (ch >= 0x1a65 && ch <= 0x1a6c) { return 1; }
    else if (ch >= 0x1a73 && ch <= 0x1a7c) { return 1; }
    else if (ch >= 0x1a7f && ch <= 0x1a7f) { return 1; }
    else if (ch >= 0x1b00 && ch <= 0x1b03) { return 1; }
    else if (ch >= 0x1b34 && ch <= 0x1b34) { return 1; }
    else if (ch >= 0x1b36 && ch <= 0x1b3a) { return 1; }
    else if (ch >= 0x1b3c && ch <= 0x1b3c) { return 1; }
    else if (ch >= 0x1b42 && ch <= 0x1b42) { return 1; }
    else if (ch >= 0x1b6b && ch <= 0x1b73) { return 1; }
    else if (ch >= 0x1b80 && ch <= 0x1b81) { return 1; }
    else if (ch >= 0x1ba2 && ch <= 0x1ba5) { return 1; }
    else if (ch >= 0x1ba8 && ch <= 0x1ba9) { return 1; }
    else if (ch >= 0x1bab && ch <= 0x1bab) { return 1; }
    else if (ch >= 0x1be6 && ch <= 0x1be6) { return 1; }
    else if (ch >= 0x1be8 && ch <= 0x1be9) { return 1; }
    else if (ch >= 0x1bed && ch <= 0x1bed) { return 1; }
    else if (ch >= 0x1bef && ch <= 0x1bf1) { return 1; }
    else if (ch >= 0x1c2c && ch <= 0x1c33) { return 1; }
    else if (ch >= 0x1c36 && ch <= 0x1c37) { return 1; }
    else if (ch >= 0x1cd0 && ch <= 0x1cd2) { return 1; }
    else if (ch >= 0x1cd4 && ch <= 0x1ce0) { return 1; }
    else if (ch >= 0x1ce2 && ch <= 0x1ce8) { return 1; }
    else if (ch >= 0x1ced && ch <= 0x1ced) { return 1; }
    else if (ch >= 0x1cf4 && ch <= 0x1cf4) { return 1; }
    else if (ch >= 0x1dc0 && ch <= 0x1de6) { return 1; }
    else if (ch >= 0x1dfc && ch <= 0x1dff) { return 1; }
    else if (ch >= 0x20d0 && ch <= 0x20dc) { return 1; }
    else if (ch >= 0x20e1 && ch <= 0x20e1) { return 1; }
    else if (ch >= 0x20e5 && ch <= 0x20f0) { return 1; }
    else if (ch >= 0x2cef && ch <= 0x2cf1) { return 1; }
    else if (ch >= 0x2d7f && ch <= 0x2d7f) { return 1; }
    else if (ch >= 0x2de0 && ch <= 0x2dff) { return 1; }
    else if (ch >= 0x302a && ch <= 0x302d) { return 1; }
    else if (ch >= 0x3099 && ch <= 0x309a) { return 1; }
    else if (ch >= 0xa66f && ch <= 0xa66f) { return 1; }
    else if (ch >= 0xa674 && ch <= 0xa67d) { return 1; }
    else if (ch >= 0xa69f && ch <= 0xa69f) { return 1; }
    else if (ch >= 0xa6f0 && ch <= 0xa6f1) { return 1; }
    else if (ch >= 0xa802 && ch <= 0xa802) { return 1; }
    else if (ch >= 0xa806 && ch <= 0xa806) { return 1; }
    else if (ch >= 0xa80b && ch <= 0xa80b) { return 1; }
    else if (ch >= 0xa825 && ch <= 0xa826) { return 1; }
    else if (ch >= 0xa8c4 && ch <= 0xa8c4) { return 1; }
    else if (ch >= 0xa8e0 && ch <= 0xa8f1) { return 1; }
    else if (ch >= 0xa926 && ch <= 0xa92d) { return 1; }
    else if (ch >= 0xa947 && ch <= 0xa951) { return 1; }
    else if (ch >= 0xa980 && ch <= 0xa982) { return 1; }
    else if (ch >= 0xa9b3 && ch <= 0xa9b3) { return 1; }
    else if (ch >= 0xa9b6 && ch <= 0xa9b9) { return 1; }
    else if (ch >= 0xa9bc && ch <= 0xa9bc) { return 1; }
    else if (ch >= 0xaa29 && ch <= 0xaa2e) { return 1; }
    else if (ch >= 0xaa31 && ch <= 0xaa32) { return 1; }
    else if (ch >= 0xaa35 && ch <= 0xaa36) { return 1; }
    else if (ch >= 0xaa43 && ch <= 0xaa43) { return 1; }
    else if (ch >= 0xaa4c && ch <= 0xaa4c) { return 1; }
    else if (ch >= 0xaab0 && ch <= 0xaab0) { return 1; }
    else if (ch >= 0xaab2 && ch <= 0xaab4) { return 1; }
    else if (ch >= 0xaab7 && ch <= 0xaab8) { return 1; }
    else if (ch >= 0xaabe && ch <= 0xaabf) { return 1; }
    else if (ch >= 0xaac1 && ch <= 0xaac1) { return 1; }
    else if (ch >= 0xaaec && ch <= 0xaaed) { return 1; }
    else if (ch >= 0xaaf6 && ch <= 0xaaf6) { return 1; }
    else if (ch >= 0xabe5 && ch <= 0xabe5) { return 1; }
    else if (ch >= 0xabe8 && ch <= 0xabe8) { return 1; }
    else if (ch >= 0xabed && ch <= 0xabed) { return 1; }
    else if (ch >= 0xfb1e && ch <= 0xfb1e) { return 1; }
    else if (ch >= 0xfe00 && ch <= 0xfe0f) { return 1; }
    else if (ch >= 0xfe20 && ch <= 0xfe26) { return 1; }
    else if (ch >= 0x101fd && ch <= 0x101fd) { return 1; }
    else if (ch >= 0x10a01 && ch <= 0x10a03) { return 1; }
    else if (ch >= 0x10a05 && ch <= 0x10a06) { return 1; }
    else if (ch >= 0x10a0c && ch <= 0x10a0f) { return 1; }
    else if (ch >= 0x10a38 && ch <= 0x10a3a) { return 1; }
    else if (ch >= 0x10a3f && ch <= 0x10a3f) { return 1; }
    else if (ch >= 0x11001 && ch <= 0x11001) { return 1; }
    else if (ch >= 0x11038 && ch <= 0x11046) { return 1; }
    else if (ch >= 0x11080 && ch <= 0x11081) { return 1; }
    else if (ch >= 0x110b3 && ch <= 0x110b6) { return 1; }
    else if (ch >= 0x110b9 && ch <= 0x110ba) { return 1; }
    else if (ch >= 0x11100 && ch <= 0x11102) { return 1; }
    else if (ch >= 0x11127 && ch <= 0x1112b) { return 1; }
    else if (ch >= 0x1112d && ch <= 0x11134) { return 1; }
    else if (ch >= 0x11180 && ch <= 0x11181) { return 1; }
    else if (ch >= 0x111b6 && ch <= 0x111be) { return 1; }
    else if (ch >= 0x116ab && ch <= 0x116ab) { return 1; }
    else if (ch >= 0x116ad && ch <= 0x116ad) { return 1; }
    else if (ch >= 0x116b0 && ch <= 0x116b5) { return 1; }
    else if (ch >= 0x116b7 && ch <= 0x116b7) { return 1; }
    else if (ch >= 0x16f8f && ch <= 0x16f92) { return 1; }
    else if (ch >= 0x1d167 && ch <= 0x1d169) { return 1; }
    else if (ch >= 0x1d17b && ch <= 0x1d182) { return 1; }
    else if (ch >= 0x1d185 && ch <= 0x1d18b) { return 1; }
    else if (ch >= 0x1d1aa && ch <= 0x1d1ad) { return 1; }
    else if (ch >= 0x1d242 && ch <= 0x1d244) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_nd(esch_unicode ch)
{
    if (ch >= 0x30 && ch <= 0x39) { return 1; }
    else if (ch >= 0x660 && ch <= 0x669) { return 1; }
    else if (ch >= 0x6f0 && ch <= 0x6f9) { return 1; }
    else if (ch >= 0x7c0 && ch <= 0x7c9) { return 1; }
    else if (ch >= 0x966 && ch <= 0x96f) { return 1; }
    else if (ch >= 0x9e6 && ch <= 0x9ef) { return 1; }
    else if (ch >= 0xa66 && ch <= 0xa6f) { return 1; }
    else if (ch >= 0xae6 && ch <= 0xaef) { return 1; }
    else if (ch >= 0xb66 && ch <= 0xb6f) { return 1; }
    else if (ch >= 0xbe6 && ch <= 0xbef) { return 1; }
    else if (ch >= 0xc66 && ch <= 0xc6f) { return 1; }
    else if (ch >= 0xce6 && ch <= 0xcef) { return 1; }
    else if (ch >= 0xd66 && ch <= 0xd6f) { return 1; }
    else if (ch >= 0xe50 && ch <= 0xe59) { return 1; }
    else if (ch >= 0xed0 && ch <= 0xed9) { return 1; }
    else if (ch >= 0xf20 && ch <= 0xf29) { return 1; }
    else if (ch >= 0x1040 && ch <= 0x1049) { return 1; }
    else if (ch >= 0x1090 && ch <= 0x1099) { return 1; }
    else if (ch >= 0x17e0 && ch <= 0x17e9) { return 1; }
    else if (ch >= 0x1810 && ch <= 0x1819) { return 1; }
    else if (ch >= 0x1946 && ch <= 0x194f) { return 1; }
    else if (ch >= 0x19d0 && ch <= 0x19d9) { return 1; }
    else if (ch >= 0x1a80 && ch <= 0x1a89) { return 1; }
    else if (ch >= 0x1a90 && ch <= 0x1a99) { return 1; }
    else if (ch >= 0x1b50 && ch <= 0x1b59) { return 1; }
    else if (ch >= 0x1bb0 && ch <= 0x1bb9) { return 1; }
    else if (ch >= 0x1c40 && ch <= 0x1c49) { return 1; }
    else if (ch >= 0x1c50 && ch <= 0x1c59) { return 1; }
    else if (ch >= 0xa620 && ch <= 0xa629) { return 1; }
    else if (ch >= 0xa8d0 && ch <= 0xa8d9) { return 1; }
    else if (ch >= 0xa900 && ch <= 0xa909) { return 1; }
    else if (ch >= 0xa9d0 && ch <= 0xa9d9) { return 1; }
    else if (ch >= 0xaa50 && ch <= 0xaa59) { return 1; }
    else if (ch >= 0xabf0 && ch <= 0xabf9) { return 1; }
    else if (ch >= 0xff10 && ch <= 0xff19) { return 1; }
    else if (ch >= 0x104a0 && ch <= 0x104a9) { return 1; }
    else if (ch >= 0x11066 && ch <= 0x1106f) { return 1; }
    else if (ch >= 0x110f0 && ch <= 0x110f9) { return 1; }
    else if (ch >= 0x11136 && ch <= 0x1113f) { return 1; }
    else if (ch >= 0x111d0 && ch <= 0x111d9) { return 1; }
    else if (ch >= 0x116c0 && ch <= 0x116c9) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_nl(esch_unicode ch)
{
    if (ch >= 0x16ee && ch <= 0x16f0) { return 1; }
    else if (ch >= 0x2160 && ch <= 0x2182) { return 1; }
    else if (ch >= 0x2185 && ch <= 0x2188) { return 1; }
    else if (ch >= 0x3007 && ch <= 0x3007) { return 1; }
    else if (ch >= 0x3021 && ch <= 0x3029) { return 1; }
    else if (ch >= 0x3038 && ch <= 0x303a) { return 1; }
    else if (ch >= 0xa6e6 && ch <= 0xa6ef) { return 1; }
    else if (ch >= 0x10140 && ch <= 0x10174) { return 1; }
    else if (ch >= 0x10341 && ch <= 0x10341) { return 1; }
    else if (ch >= 0x1034a && ch <= 0x1034a) { return 1; }
    else if (ch >= 0x103d1 && ch <= 0x103d5) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_no(esch_unicode ch)
{
    if (ch >= 0xb2 && ch <= 0xb3) { return 1; }
    else if (ch >= 0xb9 && ch <= 0xb9) { return 1; }
    else if (ch >= 0xbc && ch <= 0xbe) { return 1; }
    else if (ch >= 0x9f4 && ch <= 0x9f9) { return 1; }
    else if (ch >= 0xb72 && ch <= 0xb77) { return 1; }
    else if (ch >= 0xbf0 && ch <= 0xbf2) { return 1; }
    else if (ch >= 0xc78 && ch <= 0xc7e) { return 1; }
    else if (ch >= 0xd70 && ch <= 0xd75) { return 1; }
    else if (ch >= 0xf2a && ch <= 0xf33) { return 1; }
    else if (ch >= 0x1369 && ch <= 0x137c) { return 1; }
    else if (ch >= 0x17f0 && ch <= 0x17f9) { return 1; }
    else if (ch >= 0x19da && ch <= 0x19da) { return 1; }
    else if (ch >= 0x2070 && ch <= 0x2070) { return 1; }
    else if (ch >= 0x2074 && ch <= 0x2079) { return 1; }
    else if (ch >= 0x2080 && ch <= 0x2089) { return 1; }
    else if (ch >= 0x2150 && ch <= 0x215f) { return 1; }
    else if (ch >= 0x2189 && ch <= 0x2189) { return 1; }
    else if (ch >= 0x2460 && ch <= 0x249b) { return 1; }
    else if (ch >= 0x24ea && ch <= 0x24ff) { return 1; }
    else if (ch >= 0x2776 && ch <= 0x2793) { return 1; }
    else if (ch >= 0x2cfd && ch <= 0x2cfd) { return 1; }
    else if (ch >= 0x3192 && ch <= 0x3195) { return 1; }
    else if (ch >= 0x3220 && ch <= 0x3229) { return 1; }
    else if (ch >= 0x3248 && ch <= 0x324f) { return 1; }
    else if (ch >= 0x3251 && ch <= 0x325f) { return 1; }
    else if (ch >= 0x3280 && ch <= 0x3289) { return 1; }
    else if (ch >= 0x32b1 && ch <= 0x32bf) { return 1; }
    else if (ch >= 0xa830 && ch <= 0xa835) { return 1; }
    else if (ch >= 0x10107 && ch <= 0x10133) { return 1; }
    else if (ch >= 0x10175 && ch <= 0x10178) { return 1; }
    else if (ch >= 0x1018a && ch <= 0x1018a) { return 1; }
    else if (ch >= 0x10320 && ch <= 0x10323) { return 1; }
    else if (ch >= 0x10858 && ch <= 0x1085f) { return 1; }
    else if (ch >= 0x10916 && ch <= 0x1091b) { return 1; }
    else if (ch >= 0x10a40 && ch <= 0x10a47) { return 1; }
    else if (ch >= 0x10a7d && ch <= 0x10a7e) { return 1; }
    else if (ch >= 0x10b58 && ch <= 0x10b5f) { return 1; }
    else if (ch >= 0x10b78 && ch <= 0x10b7f) { return 1; }
    else if (ch >= 0x10e60 && ch <= 0x10e7e) { return 1; }
    else if (ch >= 0x11052 && ch <= 0x11065) { return 1; }
    else if (ch >= 0x1d360 && ch <= 0x1d371) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_pc(esch_unicode ch)
{
    if (ch >= 0x5f && ch <= 0x5f) { return 1; }
    else if (ch >= 0x203f && ch <= 0x2040) { return 1; }
    else if (ch >= 0x2054 && ch <= 0x2054) { return 1; }
    else if (ch >= 0xfe33 && ch <= 0xfe34) { return 1; }
    else if (ch >= 0xfe4d && ch <= 0xfe4f) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_pd(esch_unicode ch)
{
    if (ch >= 0x2d && ch <= 0x2d) { return 1; }
    else if (ch >= 0x58a && ch <= 0x58a) { return 1; }
    else if (ch >= 0x5be && ch <= 0x5be) { return 1; }
    else if (ch >= 0x1400 && ch <= 0x1400) { return 1; }
    else if (ch >= 0x1806 && ch <= 0x1806) { return 1; }
    else if (ch >= 0x2010 && ch <= 0x2015) { return 1; }
    else if (ch >= 0x2e17 && ch <= 0x2e17) { return 1; }
    else if (ch >= 0x2e1a && ch <= 0x2e1a) { return 1; }
    else if (ch >= 0x2e3a && ch <= 0x2e3b) { return 1; }
    else if (ch >= 0x301c && ch <= 0x301c) { return 1; }
    else if (ch >= 0x3030 && ch <= 0x3030) { return 1; }
    else if (ch >= 0x30a0 && ch <= 0x30a0) { return 1; }
    else if (ch >= 0xfe31 && ch <= 0xfe32) { return 1; }
    else if (ch >= 0xfe58 && ch <= 0xfe58) { return 1; }
    else if (ch >= 0xfe63 && ch <= 0xfe63) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_po(esch_unicode ch)
{
    if (ch >= 0x21 && ch <= 0x23) { return 1; }
    else if (ch >= 0x25 && ch <= 0x27) { return 1; }
    else if (ch >= 0x2a && ch <= 0x2a) { return 1; }
    else if (ch >= 0x2c && ch <= 0x2c) { return 1; }
    else if (ch >= 0x2e && ch <= 0x2f) { return 1; }
    else if (ch >= 0x3a && ch <= 0x3b) { return 1; }
    else if (ch >= 0x3f && ch <= 0x40) { return 1; }
    else if (ch >= 0x5c && ch <= 0x5c) { return 1; }
    else if (ch >= 0xa1 && ch <= 0xa1) { return 1; }
    else if (ch >= 0xa7 && ch <= 0xa7) { return 1; }
    else if (ch >= 0xb6 && ch <= 0xb7) { return 1; }
    else if (ch >= 0xbf && ch <= 0xbf) { return 1; }
    else if (ch >= 0x37e && ch <= 0x37e) { return 1; }
    else if (ch >= 0x387 && ch <= 0x387) { return 1; }
    else if (ch >= 0x55a && ch <= 0x55f) { return 1; }
    else if (ch >= 0x589 && ch <= 0x589) { return 1; }
    else if (ch >= 0x5c0 && ch <= 0x5c0) { return 1; }
    else if (ch >= 0x5c3 && ch <= 0x5c3) { return 1; }
    else if (ch >= 0x5c6 && ch <= 0x5c6) { return 1; }
    else if (ch >= 0x5f3 && ch <= 0x5f4) { return 1; }
    else if (ch >= 0x609 && ch <= 0x60a) { return 1; }
    else if (ch >= 0x60c && ch <= 0x60d) { return 1; }
    else if (ch >= 0x61b && ch <= 0x61b) { return 1; }
    else if (ch >= 0x61e && ch <= 0x61f) { return 1; }
    else if (ch >= 0x66a && ch <= 0x66d) { return 1; }
    else if (ch >= 0x6d4 && ch <= 0x6d4) { return 1; }
    else if (ch >= 0x700 && ch <= 0x70d) { return 1; }
    else if (ch >= 0x7f7 && ch <= 0x7f9) { return 1; }
    else if (ch >= 0x830 && ch <= 0x83e) { return 1; }
    else if (ch >= 0x85e && ch <= 0x85e) { return 1; }
    else if (ch >= 0x964 && ch <= 0x965) { return 1; }
    else if (ch >= 0x970 && ch <= 0x970) { return 1; }
    else if (ch >= 0xaf0 && ch <= 0xaf0) { return 1; }
    else if (ch >= 0xdf4 && ch <= 0xdf4) { return 1; }
    else if (ch >= 0xe4f && ch <= 0xe4f) { return 1; }
    else if (ch >= 0xe5a && ch <= 0xe5b) { return 1; }
    else if (ch >= 0xf04 && ch <= 0xf12) { return 1; }
    else if (ch >= 0xf14 && ch <= 0xf14) { return 1; }
    else if (ch >= 0xf85 && ch <= 0xf85) { return 1; }
    else if (ch >= 0xfd0 && ch <= 0xfd4) { return 1; }
    else if (ch >= 0xfd9 && ch <= 0xfda) { return 1; }
    else if (ch >= 0x104a && ch <= 0x104f) { return 1; }
    else if (ch >= 0x10fb && ch <= 0x10fb) { return 1; }
    else if (ch >= 0x1360 && ch <= 0x1368) { return 1; }
    else if (ch >= 0x166d && ch <= 0x166e) { return 1; }
    else if (ch >= 0x16eb && ch <= 0x16ed) { return 1; }
    else if (ch >= 0x1735 && ch <= 0x1736) { return 1; }
    else if (ch >= 0x17d4 && ch <= 0x17d6) { return 1; }
    else if (ch >= 0x17d8 && ch <= 0x17da) { return 1; }
    else if (ch >= 0x1800 && ch <= 0x1805) { return 1; }
    else if (ch >= 0x1807 && ch <= 0x180a) { return 1; }
    else if (ch >= 0x1944 && ch <= 0x1945) { return 1; }
    else if (ch >= 0x1a1e && ch <= 0x1a1f) { return 1; }
    else if (ch >= 0x1aa0 && ch <= 0x1aa6) { return 1; }
    else if (ch >= 0x1aa8 && ch <= 0x1aad) { return 1; }
    else if (ch >= 0x1b5a && ch <= 0x1b60) { return 1; }
    else if (ch >= 0x1bfc && ch <= 0x1bff) { return 1; }
    else if (ch >= 0x1c3b && ch <= 0x1c3f) { return 1; }
    else if (ch >= 0x1c7e && ch <= 0x1c7f) { return 1; }
    else if (ch >= 0x1cc0 && ch <= 0x1cc7) { return 1; }
    else if (ch >= 0x1cd3 && ch <= 0x1cd3) { return 1; }
    else if (ch >= 0x2016 && ch <= 0x2017) { return 1; }
    else if (ch >= 0x2020 && ch <= 0x2027) { return 1; }
    else if (ch >= 0x2030 && ch <= 0x2038) { return 1; }
    else if (ch >= 0x203b && ch <= 0x203e) { return 1; }
    else if (ch >= 0x2041 && ch <= 0x2043) { return 1; }
    else if (ch >= 0x2047 && ch <= 0x2051) { return 1; }
    else if (ch >= 0x2053 && ch <= 0x2053) { return 1; }
    else if (ch >= 0x2055 && ch <= 0x205e) { return 1; }
    else if (ch >= 0x2cf9 && ch <= 0x2cfc) { return 1; }
    else if (ch >= 0x2cfe && ch <= 0x2cff) { return 1; }
    else if (ch >= 0x2d70 && ch <= 0x2d70) { return 1; }
    else if (ch >= 0x2e00 && ch <= 0x2e01) { return 1; }
    else if (ch >= 0x2e06 && ch <= 0x2e08) { return 1; }
    else if (ch >= 0x2e0b && ch <= 0x2e0b) { return 1; }
    else if (ch >= 0x2e0e && ch <= 0x2e16) { return 1; }
    else if (ch >= 0x2e18 && ch <= 0x2e19) { return 1; }
    else if (ch >= 0x2e1b && ch <= 0x2e1b) { return 1; }
    else if (ch >= 0x2e1e && ch <= 0x2e1f) { return 1; }
    else if (ch >= 0x2e2a && ch <= 0x2e2e) { return 1; }
    else if (ch >= 0x2e30 && ch <= 0x2e39) { return 1; }
    else if (ch >= 0x3001 && ch <= 0x3003) { return 1; }
    else if (ch >= 0x303d && ch <= 0x303d) { return 1; }
    else if (ch >= 0x30fb && ch <= 0x30fb) { return 1; }
    else if (ch >= 0xa4fe && ch <= 0xa4ff) { return 1; }
    else if (ch >= 0xa60d && ch <= 0xa60f) { return 1; }
    else if (ch >= 0xa673 && ch <= 0xa673) { return 1; }
    else if (ch >= 0xa67e && ch <= 0xa67e) { return 1; }
    else if (ch >= 0xa6f2 && ch <= 0xa6f7) { return 1; }
    else if (ch >= 0xa874 && ch <= 0xa877) { return 1; }
    else if (ch >= 0xa8ce && ch <= 0xa8cf) { return 1; }
    else if (ch >= 0xa8f8 && ch <= 0xa8fa) { return 1; }
    else if (ch >= 0xa92e && ch <= 0xa92f) { return 1; }
    else if (ch >= 0xa95f && ch <= 0xa95f) { return 1; }
    else if (ch >= 0xa9c1 && ch <= 0xa9cd) { return 1; }
    else if (ch >= 0xa9de && ch <= 0xa9df) { return 1; }
    else if (ch >= 0xaa5c && ch <= 0xaa5f) { return 1; }
    else if (ch >= 0xaade && ch <= 0xaadf) { return 1; }
    else if (ch >= 0xaaf0 && ch <= 0xaaf1) { return 1; }
    else if (ch >= 0xabeb && ch <= 0xabeb) { return 1; }
    else if (ch >= 0xfe10 && ch <= 0xfe16) { return 1; }
    else if (ch >= 0xfe19 && ch <= 0xfe19) { return 1; }
    else if (ch >= 0xfe30 && ch <= 0xfe30) { return 1; }
    else if (ch >= 0xfe45 && ch <= 0xfe46) { return 1; }
    else if (ch >= 0xfe49 && ch <= 0xfe4c) { return 1; }
    else if (ch >= 0xfe50 && ch <= 0xfe52) { return 1; }
    else if (ch >= 0xfe54 && ch <= 0xfe57) { return 1; }
    else if (ch >= 0xfe5f && ch <= 0xfe61) { return 1; }
    else if (ch >= 0xfe68 && ch <= 0xfe68) { return 1; }
    else if (ch >= 0xfe6a && ch <= 0xfe6b) { return 1; }
    else if (ch >= 0xff01 && ch <= 0xff03) { return 1; }
    else if (ch >= 0xff05 && ch <= 0xff07) { return 1; }
    else if (ch >= 0xff0a && ch <= 0xff0a) { return 1; }
    else if (ch >= 0xff0c && ch <= 0xff0c) { return 1; }
    else if (ch >= 0xff0e && ch <= 0xff0f) { return 1; }
    else if (ch >= 0xff1a && ch <= 0xff1b) { return 1; }
    else if (ch >= 0xff1f && ch <= 0xff20) { return 1; }
    else if (ch >= 0xff3c && ch <= 0xff3c) { return 1; }
    else if (ch >= 0xff61 && ch <= 0xff61) { return 1; }
    else if (ch >= 0xff64 && ch <= 0xff65) { return 1; }
    else if (ch >= 0x10100 && ch <= 0x10102) { return 1; }
    else if (ch >= 0x1039f && ch <= 0x1039f) { return 1; }
    else if (ch >= 0x103d0 && ch <= 0x103d0) { return 1; }
    else if (ch >= 0x10857 && ch <= 0x10857) { return 1; }
    else if (ch >= 0x1091f && ch <= 0x1091f) { return 1; }
    else if (ch >= 0x1093f && ch <= 0x1093f) { return 1; }
    else if (ch >= 0x10a50 && ch <= 0x10a58) { return 1; }
    else if (ch >= 0x10a7f && ch <= 0x10a7f) { return 1; }
    else if (ch >= 0x10b39 && ch <= 0x10b3f) { return 1; }
    else if (ch >= 0x11047 && ch <= 0x1104d) { return 1; }
    else if (ch >= 0x110bb && ch <= 0x110bc) { return 1; }
    else if (ch >= 0x110be && ch <= 0x110c1) { return 1; }
    else if (ch >= 0x11140 && ch <= 0x11143) { return 1; }
    else if (ch >= 0x111c5 && ch <= 0x111c8) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_sc(esch_unicode ch)
{
    if (ch >= 0x24 && ch <= 0x24) { return 1; }
    else if (ch >= 0xa2 && ch <= 0xa5) { return 1; }
    else if (ch >= 0x58f && ch <= 0x58f) { return 1; }
    else if (ch >= 0x60b && ch <= 0x60b) { return 1; }
    else if (ch >= 0x9f2 && ch <= 0x9f3) { return 1; }
    else if (ch >= 0x9fb && ch <= 0x9fb) { return 1; }
    else if (ch >= 0xaf1 && ch <= 0xaf1) { return 1; }
    else if (ch >= 0xbf9 && ch <= 0xbf9) { return 1; }
    else if (ch >= 0xe3f && ch <= 0xe3f) { return 1; }
    else if (ch >= 0x17db && ch <= 0x17db) { return 1; }
    else if (ch >= 0x20a0 && ch <= 0x20b9) { return 1; }
    else if (ch >= 0xa838 && ch <= 0xa838) { return 1; }
    else if (ch >= 0xfdfc && ch <= 0xfdfc) { return 1; }
    else if (ch >= 0xfe69 && ch <= 0xfe69) { return 1; }
    else if (ch >= 0xff04 && ch <= 0xff04) { return 1; }
    else if (ch >= 0xffe0 && ch <= 0xffe1) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_sk(esch_unicode ch)
{
    if (ch >= 0x5e && ch <= 0x5e) { return 1; }
    else if (ch >= 0x60 && ch <= 0x60) { return 1; }
    else if (ch >= 0xa8 && ch <= 0xa8) { return 1; }
    else if (ch >= 0xaf && ch <= 0xaf) { return 1; }
    else if (ch >= 0xb4 && ch <= 0xb4) { return 1; }
    else if (ch >= 0xb8 && ch <= 0xb8) { return 1; }
    else if (ch >= 0x2c2 && ch <= 0x2c5) { return 1; }
    else if (ch >= 0x2d2 && ch <= 0x2df) { return 1; }
    else if (ch >= 0x2e5 && ch <= 0x2eb) { return 1; }
    else if (ch >= 0x2ed && ch <= 0x2ed) { return 1; }
    else if (ch >= 0x2ef && ch <= 0x2ff) { return 1; }
    else if (ch >= 0x375 && ch <= 0x375) { return 1; }
    else if (ch >= 0x384 && ch <= 0x385) { return 1; }
    else if (ch >= 0x1fbd && ch <= 0x1fbd) { return 1; }
    else if (ch >= 0x1fbf && ch <= 0x1fc1) { return 1; }
    else if (ch >= 0x1fcd && ch <= 0x1fcf) { return 1; }
    else if (ch >= 0x1fdd && ch <= 0x1fdf) { return 1; }
    else if (ch >= 0x1fed && ch <= 0x1fef) { return 1; }
    else if (ch >= 0x1ffd && ch <= 0x1ffe) { return 1; }
    else if (ch >= 0x309b && ch <= 0x309c) { return 1; }
    else if (ch >= 0xa700 && ch <= 0xa716) { return 1; }
    else if (ch >= 0xa720 && ch <= 0xa721) { return 1; }
    else if (ch >= 0xa789 && ch <= 0xa78a) { return 1; }
    else if (ch >= 0xfbb2 && ch <= 0xfbc1) { return 1; }
    else if (ch >= 0xff3e && ch <= 0xff3e) { return 1; }
    else if (ch >= 0xff40 && ch <= 0xff40) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_sm(esch_unicode ch)
{
    if (ch >= 0x2b && ch <= 0x2b) { return 1; }
    else if (ch >= 0x3c && ch <= 0x3e) { return 1; }
    else if (ch >= 0x7c && ch <= 0x7c) { return 1; }
    else if (ch >= 0x7e && ch <= 0x7e) { return 1; }
    else if (ch >= 0xac && ch <= 0xac) { return 1; }
    else if (ch >= 0xb1 && ch <= 0xb1) { return 1; }
    else if (ch >= 0xd7 && ch <= 0xd7) { return 1; }
    else if (ch >= 0xf7 && ch <= 0xf7) { return 1; }
    else if (ch >= 0x3f6 && ch <= 0x3f6) { return 1; }
    else if (ch >= 0x606 && ch <= 0x608) { return 1; }
    else if (ch >= 0x2044 && ch <= 0x2044) { return 1; }
    else if (ch >= 0x2052 && ch <= 0x2052) { return 1; }
    else if (ch >= 0x207a && ch <= 0x207c) { return 1; }
    else if (ch >= 0x208a && ch <= 0x208c) { return 1; }
    else if (ch >= 0x2118 && ch <= 0x2118) { return 1; }
    else if (ch >= 0x2140 && ch <= 0x2144) { return 1; }
    else if (ch >= 0x214b && ch <= 0x214b) { return 1; }
    else if (ch >= 0x2190 && ch <= 0x2194) { return 1; }
    else if (ch >= 0x219a && ch <= 0x219b) { return 1; }
    else if (ch >= 0x21a0 && ch <= 0x21a0) { return 1; }
    else if (ch >= 0x21a3 && ch <= 0x21a3) { return 1; }
    else if (ch >= 0x21a6 && ch <= 0x21a6) { return 1; }
    else if (ch >= 0x21ae && ch <= 0x21ae) { return 1; }
    else if (ch >= 0x21ce && ch <= 0x21cf) { return 1; }
    else if (ch >= 0x21d2 && ch <= 0x21d2) { return 1; }
    else if (ch >= 0x21d4 && ch <= 0x21d4) { return 1; }
    else if (ch >= 0x21f4 && ch <= 0x22ff) { return 1; }
    else if (ch >= 0x2308 && ch <= 0x230b) { return 1; }
    else if (ch >= 0x2320 && ch <= 0x2321) { return 1; }
    else if (ch >= 0x237c && ch <= 0x237c) { return 1; }
    else if (ch >= 0x239b && ch <= 0x23b3) { return 1; }
    else if (ch >= 0x23dc && ch <= 0x23e1) { return 1; }
    else if (ch >= 0x25b7 && ch <= 0x25b7) { return 1; }
    else if (ch >= 0x25c1 && ch <= 0x25c1) { return 1; }
    else if (ch >= 0x25f8 && ch <= 0x25ff) { return 1; }
    else if (ch >= 0x266f && ch <= 0x266f) { return 1; }
    else if (ch >= 0x27c0 && ch <= 0x27c4) { return 1; }
    else if (ch >= 0x27c7 && ch <= 0x27e5) { return 1; }
    else if (ch >= 0x27f0 && ch <= 0x27ff) { return 1; }
    else if (ch >= 0x2900 && ch <= 0x2982) { return 1; }
    else if (ch >= 0x2999 && ch <= 0x29d7) { return 1; }
    else if (ch >= 0x29dc && ch <= 0x29fb) { return 1; }
    else if (ch >= 0x29fe && ch <= 0x2aff) { return 1; }
    else if (ch >= 0x2b30 && ch <= 0x2b44) { return 1; }
    else if (ch >= 0x2b47 && ch <= 0x2b4c) { return 1; }
    else if (ch >= 0xfb29 && ch <= 0xfb29) { return 1; }
    else if (ch >= 0xfe62 && ch <= 0xfe62) { return 1; }
    else if (ch >= 0xfe64 && ch <= 0xfe66) { return 1; }
    else if (ch >= 0xff0b && ch <= 0xff0b) { return 1; }
    else if (ch >= 0xff1c && ch <= 0xff1e) { return 1; }
    else if (ch >= 0xff5c && ch <= 0xff5c) { return 1; }
    else if (ch >= 0xff5e && ch <= 0xff5e) { return 1; }
    else if (ch >= 0xffe2 && ch <= 0xffe2) { return 1; }
    else if (ch >= 0xffe9 && ch <= 0xffec) { return 1; }
    else if (ch >= 0x1d6c1 && ch <= 0x1d6c1) { return 1; }
    else if (ch >= 0x1d6db && ch <= 0x1d6db) { return 1; }
    else if (ch >= 0x1d6fb && ch <= 0x1d6fb) { return 1; }
    else if (ch >= 0x1d715 && ch <= 0x1d715) { return 1; }
    else if (ch >= 0x1d735 && ch <= 0x1d735) { return 1; }
    else if (ch >= 0x1d74f && ch <= 0x1d74f) { return 1; }
    else if (ch >= 0x1d76f && ch <= 0x1d76f) { return 1; }
    else if (ch >= 0x1d789 && ch <= 0x1d789) { return 1; }
    else if (ch >= 0x1d7a9 && ch <= 0x1d7a9) { return 1; }
    else if (ch >= 0x1d7c3 && ch <= 0x1d7c3) { return 1; }
    else { return 0; }
}
int
esch_unicode_is_range_so(esch_unicode ch)
{
    if (ch >= 0xa6 && ch <= 0xa6) { return 1; }
    else if (ch >= 0xa9 && ch <= 0xa9) { return 1; }
    else if (ch >= 0xae && ch <= 0xae) { return 1; }
    else if (ch >= 0xb0 && ch <= 0xb0) { return 1; }
    else if (ch >= 0x482 && ch <= 0x482) { return 1; }
    else if (ch >= 0x60e && ch <= 0x60f) { return 1; }
    else if (ch >= 0x6de && ch <= 0x6de) { return 1; }
    else if (ch >= 0x6e9 && ch <= 0x6e9) { return 1; }
    else if (ch >= 0x6fd && ch <= 0x6fe) { return 1; }
    else if (ch >= 0x7f6 && ch <= 0x7f6) { return 1; }
    else if (ch >= 0x9fa && ch <= 0x9fa) { return 1; }
    else if (ch >= 0xb70 && ch <= 0xb70) { return 1; }
    else if (ch >= 0xbf3 && ch <= 0xbf8) { return 1; }
    else if (ch >= 0xbfa && ch <= 0xbfa) { return 1; }
    else if (ch >= 0xc7f && ch <= 0xc7f) { return 1; }
    else if (ch >= 0xd79 && ch <= 0xd79) { return 1; }
    else if (ch >= 0xf01 && ch <= 0xf03) { return 1; }
    else if (ch >= 0xf13 && ch <= 0xf13) { return 1; }
    else if (ch >= 0xf15 && ch <= 0xf17) { return 1; }
    else if (ch >= 0xf1a && ch <= 0xf1f) { return 1; }
    else if (ch >= 0xf34 && ch <= 0xf34) { return 1; }
    else if (ch >= 0xf36 && ch <= 0xf36) { return 1; }
    else if (ch >= 0xf38 && ch <= 0xf38) { return 1; }
    else if (ch >= 0xfbe && ch <= 0xfc5) { return 1; }
    else if (ch >= 0xfc7 && ch <= 0xfcc) { return 1; }
    else if (ch >= 0xfce && ch <= 0xfcf) { return 1; }
    else if (ch >= 0xfd5 && ch <= 0xfd8) { return 1; }
    else if (ch >= 0x109e && ch <= 0x109f) { return 1; }
    else if (ch >= 0x1390 && ch <= 0x1399) { return 1; }
    else if (ch >= 0x1940 && ch <= 0x1940) { return 1; }
    else if (ch >= 0x19de && ch <= 0x19ff) { return 1; }
    else if (ch >= 0x1b61 && ch <= 0x1b6a) { return 1; }
    else if (ch >= 0x1b74 && ch <= 0x1b7c) { return 1; }
    else if (ch >= 0x2100 && ch <= 0x2101) { return 1; }
    else if (ch >= 0x2103 && ch <= 0x2106) { return 1; }
    else if (ch >= 0x2108 && ch <= 0x2109) { return 1; }
    else if (ch >= 0x2114 && ch <= 0x2114) { return 1; }
    else if (ch >= 0x2116 && ch <= 0x2117) { return 1; }
    else if (ch >= 0x211e && ch <= 0x2123) { return 1; }
    else if (ch >= 0x2125 && ch <= 0x2125) { return 1; }
    else if (ch >= 0x2127 && ch <= 0x2127) { return 1; }
    else if (ch >= 0x2129 && ch <= 0x2129) { return 1; }
    else if (ch >= 0x212e && ch <= 0x212e) { return 1; }
    else if (ch >= 0x213a && ch <= 0x213b) { return 1; }
    else if (ch >= 0x214a && ch <= 0x214a) { return 1; }
    else if (ch >= 0x214c && ch <= 0x214d) { return 1; }
    else if (ch >= 0x214f && ch <= 0x214f) { return 1; }
    else if (ch >= 0x2195 && ch <= 0x2199) { return 1; }
    else if (ch >= 0x219c && ch <= 0x219f) { return 1; }
    else if (ch >= 0x21a1 && ch <= 0x21a2) { return 1; }
    else if (ch >= 0x21a4 && ch <= 0x21a5) { return 1; }
    else if (ch >= 0x21a7 && ch <= 0x21ad) { return 1; }
    else if (ch >= 0x21af && ch <= 0x21cd) { return 1; }
    else if (ch >= 0x21d0 && ch <= 0x21d1) { return 1; }
    else if (ch >= 0x21d3 && ch <= 0x21d3) { return 1; }
    else if (ch >= 0x21d5 && ch <= 0x21f3) { return 1; }
    else if (ch >= 0x2300 && ch <= 0x2307) { return 1; }
    else if (ch >= 0x230c && ch <= 0x231f) { return 1; }
    else if (ch >= 0x2322 && ch <= 0x2328) { return 1; }
    else if (ch >= 0x232b && ch <= 0x237b) { return 1; }
    else if (ch >= 0x237d && ch <= 0x239a) { return 1; }
    else if (ch >= 0x23b4 && ch <= 0x23db) { return 1; }
    else if (ch >= 0x23e2 && ch <= 0x23f3) { return 1; }
    else if (ch >= 0x2400 && ch <= 0x2426) { return 1; }
    else if (ch >= 0x2440 && ch <= 0x244a) { return 1; }
    else if (ch >= 0x249c && ch <= 0x24e9) { return 1; }
    else if (ch >= 0x2500 && ch <= 0x25b6) { return 1; }
    else if (ch >= 0x25b8 && ch <= 0x25c0) { return 1; }
    else if (ch >= 0x25c2 && ch <= 0x25f7) { return 1; }
    else if (ch >= 0x2600 && ch <= 0x266e) { return 1; }
    else if (ch >= 0x2670 && ch <= 0x26ff) { return 1; }
    else if (ch >= 0x2701 && ch <= 0x2767) { return 1; }
    else if (ch >= 0x2794 && ch <= 0x27bf) { return 1; }
    else if (ch >= 0x2800 && ch <= 0x28ff) { return 1; }
    else if (ch >= 0x2b00 && ch <= 0x2b2f) { return 1; }
    else if (ch >= 0x2b45 && ch <= 0x2b46) { return 1; }
    else if (ch >= 0x2b50 && ch <= 0x2b59) { return 1; }
    else if (ch >= 0x2ce5 && ch <= 0x2cea) { return 1; }
    else if (ch >= 0x2e80 && ch <= 0x2e99) { return 1; }
    else if (ch >= 0x2e9b && ch <= 0x2ef3) { return 1; }
    else if (ch >= 0x2f00 && ch <= 0x2fd5) { return 1; }
    else if (ch >= 0x2ff0 && ch <= 0x2ffb) { return 1; }
    else if (ch >= 0x3004 && ch <= 0x3004) { return 1; }
    else if (ch >= 0x3012 && ch <= 0x3013) { return 1; }
    else if (ch >= 0x3020 && ch <= 0x3020) { return 1; }
    else if (ch >= 0x3036 && ch <= 0x3037) { return 1; }
    else if (ch >= 0x303e && ch <= 0x303f) { return 1; }
    else if (ch >= 0x3190 && ch <= 0x3191) { return 1; }
    else if (ch >= 0x3196 && ch <= 0x319f) { return 1; }
    else if (ch >= 0x31c0 && ch <= 0x31e3) { return 1; }
    else if (ch >= 0x3200 && ch <= 0x321e) { return 1; }
    else if (ch >= 0x322a && ch <= 0x3247) { return 1; }
    else if (ch >= 0x3250 && ch <= 0x3250) { return 1; }
    else if (ch >= 0x3260 && ch <= 0x327f) { return 1; }
    else if (ch >= 0x328a && ch <= 0x32b0) { return 1; }
    else if (ch >= 0x32c0 && ch <= 0x32fe) { return 1; }
    else if (ch >= 0x3300 && ch <= 0x33ff) { return 1; }
    else if (ch >= 0x4dc0 && ch <= 0x4dff) { return 1; }
    else if (ch >= 0xa490 && ch <= 0xa4c6) { return 1; }
    else if (ch >= 0xa828 && ch <= 0xa82b) { return 1; }
    else if (ch >= 0xa836 && ch <= 0xa837) { return 1; }
    else if (ch >= 0xa839 && ch <= 0xa839) { return 1; }
    else if (ch >= 0xaa77 && ch <= 0xaa79) { return 1; }
    else if (ch >= 0xfdfd && ch <= 0xfdfd) { return 1; }
    else if (ch >= 0xffe4 && ch <= 0xffe4) { return 1; }
    else if (ch >= 0xffe8 && ch <= 0xffe8) { return 1; }
    else if (ch >= 0xffed && ch <= 0xffee) { return 1; }
    else if (ch >= 0xfffc && ch <= 0xfffd) { return 1; }
    else if (ch >= 0x10137 && ch <= 0x1013f) { return 1; }
    else if (ch >= 0x10179 && ch <= 0x10189) { return 1; }
    else if (ch >= 0x10190 && ch <= 0x1019b) { return 1; }
    else if (ch >= 0x101d0 && ch <= 0x101fc) { return 1; }
    else if (ch >= 0x1d000 && ch <= 0x1d0f5) { return 1; }
    else if (ch >= 0x1d100 && ch <= 0x1d126) { return 1; }
    else if (ch >= 0x1d129 && ch <= 0x1d164) { return 1; }
    else if (ch >= 0x1d16a && ch <= 0x1d16c) { return 1; }
    else if (ch >= 0x1d183 && ch <= 0x1d184) { return 1; }
    else if (ch >= 0x1d18c && ch <= 0x1d1a9) { return 1; }
    else if (ch >= 0x1d1ae && ch <= 0x1d1dd) { return 1; }
    else if (ch >= 0x1d200 && ch <= 0x1d241) { return 1; }
    else if (ch >= 0x1d245 && ch <= 0x1d245) { return 1; }
    else if (ch >= 0x1d300 && ch <= 0x1d356) { return 1; }
    else if (ch >= 0x1f000 && ch <= 0x1f02b) { return 1; }
    else if (ch >= 0x1f030 && ch <= 0x1f093) { return 1; }
    else if (ch >= 0x1f0a0 && ch <= 0x1f0ae) { return 1; }
    else if (ch >= 0x1f0b1 && ch <= 0x1f0be) { return 1; }
    else if (ch >= 0x1f0c1 && ch <= 0x1f0cf) { return 1; }
    else if (ch >= 0x1f0d1 && ch <= 0x1f0df) { return 1; }
    else if (ch >= 0x1f110 && ch <= 0x1f12e) { return 1; }
    else if (ch >= 0x1f130 && ch <= 0x1f16b) { return 1; }
    else if (ch >= 0x1f170 && ch <= 0x1f19a) { return 1; }
    else if (ch >= 0x1f1e6 && ch <= 0x1f202) { return 1; }
    else if (ch >= 0x1f210 && ch <= 0x1f23a) { return 1; }
    else if (ch >= 0x1f240 && ch <= 0x1f248) { return 1; }
    else if (ch >= 0x1f250 && ch <= 0x1f251) { return 1; }
    else if (ch >= 0x1f300 && ch <= 0x1f320) { return 1; }
    else if (ch >= 0x1f330 && ch <= 0x1f335) { return 1; }
    else if (ch >= 0x1f337 && ch <= 0x1f37c) { return 1; }
    else if (ch >= 0x1f380 && ch <= 0x1f393) { return 1; }
    else if (ch >= 0x1f3a0 && ch <= 0x1f3c4) { return 1; }
    else if (ch >= 0x1f3c6 && ch <= 0x1f3ca) { return 1; }
    else if (ch >= 0x1f3e0 && ch <= 0x1f3f0) { return 1; }
    else if (ch >= 0x1f400 && ch <= 0x1f43e) { return 1; }
    else if (ch >= 0x1f440 && ch <= 0x1f440) { return 1; }
    else if (ch >= 0x1f442 && ch <= 0x1f4f7) { return 1; }
    else if (ch >= 0x1f4f9 && ch <= 0x1f4fc) { return 1; }
    else if (ch >= 0x1f500 && ch <= 0x1f53d) { return 1; }
    else if (ch >= 0x1f540 && ch <= 0x1f543) { return 1; }
    else if (ch >= 0x1f550 && ch <= 0x1f567) { return 1; }
    else if (ch >= 0x1f5fb && ch <= 0x1f640) { return 1; }
    else if (ch >= 0x1f645 && ch <= 0x1f64f) { return 1; }
    else if (ch >= 0x1f680 && ch <= 0x1f6c5) { return 1; }
    else { return 0; }
}
