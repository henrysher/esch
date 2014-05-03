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

/*
 * Data structure and algorithm.
 *
 * A typical mark-and-sweep algorithm requires two three data structures:
 * - Object management table, represented by slots array,
 * - Reachability seach tree, represented by root container,
 * - In-use table, represented by inuse_flags array.
 *
 * When a system starts with esch_gc to manage its object system, the
 * objects registered to esch_gc does not need to be explicitly deleted.
 * Instead, the objects must be registered to GC system with attach()
 * operation. It basically takes three steps:
 *
 * 1. When an object is registered, esch_gc allocates a pointer
 *    in `slots' and one bit in `inuse_flags'. The root object also
 *    takes a slot.
 * 2. The esch_object::gc field points to esch_gc object.
 * 3. The esch_object::gc_id field points to the offset of bit in
 *    inuse_flags.
 *
 * When a GC action is triggered, the action contains three steps:
 *
 * 1. Mark all objects as deletable.
 * 2. Visit all objects from reachability search tree, and mark every
 *    reachable object as in-use.
 * 3. Destruct all deletable objects.
 *
 * Step 1 is simple. Just memset() inuse_flags table.
 *
 * Step 2 starts from `root' object, and traverses every child object
 * with iterator. If the child node itself is a container again, the
 * children of child node is visited again. To make sure the traversal
 * takes predictable memory/stack usage, the `recycle_stack' array is
 * introduced to perform a non-recursive traversal. This is required
 * because GC recycling happens when memory is not enough, we should
 * avoid allocating memory at this time.
 *
 * Step 3 starts after step 2. It visits every bit of inuse_flags, and
 * delete the corresponding objects in `slots' array, if it's not marked
 * as in-use. After the object is deleted, the slot it takes is returned
 * to linked list head by `available_slot_offset'.
 *
 * Data structure used in steps:
 *
 * The reachability search tree starts from root. The root object is
 * usually a container type (list, vector, or anything, but I use vector
 * for most of times).
 *
 * The `slots' array is not full all the time. To manage the unallocated
 * slots without resizing/moving, the available slots are constructed as
 * an linked list, refereneced by its index. The `available_slot_offset'
 * field is introduced, indicate the beginning of available slots. The
 * value of `available_slot_offset' always starts from last element in
 * `slots' array.
 *
 * NOTE: the first element in `slots' array is always root.
 *
 */
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
