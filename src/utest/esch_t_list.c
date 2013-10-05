#include "esch_utest.h"

esch_error
test_list()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_parser* parser = NULL;
    esch_config config = { ESCH_TYPE_UNKNOWN, NULL, NULL };
    esch_list* lst = NULL;
    esch_log* do_nothing = NULL;
    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    config.log = g_testLog;
    ret = esch_alloc_new_c_default(&config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    config.alloc = alloc;
    config.log = NULL;
    ret = esch_list_new(&config, &lst);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && lst == NULL,
                    "Failed to create log - no log", ret);

    config.alloc = NULL;
    config.log = log;
    ret = esch_list_new(&config, &lst);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER && lst == NULL,
                    "Failed to create log - no alloc", ret);

    config.alloc = alloc;
    config.log = log;
    ret = esch_list_new(&config, &lst);
    ESCH_TEST_CHECK(ret == ESCH_OK && lst != NULL,
                    "Failed to create log - no alloc", ret);

    ret = esch_list_delete(lst);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete list.", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);
Exit:
    return ret;
}
