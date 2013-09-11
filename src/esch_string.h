/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_STRING_H_
#define _ESCH_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_string
{
    esch_object base;
    esch_utf8_char* utf8;
    esch_unicode* unicode;
};

#define ESCH_IS_VALID_STRING(obj) \
    ((obj)->base.type == ESCH_TYPE_STRING && \
     ESCH_IS_VALID_OBJECT(obj))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_STRING_H_ */