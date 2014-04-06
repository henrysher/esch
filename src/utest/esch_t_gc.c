#include "esch_utest.h"
#include "esch_gc.h"
esch_error test_gc(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_gc* gc1 = NULL;
    esch_gc* gc2 = NULL;
    esch_gc* gc3 = NULL;
    esch_log_info(g_testLog, "Case 1: Create GC object");
    ret = esch_gc_new_naive_mark_sweep(config, &gc1);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc1 != NULL,
                    "Failed to create gc1", ret);
    ESCH_TEST_CHECK(gc1->cell.object_count == ESCH_GC_NAIVE_DEFAULT_CELLS,
                    "Failed to create gc1", ret);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc1));
    gc1 = NULL;

    esch_log_info(g_testLog, "Case 2: Create GC object with new slots");
    ret = esch_config_set_int(config,
                              ESCH_CONFIG_KEY_GC_NAIVE_INITIAL_CELLS, 256);
    ret = esch_gc_new_naive_mark_sweep(config, &gc2);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc2 != NULL,
                    "Failed to create gc2", ret);
    ESCH_TEST_CHECK(gc2->cell.object_count == 256,
                    "Failed to create gc1", ret);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc2));
    gc2 = NULL;

    esch_log_info(g_testLog, "Case 2: Create GC object with system default");
    ret = esch_config_set_int(config,
                              ESCH_CONFIG_KEY_GC_NAIVE_INITIAL_CELLS, -1);
    ret = esch_gc_new_naive_mark_sweep(config, &gc3);
    ESCH_TEST_CHECK(ret == ESCH_OK && gc3 != NULL,
                    "Failed to create gc2", ret);
    ESCH_TEST_CHECK(gc3->cell.object_count == ESCH_GC_NAIVE_DEFAULT_CELLS,
                    "Failed to create gc1", ret);
    ret = esch_object_delete(ESCH_CAST_TO_OBJECT(gc3));
    gc3 = NULL;

Exit:
    return ret;
}
