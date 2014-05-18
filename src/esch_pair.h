/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_PAIR_H_
#define _ESCH_PAIR_H_

#include "esch.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_pair
{
    esch_byte next_is_pair; /* Bit won't work with clang */
    esch_value values[2];
};

extern esch_error
esch_pair_new_i(esch_config* config,
                esch_value* head, esch_value* tail,
                esch_pair** pair);
extern struct esch_builtin_type esch_pair_type;

#define ESCH_IS_VALID_PAIR(pa) \
    ((pa) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(pa)) && \
     ESCH_OBJECT_GET_TYPE(ESCH_CAST_TO_OBJECT(pa)) == \
       &(esch_pair_type.type))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_PAIR_H_ */
