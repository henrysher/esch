/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_VECTOR_H_
#define _ESCH_VECTOR_H_
#include <stdlib.h>
#include "esch.h"
#include "esch_object.h"
#include "esch_type.h"
#include "esch_alloc.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_vector
{
    esch_bool enlarge;
    size_t slots;
    esch_object** begin;
    esch_object** next; /* Next available slot */
};

#define ESCH_IS_VALID_VECTOR(vec) \
    ((vec) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(vec)) && \
     (ESCH_OBJECT_GET_TYPE(ESCH_CAST_TO_OBJECT(vec)) == \
      &(esch_vector_type.type)) \
     )

extern struct esch_builtin_type esch_vector_type;
extern const size_t ESCH_VECTOR_MINIMAL_INITIAL_LENGTH;
extern const size_t ESCH_VECTOR_MAX_LENGTH;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_VECTOR_H_ */
