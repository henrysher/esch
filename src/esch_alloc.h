/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_ALLOC_H_
#define _ESCH_ALLOC_H_
#include "esch.h"
#include "esch_object.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_alloc
{
    ESCH_COMMON_HEADER;
    int allocate_count; /**< How many buffer are allocated. */
    int deallocate_count; /**< How many buffer are freed. */
};

#define ESCH_IS_VALID_ALLOC(obj) \
    (ESCH_IS_VALID_OBJECT(obj) && \
     (ESCH_GET_TYPE(obj) == ESCH_TYPE_ALLOC_C_DEFAULT) && \
     ESCH_GET_ALLOC(obj) == (obj))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_ALLOC_H_ */

