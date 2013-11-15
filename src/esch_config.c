#include "esch.h"
#include "esch_debug.h"
#include "esch_config.h"
#include <assert.h>
#include <string.h>

const char* ESCH_CONFIG_ALLOC_KEY = "config:alloc";
const char* ESCH_CONFIG_LOG_KEY = "config:log";

/**
 * Create a new config object.
 * @param alloc Alloc object to create new object.
 * @param config Returned config object.
 * @return error code. ESCH_OK if success, others on error.
 */
esch_error
esch_config_new(esch_alloc* alloc, esch_config** config)
{
    esch_error ret = ESCH_OK;
    esch_config* new_config = NULL;
    esch_log* do_nothing_log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);

    ret = esch_alloc_malloc(alloc, sizeof(esch_config), (void**)&new_config);
    ESCH_CHECK(ret == ESCH_OK, alloc, "Can't malloc config", ret);
    memset(new_config, 0, sizeof(esch_config));
    ESCH_GET_TYPE(new_config) = ESCH_TYPE_CONFIG;
    ESCH_GET_ALLOC(new_config) = alloc;
    ret = esch_log_new_do_nothing(&do_nothing_log);
    ESCH_CHECK(ret == ESCH_OK, alloc, "Can't create do nothing log", ret);
    ESCH_GET_LOG(new_config) = do_nothing_log;

    // Fill key in advance
    strncpy(new_config->config[0].key,
            ESCH_CONFIG_ALLOC_KEY, ESCH_CONFIG_KEY_LENGTH);
    strncpy(new_config->config[1].key,
            ESCH_CONFIG_LOG_KEY, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[0].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;
    new_config->config[1].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;

    (*config) = new_config;
    new_config = NULL;
Exit:
    if (new_config != NULL)
    {
        esch_config_delete(new_config);
    }
    return ret;
}

/**
 * Delete a given config object.
 * @param config The config object to free.
 * @return error code. ESCH_OK if success, others on error.
 */
esch_error
esch_config_delete(esch_config* config)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    alloc = ESCH_GET_ALLOC(config);
    assert(alloc != NULL);
    ret = esch_alloc_free(alloc, config);
    ESCH_CHECK(ret == ESCH_OK, alloc, "Can't free config", ret);
Exit:
    return ret;
}

esch_error
esch_config_get_int(esch_config* config, const char* key, int* value)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_config_set_int(esch_config* config, const char* key, int value)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_config_get_str(esch_config* config, const char* key, char** value)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_config_set_str(esch_config* config, const char* key, char* value)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_config_get_data(esch_config* config, const char* key, void* data)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}
esch_error
esch_config_set_data(esch_config* config, const char* key, void* data)
{
    return ESCH_ERROR_NOT_IMPLEMENTED;
}

