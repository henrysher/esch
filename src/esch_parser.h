/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_PARSER_H_
#define _ESCH_PARSER_H_

#include "esch.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum esch_token_type
{
    ESCH_TOKEN_UNKNOWN        = 0,       /**< Unknown identifier */
    ESCH_TOKEN_IDENTIFIER     = 1,       /**< Identifier */
    ESCH_TOKEN_STRING,                   /**< String */
    ESCH_TOKEN_SYMBOL,                   /**< Symbol */
    ESCH_TOKEN_LEFT_BRACKET,             /**< Left bracket: ( */
    ESCH_TOKEN_RIGHT_BRACKET,            /**< Right bracket: ) */
    ESCH_TOKEN_LEFT_SQUARE_BRACKET,      /**< Left bracket: [ */
    ESCH_TOKEN_RIGHT_SQUARE_BRACKET,     /**< Right bracket: ] */
};

/*
 * We don't need to set a separated lexical analyzer for Scheme, as the
 * syntax is... well, almost no syntax. In all cases, it should be able
 * for us to determine a lexical element by looking ahead of at most
 * three characters.
 */
struct esch_parser
{
    ESCH_COMMON_HEADER
};

#define ESCH_IS_VALID_PARSER(obj) \
    (ESCH_IS_VALID_OBJECT(obj) && \
     ESCH_GET_TYPE(obj) == ESCH_TYPE_PARSER && \
     ESCH_GET_ALLOC(obj) != NULL && \
     ESCH_GET_LOG(obj) != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_PARSER_H_ */

