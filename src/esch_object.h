/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_OBJECT_H_
#define _ESCH_OBJECT_H_
#include "esch.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Common header for all objects defined in esch system.
 */
struct esch_object
{
    esch_type*      type;        /**< Registered type ID */
    esch_alloc*     alloc;       /**< Allocator object to manage memory.*/
    esch_log*       log;         /**< Log object to write trace/errors.*/
    esch_gc*        gc;          /**< GC object to control lifetime. */
    void*           gc_id;       /**< Assigned ID from GC system */
};

/*
struct esch_alloc
{
    ESCH_COMMON_HEADER;
    esch_alloc_realloc_f alloc_realloc;
    esch_alloc_free_f    alloc_free;
};
*/
/*
 * Rules for type system.
 * 1. User can customize either primitive or container type.
 * 2. When defining container type, it must define iterator.
 * 3. All types must function when either registered to GC or not.
 * 4. The common function, esch_object_destroy() is the only public
 *    interface.
 * 5. The esch_object_*() functions takes care of all functions.
 * 6. All managed objects must satrt from ESCH_COMMON_HEADER.
 * 7. The lifetime of passed in objects must be longer than object:
 *    type, aloc, log, gc, gc_id.
 */

#define ESCH_OBJECT_GET_CONFIG(obj)         ((obj)->config)
#define ESCH_OBJECT_GET_TYPE(obj)           ((obj)->type)
#define ESCH_OBJECT_GET_LOG(obj)            ((obj)->log)
#define ESCH_OBJECT_GET_ALLOC(obj)          ((obj)->alloc)
#define ESCH_OBJECT_GET_GC(obj)             ((obj)->gc)
#define ESCH_OBJECT_GET_GC_ID(obj)          ((obj)->gc_id)

#define ESCH_CAST_TO_OBJECT(data) \
    ((esch_object*)((char*)data - sizeof(esch_object)))
#define ESCH_CAST_FROM_OBJECT(obj, name) \
    ((name*)((char*)obj + sizeof(esch_object)))

/* 
 * For some global static objects, it's possible that it comes with no
 * alloc object. So we don't check obj->alloc.
 */
#define ESCH_IS_VALID_OBJECT(obj) \
    ((ESCH_OBJECT_GET_TYPE(obj) != NULL) && \
     (ESCH_TYPE_GET_VERSION(ESCH_OBJECT_GET_TYPE(obj)) == ESCH_VERSION) && \
     (ESCH_OBJECT_GET_LOG(obj) != NULL))

esch_error esch_object_new_i(esch_config* config, esch_type* type,
                             esch_object** obj);
esch_error esch_object_delete_i(esch_object* obj, esch_bool force_delete);
esch_error esch_object_get_iterator_i(esch_object* obj, esch_iterator* iter);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_OBJECT_H_ */
