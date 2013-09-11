/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch.h"
#include "esch_string.h"
#include "esch_log.h"
#include "esch_debug.h"
#include <string.h>

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

    ret = esch_alloc_malloc(alloc, sizeof(esch_string), (void**)new_str);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc for string", ret);

    ret = esch_alloc_malloc(alloc, sizeof(char) * (len + 1), (void**)&new_utf8);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc UTF-8", ret);
    strncpy(new_utf8, (utf8 + begin), len);

    /* TODO Also convert Unicode */

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
esch_error esch_string_new(esch_config* config, esch_string* input,
                           int begin, int end, esch_string** str);
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

