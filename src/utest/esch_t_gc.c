#include "esch_utest.h"
#include "esch_gc.h"
#include "esch_vector.h"
#include "esch_config.h"

esch_error test_gcCreateDelete(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_gc* gc1 = NULL;
    esch_gc* gc2 = NULL;
    esch_gc* gc3 = NULL;
    esch_vector* root = NULL;

    ret = esch_vector_new(config, &root);
    ESCH_TEST_CHECK(ret == ESCH_OK && root != NULL,
                    "Failed to create gc root", ret);

    esch_log_info(g_testLog, "Case 1: Create GC object");

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set GC root", ret);
    esch_log_info(g_testLog, "root: %x", ESCH_CAST_TO_OBJECT(root));
    ret = esch_gc_new_naive_mark_sweep(config, &gc1);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc1 != NULL,
                    "Failed to create gc1", ret);
    ESCH_TEST_CHECK(gc1->slot_count == ESCH_GC_NAIVE_DEFAULT_SLOTS,
                    "Default slot count is wrong", ret);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc1));
    gc1 = NULL;

    esch_log_info(g_testLog, "Case 2: Create GC object with system default");
    ret = esch_vector_new(config, &root);
    ESCH_TEST_CHECK(ret == ESCH_OK && root != NULL,
                    "Failed to create gc root", ret);
    esch_log_info(g_testLog, "root: %x", ESCH_CAST_TO_OBJECT(root));

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set GC root", ret);

    ret = esch_config_set_int(config,
                              ESCH_CONFIG_KEY_GC_NAIVE_SLOTS, -1);
    ret = esch_gc_new_naive_mark_sweep(config, &gc2);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc2 != NULL,
                    "Failed to create gc2", ret);
    ESCH_TEST_CHECK(gc2->slot_count == ESCH_GC_NAIVE_DEFAULT_SLOTS,
                    "Slot count is not correct", ret);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc2));
    gc2 = NULL;

    esch_log_info(g_testLog, "Case 3: Create GC object with new slots");
    ret = esch_vector_new(config, &root);
    ESCH_TEST_CHECK(ret == ESCH_OK && root != NULL,
                    "Failed to create gc root", ret);
    esch_log_info(g_testLog, "root: %x", ESCH_CAST_TO_OBJECT(root));

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set GC root", ret);

    ret = esch_config_set_int(config,
                              ESCH_CONFIG_KEY_GC_NAIVE_SLOTS, 256);
    ret = esch_gc_new_naive_mark_sweep(config, &gc3);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc3 != NULL,
                    "Failed to create gc3", ret);
    ESCH_TEST_CHECK(gc3->slot_count == 256,
                    "Slot count is incorrect", ESCH_ERROR_INVALID_STATE);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc3));
    gc3 = NULL;
Exit:
    if (gc1 != NULL)
    {
        esch_object_delete(ESCH_CAST_TO_OBJECT(gc1));
    }
    if (gc2 != NULL)
    {
        esch_object_delete(ESCH_CAST_TO_OBJECT(gc2));
    }
    if (gc3 != NULL)
    {
        esch_object_delete(ESCH_CAST_TO_OBJECT(gc3));
    }
    return ret;
}

esch_error test_gcRecycleLogic(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_gc* gc = NULL;
    esch_gc* bad_gc = NULL;
    esch_vector* root_scope = NULL;
    esch_vector* child_scope_1_1 = NULL;
    esch_vector* child_scope_1_2 = NULL;
    esch_vector* child_scope_2_1 = NULL;
    esch_string* str1 = NULL;
    esch_type* tt = NULL;
    esch_type* tt2 = NULL;
    esch_vector* circle1 = NULL;
    esch_vector* circle2 = NULL;
    esch_vector* circle3 = NULL;
    esch_vector* circle4 = NULL;

    /*
     * Test scenario:
     * 0. esch_object should honor gc object in config.
     * 1. gc should never be managed by another gc.
     * 2. A scope may hold non-container object. Not be freed.
     * 3. A scope may hold child container. Must be visited.
     * 4. A child scope may hold child container. Must be visited.
     * 5. Reference circle should not affect releasing.
     */
    ret = esch_vector_new(config, &root_scope);
    ESCH_TEST_CHECK(ret == ESCH_OK && root_scope,
                    "Failed to create gc root", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root_scope));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set gc root", ret);

    ret = esch_gc_new_naive_mark_sweep(config, &gc);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc, "Failed to create gc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                              ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set gc", ret);

    esch_log_info(g_testLog,
            "1. GC should never be managed by another GC.");
    ret = esch_gc_new_naive_mark_sweep(config, &bad_gc);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_OBJECT_UNEXPECTED_GC_ATTACHED
                        && bad_gc == NULL,
                    "Expect a failure when creating GC with GC", ret);

    /* Prepare environment. */

    /* NOTE: This is safe because public interface will not expose
     * definition of esch_object.
     */
    esch_log_info(g_testLog,
            "0. GC setting shall be picked up with new object.");
    ret = esch_string_new_from_utf8(config, "hello", 0, -1, &str1);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create string", ret);
    ESCH_TEST_CHECK(ESCH_CAST_TO_OBJECT(str1)->gc == gc,
                    "GC is not attached", ret);

    esch_log_info(g_testLog, "2. Scope may hold non-conainer types.");
    ret = esch_type_new(config, &tt);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create tt", ret);
    ret = esch_vector_append_object(root_scope, ESCH_CAST_TO_OBJECT(tt));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to touch tt to scope", ret);
    ret = esch_vector_append_integer(root_scope, 100);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to add int to scope", ret);
    ret = esch_vector_append_float(root_scope, 100.1234);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to add float scope", ret);
    ret = esch_vector_append_byte(root_scope, 'a');
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to add byte scope", ret);
    ret = esch_vector_append_unicode(root_scope, (esch_unicode)0x4f60);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to add unicode scope", ret);

    ret = esch_type_new(config, &tt2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create tt2", ret);
    /* Structure:
     *
     * root_scope -
     *     tt
     *     child_scope_1_1 - (scope with container, but no children)
     *         str1
     *     child_scope_1_2 - (scepe with container, and has children)
     *         child_scope_2_1 -
     *             circle1 and circle2 form a refernece circle.
     * circle3 and circle4 form a refernece circle.
     * tt2
     */
    esch_log_info(g_testLog, "3. Scope may hold container objects.");
    ret = esch_vector_new(config, &child_scope_1_1);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create scope11", ret);
    ret = esch_vector_append_object(root_scope,
                                    ESCH_CAST_TO_OBJECT(child_scope_1_1));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append scope11", ret);

    ret = esch_vector_new(config, &child_scope_1_2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create scope12", ret);
    ret = esch_vector_append_object(root_scope,
                                    ESCH_CAST_TO_OBJECT(child_scope_1_2));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append scope12", ret);

    ret = esch_vector_new(config, &child_scope_2_1);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create scope21", ret);
    ret = esch_vector_append_object(child_scope_1_1,
                                    ESCH_CAST_TO_OBJECT(str1));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append scope12", ret);


    esch_log_info(g_testLog, "4. A child scope may hold child container.");
    ret = esch_vector_append_object(child_scope_1_2,
                             ESCH_CAST_TO_OBJECT(child_scope_2_1));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append scope21", ret);


    esch_log_info(g_testLog,
            "5. Reference circle should not affect releasing.");
    ret = esch_vector_new(config, &circle1);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create circle1", ret);
    ret = esch_vector_new(config, &circle2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create circle2", ret);
    ret = esch_vector_new(config, &circle3);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create circle3", ret);
    ret = esch_vector_new(config, &circle4);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create circle4", ret);

    ret = esch_vector_append_object(child_scope_2_1,
                                    ESCH_CAST_TO_OBJECT(circle1));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append circle2", ret);

    ret = esch_vector_append_object(circle1, ESCH_CAST_TO_OBJECT(circle2));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append circle2", ret);
    ret = esch_vector_append_object(circle2, ESCH_CAST_TO_OBJECT(circle1));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append circle1", ret);

    ret = esch_vector_append_object(circle3, ESCH_CAST_TO_OBJECT(circle4));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append circle4", ret);
    ret = esch_vector_append_object(circle4, ESCH_CAST_TO_OBJECT(circle3));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to append circle3", ret);

    ret = esch_gc_recycle(gc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to do recycle", ret);
    /* After here, we should verify:
     * str1 is deleted.
     * circle3 and circle4 are deleted.
     */
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete GC", ret);
    gc = NULL;

    /* TODO Need a callback system in GC to verify object deleting. */
    /* TODO Need method for vector to clear or remove single element. */
    /* TODO Make string become container. */
    /* TODO We should add case when list object is implemented.  */
    /* TODO Stack full.  */
Exit:
    if (gc != NULL)
    {
        ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
    }
    (void)esch_config_set_obj(config, ESCH_CONFIG_KEY_GC, NULL);
    (void)esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT, NULL);
    return ret;
}

esch_error test_gcNoExpand(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_gc* gc = NULL;
    esch_vector* root = NULL;
    esch_alloc* alloc = NULL;
    esch_string** objs = NULL;
    size_t i = 0;
    const size_t shortlen = 32;


    alloc = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_ALLOC(config),
                                  esch_alloc);

    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH, shortlen);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_SLOTS, shortlen);
    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 0);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_ENLARGE, 0);

    ret = esch_vector_new(config, &root);
    ESCH_TEST_CHECK(ret == ESCH_OK && root,
                    "Failed to create gc root", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set gc root", ret);

    ret = esch_gc_new_naive_mark_sweep(config, &gc);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc, "Failed to create gc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                              ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set gc", ret);

    /* Now we can start. */
    esch_log_info(g_testLog, "Case 1: fill all slots.");
    esch_alloc_realloc(alloc, NULL, sizeof(esch_string*) * shortlen,
                       (void**)&objs);
    for (i = 0; i < shortlen - 1; ++i) {
        ret = esch_string_new_from_utf8(config, "Value", 0, -1, &objs[i]);
        ESCH_TEST_CHECK(ret == ESCH_OK && objs[i],
                        "Can't create objects within slot", ret);
    }
    esch_log_info(g_testLog, "Case 2: Slot overflow");
    ret = esch_string_new_from_utf8(config, "Value", 0, -1, &objs[i]);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_CONTAINER_FULL && objs[i] == NULL,
                    "Can't create objects within slot", ret);
    ESCH_TEST_CHECK(gc->slot_count == shortlen,
                    "Slot count should be expanded.",
                    ESCH_ERROR_INVALID_STATE);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK,
                    "Can't create objects within slot", ret);
    gc = NULL;

Exit:
    if (gc != NULL)
    {
        esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
    }
    esch_config_set_obj(config, ESCH_CONFIG_KEY_GC, NULL);
    esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT, NULL);
    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH, -1);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_SLOTS, -1);
    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 0);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_ENLARGE, 0);

    esch_alloc_free(alloc, objs);
    return ret;
}

esch_error test_gcExpand(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_gc* gc = NULL;
    esch_vector* root = NULL;
    esch_alloc* alloc = NULL;
    esch_string** objs = NULL;
    size_t i = 0;
    const size_t shortlen = 32;


    alloc = ESCH_CAST_FROM_OBJECT(ESCH_CONFIG_GET_ALLOC(config),
                                  esch_alloc);

    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH, shortlen);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_SLOTS, shortlen);
    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 1);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_ENLARGE, 1);

    ret = esch_vector_new(config, &root);
    ESCH_TEST_CHECK(ret == ESCH_OK && root,
                    "Failed to create gc root", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set gc root", ret);

    ret = esch_gc_new_naive_mark_sweep(config, &gc);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc, "Failed to create gc", ret);

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                              ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set gc", ret);

    /* Now we can start. */
    esch_log_info(g_testLog, "Case 1: fill all slots.");
    esch_alloc_realloc(alloc, NULL, sizeof(esch_string*) * shortlen,
                       (void**)&objs);
    for (i = 0; i < shortlen; ++i) {
        ret = esch_string_new_from_utf8(config, "Value", 0, -1, &objs[i]);
        ESCH_TEST_CHECK(ret == ESCH_OK && objs[i],
                        "Can't create objects within slot", ret);
    }
    ESCH_TEST_CHECK(gc->slot_count == shortlen * 2,
                    "Slot count should be expanded.",
                    ESCH_ERROR_INVALID_STATE);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK,
                    "Can't create objects within slot", ret);
    gc = NULL;

Exit:
    if (gc != NULL)
    {
        esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
    }
    esch_config_set_obj(config, ESCH_CONFIG_KEY_GC, NULL);
    esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT, NULL);
    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_LENGTH, -1);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_SLOTS, -1);
    esch_config_set_int(config, ESCH_CONFIG_KEY_VECTOR_ENLARGE, 0);
    esch_config_set_int(config, ESCH_CONFIG_KEY_GC_NAIVE_ENLARGE, 0);

    esch_alloc_free(alloc, objs);
    return ret;
}

