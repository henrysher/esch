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
extern esch_error test_AllocCreateDeleteCDefault(esch_config* config);
extern esch_error test_string(esch_config* config);
extern esch_error test_identifier();
extern esch_error test_config(esch_config* config);
extern esch_error test_vectorBase(esch_config* config);
extern esch_error test_vectorElementType(esch_config* config);
extern esch_error test_vectorIteration(esch_config* config);
extern esch_error test_vectorResizeFlag(esch_config* config);
extern esch_error test_vectorDifferentValues(esch_config* config);
extern esch_error test_integer();
extern esch_error test_gcCreateDelete(esch_config* config);
extern esch_error test_gcRecycleLogic(esch_config* config);
extern esch_error test_gcNoExpand(esch_config* config);
extern esch_error test_gcExpand(esch_config* config);
extern esch_error test_pairBase(esch_config* config);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_UTEST_H_ */
