#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"

int test_AllocCreateParser()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;

    ret = esch_alloc_new_c_default(&alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    ret = esch_parser_new(alloc, NULL, &parser);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER, "Failed to create parser - no log", ret);

    ret = esch_parser_new(NULL, g_testLog, &parser);
    ESCH_TEST_CHECK(ret == ESCH_ERROR_INVALID_PARAMETER, "Failed to create parser - no alloc", ret);

    ret = esch_parser_new(alloc, g_testLog, &parser);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create parser", ret);

    ret = esch_parser_delete(parser);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete parser", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);
Exit:
    return ret;
}
