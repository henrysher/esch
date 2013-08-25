/* vim:ft=c expandtab tw=72 sw=4
 */
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_PARSER_H_ */
/*
 * +=================================================================+
 *
 * From: http://opensource.org/licenses/BSD-3-Clause 
 *
 * Copyright (c) 2013, Fuzhou Chen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Fuzhou Chen nor other names of the contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * +=================================================================+
 */

