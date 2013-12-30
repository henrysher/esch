#include "esch_gc.h"
#include <assert.h>
esch_error
esch_gc_new_mark_sweep(esch_config* config, esch_gc** gc)
{
    (void)config;
    (void)gc;
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_gc_perform_gc(esch_gc* gc, esch_alloc* alloc)
{
    (void)gc;
    (void)alloc;
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_gc_register(esch_gc* gc, esch_object* obj)
{
    (void)gc;
    (void)obj;
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_gc_retain(esch_object* obj)
{
    (void)obj;
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_gc_release(esch_object* obj)
{
    (void)obj;
    return ESCH_ERROR_NOT_IMPLEMENTED;
}

