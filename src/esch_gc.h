/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_GC_H_
#define _ESCH_GC_H_
#include "esch_object.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*
 * This file defines a common interface GC module, which is also
 * referenced by esch_object.
 *
 * So far there's only one gc supported - a naive mark-and-sweep GC. The
 * esch_gc is exposed for future extension, but no inheritance exists in
 * current code.
 *
 * I decied NOT to support reference count to simplify implementation.
 */

/* Attach object to GC system. */
typedef esch_error (*esch_gc_attach_f)(esch_gc*, esch_object*);
/* Recycle objects for GC. */
typedef esch_error (*esch_gc_recycle_f)(esch_gc*);

/* Internal function for esch_object. */
esch_error esch_gc_attach_i(esch_gc* gc, esch_object* obj);
esch_error esch_gc_recycle_i(esch_gc* gc);

union esch_object_or_next
{
    esch_object* obj;
    size_t next;
};

struct esch_gc
{
    esch_gc_attach_f     attach;
    esch_gc_recycle_f    recycle;

    unsigned char* inuse_flags;
    union esch_object_or_next* slots;
    esch_object** recycle_stack;
    esch_object*  root;
    size_t available_slot_offset;
    size_t slot_count;
};

extern const int ESCH_GC_NAIVE_DEFAULT_SLOTS;

#define ESCH_IS_VALID_GC(gc) \
    ((gc) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(gc)) && \
     ESCH_CAST_TO_OBJECT(gc)->gc == NULL && \
     (gc)->attach != NULL && \
     (gc)->recycle != NULL && \
     (gc)->inuse_flags != NULL && \
     (gc)->slots != NULL && \
     (gc)->recycle_stack != NULL && \
     (gc)->available_slot_offset > 0 && \
     (gc)->available_slot_offset < (gc)->slot_count)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_GC_H_ */
