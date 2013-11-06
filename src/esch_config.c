#include "esch.h"
#include "esch_debug.h"
#include "esch_config.h"

const char* ESCH_CONFIG_ALLOC_KEY = "config:alloc";
const char* ESCH_CONFIG_LOG_KEY = "config:log";

esch_error
esch_config_new(esch_alloc* alloc, esch_config** config)
{
    esch_error ret = ESCH_OK;
    esch_config* new_config = NULL;
    esch_log* do_nothing_log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);

    ret = esch_alloc_malloc(alloc, sizeof(esch_config), &new_config);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc config", ret);
    bzero(new_config, sizeof(esch_config));
    ESCH_GET_TYPE(new_config) = ESCH_TYPE_CONFIG;
    ESCH_GET_ALLOC(new_config) = alloc;
    ret = esch_log_new_do_nothing(&do_nothing_log);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't create do nothing log", ret);
    ESCH_GET_LOG(new_config) = do_nothing_log;

    // Fill key in advance
    strncpy(new_config->config[0].key,
            ESCH_CONFIG_ALLOC_KEY, ESCH_CONFIG_KEY_LENGTH);
    strncpy(new_config->config[1].key,
            ESCH_CONFIG_LOG_KEY, ESCH_CONFIG_KEY_LENGTH);

    (*config) = new_config;
    new_config = NULL;
Exit:
    if (new_config != NULL)
    {
        esch_config_delete(new_config);
    }
    return ret;
}
esch_error
esch_config_delete(esch_config* config);
esch_error
esch_config_get_int(esch_config* config, const char* key, int* value);
esch_error
esch_config_set_int(esch_config* config, const char* key, int value);
esch_error
esch_config_get_str(esch_config* config, const char* key, char** value);
esch_error
esch_config_set_str(esch_config* config, const char* key, char* value);
esch_error
esch_config_get_data(esch_config* config, const char* key, void* data);
esch_error
esch_config_set_data(esch_config* config, const char* key, void* data);

