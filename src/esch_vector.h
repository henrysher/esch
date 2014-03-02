/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_VECTOR_H_
#define _ESCH_VECTOR_H_
#include <stdlib.h>
#include "esch.h"
#include "esch_object.h"
#include "esch_type.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_vector
{
    size_t slots;
    esch_object** begin;
    esch_object** end;
    esch_object** next;
};

struct esch_builtin_type esch_vector_type;

#define ESCH_IS_VALID_VECTOR(vec) \
    ((vec) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(vec)) && \
     (ESCH_OBJECT_GET_TYPE(ESCH_CAST_TO_OBJECT(vect)) == \
          esch_vector_type.type) \
     )

const size_t ESCH_VECTOR_MINIMAL_INITIAL_LENGTH;
const size_t ESCH_VECTOR_MAX_LENGTH;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_VECTOR_H_ */
