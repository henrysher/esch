/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#include "esch.h"
#include "esch_parser.h"
#include "esch_alloc.h"
#include "esch_log.h"
#include "esch_debug.h"

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
    ESCH_CHECK_PARAM_PUBLIC(config->alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config->log != NULL);

    alloc = config->alloc;
    log = config->log;

    ret = esch_alloc_malloc(alloc, sizeof(esch_parser), (void**)&new_parser);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log, "Can't malloc for parser", ret);

    new_parser->base.type = ESCH_TYPE_PARSER;
    new_parser->base.log = log;
    new_parser->base.alloc = alloc;
    (*parser) = new_parser;
    new_parser = NULL;
    assert(ESCH_IS_PARSER(*parser));
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
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_PARSER(parser));

    alloc = parser->base.alloc;
    esch_alloc_free(alloc, parser);
Exit:
    return ret;
}

/* --- Helper functions --- */
