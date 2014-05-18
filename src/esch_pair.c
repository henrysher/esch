#include "esch_pair.h"
#include "esch_debug.h"
#include "esch_type.h"
#include "esch_config.h"

#define HEAD(pa) ((pa)->values[0])
#define TAIL(pa) ((pa)->values[1])
#define HEAD_ID 0
#define TAIL_ID 1
#define EMPTY_ID 2

esch_value esch_pair_empty = { ESCH_VALUE_TYPE_END, 0 };

static esch_error
esch_pair_new_default_as_object_i(esch_config* config, esch_object** obj);
static esch_error
esch_pair_destructor_i(esch_object* pair);
static esch_error
esch_pair_get_iterator_i(esch_object* obj, esch_iterator* iter);
static esch_error
esch_pair_iterator_get_value_i(esch_iterator* iter, esch_value* value);
static esch_error
esch_pair_iterator_get_next_i(esch_iterator* iter);

struct esch_builtin_type esch_pair_type =
{
    {
        &(esch_meta_type.type),
        NULL, /* No alloc */
        &(esch_log_do_nothing.log),
        NULL, /* Non-GC object */
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_pair),
        esch_pair_new_default_as_object_i,
        esch_pair_destructor_i,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form, /* TODO We should have one */
        esch_type_default_no_doc,
        esch_pair_get_iterator_i,
    }
};

static esch_error
esch_pair_new_default_as_object_i(esch_config* config, esch_object** obj)
{
    /* Just as we know, there's no meaningful semantic to define a
     * "default" pair. */
    return ESCH_ERROR_NOT_SUPPORTED;
}
static esch_error
esch_pair_destructor_i(esch_object* pair)
{
    /* Just do nothing. */
    return ESCH_OK;
}

esch_error
esch_pair_new_empty(esch_config* config, esch_pair** pair)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(pair != NULL);

    ret = esch_pair_new_i(config,
                          &esch_pair_empty,
                          &esch_pair_empty,
                          pair);
Exit:
    return ret;
}
esch_error
esch_pair_new(esch_config* config,
              esch_value* head, esch_value* tail,
              esch_pair** pair)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(pair != NULL);
    ESCH_CHECK_PARAM_PUBLIC(head != NULL);
    ESCH_CHECK_PARAM_PUBLIC(tail != NULL);
    ESCH_CHECK_PARAM_PUBLIC((head == &esch_pair_empty) ||
                            (head->type > ESCH_VALUE_TYPE_UNKNOWN && 
                             head->type < ESCH_VALUE_TYPE_END));
    ESCH_CHECK_PARAM_PUBLIC((tail == &esch_pair_empty) ||
                            (tail->type > ESCH_VALUE_TYPE_UNKNOWN && 
                             tail->type < ESCH_VALUE_TYPE_END));
    ESCH_CHECK_PARAM_PUBLIC(ESCH_CONFIG_GET_ALLOC(config) != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_CONFIG_GET_LOG(config) != NULL);

    ret = esch_pair_new_i(config, head, tail, pair);
Exit:
    return ret;
}

esch_error
esch_pair_new_i(esch_config* config,
                esch_value* head, esch_value* tail,
                esch_pair** pair)
{
    esch_error ret = ESCH_OK;
    esch_object* new_obj = NULL;
    esch_pair* new_pair = NULL;
    esch_log* log = NULL;

    ESCH_CHECK_PARAM_INTERNAL(config != NULL);
    ESCH_CHECK_PARAM_INTERNAL(pair != NULL);
    ESCH_CHECK_PARAM_INTERNAL(head != NULL);
    ESCH_CHECK_PARAM_INTERNAL(tail != NULL);
    /* Do not allow user code use ESCH_VALUE_TYPE_END */
    ESCH_CHECK_PARAM_INTERNAL((head == &esch_pair_empty) ||
                              (head->type > ESCH_VALUE_TYPE_UNKNOWN && 
                               head->type < ESCH_VALUE_TYPE_END));
    ESCH_CHECK_PARAM_INTERNAL((tail == &esch_pair_empty) ||
                              (tail->type > ESCH_VALUE_TYPE_UNKNOWN && 
                               tail->type < ESCH_VALUE_TYPE_END));
    ESCH_CHECK_PARAM_INTERNAL(ESCH_CONFIG_GET_LOG(config) != NULL);

    log = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_LOG(config), esch_log);

    ret = esch_object_new_i(config, &(esch_pair_type.type), &new_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "pair:new:Can't create object", ret);
    new_pair = ESCH_CAST_FROM_OBJECT(new_obj, esch_pair);

    HEAD(new_pair).type = head->type;
    HEAD(new_pair).val = head->val;
    TAIL(new_pair).type = tail->type;
    TAIL(new_pair).val = tail->val;
    if (tail->type == ESCH_VALUE_TYPE_OBJECT &&
            ESCH_OBJECT_GET_TYPE(tail->val.o) == &(esch_pair_type.type))
    {
        new_pair->next_is_pair = 1;
    } else {
        new_pair->next_is_pair = 0;
    }
    (*pair) = new_pair;
    new_pair = NULL;
Exit:
    if (new_pair != NULL) {
        (void)esch_object_delete(new_obj);
    }
    return ret;
}

/* ===================================================================
 * Iterator functions
 * We use dispatch table to avoid if-else type check, which could
 * help reducing runtime cost.
 * =================================================================== */

static esch_value*
esch_pair_get_value_by_id(esch_iterator* iter, size_t idx)
{
    esch_pair* pair = ESCH_CAST_FROM_OBJECT(iter->container, esch_pair);
    ESCH_CHECK_PARAM_INTERNAL(idx >= HEAD_ID && idx < EMPTY_ID);
    return &(pair->values[idx]);
}
static esch_value*
esch_pair_get_end_value(esch_iterator* iter, size_t idx)
{
    return &esch_pair_empty;
}

typedef esch_value* (*esch_pair_value_f)(esch_iterator*, size_t);
 
static esch_pair_value_f esch_pair_value_dispatch[3] = {
    esch_pair_get_value_by_id, /* current: head */
    esch_pair_get_value_by_id, /* current: tail */
    esch_pair_get_end_value, /* current: empty */
};

static esch_error
esch_pair_iterator_get_value_i(esch_iterator* iter, esch_value* value)
{
    size_t idx = 0;
    esch_value* retval = NULL;
    ESCH_CHECK_PARAM_INTERNAL(iter != NULL);
    ESCH_CHECK_PARAM_INTERNAL(value != NULL);

    idx = (size_t)(iter->iterator);

    retval = esch_pair_value_dispatch[idx](iter, idx);
    value->type = retval->type;
    value->val = retval->val;
    return ESCH_OK;
}

typedef void (*esch_pair_object_check_f)(esch_iterator*, esch_pair*);

static void
esch_pair_assign_current_tail(esch_iterator* iter, esch_pair* pair)
{
    iter->iterator = (void*)TAIL_ID;
}
static void
esch_pair_assign_next_head(esch_iterator* iter, esch_pair* pair)
{
    size_t idx = (size_t)iter->iterator;
    ESCH_CHECK_PARAM_INTERNAL(iter != NULL);
    ESCH_CHECK_PARAM_INTERNAL(pair != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_PAIR(pair));
    iter->container = TAIL(pair).val.o; /* Switch to next head pair */
    iter->iterator = (void*)HEAD_ID;
}
static void
esch_pair_assign_empty_id(esch_iterator* iter, esch_pair* pair)
{
    iter->iterator = (void*)EMPTY_ID;
}

static esch_pair_object_check_f esch_pair_next_pair_dispatch[2] = {
    esch_pair_assign_current_tail, /* next is not pair: get tail */
    esch_pair_assign_next_head /* next is pair: get head of next */
};

static void
esch_pair_assign_next_or_object(esch_iterator* iter, esch_pair* pair)
{
    ESCH_CHECK_PARAM_INTERNAL(iter != NULL);
    ESCH_CHECK_PARAM_INTERNAL(pair != NULL);
    esch_pair_next_pair_dispatch[pair->next_is_pair](iter, pair);
}

static esch_pair_object_check_f esch_pair_assign_next[3] = {
    esch_pair_assign_next_or_object, /* Current: head; next: tail */
    esch_pair_assign_empty_id, /* Current: tail; next: empty */
    esch_pair_assign_empty_id, /* Current: empty; next: empty */
};

static esch_error
esch_pair_iterator_get_next_i(esch_iterator* iter)
{
    esch_error ret = ESCH_OK;
    esch_pair* current_pair = NULL;
    size_t idx = 0;
    ESCH_CHECK_PARAM_INTERNAL(iter != NULL);
    ESCH_CHECK_PARAM_INTERNAL(iter->container != NULL);

    current_pair = ESCH_CAST_FROM_OBJECT(iter->container, esch_pair);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_PAIR(current_pair));
    idx = (size_t)(iter->iterator);

    /* Get next object */
    esch_pair_assign_next[idx](iter, current_pair);
    /* Equivalent if-check style code
    if (val->type == ESCH_VALUE_TYPE_OBJECT) {
        ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_OBJECT(val->val.o));
        if (ESCH_OBJECT_GET_TYPE(val->val.o) == &(esch_pair_type.type)) {
            next_pair = ESCH_CAST_FROM_OBJECT(val->val.o, esch_pair);
            iter->container = val->val.o;
            iter->iterator = (void*)HAED_ID;
            goto Exit;
        }
    }
    if (val == &HEAD(current_pair)) {
        iter->iterator = (void*)TAIL_ID;
    } else {
        iter->iterator = (void*)EMPTY_ID;
    }
    */
Exit:
    return ret;
}

esch_error
esch_pair_get_iterator_i(esch_object* obj, esch_iterator* iter)
{
    esch_error ret = ESCH_OK;
    esch_pair* pair = NULL;
    ESCH_CHECK_PARAM_INTERNAL(obj != NULL);
    ESCH_CHECK_PARAM_INTERNAL(iter != NULL);
    ESCH_CHECK_PARAM_INTERNAL(ESCH_IS_VALID_PAIR(
                ESCH_CAST_FROM_OBJECT(obj, esch_pair)));

    iter->container = obj;
    iter->iterator = (void*)HEAD_ID;
    iter->get_value = esch_pair_iterator_get_value_i;
    iter->get_next = esch_pair_iterator_get_next_i;
Exit:
    return ret;
}
