/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_GC_H_
#define _ESCH_GC_H_
#include "esch.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*
 * This file defines a common interface GC module, which is also
 * referenced by esch_object.
 */
typedef esch_error (*esch_gc_perform_gc_f)(esch_gc*, esch_alloc*);
typedef esch_error (*esch_gc_register_f)(esch_gc*, esch_object* obj);
typedef esch_error (*esch_gc_retain_f)(esch_gc*, esch_object*);
typedef esch_error (*esch_gc_release_f)(esch_gc*, esch_object*);

struct esch_gc
{
    esch_gc_perform_gc_f gc_perform_gc;
    esch_gc_register_f   gc_register;
    esch_gc_retain_f     gc_retain;
    esch_gc_release_f    gc_release;
};

union esch_gc_mark_sweep_tracking_slots
{
    esch_object* obj;
    esch_object** next;
};

struct esch_gc_mark_sweep
{
    esch_gc base;
    union esch_gc_mark_sweep_tracking_slots** tracking_slot;
    size_t slot_blocks;
    esch_object** first_available_slot;
    unsigned char* mark_table;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_GC_H_ */
