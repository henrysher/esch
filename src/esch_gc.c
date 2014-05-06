#include "esch_gc.h"
#include "esch_type.h"
#include "esch_object.h"
#include "esch_config.h"
#include "esch_log.h"
#include "esch_alloc.h"
#include "esch_debug.h"
#include <string.h>

#define ESCH_GC_MARK_INUSE(gc, obj) { \
    gc->inuse_flags[((size_t)((obj)->gc_id) >> 3)] |= \
                            (0x1 << ((size_t)((obj)->gc_id) & 0xF));\
}

#define ESCH_GC_IS_MARKED(gc, gc_id) \
    (gc->inuse_flags[((size_t)(gc_id) >> 3)] & \
                              (0x1 << ((size_t)(gc_id) & 0xF)))

const int ESCH_GC_NAIVE_DEFAULT_SLOTS = 4096;
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
    size_t i = 0;

    ESCH_ASSERT(obj != NULL);
    gc = ESCH_CAST_FROM_OBJECT(obj, esch_gc);
    ESCH_ASSERT(ESCH_IS_VALID_GC(gc));

    alloc = obj->alloc;
    log = obj->log;
    ESCH_ASSERT(alloc != NULL && ESCH_IS_VALID_ALLOC(alloc));
    ESCH_ASSERT(log != NULL && ESCH_IS_VALID_LOG(log));

    /* Make sure every managed object is deleted. */
    for (i = 0; i < gc->slot_count; ++i) {
        if (ESCH_GC_IS_MARKED(gc, i)) {
            esch_log_info(log, "delete object[%d] = %x", i,
                          gc->slots[i].obj);
            ESCH_ASSERT(ESCH_IS_VALID_OBJECT(gc->slots[i].obj));
            ESCH_ASSERT(gc->slots[i].obj->gc == gc);
            gc->slots[i].obj->gc = NULL;
            gc->slots[i].obj->gc_id = 0;
            ret = esch_object_delete_i(gc->slots[i].obj);
            if (ret != ESCH_OK) {
                esch_log_warn(log,
                        "gc:recycle: Can't delete object: %x (ignore)",
                        gc->slots[i].obj);
            }
        }
    }

    ESCH_CHECK_1(ret == ESCH_OK, log,
                 "gc:attach: FATAL: Can't delete root: %x", obj, ret);
    /* Always delete root has been deleted. */
    gc->root->gc = NULL;
    gc->root->gc_id = 0;

    (void)esch_alloc_free(alloc, gc->slots);
    (void)esch_alloc_free(alloc, gc->inuse_flags);
    (void)esch_alloc_free(alloc, gc->recycle_stack);
    /* Note: Don't destroy itself. Will be handled by esch_object */
Exit:
    return ret;
}

static esch_error
esch_gc_naive_mark_sweep_attach_i(esch_gc* gc, esch_object* obj)
{
    esch_error ret = ESCH_OK;
    esch_log* log = obj->log;
    esch_alloc* alloc = NULL;
    esch_object** allocated_slot = NULL;

    size_t new_count = 0;
    union esch_object_or_next* new_slots = NULL;
    esch_byte* new_flags = NULL;
    size_t new_offset = 0;

    ESCH_ASSERT(gc != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_GC(gc));
    alloc = ESCH_CAST_TO_OBJECT(gc)->alloc;
    ESCH_ASSERT(alloc != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_ALLOC(alloc));
    ESCH_ASSERT(log != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_LOG(log));

    if (obj->gc == gc) {
        esch_log_info(log, "gc:attach: Already attached. Do nothing.");
        goto Exit;
    }

    /* Do now allow object switch from one GC system to another.
     * We do this because there's no cheap and clean way to remove an
     * object from esch's GC system. */
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_ASSERT(obj->gc != NULL && obj->gc != gc);
    ESCH_CHECK_1(obj->gc != NULL && obj->gc != gc, log,
            "gc:attach: FATAL: switch GC system: obj: %x", obj, ret);

    if (gc->slots[gc->available_slot_offset].obj == gc->root) {
        /* Running out of slots. Need to allocate new */
        int i = 0;
        (void)esch_log_info(log,
                "gc:attach: No more slot. Reallocate. %d",
                gc->slot_count);
        new_count = gc->slot_count * 2;

        /* Reallocate a larger buffer */
        ret = esch_alloc_realloc_i(alloc, gc->slots,
                         sizeof(union esch_object_or_next) * new_count,
                         (void**)&new_slots);
        ESCH_CHECK(ret == ESCH_OK, log,
                "gc:attach: FATAL: Can't allocate new slots", ret);
        ret = esch_alloc_realloc_i(alloc, gc->inuse_flags,
                                   sizeof(esch_byte) / 8 * new_count,
                                   (void**)&new_flags);
        ESCH_CHECK(ret == ESCH_OK, log,
                "gc:attach: FATAL: Can't allocate new flags", ret);
        /* Content of original buffer has been moved to new buffer,
         * Now update availability slot list. */
        new_slots[gc->slot_count].obj = gc->root;
        for (i = gc->slot_count + 1; i < new_count - 1; ++i) {
            new_slots[i + 1].next = i;
        }

        gc->slot_count = new_count;
        gc->slots = new_slots;
        gc->inuse_flags = new_flags;
        /* Always count availability chain from last element */
        gc->available_slot_offset = new_count -1;

        new_slots = NULL;
        new_flags = NULL;
    }
    new_offset = gc->available_slot_offset;
    (void)esch_log_info(log,
            "gc:attach: Allocate new slot: id: %d", new_offset);

    gc->available_slot_offset = gc->slots[gc->available_slot_offset].next;
    allocated_slot = &(gc->slots[new_offset].obj);
    (*allocated_slot) = obj;

    obj->gc = gc;
    obj->gc_id = (void*)new_offset;
    ESCH_ASSERT(!ESCH_GC_IS_MARKED(gc, obj->gc_id));
Exit:
    esch_alloc_free_i(alloc, new_slots);
    esch_alloc_free_i(alloc, new_flags);
    return ret;
}

static esch_error
esch_gc_naive_mark_sweep_recycle_i(esch_gc* gc)
{
    esch_error ret = ESCH_OK;
    esch_log* log = NULL;
    esch_element element = { ESCH_ELEMENT_TYPE_END, 0 };
    esch_object* child = NULL;
    esch_object** stack_ptr = NULL;
    esch_type* element_type = NULL;
    esch_iterator iter = {0};
    size_t i = 0;
    size_t free_objs = 0;

    ESCH_ASSERT(gc != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_GC(gc));
    log = ESCH_OBJECT_GET_LOG(ESCH_CAST_TO_OBJECT(gc));
    ESCH_ASSERT(log != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_LOG(log));

    /* Please also note that we don't allocate anything here, because
     * recycle() happens only when system is running out of memory. */
    esch_log_info(log, "gc:recycle: Trigger GC on root: %x", gc->root);
    ESCH_ASSERT(ESCH_TYPE_IS_CONTAINER(ESCH_OBJECT_GET_TYPE(gc->root)));

    /* This is a classical (and slow) mark-and-sweep algorithm.
     * 1. Mark all objects as deletable, then
     * 2. Start from top root object to set inuse flag, base on 
     *    reachability.
     * 3. Remove all objects that marked as NOT IN USE.
     *
     * We use recycle_stack as a stack for enumerating the object
     * reachability tree, starting from root. We make sure all
     * children of root are managed by the same GC.
     *
     * Meanwhile, recycle_stack is also used to store unused objects
     */
    /* Step 1: Mark every object as deletable. */
    memset(gc->inuse_flags, 0, sizeof(esch_byte) / 8 * gc->slot_count);
    /* 
     * Step 2: Search objects from root node, and mark reachable object
     * as in-use.
     */
    stack_ptr = &(gc->recycle_stack[0]);
    (*(stack_ptr++)) = gc->root; /* Root is always in use */
    do
    {
        ret = esch_object_get_iterator_i(*(stack_ptr - 1), &iter);
        ESCH_ASSERT(ret == ESCH_OK);
        ESCH_GC_MARK_INUSE(gc, *(stack_ptr - 1));
        while(ESCH_TRUE) {
            ret = iter.get_value(&iter, &element);
            ESCH_ASSERT(ret == ESCH_OK);
            if (element.type == ESCH_ELEMENT_TYPE_END) {
                esch_log_info(log, "gc:recycle: end of objects.");
                break;
            } else if (element.type != ESCH_ELEMENT_TYPE_OBJECT) {
                esch_log_info(log, "gc:recycle: primitive type, skip.");
                continue;
            }
            /* IMPORTANT
             * I set assertion because I can't find a way to behave
             * correctly if I mix two GC systems in one object system,
             * without causing semantic problems.
             *
             * If anyone know how to do it, please ping me.
             */
            child = element.val.o;
            ESCH_ASSERT(child != NULL);
            ESCH_ASSERT(child->gc == gc);
            element_type = ESCH_OBJECT_GET_TYPE(child);
            ESCH_ASSERT(element_type != NULL);
            ESCH_ASSERT(ESCH_IS_VALID_TYPE(element_type));
            if (ESCH_TYPE_IS_CONTAINER(element_type)) {
                esch_log_info(log, "gc:recycle: container. Mark/stack.");
                /* Never mark twice so we won't fall into endless loop
                 * if we hit a reference circle. */
                if (!ESCH_GC_IS_MARKED(gc, child->gc_id)) {
                    ESCH_GC_MARK_INUSE(gc, child);
                    (*(stack_ptr++)) = child;
                }
            } else {
                esch_log_info(log, "gc:recycle: non-container. Mark.");
                if (!ESCH_GC_IS_MARKED(gc, child->gc_id)) {
                    ESCH_GC_MARK_INUSE(gc, child);
                }
            }
        }
    } while(stack_ptr != &(gc->recycle_stack[0]));
    /* By now we have marked all required objects, free the rest and
     * rebuild availability slot list. */
    ESCH_ASSERT(gc->slots[0].obj == gc->root);
    ESCH_GC_MARK_INUSE(gc, gc->slots[0].obj);

    /* Step 3: Delete all objects marked as deletable. */
    /* gc->available_slot_offset = 0; */
    for (free_objs = 0, i = 1; i < gc->slot_count; ++i) {
        /* Skip i = 0 because first element is root, which is always
         * in use.
         */
        if (!ESCH_GC_IS_MARKED(gc, i)) {
            ESCH_ASSERT(ESCH_IS_VALID_OBJECT(gc->slots[i].obj));
            ESCH_ASSERT(gc->slots[i].obj->gc == gc);
            gc->slots[i].obj->gc = NULL;
            gc->slots[i].obj->gc_id = 0;
            ret = esch_object_delete_i(gc->slots[i].obj);
            if (ret != ESCH_OK) {
                esch_log_warn(log,
                        "gc:recycle: Can't delete object: %x (ignore)",
                        gc->slots[i].obj);
            }
            gc->slots[i].next = gc->available_slot_offset;
            gc->available_slot_offset = i;
            ++free_objs;
        }
    }
    if (free_objs == 0) {
        /*
         * Wow. It's not really wrong thing, but all objects
         * are in use, and it triggers GC (which means memory pool is
         * out of stock. What happen to caller's code to make it use so
         * much memory?
         */
        esch_log_warn(log, "gc:recycle: 0 object freed. Check memory usage.");
    } else {
        esch_log_info(log, "gc:recycle: %d objects are freed.", free_objs);
    }
    return ret;
}
static esch_error
esch_gc_new_naive_mark_sweep_i(esch_config* config, esch_object** gc)
{
    esch_error ret = ESCH_OK;
    esch_object* new_obj = NULL;
    esch_gc* new_gc = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* root = NULL;
    int initial_slots = 0;
    unsigned char* inuse_flags = NULL;
    union esch_object_or_next* slots = NULL;
    esch_object** recycle_stack = NULL;
    int i = 0;

    initial_slots = ESCH_CONFIG_GET_GC_NAIVE_INITIAL_SLOTS(config);
    if (initial_slots <= ESCH_GC_NAIVE_DEFAULT_SLOTS) {
        initial_slots = ESCH_GC_NAIVE_DEFAULT_SLOTS;
    }
    alloc = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_ALLOC(config), esch_alloc);

    log = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_LOG(config), esch_log);
    root = ESCH_CONFIG_GET_GC_NAIVE_ROOT(config);

    ESCH_ASSERT(alloc != NULL && ESCH_IS_VALID_ALLOC(alloc));
    ESCH_ASSERT(log != NULL && ESCH_IS_VALID_LOG(log));
    ESCH_ASSERT(root != NULL && ESCH_IS_VALID_OBJECT(root));
    ESCH_ASSERT(ESCH_TYPE_IS_CONTAINER(ESCH_OBJECT_GET_TYPE(root)));
    ESCH_ASSERT(root->gc == NULL);

    /* Now create object */
    (void)esch_log_info(log, "GC:new: Prepare slots");
    ret = esch_alloc_realloc_i(alloc, NULL, (initial_slots / 8),
                               (void**)&inuse_flags);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:naive_new:Can't create flags", ret);
    ret = esch_alloc_realloc_i(alloc, NULL,
                    sizeof(union esch_object_or_next) * initial_slots,
                    (void**)&slots);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:naive_new:Can't create slots", ret);

    ret = esch_alloc_realloc_i(alloc, NULL,
                    sizeof(esch_object*) * initial_slots,
                    (void**)&recycle_stack);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:naive_new:Can't create stack", ret);

    (void)esch_log_info(log, "GC: Create objects");
    ret = esch_object_new_i(config, &(esch_gc_type.type), &new_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:Can't create gc object", ret);
    new_gc = ESCH_CAST_FROM_OBJECT(new_obj, esch_gc);

    /* Create availability object link. It should be an linked list, so
     * slots can be returned in different order when it's returned. */

    /* First object is reversed as END_OF_LIST mark. Others are linked. */
    slots[0].obj = root;
    for(i = 1; i < initial_slots - 1; ++i) {
        slots[i + 1].next = i;
    }

    /* Always start from last slot */
    new_gc->available_slot_offset = (initial_slots - 1);
    new_gc->attach = esch_gc_naive_mark_sweep_attach_i;
    new_gc->recycle = esch_gc_naive_mark_sweep_recycle_i;
    new_gc->inuse_flags = inuse_flags;
    new_gc->slots = slots;
    new_gc->recycle_stack = recycle_stack;
    new_gc->slot_count = initial_slots;
    /* Let GC manage root */
    root->gc = new_gc;
    root->gc_id = (void*)0;
    new_gc->root = root;
    ESCH_GC_MARK_INUSE(new_gc, root);

    (*gc) = new_obj;

    inuse_flags = NULL;
    slots = NULL;
    recycle_stack = NULL;
    new_obj = NULL;
Exit:
    esch_alloc_free(alloc, inuse_flags);
    esch_alloc_free(alloc, slots);
    esch_alloc_free(alloc, recycle_stack);
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
    esch_object* root = NULL;
    esch_type* root_type = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    ESCH_CHECK_PARAM_PUBLIC(gc != NULL);

    log_obj = ESCH_CONFIG_GET_LOG(config);
    ESCH_CHECK_PARAM_PUBLIC(log_obj != NULL);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LOG(log));
    /* Never allow GC be managed by another GC */
    ESCH_CHECK(ESCH_CONFIG_GET_GC(config) == NULL, log,
               "GC:new_naive: Unexpected GC passed from config",
               ESCH_ERROR_OBJECT_UNEXPECTED_GC_ATTACHED);
    /* Make sure there's a correct root passed in */
    root = ESCH_CONFIG_GET_GC_NAIVE_ROOT(config);
    ESCH_CHECK(root != NULL && ESCH_IS_VALID_OBJECT(root), log,
            "GC:new_naive: root object", ESCH_ERROR_GC_ROOT_MISSING);
    root_type = ESCH_OBJECT_GET_TYPE(root);
    ESCH_ASSERT(root_type != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_TYPE(root_type));
    ESCH_CHECK(root_type != NULL && ESCH_TYPE_IS_CONTAINER(root_type),
            log, "GC:new_naive: Root object is not container",
            ESCH_ERROR_GC_ROOT_NOT_CONTAINER);
    ESCH_CHECK(root->gc == NULL,
            log, "GC:new_naive: Root object is managed by other GC",
            ESCH_ERROR_OBJECT_UNEXPECTED_GC_ATTACHED);

    /* Now we can create object */
    esch_log_info(log, "GC:new_naive: Create GC object.");
    ret = esch_gc_new_naive_mark_sweep_i(config, &new_gc_obj);
    ESCH_CHECK(ret == ESCH_OK, log, "GC:new_naive: Can't create GC", ret);

    (*gc) = ESCH_CAST_FROM_OBJECT(new_gc_obj, esch_gc);
    new_gc_obj = NULL;
    esch_log_info(log, "GC:new_naive: GC object created.");
Exit:
    if (new_gc_obj != NULL) {
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
    ESCH_ASSERT(gc != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_GC(gc));

    ret = esch_gc_naive_mark_sweep_attach_i(gc, obj);
Exit:
    return ret;
}

esch_error
esch_gc_recycle_i(esch_gc* gc)
{
    esch_error ret = ESCH_OK;
    ESCH_ASSERT(gc != NULL);
    ESCH_ASSERT(ESCH_IS_VALID_GC(gc));

    ret = esch_gc_naive_mark_sweep_recycle_i(gc);
Exit:
    return ret;
}
