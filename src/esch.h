/* vim:ft=c expandtab tw=72 sw=4
 */
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
#ifndef _ESCH_ESCH_H_
#define _ESCH_ESCH_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * This is the only public include file for Esch interpreter. 
 */

enum esch_err
{
    ESCH_ERROR_OK = 0,
};
struct esch_vm;
struct esch_parser;
struct esch_ast;
struct esch_vm_config;
struct esch_parser_config;

/* --- Esch virtual machine and runtime --- */
struct esch_vm* esch_vm_new(struct esch_vm_config* config);
void            esch_vm_delete(struct esch_vm* vm);
enum esch_err esch_vm_execute(struct esch_vm* vm, struct esch_ast* ast);

/* --- Syntax parser --- */
struct esch_parser* esch_parser_new(struct esch_parser_config* config);
void                esch_parser_delete(struct esch_parser* parser);
struct esch_ast*    esch_parser_parse(struct esch_parser* parser, char* src);

/* --- Abstract syntax tree --- */
void esch_ast_delete(struct esch_ast* ast);
enum esch_err esch_ast_get_error_status(struct esch_ast* ast);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_ESCH_H_ */
