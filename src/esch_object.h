/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_OBJECT_H_
#define _ESCH_OBJECT_H_
#include "esch.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* --- Common object --- */
/**
 * The public type info structure. This is the header of all structures
 * used in Esch. All objects can be casted to esch_object to get type
 * information.
 */
struct esch_object
{
    unsigned char version;  /**< Object version */
    esch_type   type;       /**< Registered type ID */
    esch_alloc* alloc;      /**< Allocator object to manage memory. */
    esch_log*   log;        /**< Log object to write trace/errors.*/
};

#define ESCH_COMMON_HEADER    esch_object header

#define ESCH_GET_CONFIG(obj)         ((esch_object*)obj)
#define ESCH_GET_VERSION(obj)        (((esch_object*)obj)->version)
#define ESCH_GET_TYPE(obj)           (((esch_object*)obj)->type)
#define ESCH_GET_LOG(obj)            (((esch_object*)obj)->log)
#define ESCH_GET_ALLOC(obj)          (((esch_object*)obj)->alloc)

#define ESCH_IS_VALID_OBJECT(obj) \
    (ESCH_GET_VERSION(obj) == ESCH_VERSION && \
     (ESCH_GET_TYPE(obj) & ESCH_TYPE_MAGIC) == ESCH_TYPE_MAGIC && \
     ESCH_GET_LOG(obj) != NULL && \
     ESCH_GET_ALLOC(obj) != NULL)

#define ESCH_IS_NO_DELETE(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_NO_DELETE) == ESCH_TYPE_NO_DELETE)
#define ESCH_IS_PRIMITIVE(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_PRIMITIVE) == ESCH_TYPE_PRIMITIVE)
#define ESCH_IS_CONTAINER(obj) \
    ((ESCH_GET_TYPE(obj) & ESCH_TYPE_CONTAINER) == ESCH_TYPE_CONTAINER)

esch_error esch_object_delete(esch_object* data, int delete_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_OBJECT_H_ */
