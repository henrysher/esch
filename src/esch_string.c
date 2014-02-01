/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch_string.h"
#include "esch_log.h"
#include "esch_debug.h"
#include "esch_config.h"
#include "esch_alloc.h"
#include <string.h>

static esch_error
esch_string_default_new(esch_config* config, esch_object** obj);
static esch_error
esch_string_destructor(esch_object* obj);
static esch_error
esch_string_copy_object(esch_object* input, esch_object** output);
static esch_error
esch_string_copy_string(esch_object* input, esch_string** output);
static int
utf8_get_unicode_len(char* utf8, int begin, int end, int* bad_index);
static void
write_unicode_from_utf8(char* utf8, int begin, int end,
                        esch_unicode* unicode);
static esch_error
decode_utf8(char* utf8, int begin, int end, 
            esch_config* config, esch_unicode** str, size_t* unicode_len);

struct esch_builtin_type esch_string_type = 
{
    {
        &(esch_string_type.type),
        NULL, /* No alloc */
        &(esch_log_do_nothing.log),
        NULL, /* Non-GC object */
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_string),
        esch_string_default_new,
        esch_string_destructor,
        esch_string_copy_object, /* String copy */
        esch_string_copy_string, /* String.toString() */
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    },
};

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

/*
 * ---------------------------------------------------------------
 * Public interfaces
 * ---------------------------------------------------------------
 */
esch_error
esch_string_new_from_utf8(esch_config* config, const char* utf8,
                          int begin, int end, esch_string** str)
{
    esch_error ret = ESCH_OK;
    esch_object* new_obj = NULL;
    esch_string* new_str = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_utf8* new_utf8 = NULL;
    esch_unicode* new_unicode = NULL;
    size_t len = 0;
    size_t unicode_len = 0;
    ESCH_CHECK_PARAM_PUBLIC(utf8 != NULL);
    ESCH_CHECK_PARAM_PUBLIC(str != NULL);

    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    alloc_obj = ESCH_CONFIG_GET_ALLOC(config);
    ESCH_CHECK_PARAM_PUBLIC(log_obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(alloc_obj != NULL);

    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);
    ESCH_CHECK_PARAM_INTERNAL(log != NULL);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);

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

    ret = esch_alloc_realloc(alloc, NULL, sizeof(char) * (len + 1),
                             (void**)&new_utf8);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc UTF-8", ret);
    (void)strncpy(new_utf8, (utf8 + begin), len);

    ret = decode_utf8(new_utf8, 0, len, config, &new_unicode, &unicode_len);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't decode UTF-8", ret);

    ret = esch_object_new_i(config, &(esch_string_type.type), &new_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't create string object", ret);
    new_str = ESCH_CAST_FROM_OBJECT(new_obj, esch_string);

    new_str->utf8 = new_utf8;
    new_str->unicode = new_unicode;
    new_str->utf8_len = len;
    new_str->unicode_len = unicode_len;
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_STRING(new_str));
    (*str) = new_str;

    new_str = NULL;
    new_utf8 = NULL;
    new_unicode = NULL;
    new_obj = NULL;
Exit:
    esch_alloc_free(alloc, new_utf8);
    esch_alloc_free(alloc, new_unicode);
    esch_alloc_free(alloc, new_str);
    if (new_obj != NULL)
    {
        esch_object_delete(new_obj);
    }
    return ret;
}

char*
esch_string_get_utf8_ref(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    ESCH_CHECK_PARAM_INTERNAL(str != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_STRING(str));
    return (str == NULL? NULL: str->utf8);
}

esch_unicode*
esch_string_get_unicode_ref(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    ESCH_CHECK_PARAM_INTERNAL(str != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_STRING(str));
    return (str == NULL? NULL: str->unicode);
}

size_t
esch_string_get_utf8_length(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    ESCH_CHECK_PARAM_INTERNAL(str != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_STRING(str));
    return (str == NULL? 0: str->utf8_len);
}

size_t
esch_string_get_unicode_length(esch_string* str)
{
    /* NOTE: For performance consideration, we don't check input. */
    ESCH_CHECK_PARAM_INTERNAL(str != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_STRING(str));
    return (str == NULL? 0: str->unicode_len);
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

esch_error
esch_string_duplicate(esch_string* input, esch_string** output)
{
    esch_error ret = ESCH_OK;
    esch_string* new_str = NULL;
    esch_object* input_obj = NULL;
    esch_config* config = NULL;
    esch_log* log = NULL;
    esch_alloc* alloc = NULL;
    esch_gc* gc = NULL;

    ESCH_CHECK_PARAM_PUBLIC(input != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(input));
    input_obj = ESCH_CAST_TO_OBJECT(input);

    log = ESCH_OBJECT_GET_LOG(input_obj);
    ESCH_CHECK_PARAM_INTERNAL(log != NULL);
    alloc = ESCH_OBJECT_GET_ALLOC(input_obj);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
    gc = ESCH_OBJECT_GET_GC(input_obj); /* May or may not be NULL */

    ret = esch_config_new(log, alloc, &config);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't alloc config", ret);

    if (gc != NULL) {
        ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                                  ESCH_CAST_TO_OBJECT(gc));
        ESCH_CHECK(ret == ESCH_OK, log, "Can't set GC", ret);
    }
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC,
                              ESCH_CAST_TO_OBJECT(alloc));
    ESCH_CHECK(ret == ESCH_OK, log, "Can't set alloc", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG,
                              ESCH_CAST_TO_OBJECT(log));
    ESCH_CHECK(ret == ESCH_OK, log, "Can't set log", ret);

    ret = esch_string_new_from_utf8(config, input->utf8, 0, -1, &new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't copy string", ret);

    (*output) = new_str;
    new_str = NULL;
Exit:
    if (new_str != NULL)
    {
        (void)esch_object_delete(ESCH_CAST_TO_OBJECT(new_str));
    }
    if (config != NULL)
    {
        (void)esch_object_delete(ESCH_CAST_TO_OBJECT(config));
    }
    return ret;
}

/*
 * -----------------------------------------------------------------
 * Internal functions.
 * -----------------------------------------------------------------
 */
static esch_error
esch_string_destructor(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_string* str = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;

    if (obj == NULL)
    {
        return ret;
    }

    str = ESCH_CAST_FROM_OBJECT(obj, esch_string);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(str));
    log = ESCH_OBJECT_GET_LOG(obj);
    alloc = ESCH_OBJECT_GET_ALLOC(obj);
    ESCH_CHECK_PARAM_INTERNAL(log != NULL);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
    ret = esch_alloc_free(alloc, str->utf8);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't free UTF-8 string", ret);
    ret = esch_alloc_free(alloc, str->unicode);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't free Unicode string", ret);

    str->utf8 = NULL;
    str->unicode = NULL;
    str->utf8_len = 0;
    str->unicode_len = 0;
Exit:
    return ret;
}

static esch_error
esch_string_default_new(esch_config* config, esch_object** obj)
{
    /*
     * Note: I set it to return NOT_SUPPORTED because esch defines
     * string as constant value after it's created.
     * This is NOT a bug or unimplemeted feature. Just not supported.
     */
    (void)config;
    (void)obj;
    return ESCH_ERROR_NOT_SUPPORTED;
}

static esch_error
esch_string_copy_object(esch_object* input, esch_object** output)
{
    esch_error ret = ESCH_OK;
    esch_string* str = NULL;
    esch_string* out_str = NULL;
    ESCH_CHECK_PARAM_PUBLIC(input != NULL);
    ESCH_CHECK_PARAM_PUBLIC(output != NULL);

    str = ESCH_CAST_FROM_OBJECT(input, esch_string);
    ESCH_CHECK_PARAM_PUBLIC(str != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(str));
    ret = esch_string_duplicate(str, &out_str);
    if (out_str != NULL)
    {
        /* All error handlings should have been handled within
         * esch_string_duplicate() */
        (*output) = ESCH_CAST_TO_OBJECT(out_str);
    }
Exit:
    return ret;
}
static esch_error
esch_string_copy_string(esch_object* input, esch_string** output)
{
    esch_error ret = ESCH_OK;
    esch_string* str = NULL;
    esch_string* out_str = NULL;
    ESCH_CHECK_PARAM_PUBLIC(input != NULL);
    ESCH_CHECK_PARAM_PUBLIC(output != NULL);

    str = ESCH_CAST_FROM_OBJECT(input, esch_string);
    ESCH_CHECK_PARAM_PUBLIC(str != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_STRING(str));
    ret = esch_string_duplicate(str, &out_str);
    if (out_str != NULL)
    {
        /* All error handlings should have been handled within
         * esch_string_duplicate() */
        (*output) = out_str;
    }
Exit:
    return ret;
}

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
                    ESCH_CHECK_PARAM_INTERNAL(0);
                    state = ESCH_WRONG_BYTE;
                    break;
            }
        }
    }
    ESCH_CHECK_PARAM_INTERNAL(state != ESCH_WRONG_BYTE);
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

    alloc = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_ALLOC(config), esch_alloc);
    log = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_LOG(config), esch_log);

    len = utf8_get_unicode_len(utf8, begin, end, &bad_index);
    ESCH_CHECK_1(len >= 0, log, "Bad Unicode at index %d", bad_index,
                 ESCH_ERROR_INVALID_PARAMETER);

    buf_size = sizeof(esch_unicode) * (len + 1);
    ret = esch_alloc_realloc(alloc, NULL, buf_size, (void**)&new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc new buffer", ret);
    write_unicode_from_utf8(utf8, begin, end, new_str);
    (*str) = new_str;
    (*unicode_len) = (size_t)len;
    new_str = NULL;
Exit:
    esch_alloc_free(alloc, new_str);
    return ret;
}
