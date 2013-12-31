/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch_log.h"
#include "esch_alloc.h"
#include "esch_debug.h"
#include <stdio.h>
#include <assert.h>

/* ================================================================= */
/*             Definitions for esch_log_do_nothing                   */
/* ================================================================= */

static esch_error esch_log_destructor_s(esch_object* obj);
static esch_error esch_log_new_do_nothing_s(esch_config* config,
                                            esch_object** obj);
static esch_error esch_log_new_printf_s(esch_config* config,
                                        esch_object** obj);
static esch_error esch_log_error_printf(esch_log* log,
                                const char* fmt, va_list args);
static esch_error esch_log_info_printf(esch_log* log,
                                const char* fmt, va_list args);
static esch_error esch_log_message_do_nothing(esch_log* log,
                                const char* fmt, va_list args);

struct esch_builtin_type esch_log_do_nothing_type = 
{
    {
        &(esch_log_do_nothing_type.type),
        NULL, /* No alloc */
        &(esch_log_do_nothing.log),
        NULL,
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_log),
        esch_log_new_do_nothing_s,
        esch_log_destructor_s,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    },
};
struct esch_builtin_type esch_log_printf_type = 
{
    {
        &(esch_log_printf_type.type),
        NULL, /* No Alloc */
        &(esch_log_do_nothing.log),
        NULL,
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_log),
        esch_log_new_printf_s,
        esch_log_destructor_s,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    },
};


struct esch_log_builtin_static esch_log_do_nothing =
{
    {
        &(esch_log_do_nothing_type.type),
        NULL,
        &(esch_log_do_nothing.log),
        NULL, /* No GC */
        NULL,
    },
    {
        esch_log_message_do_nothing,
        esch_log_message_do_nothing
    },
};
static struct esch_log_builtin_static esch_log_printf =
{
    {
        &(esch_log_printf_type.type),
        NULL,
        &(esch_log_printf.log),
        NULL, /* No GC */
        NULL,
    },
    {
        esch_log_error_printf,
        esch_log_info_printf
    },
};
/*
 * By default, the global log uses do-nothing. In debug mode, we print
 * everything to standard output.
 */
#ifdef NDEBUG
esch_log* esch_global_log = &(esch_log_do_nothing.log);
#else
esch_log* esch_global_log = &(esch_log_printf.log);
#endif

/**
 * Create a log that does nothing.
 * @param config Config object to passed shared configuration.
 * @param log Returned created log object.
 * @return Error code.
 */
esch_error
esch_log_new_do_nothing(esch_config* config, esch_log** log)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);
    (void)config; /* Do-nothing log takes nothing. :) */

    (*log) = &(esch_log_do_nothing.log);
    assert(ESCH_IS_VALID_LOG(*log));
Exit:
    return ret;
}

/**
 * Create a log that print string to use printf().
 * @param config Config object to passed shared configuration.
 * @param log Returned created log object.
 * @return Error code.
 */
esch_error
esch_log_new_printf(esch_config* config, esch_log** log)
{
    esch_error ret = ESCH_OK;
    esch_log* new_log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);

    (void)config; /* Do-nothing log takes nothing. :) */

    (*log) = &(esch_log_printf.log);
    assert(ESCH_IS_VALID_LOG(*log));
Exit:
    return ret;
}

/**
 * Public function to log error.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @return Error code;
 */
esch_error
esch_log_error(esch_log* log, const char* fmt, ...)
{
    esch_error ret = ESCH_OK;
    va_list ap;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(fmt != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_LOG(log), ESCH_ERROR_INVALID_PARAMETER);

    va_start(ap, fmt);
    ret = log->log_error(log, fmt, ap);
    va_end(ap);
Exit:
    return ret;
}

/**
 * Public function to log general information.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @return Error code;
 */
esch_error
esch_log_info(esch_log* log, const char* fmt, ...)
{
    esch_error ret = ESCH_OK;
    va_list ap;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(fmt != NULL, ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_LOG(log), ESCH_ERROR_INVALID_PARAMETER);

    va_start(ap, fmt);
    ret = log->log_info(log, fmt, ap);
    va_end(ap);
Exit:
    return ret;
}

/**
 * Delete a log object.
 * @param log The log object.
 * @return Error code;
 */
static esch_error
esch_log_destructor_s(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    if (obj == NULL) {
        return ret;
    }
    if ((void*)obj == (void*)&esch_log_do_nothing || 
            (void*)obj == (void*)&esch_log_printf)
    {
        /* Just do nothing */
    }
    else
    {
        /* The logic will change when we expose log object to public. */
        ret = ESCH_ERROR_NOT_SUPPORTED;
    }
Exit:
    return ret;
}

static esch_error
esch_log_new_do_nothing_s(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    ret = esch_log_new_do_nothing(config, &log);
    if (ret == ESCH_OK)
    {
        (*obj) = ESCH_CAST_TO_OBJECT(log);
    }
    return ret;
}

static esch_error
esch_log_new_printf_s(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    ret = esch_log_new_printf(config, &log);
    if (ret == ESCH_OK)
    {
        (*obj) = ESCH_CAST_TO_OBJECT(log);
    }
    return ret;
}

/**
 * Function to log error. Belongs to do-printf log.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @param args Variable length of arguments.
 * @return Error code;
 */
static esch_error
esch_log_error_printf(esch_log* log, const char* fmt, va_list args)
{
    (void)log;
    printf("[ERROR] ");
    vprintf(fmt, args);
    printf("\n");
    return ESCH_OK;
}

/**
 * Function to log error. Belongs to do-printf log.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @param args Variable length of arguments.
 * @return Error code;
 */
static esch_error
esch_log_info_printf(esch_log* log, const char* fmt, va_list args)
{
    (void)log;
    printf("[INFO]  ");
    vprintf(fmt, args);
    printf("\n");
    return ESCH_OK;
}


/**
 * Function to log any information. Belongs to do-nothing log.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @param args Variable length of arguments.
 * @return Error code;
 */
static esch_error
esch_log_message_do_nothing(esch_log* log, const char* fmt, va_list args)
{
    (void)log;
    (void)fmt;
    (void)args;
    return ESCH_OK;
}
