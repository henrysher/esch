#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "esch_config.h"
#include "esch_debug.h"
#include "esch_number.h"

static int
esch_integer_convert_char(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    else if (ch >= 'a' && ch <= 'f')
    {
        return 10 + (ch - 'a');
    }
    else if (ch >= 'A' && ch <= 'F')
    {
        return 10 + (ch - 'A');
    }
    assert(0); /* It will ot happen */
    return 0;
}

#ifdef NDEBUG

#define ESCH_INTEGER_VERIFY(begin, end) (void)(0)

#else

static void
esch_integer_verify(const char* begin, const char* end)
{
    const char* iter = begin;
    for(; iter != end; ++iter)
    {
        assert((*iter) >= '0' && (*iter) <= '9');
    }
}
#define ESCH_INTEGER_VERIFY(begin, end) esch_integer_verify(begin, end)

#endif /* NDEBUG */

/**
 * Create a new integer object. Internal use only.
 * @param config Configuration object.
 * @param begin An integer number, represented in string.
 * @param end End of string. Give NULL means check until '\0'.
 * @param val Created integer value.
 * @return Return code. ESCH_OK if success.
 */
esch_error
esch_integer_new_from_base10(esch_config* config,
                             const char* begin,
                             const char* end,
                             esch_integer** val)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    int negative = ESCH_FALSE;
    int big = ESCH_FALSE;
    unsigned int intval = 0;
    unsigned int nextval = 0;
    const char* iter = NULL;
    unsigned char* digit = NULL;
    esch_integer* newval = NULL;

    ESCH_CHECK_PARAM_INTERNAL(config != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_CONFIG(config));
    ESCH_CHECK_PARAM_INTERNAL(begin != NULL);
    ESCH_CHECK_PARAM_INTERNAL(val != NULL);

    alloc = ESCH_INTERNAL_CONFIG_GET_ALLOC(config);
    log = ESCH_INTERNAL_CONFIG_GET_LOG(config);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
    ESCH_CHECK_PARAM_INTERNAL(log != NULL);

    if ((*begin) == '-' || (*begin) == '+')
    {
        negative = (((*begin) == '-')? ESCH_TRUE: ESCH_FALSE);
        ++begin;
    }
    if (end == NULL)
    {
        for (end = begin; (*end) != '\0'; ++end);
    }

    /*
     * NOTE: We don't validate the format of str in release mode. The
     * work will be done with Esch intepreter.
     */
    ESCH_INTEGER_VERIFY(begin, end);

    ret = esch_alloc_malloc(alloc, sizeof(esch_integer), (void**)&newval);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc int object", ret);
    ESCH_GET_VERSION(newval) = ESCH_VERSION;
    ESCH_GET_TYPE(newval) = ESCH_TYPE_INTEGER;
    ESCH_GET_ALLOC(newval) = alloc;
    ESCH_GET_LOG(newval) = log;
    newval->negative = negative;
    newval->big = ESCH_FALSE;
    newval->value.ival = 0;
    /* If the integer is not big enough, keep it a regular integer. */
    iter = begin;
    do
    {
        nextval = (((intval << 3) + (intval << 1))) + 
                      esch_integer_convert_char(*iter);
        if (nextval < intval)
        {
            /* Overloaded, expand buffer. */
            big = ESCH_TRUE;
        }
        else
        {
            /* Not overload */
            intval = nextval;
            ++iter;
        }
    } while (!big && iter != end);
    /* a big integer */
    if (big)
    {
        size_t buflen = 0;
        size_t i = 0;
        unsigned int lowbit = 0;
        unsigned int highbit = 0;
        esch_log_info(log, "Big integer detected: addr: 0x%x", begin);
        buflen = ((end - begin) % 2 == 0?
                      (end - begin) / 2:
                      ((end - begin) / 2 + 1));
        ret = esch_alloc_malloc(alloc, buflen, (void**)&digit);
        memset(digit, 0, buflen);
        ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc bigint", ret);
        for (i = 0, iter = end; iter != begin; --iter, ++i)
        {
            if ((i & 1) == 0)
            {
                lowbit = esch_integer_convert_char((*(iter - 1)));
                highbit = 0;
            }
            else
            {
                highbit = esch_integer_convert_char((*(iter - 1)));
                digit[(i >> 1)] = lowbit + (highbit << 4);
            }
        }
        if (highbit == 0)
        {
            digit[(i >> 1)] = lowbit;
        }
        newval->big = ESCH_TRUE;
        newval->value.bval.length = buflen;
        newval->value.bval.digit = digit;
        digit = NULL;
    }
    else
    {
        newval->value.ival = intval;
    }
    (*val) = newval;
    newval = NULL;
Exit:
    if (newval != NULL)
    {
        (void)esch_integer_delete(newval);
    }
    if (digit != NULL)
    {
        assert(alloc != NULL);
        (void)esch_alloc_free(alloc, digit);
    }
    return ret;
}
esch_error
esch_integer_new_from_int(esch_config* config,
                          int intval, esch_integer** val)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}

/**
 * Delete integer object. Internal only.
 * @param val Given integer object.
 * @return Return code. ESCH_OK if success.
 */
esch_error
esch_integer_delete(esch_integer* val)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    if (val == NULL)
    {
        return ret;
    }
    ESCH_CHECK_PARAM_INTERNAL(val != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_INTEGER(val));
    alloc = ESCH_GET_ALLOC(val);
    log = ESCH_GET_LOG(val);
    ESCH_CHECK_PARAM_INTERNAL(alloc != NULL);
    ESCH_CHECK_PARAM_INTERNAL(log != NULL);

    if (val->big)
    {
        (void)esch_alloc_free(alloc, val->value.bval.digit);
    }
    esch_alloc_free(alloc, val);
Exit:
    return ret;
}

esch_error
esch_integer_add(esch_integer* val1, esch_integer* val2, esch_integer** val)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_integer_sub(esch_integer* val1, esch_integer* val2, esch_integer** val)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_integer_mul(esch_integer* val1, esch_integer* val2, esch_integer** val)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_integer_div(esch_integer* val1, esch_integer* val2, esch_fraction* val)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}

