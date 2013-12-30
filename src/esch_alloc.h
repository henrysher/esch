/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_ALLOC_H_
#define _ESCH_ALLOC_H_
#include "esch.h"
#include "esch_object.h"
#include "esch_type.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_alloc
{
    esch_alloc_realloc_f realloc;
    esch_alloc_free_f free;
};
struct esch_alloc_c_default
{
    esch_alloc base;
    int allocate_count; /**< How many buffer are allocated. */
    int deallocate_count; /**< How many buffer are freed. */
};
typedef struct esch_alloc_c_default esch_alloc_c_default;

struct esch_builtin_type esch_alloc_c_default_type;

#define ESCH_IS_VALID_ALLOC(alloc) \
    (ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(alloc)) && \
     (alloc->realloc != NULL) && \
     (alloc->free != NULL))

#define ESCH_IS_VALID_C_DEFAULT_ALLOC(alloc) \
    (ESCH_IS_VALID_ALLOC(((esch_alloc*)(alloc)))           && \
     (ESCH_OBJECT_GET_TYPE(ESCH_CAST_TO_OBJECT(alloc)) \
               == &(esch_alloc_c_default_type.type))        && \
     ESCH_OBJECT_GET_ALLOC(ESCH_CAST_TO_OBJECT(alloc)) \
               == ((esch_alloc*)(alloc)))

esch_error
esch_alloc_realloc_i(esch_alloc* alloc, void* in, size_t size, void** out);

esch_error
esch_alloc_free_i(esch_alloc* alloc, void* ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_ALLOC_H_ */

