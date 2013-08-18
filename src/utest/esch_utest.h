#ifndef _ESCH_UTEST_H_
#define _ESCH_UTEST_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <esch.h>
#include "esch_debug.h"

extern esch_log* g_testLog;

#define ESCH_TEST_CHECK(cond, msg, errorcode) \
    ESCH_CHECK(cond, g_testLog, msg, errorcode)

/* test cases */
int test_AllocCreateDeleteCDefault();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_UTEST_H_ */
