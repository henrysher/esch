/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_ALLOC_H_
#define _ESCH_ALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_alloc
{
    esch_object base;
    int allocate_count; /**< How many buffer are allocated. */
    int deallocate_count; /**< How many buffer are freed. */
};

#define ESCH_IS_VALID_ALLOC(obj) \
    ((((obj)->base.type & ESCH_TYPE_ALLOC) == ESCH_TYPE_ALLOC) && \
     (obj)->base.alloc != NULL && \
     (obj)->base.log != NULL && \
     (obj)->base.alloc == obj)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_ALLOC_H_ */

