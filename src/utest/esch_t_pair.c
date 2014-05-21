#include <stdio.h>
#include "esch.h"
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_pair.h"

esch_error test_pairBase(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_gc* gc = NULL;
    esch_vector* root = NULL;
    const size_t len = 10; 
    esch_pair* pairs[10];
    size_t i = 0, count = 0;
    esch_value value1, value2, data;
    esch_iterator iter;
    esch_pair* bad_pair = NULL;
    esch_pair* short_pair = NULL;

    /* To simplify the work we just keep it with GC */
    ret = esch_vector_new(config, &root);
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't create root", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC_NAIVE_ROOT,
                              ESCH_CAST_TO_OBJECT(root));
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't set root", ret);
    ret = esch_gc_new_naive_mark_sweep(config, &gc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't create gc", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_GC,
                              ESCH_CAST_TO_OBJECT(gc));
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't set gc", ret);

    esch_log_info(g_testLog, "Case 1: Enumerate without empty list.");
    value1.type = ESCH_VALUE_TYPE_INTEGER;
    value1.val.i = 100;
    value2.type = ESCH_VALUE_TYPE_INTEGER;
    value2.val.i = 99;
    ret = esch_pair_new(config, &value1, &value2, &pairs[0]);
    ESCH_TEST_CHECK(ret == ESCH_OK && pairs[i] != NULL,
                    "pair:Can't set pair", ret);
    for (i = 1; i < len; ++i) {
        value1.type = ESCH_VALUE_TYPE_INTEGER;
        value1.val.i = i + 100;
        value2.type = ESCH_VALUE_TYPE_OBJECT;
        value2.val.o = ESCH_CAST_TO_OBJECT(pairs[i - 1]);
        ret = esch_pair_new(config, &value1, &value2, &pairs[i]);
        ESCH_TEST_CHECK(ret == ESCH_OK && pairs[i] != NULL,
                        "pair:Can't set pair", ret);
    }

    /* Enumerate everything */
    count = 0;
    i = 9;
    ret = esch_object_get_iterator(ESCH_CAST_TO_OBJECT(pairs[len - 1]),
                                   &iter);
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get iterator", ret);
    while (ESCH_TRUE) {
        ret = iter.get_value(&iter, &data);
        ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get data", ret);
        if (data.type == ESCH_VALUE_TYPE_END) {
            break;
        }
        ESCH_TEST_CHECK(data.type == ESCH_VALUE_TYPE_INTEGER,
                        "pair:data is not integer",
                        ESCH_ERROR_INVALID_STATE);
        ESCH_TEST_CHECK(data.val.i == 100 + i,
                        "pair:data is not integer",
                        ESCH_ERROR_INVALID_STATE);
        ret = iter.get_next(&iter);
        ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get next data",
                        ESCH_ERROR_BAD_VALUE_TYPE);
        --i;
        ++count;
    }
    ESCH_TEST_CHECK(count == 11, "pair:Total count is not correct",
                    ESCH_ERROR_BAD_VALUE_TYPE);

    esch_log_info(g_testLog, "Case 2: Can't create object with END");
    value1.type = ESCH_VALUE_TYPE_END;
    value1.val.i = 0;
    value2.type = ESCH_VALUE_TYPE_INTEGER;
    value2.val.i = 12345;
    ret = esch_pair_new(config, &value1, &value2, &bad_pair);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && !bad_pair,
                    "pair:Can't set pair", ret);
    esch_log_info(g_testLog, "Expected failure captured 1");
    ret = ESCH_OK;

    value1.type = ESCH_VALUE_TYPE_INTEGER;
    value1.val.i = 54321;
    value2.type = ESCH_VALUE_TYPE_END;
    value2.val.i = 0;
    ret = esch_pair_new(config, &value1, &value2, &bad_pair);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && !bad_pair,
                    "pair:Can't set pair", ret);
    esch_log_info(g_testLog, "Expected failure captured 2");
    ret = ESCH_OK;

    esch_log_info(g_testLog, "Case 3: Create empty list.");
    ret = esch_pair_new_empty(config, &short_pair);
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get data", ret);
    value1.type = ESCH_VALUE_TYPE_OBJECT;
    value1.val.o = ESCH_CAST_TO_OBJECT(short_pair);
    value2.type = ESCH_VALUE_TYPE_OBJECT;
    value2.val.o = ESCH_CAST_TO_OBJECT(short_pair);
    ret = esch_pair_new(config, &value1, &value2, &short_pair);
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get data", ret);
    /* Enumerate everything */
    count = 0;
    ret = esch_object_get_iterator(ESCH_CAST_TO_OBJECT(short_pair),
                                   &iter);
    ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get iterator", ret);
    while (ESCH_TRUE) {
        ret = iter.get_value(&iter, &data);
        ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get data", ret);
        if (data.type == ESCH_VALUE_TYPE_END) {
            break;
        }
        /*
        ESCH_TEST_CHECK(data.type == ESCH_VALUE_TYPE_OBJECT,
                        "pair:data is not object",
                        ESCH_ERROR_INVALID_STATE);
                        */
        ret = iter.get_next(&iter);
        ESCH_TEST_CHECK(ret == ESCH_OK, "pair:Can't get next data",
                        ESCH_ERROR_BAD_VALUE_TYPE);
        ++count;
    }
    ESCH_TEST_CHECK(count == 1, "pair:Total count is not correct",
                    ESCH_ERROR_BAD_VALUE_TYPE);


Exit:
    if (gc != NULL) {
        esch_object_delete(ESCH_CAST_TO_OBJECT(gc));
        esch_config_set_obj(config, ESCH_CONFIG_KEY_GC, NULL);
    }
    return ret;
}
