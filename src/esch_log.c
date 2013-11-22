/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include <stdio.h>
#include <assert.h>
#include "esch_alloc.h"
#include "esch_log.h"
#include "esch_debug.h"

esch_alloc esch_dummy_alloc = {
    {
        ESCH_VERSION,
        ESCH_TYPE_ALLOC_DUMMY,
        &esch_dummy_alloc,
        &esch_log_do_nothing
    },
};
esch_log esch_log_printf = {
    {
        ESCH_VERSION,
        ESCH_TYPE_LOG_PRINTF,
        &esch_dummy_alloc,
        &esch_log_printf
    },
    esch_log_error_printf,
    esch_log_info_printf
};
esch_log esch_log_do_nothing = {
    {
        ESCH_VERSION,
        ESCH_TYPE_LOG_DO_NOTHING,
        &esch_dummy_alloc,
        &esch_log_do_nothing
    },
    esch_log_message_do_nothing,
    esch_log_message_do_nothing
};

/* By default, the global log uses printf(). */
esch_log* esch_global_log = &esch_log_printf;

/**
 * Create a log that print string to use printf().
 * @param log Returned created log object.
 * @return Error code.
 */
esch_error
esch_log_new_printf(esch_log** log)
{
    esch_error ret = ESCH_OK;
    esch_log* new_obj = NULL;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);

    (*log) = &esch_log_printf;
    assert(ESCH_IS_VALID_OBJECT(*log));
Exit:
    return ret;
}

/**
 * Create a log that does nothing.
 * @param log Returned created log object.
 * @return Error code.
 */
esch_error
esch_log_new_do_nothing(esch_log** log)
{
    esch_error ret = ESCH_OK;
    esch_log* new_obj = NULL;

    ESCH_CHECK_NO_LOG(log != NULL, ESCH_ERROR_INVALID_PARAMETER);

    (*log) = &esch_log_do_nothing;
    assert(ESCH_IS_VALID_OBJECT(*log));
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
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_OBJECT(log), ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(log == &esch_log_do_nothing ||
                      log == &esch_log_printf,
                     ESCH_ERROR_INVALID_PARAMETER);

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
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_OBJECT(log), ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(log == &esch_log_do_nothing ||
                      log == &esch_log_printf,
                     ESCH_ERROR_INVALID_PARAMETER);

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
esch_error
esch_log_delete(esch_log* log)
{
    esch_error ret = ESCH_OK;
    if (log == NULL) {
        return ret;
    }
    ESCH_CHECK_NO_LOG(ESCH_IS_VALID_OBJECT(log), ESCH_ERROR_INVALID_PARAMETER);
    ESCH_CHECK_NO_LOG(log == &esch_log_do_nothing ||
                      log == &esch_log_printf,
                     ESCH_ERROR_INVALID_PARAMETER);
    /* Just do nothing */
Exit:
    return ret;
}

/**
 * Function to log error. Belongs to do-printf log.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @param args Variable length of arguments.
 * @return Error code;
 */
esch_error
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
esch_error
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
esch_error
esch_log_message_do_nothing(esch_log* log, const char* fmt, va_list args)
{
    (void)log;
    (void)fmt;
    (void)args;
    return ESCH_OK;
}

