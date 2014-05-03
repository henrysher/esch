#include "esch_utest.h"
#include "esch_gc.h"
#include "esch_vector.h"

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
                              ESCH_CONFIG_KEY_GC_NAIVE_INITIAL_SLOTS, -1);
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
                              ESCH_CONFIG_KEY_GC_NAIVE_INITIAL_SLOTS, 256);
    ret = esch_gc_new_naive_mark_sweep(config, &gc3);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc3 != NULL,
                    "Failed to create gc3", ret);
    ESCH_TEST_CHECK(gc3->slot_count == ESCH_GC_NAIVE_DEFAULT_SLOTS,
                    "Slot count is incorrect", ret);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc3));
    gc3 = NULL;
Exit:
    return ret;
}
