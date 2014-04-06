#include "esch_gc.h"
#include "esch_type.h"
#include "esch_object.h"
#include "esch_config.h"
#include "esch_log.h"
#include "esch_alloc.h"
#include "esch_debug.h"
#include <assert.h>

const int ESCH_GC_NAIVE_DEFAULT_CELLS = 4096;
static esch_error
esch_gc_destructor_i(esch_object* obj);
static esch_error
esch_gc_naive_mark_sweep_attach_i(esch_gc* gc, esch_object* obj);
static esch_error
esch_gc_naive_mark_sweep_recycle_i(esch_gc* gc);
static esch_error
esch_gc_new_naive_mark_sweep_i(esch_config* config, esch_object** obj);

struct esch_builtin_type esch_gc_type = 
{
    /* meta type */
    {
        &(esch_meta_type.type),
        NULL, /* No alloc */
        &(esch_log_do_nothing.log),
        NULL, /* No GC */
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_gc),
        esch_gc_new_naive_mark_sweep_i,
        esch_gc_destructor_i,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    }
};

static esch_error
esch_gc_destructor_i(esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_gc* gc = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    int i = 0;

    ESCH_ASSERT(obj != NULL);
    gc = ESCH_CAST_FROM_OBJECT(obj, esch_gc);
    ESCH_ASSERT(ESCH_IS_VALID_GC(gc));

    alloc = obj->alloc;
    log = obj->log;
    ESCH_ASSERT(alloc != NULL && ESCH_IS_VALID_ALLOC(alloc));
    ESCH_ASSERT(log != NULL && ESCH_IS_VALID_LOG(log));


    /* We don't do much on GC deleting. Just validate that all objects
     * are deleted. If not, there must be something wrong. */
    for (i = 0; i < gc->cell.object_count; ++i)
    {
        if (gc->cell.objects[i] != NULL)
        {
            esch_log_error(log, "GC:destructor: non-freed obj: %x", 
                    (gc->cell.objects[i]));
        }
    }
    (void)esch_alloc_free(alloc, gc->cell.objects);
    (void)esch_alloc_free(alloc, gc->cell.inuse_flags);
    /* Note: Don't destroy itself. Will be handled by esch_object */
Exit:
    return ret;
}

static esch_error
esch_gc_naive_mark_sweep_attach_i(esch_gc* gc, esch_object* obj)
{
    esch_error ret = ESCH_ERROR_NOT_IMPLEMENTED;
    esch_log* log = obj->log;

    assert(log != NULL);
    if (obj->gc == gc)
    {
        esch_log_info(log, "gc:attach: Already attached.");
        return ESCH_OK;
    }

    /* Do now allow object switch from one GC system to another.
     * We do this because there's no cheap and clean way to remove an
     * object from esch's GC system. */
    ret = ESCH_ERROR_INVALID_STATE;
    assert(obj->gc != NULL && obj->gc != gc);
    ESCH_CHECK_1(obj->gc != NULL && obj->gc != gc, log,
            "gc:attach: FATAL: switch GC system: obj: %x", obj, ret);

    /* TODO: For new object, allocate a cell, and assign object. */
Exit:
    return ret;
}
static esch_error
esch_gc_naive_mark_sweep_detach_i(esch_gc* gc, esch_object* obj)
{
    esch_error ret = ESCH_ERROR_NOT_IMPLEMENTED;
    return ret;
}

static esch_error
esch_gc_naive_mark_sweep_recycle_i(esch_gc* gc)
{
    esch_error ret = ESCH_ERROR_NOT_IMPLEMENTED;
    return ret;
}

static esch_error
esch_gc_new_naive_mark_sweep_i(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_object* new_obj = NULL;
    esch_gc* new_gc = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    int initial_cells = 0;
    unsigned char* inuse_flags = NULL;
    esch_object** objects = NULL;

    initial_cells = ESCH_CONFIG_GET_GC_NAIVE_INITIAL_CELLS(config);
    initial_cells = initial_cells < 0?
                        ESCH_GC_NAIVE_DEFAULT_CELLS:
                        initial_cells;

    alloc = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_ALLOC(config), esch_alloc);
    ESCH_ASSERT(alloc != NULL && ESCH_IS_VALID_ALLOC(alloc));

    log = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_LOG(config), esch_log);
    ESCH_ASSERT(log != NULL && ESCH_IS_VALID_LOG(log));

    (void)esch_log_info(log, "GC: Prepare cells");
    ret = esch_alloc_realloc(alloc, NULL, initial_cells, (void**)&inuse_flags);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:Can't create cell inuse list", ret);
    ret = esch_alloc_realloc(alloc, NULL,
                             sizeof(esch_object*) * initial_cells,
                             (void**)&objects);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:Can't create object list", ret);

    (void)esch_log_info(log, "GC: Create objects");
    ret = esch_object_new_i(config, &(esch_gc_type.type), &new_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:Can't create gc object", ret);
    new_gc = ESCH_CAST_FROM_OBJECT(new_obj, esch_gc);

    new_gc->attach = esch_gc_naive_mark_sweep_attach_i;
    new_gc->detach = esch_gc_naive_mark_sweep_detach_i;
    new_gc->recycle = esch_gc_naive_mark_sweep_recycle_i;
    new_gc->cell.inuse_flags = inuse_flags;
    new_gc->cell.objects = objects;
    new_gc->cell.object_count = initial_cells;
    (*obj) = new_obj;
    inuse_flags = NULL;
    objects = NULL;
    new_obj = NULL;

Exit:
    if (inuse_flags)
    {
        esch_alloc_free(alloc, inuse_flags);
    }
    if (objects)
    {
        esch_alloc_free(alloc, objects);
    }
    return ret;
}

/* ===============================================================
 * Public interfaces
 * =============================================================== */

esch_error
esch_gc_new_naive_mark_sweep(esch_config* config, esch_gc** gc)
{
    esch_error ret = ESCH_OK;
    esch_object* new_gc_obj = NULL;
    esch_log* log = NULL;
    esch_object* log_obj = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    ESCH_CHECK_PARAM_PUBLIC(gc != NULL);

    log_obj = ESCH_CONFIG_GET_LOG(config);
    ESCH_CHECK_PARAM_PUBLIC(log_obj != NULL);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LOG(log));

    esch_log_info(log, "GC:new_naive: Create GC object.");
    ret = esch_gc_new_naive_mark_sweep_i(config, &new_gc_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:new_naive: Can't create GC", ret);

    (*gc) = ESCH_CAST_FROM_OBJECT(new_gc_obj, esch_gc);
    new_gc_obj = NULL;
    esch_log_info(log, "GC:new_naive: GC object created.");
Exit:
    if (new_gc_obj != NULL)
    {
        esch_log_info(log, "GC:new_naive: On error: delete GC object.");
        esch_object_delete(new_gc_obj);
    }
    return ret;
}

/**
 * Attach an object to GC. Internal function.
 */
esch_error
esch_gc_attach_i(esch_gc* gc, esch_object* obj)
{
    esch_error ret = ESCH_OK;
    assert(gc != NULL);
    assert(ESCH_IS_VALID_GC(gc));

    ret = esch_gc_naive_mark_sweep_attach_i(gc, obj);
Exit:
    return ret;
}

esch_error
esch_gc_detach_i(esch_gc* gc, esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    assert(gc != NULL);
    assert(ESCH_IS_VALID_GC(gc));
    assert(obj != NULL);

    ret = esch_gc_naive_mark_sweep_detach_i(gc, obj);
Exit:
    return ret;
}

esch_error
esch_gc_recycle_i(esch_gc* gc)
{
    esch_error ret = ESCH_OK;
    assert(gc != NULL);
    assert(ESCH_IS_VALID_GC(gc));

    ret = esch_gc_naive_mark_sweep_recycle_i(gc);
Exit:
    return ret;
}
