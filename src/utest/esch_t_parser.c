#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"

int test_parser()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;
    esch_config config = { ESCH_TYPE_CONFIG, NULL, NULL };

    config.log = g_testLog;
    ret = esch_alloc_new_c_default(&config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    config.alloc = alloc;
    config.log = NULL;

    ret = esch_parser_new(&config, &parser);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER, "Failed to create parser - no log", ret);

    config.alloc = NULL;
    config.log = g_testLog;
    ret = esch_parser_new(&config, &parser);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER, "Failed to create parser - no alloc", ret);

    config.alloc = alloc;
    config.log = g_testLog;
    ret = esch_parser_new(&config, &parser);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create parser", ret);

    ret = esch_parser_delete(parser);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete parser", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);
Exit:
    return ret;
}
