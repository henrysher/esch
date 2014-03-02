/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_STRING_H_
#define _ESCH_STRING_H_

#include "esch.h"
#include "esch_object.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_string
{
    esch_utf8* utf8;
    size_t utf8_len;
    esch_unicode* unicode;
    size_t unicode_len;
};

#define ESCH_IS_VALID_STRING(str) \
    ((str) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(str)) && \
     (str)->utf8 != NULL && \
     (str)->unicode != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_STRING_H_ */
