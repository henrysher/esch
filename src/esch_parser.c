/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch_parser.h"
#include "esch_alloc.h"
#include "esch_log.h"
#include "esch_debug.h"
#include "esch_config.h"

/**
 * Create a new esch_parser object.
 * @param alloc  Alloc object used by parser object.
 * @param log    Log object used by parser object.
 * @param parser Returned parameter of newly created parser object.
 * @return Error code.
 */
esch_error
esch_parser_new(esch_config* config, esch_parser** parser)
{
    esch_error ret = ESCH_OK;
    esch_parser* new_parser = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(parser != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_INTERNAL_CONFIG_GET_ALLOC(config) != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_INTERNAL_CONFIG_GET_LOG(config) != NULL);

    alloc = ESCH_INTERNAL_CONFIG_GET_ALLOC(config);
    log = ESCH_INTERNAL_CONFIG_GET_LOG(config);

    ret = esch_alloc_malloc(alloc, sizeof(esch_parser), (void**)&new_parser);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log, "Can't malloc for parser", ret);

    ESCH_GET_VERSION(new_parser) = ESCH_VERSION;
    ESCH_GET_TYPE(new_parser) = ESCH_TYPE_PARSER;
    ESCH_GET_LOG(new_parser) = log;
    ESCH_GET_ALLOC(new_parser) = alloc;
    (*parser) = new_parser;
    new_parser = NULL;
    assert(ESCH_IS_VALID_PARSER(*parser));
Exit:
    if (new_parser != NULL)
    {
        (void)esch_parser_delete(new_parser);
    }
    return ret;
}

/**
 * Delete esch_parser_config object.
 * @param parser Given parser object.
 * @return Error code.
 */
esch_error
esch_parser_delete(esch_parser* parser)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    ESCH_CHECK_PARAM_PUBLIC(parser != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_PARSER(parser));

    alloc = ESCH_GET_ALLOC(parser);
    esch_alloc_free(alloc, parser);
Exit:
    return ret;
}

/* --- Helper functions --- */
