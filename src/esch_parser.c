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
esch_parser_new(esch_alloc* alloc, esch_log* log, esch_parser** parser)
{
    esch_error ret = ESCH_OK;
    esch_parser* new_parser = NULL;
    ESCH_CHECK_PARAM_PUBLIC(parser != NULL);
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);

    ret = esch_alloc_malloc(alloc, sizeof(esch_parser), (void**)&new_parser);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log, "Can't malloc for parser", ret);

    new_parser->base.type = ESCH_TYPE_PARSER;
    new_parser->base.log = log;
    new_parser->base.alloc = alloc;
    (*parser) = new_parser;
    new_parser = NULL;
    assert(ESCH_IS_VALID_OBJECT(*parser));
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
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(parser));

    alloc = parser->base.alloc;
    esch_alloc_free(alloc, parser);
Exit:
    return ret;
}

/**
 * Parse given Scheme code line
 * @param parser Given parser object.
 * @param input  Input line of source code.
 * @return Error code
 */
esch_error
esch_parser_read_line(esch_parser* parser, char* input)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(parser != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(parser));
    ESCH_CHECK_PARAM_PUBLIC(input != NULL);
Exit:
    return ret;
}

