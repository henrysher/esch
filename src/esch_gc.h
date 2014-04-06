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
/* Detach object to GC system. */
typedef esch_error (*esch_gc_detach_f)(esch_gc*, esch_object*);
/* Recycle objects for GC. */
typedef esch_error (*esch_gc_recycle_f)(esch_gc*);

/* Internal function for esch_object. */
esch_error esch_gc_attach_i(esch_gc* gc, esch_object* obj);
esch_error esch_gc_detach_i(esch_gc* gc, esch_object* obj);
esch_error esch_gc_recycle_i(esch_gc* gc);

struct esch_gc_cell
{
    unsigned char* inuse_flags;
    esch_object** objects;
    esch_object* available_object_slot;
    int object_count;
};

struct esch_gc
{
    esch_gc_attach_f     attach;
    esch_gc_detach_f     detach;
    esch_gc_recycle_f    recycle;
    struct esch_gc_cell  cell;
};

extern const int ESCH_GC_NAIVE_DEFAULT_CELLS;

#define ESCH_IS_VALID_GC(gc) \
    ((gc) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(gc)) && \
     (gc)->attach != NULL && \
     (gc)->detach != NULL && \
     (gc)->recycle != NULL && \
     (gc)->cell.inuse_flags != NULL && \
     (gc)->cell.objects != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_GC_H_ */
