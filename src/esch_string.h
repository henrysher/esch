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
    ESCH_COMMON_HEADER;
    esch_utf8_char* utf8;
    esch_unicode* unicode;
    size_t utf8_len;
    size_t unicode_len;
};

#define ESCH_IS_VALID_STRING(obj) \
    (ESCH_GET_TYPE(obj) == ESCH_TYPE_STRING && \
     ESCH_IS_VALID_OBJECT(obj) && \
     (obj)->utf8 != NULL && \
     (obj)->unicode != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_STRING_H_ */
