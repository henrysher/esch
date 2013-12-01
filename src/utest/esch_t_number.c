#include <stdio.h>
#include "esch_utest.h"
#include "esch_debug.h"
#include "esch_number.h"

esch_error test_integer()
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_parser* parser = NULL;
    esch_config* config = NULL;
    esch_integer* val = NULL;
    esch_integer* bigval1 = NULL;
    esch_integer* bigval2 = NULL;
    esch_log* log = NULL;
    esch_log* do_nothing = NULL;
    unsigned char bval_array[] = {
        0x56, 0x34, 0x12, 0x90, 0x78, 0x56, 0x34, 0x12, 0x9
    };

    (void)esch_log_new_do_nothing(&do_nothing);
#ifdef NDEBUG
    log = do_nothing;
#else
    log = g_testLog;
#endif

    ret = esch_config_new(&config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create config", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:log", ret);

    ret = esch_alloc_new_c_default(config, &alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create alloc", ret);

    /* The config validation is enabled only in debug build  */

    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_ALLOC, (esch_object*)alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:alloc:obj", ret);
    ret = esch_config_set_obj(config, ESCH_CONFIG_KEY_LOG, (esch_object*)log);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to set config:log:obj", ret);

    /* Regular integer */
    ret = esch_integer_new_from_base10(config, "765", NULL, &val);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create int", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(!val->big, "Bad big integer detection: 765", ret);
    esch_log_info(log, "Integer: Converted int = %d, expected = 765",
                  val->value.ival);
    ESCH_TEST_CHECK(val->value.ival == 765, "Bad integer value: 765", ret);
    ret = ESCH_OK;

    /* Big integer 1 */
    ret = esch_integer_new_from_base10(config, "91234567890123456",
                                       NULL, &bigval1);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create int", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(bigval1->big,
                    "Bad big int detection: 91234567890123456", ret);
    esch_log_info(log, "Integer: Converted length = %d, expected = 9",
                  bigval1->value.bval.length);
    ESCH_TEST_CHECK(bigval1->value.bval.length == 9,
                    "Bad big int length: 91234567890123456", ret);
    esch_log_info(log, "Big int storage: %x,%x,%x,%x,%x,%x,%x,%x,%x",
            bigval1->value.bval.digit[0],
            bigval1->value.bval.digit[1],
            bigval1->value.bval.digit[2],
            bigval1->value.bval.digit[3],
            bigval1->value.bval.digit[4],
            bigval1->value.bval.digit[5],
            bigval1->value.bval.digit[6],
            bigval1->value.bval.digit[7],
            bigval1->value.bval.digit[8]);
    ESCH_TEST_CHECK(strncmp(bigval1->value.bval.digit, bval_array, 9) == 0,
                    "Bad integer storage", ret);
    ret = ESCH_OK;

    /* Big integer 2 */
    ret = esch_integer_new_from_base10(config, "1234567890123456", NULL, &bigval2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to create int", ret);
    ret = ESCH_ERROR_INVALID_STATE;
    ESCH_TEST_CHECK(bigval2->big, "Bad big int detection: 1234567890123456", ret);
    esch_log_info(log, "Integer: Converted length = %d, expected = 8",
                  bigval2->value.bval.length);
    ESCH_TEST_CHECK(bigval2->value.bval.length == 8,
                    "Bad big int length: 1234567890123456", ret);
    esch_log_info(log, "Big int storage: %x,%x,%x,%x,%x,%x,%x,%x",
            bigval2->value.bval.digit[0],
            bigval2->value.bval.digit[1],
            bigval2->value.bval.digit[2],
            bigval2->value.bval.digit[3],
            bigval2->value.bval.digit[4],
            bigval2->value.bval.digit[5],
            bigval2->value.bval.digit[6],
            bigval2->value.bval.digit[7]);
    ESCH_TEST_CHECK(strncmp(bigval2->value.bval.digit, bval_array, 8) == 0,
                    "Bad integer storage", ret);
    ret = ESCH_OK;

    ret = esch_integer_delete(val);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete int", ret);

    ret = esch_integer_delete(bigval1);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete int", ret);

    ret = esch_integer_delete(bigval2);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete int", ret);

    ret = esch_alloc_delete(alloc);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete alloc object.", ret);

    ret = esch_config_delete(config);
    ESCH_TEST_CHECK(ret == ESCH_OK, "Failed to delete config object.", ret);
Exit:
    return ret;
}
