/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_VECTOR_H_
#define _ESCH_VECTOR_H_
#include <stdlib.h>
#include "esch.h"
#include "esch_object.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_vector
{
    ESCH_COMMON_HEADER;
    size_t slots;
    esch_object** begin;
    esch_object** end;
    esch_object** next;
    esch_type element_type;
    unsigned char delete_element : 1;
};

#define ESCH_IS_VALID_VECTOR(obj) \
    (ESCH_IS_VALID_OBJECT(obj) && \
     (ESCH_GET_TYPE(obj) == ESCH_TYPE_VECTOR))

const size_t ESCH_VECTOR_MINIMAL_INITIAL_LENGTH;
const size_t ESCH_VECTOR_MAX_LENGTH;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_VECTOR_H_ */
