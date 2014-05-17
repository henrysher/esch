/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch.h"
#include "esch_value.h"
#include "esch_debug.h"

static void
esch_value_do_nothing(esch_value* to, esch_value* from)
{
    ESCH_CHECK_PARAM_INTERNAL(to != NULL);
    ESCH_CHECK_PARAM_INTERNAL(from != NULL);
}
static void
esch_value_do_assign(esch_value* to, esch_value* from)
{
    ESCH_CHECK_PARAM_INTERNAL(to != NULL);
    ESCH_CHECK_PARAM_INTERNAL(from != NULL);
    ESCH_CHECK_PARAM_INTERNAL(from->type > ESCH_VALUE_TYPE_UNKNOWN);
    ESCH_CHECK_PARAM_INTERNAL(from->type < ESCH_VALUE_TYPE_END);

    to->type = from->type;
    to->val = from->val;
}

#define ESCH_VALUE_DEFINE_GET(suffix, vt, ot, field) \
void \
esch_value_get_##suffix(void* data, esch_value* value) \
{ \
    ESCH_CHECK_PARAM_INTERNAL(data != NULL); \
    ESCH_CHECK_PARAM_INTERNAL(value != NULL); \
    (*((ot*)data)) = value->val. field; \
}

#define ESCH_VALUE_DEFINE_SET(suffix, vt, ot, field) \
void \
esch_value_set_##suffix(esch_value* value, void* data) \
{ \
    ESCH_CHECK_PARAM_INTERNAL(data != NULL); \
    ESCH_CHECK_PARAM_INTERNAL(value != NULL); \
    value->type = vt;\
    value->val. field = (*((ot*)data)); \
}

ESCH_VALUE_DEFINE_GET(object, ESCH_VALUE_TYPE_OBJECT, esch_object*, o);
ESCH_VALUE_DEFINE_GET(integer, ESCH_VALUE_TYPE_INTEGER, int, i);
ESCH_VALUE_DEFINE_GET(float, ESCH_VALUE_TYPE_FLOAT, double, f);
ESCH_VALUE_DEFINE_GET(byte, ESCH_VALUE_TYPE_BYTE, esch_byte, b);
ESCH_VALUE_DEFINE_GET(unicode, ESCH_VALUE_TYPE_UNICODE, esch_unicode, u);

ESCH_VALUE_DEFINE_SET(integer, ESCH_VALUE_TYPE_INTEGER, int, i);
ESCH_VALUE_DEFINE_SET(float, ESCH_VALUE_TYPE_FLOAT, double, f);
ESCH_VALUE_DEFINE_SET(byte, ESCH_VALUE_TYPE_BYTE, esch_byte, b);
ESCH_VALUE_DEFINE_SET(unicode, ESCH_VALUE_TYPE_UNICODE, esch_unicode, u);
ESCH_VALUE_DEFINE_SET(object, ESCH_VALUE_TYPE_OBJECT, esch_object*, o);

static esch_error
esch_value_fail(esch_value* value)
{
    (void)value;
    return ESCH_ERROR_BAD_VALUE_TYPE;
}
static esch_error
esch_value_check_nothing(esch_value* value)
{
    (void)value;
    return ESCH_OK;
}
static esch_error
esch_value_check_object(esch_value* value)
{
    return (value->val.o != NULL? ESCH_OK: ESCH_ERROR_INVALID_PARAMETER);
}

esch_value_assign_f esch_value_assign[2] = {
    esch_value_do_assign, /* status: ESCH_OK */
    esch_value_do_nothing, /* status: ESCH_ERROR_BAD_VALUE_TYPE */
};

esch_value_check_f esch_value_check[8] = {
    esch_value_fail, /* expect: ESCH_VALUE_TYPE_UNKNOWN */
    esch_value_check_nothing, /* expect: ESCH_VALUE_TYPE_BYTE */
    esch_value_check_nothing, /* expect: ESCH_VALUE_TYPE_UNICODE */
    esch_value_check_nothing, /* expect: ESCH_VALUE_TYPE_INTEGER */
    esch_value_check_nothing, /* expect: ESCH_VALUE_TYPE_FLOAT */
    esch_value_check_object, /* expect: ESCH_VALUE_TYPE_OBJECT */
    esch_value_check_nothing, /* expect: ESCH_VALUE_TYPE_NIL */
    esch_value_fail, /* expect: ESCH_VALUE_TYPE_END */
};

esch_error esch_value_type_check[8][8] = {
    /* expect: ESCH_VALUE_TYPE_UNKNOWN */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_BYTE */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_OK,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_UNICODE */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_OK,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_INTEGER */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_OK,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_FLOAT */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_OK,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_OBJECT */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_OK,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_NIL */ {
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_ERROR_BAD_VALUE_TYPE,
        ESCH_OK,
        ESCH_ERROR_BAD_VALUE_TYPE,
    },
    /* expect: ESCH_VALUE_TYPE_END */ {
        ESCH_OK,
        ESCH_OK,
        ESCH_OK,
        ESCH_OK,
        ESCH_OK,
        ESCH_OK,
        ESCH_OK, /* Always ignore type check if user passes END */
    }
};

