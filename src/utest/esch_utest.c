#include <esch.h>
#include <stdio.h>
#include "esch_utest.h"

esch_log* g_testLog = NULL;

int main(int argc, char* argv[])
{
    esch_error ret = ESCH_OK;
    ret = esch_log_new_printf(&g_testLog);

    /* Really run test */
    ret = test_AllocCreateDeleteCDefault();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_AllocCreateDeleteCDefault() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_AllocCreateDeleteCDefault()");

    ret = test_AllocCreateParser();
    ESCH_TEST_CHECK(ret == ESCH_OK, "test_AllocCreateParser() failed", ret);
    esch_log_info(g_testLog, "[PASSED] test_AllocCreateParser()");

Exit:
    (void)esch_log_delete(g_testLog);
    return ret;
}
