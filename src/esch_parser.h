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
    ESCH_TOKEN_UNKNOWN        = 0,   /**< Unknown identifier (something wrong) */
    ESCH_TOKEN_IDENTIFIER     = 1,   /**< Identifier */
    ESCH_TOKEN_NUMBER,               /**< Number */
    ESCH_TOKEN_STRING,               /**< String */
    ESCH_TOKEN_SYMBOL,               /**< Symbol */
    ESCH_TOKEN_LEFT_BRACKET,         /**< Left bracket, "(" */
    ESCH_TOKEN_RIGHT_BRACKET,        /**< Right bracket, ")" */
    ESCH_TOKEN_KEYWORD_DEFINE,       /**< Keyword: define */
    ESCH_TOKEN_KEYWORD_IF,           /**< Keyword: if     */
    ESCH_TOKEN_KEYWORD_LET,          /**< Keyword: let    */
    ESCH_TOKEN_KEYWORD_LET_S,        /**< Keyword: let*   */
    ESCH_TOKEN_KEYWORD_SET_B,        /**< Keyword: set!   */
    ESCH_TOKEN_KEYWORD_BEGIN,        /**< Keyword: begin  */
    ESCH_TOKEN_KEYWORD_LAMBDA,       /**< Keyword: lambda */
    ESCH_TOKEN_KEYWORD_TRUE,         /**< Keyword: #t     */
    ESCH_TOKEN_KEYWORD_FALSE,        /**< Keyword: #f     */
    ESCH_TOKEN_KEYWORD_PACKAGE,      /**< Keyword: package */
    ESCH_TOKEN_KEYWORD_IMPORT,       /**< Keyword: import */
    ESCH_TOKEN_KEYWORD_EXPORT,       /**< Keyword: export */
    ESCH_TOKEN_KEYWORD_DEFAULT_SYNTAX,  /**< Keyword: define-syntax */
    ESCH_TOKEN_KEYWORD_CASE,         /**< Keyword: case */
};

struct esch_ast
{
    esch_object base;
    enum esch_token_type token;
    char* value;

    char* file_name;
    int   line_number;

    struct esch_ast* first_child;
    struct esch_ast* next_sibling;
};

struct esch_parser
{
    esch_object base;
};

#define ESCH_IS_VALID_PARSER(obj) \
    ((obj)->base.type == ESCH_TYPE_PARSER && \
     (obj)->base.alloc != NULL && \
     (obj)->base.log != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_PARSER_H_ */

