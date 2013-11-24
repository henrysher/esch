#include "esch.h"
#include "esch_debug.h"
#include "esch_config.h"
#include "esch_log.h"
#include <assert.h>
#include <string.h>

const char* ESCH_CONFIG_KEY_ALLOC = "common:alloc";
const char* ESCH_CONFIG_KEY_LOG = "common:log";
const char* ESCH_CONFIG_KEY_VECTOR_ELEMENT_TYPE = "vector:element_type";
const char* ESCH_CONFIG_KEY_VECTOR_INITIAL_LENGTH = "vector:initial_length";
const char* ESCH_CONFIG_KEY_VECTOR_DELETE_ELEMENT = "vecotr:delete_element";

/**
 * Create a new config object.
 * @param config Returned config object.
 * @return error code. ESCH_OK if success, others on error.
 */
esch_error
esch_config_new(esch_config** config)
{
    esch_error ret = ESCH_OK;
    esch_config* new_config = NULL;
    esch_log* do_nothing_log = NULL;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);

    new_config = (esch_config*)malloc(sizeof(esch_config));
    ESCH_CHECK(new_config != NULL, esch_global_log,
            "Can't malloc config", ESCH_ERROR_INVALID_PARAMETER);
    memset(new_config, 0, sizeof(esch_config));
    ESCH_GET_VERSION(new_config) = ESCH_VERSION;
    ESCH_GET_TYPE(new_config) = ESCH_TYPE_CONFIG;
    ESCH_GET_ALLOC(new_config) = &esch_dummy_alloc;
    ret = esch_log_new_do_nothing(&do_nothing_log);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log, "Can't create dummy log", ret);
    ESCH_GET_LOG(new_config) = do_nothing_log;

    // Fill key in advance
    strncpy(new_config->config[0].key,
            ESCH_CONFIG_KEY_ALLOC, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[0].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;
    strncpy(new_config->config[1].key,
            ESCH_CONFIG_KEY_LOG, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[1].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;
    strncpy(new_config->config[2].key,
            ESCH_CONFIG_KEY_VECTOR_ELEMENT_TYPE, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[2].type = ESCH_CONFIG_VALUE_TYPE_INTEGER;
    new_config->config[2].data.int_value = ESCH_TYPE_UNKNOWN;
    strncpy(new_config->config[3].key,
            ESCH_CONFIG_KEY_VECTOR_INITIAL_LENGTH, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[3].type = ESCH_CONFIG_VALUE_TYPE_INTEGER;
    new_config->config[3].data.int_value = 1;
    strncpy(new_config->config[4].key,
            ESCH_CONFIG_KEY_VECTOR_DELETE_ELEMENT, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[4].type = ESCH_CONFIG_VALUE_TYPE_INTEGER;
    new_config->config[4].data.int_value = ESCH_FALSE;


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
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_CONFIG(config));
    free(config);
Exit:
    return ret;
}

/**
 * Get an integer value in configuration.
 * @param config Given config object.
 * @param key Given key of configuration.
 * @param value Returned integer value.
 * @return Error code. ESCH_ERROR_NOT_FOUND if not found.
 */
esch_error
esch_config_get_int(esch_config* config, const char* key, int* value)
{
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(key != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value != NULL);

    ret = ESCH_ERROR_NOT_FOUND;
    for (i = 0; i < ESCH_CONFIG_ITEMS; ++i)
    {
        if (strcmp(config->config[i].key, key) == 0
                && config->config[i].type == ESCH_CONFIG_VALUE_TYPE_INTEGER)
        {
            (*value) = config->config[i].data.int_value;
            ret = ESCH_OK;
            break;
        }
    }
Exit:
    return ret;
}

/**
 * Set a new integer to given key.
 * @param config Given config object.
 * @param key String key.
 * @param obj New integer value.
 * @return Return code. ESCH_ERROR_NOT_FOUND if key is not found.
 */
esch_error
esch_config_set_int(esch_config* config, const char* key, int value)
{
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(key != NULL);
    ret = ESCH_ERROR_NOT_FOUND;
    for (i = 0; i < ESCH_CONFIG_ITEMS; ++i)
    {
        if (strcmp(config->config[i].key, key) == 0
                && config->config[i].type == ESCH_CONFIG_VALUE_TYPE_INTEGER)
        {
            config->config[i].data.int_value = value;
            ret = ESCH_OK;
            break;
        }
    }
Exit:
    return ret;
}
/**
 * Get a string value in configuration.
 * @param config Given config object.
 * @param key Given key of configuration.
 * @param value Returned integer value.
 * @return Error code. ESCH_ERROR_NOT_FOUND if not found.
 */
esch_error
esch_config_get_str(esch_config* config, const char* key, char** value)
{
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(key != NULL);
    ESCH_CHECK_PARAM_PUBLIC(value != NULL);

    ret = ESCH_ERROR_NOT_FOUND;
    for (i = 0; i < ESCH_CONFIG_ITEMS; ++i)
    {
        if (strcmp(config->config[i].key, key) == 0
                && config->config[i].type == ESCH_CONFIG_VALUE_TYPE_STRING)
        {
            (*value) = config->config[i].data.str_value;
            ret = ESCH_OK;
            break;
        }
    }
Exit:
    return ret;
}

/**
 * Set a new string to given key.
 * @param config Given config object.
 * @param key String key.
 * @param obj New string. Can be NULL.
 * @return Return code. ESCH_ERROR_NOT_FOUND if key is not found.
 */
esch_error
esch_config_set_str(esch_config* config, const char* key, char* value)
{
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(key != NULL);
    ret = ESCH_ERROR_NOT_FOUND;
    for (i = 0; i < ESCH_CONFIG_ITEMS; ++i)
    {
        if (strcmp(config->config[i].key, key) == 0
                && config->config[i].type == ESCH_CONFIG_VALUE_TYPE_STRING)
        {
            strncmp(config->config[i].data.str_value,
                    value,
                    ESCH_CONFIG_VALUE_STRING_LENGTH);
            ret = ESCH_OK;
            break;
        }
    }
Exit:
    return ret;

}
/**
 * Get an esch_object value in configuration.
 * @param config Given config object.
 * @param key Given key of configuration.
 * @param value Returned integer value.
 * @return Error code. ESCH_ERROR_NOT_FOUND if not found.
 */
esch_error
esch_config_get_obj(esch_config* config, const char* key, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(key != NULL);
    ESCH_CHECK_PARAM_PUBLIC(obj != NULL);

    ret = ESCH_ERROR_NOT_FOUND;
    for (i = 0; i < ESCH_CONFIG_ITEMS; ++i)
    {
        if (strcmp(config->config[i].key, key) == 0
                && config->config[i].type == ESCH_CONFIG_VALUE_TYPE_OBJECT)
        {
            (*obj) = config->config[i].data.obj_value;
            ret = ESCH_OK;
            break;
        }
    }
Exit:
    return ret;
}

/**
 * Set a new object to given key.
 * @param config Given config object.
 * @param key String key.
 * @param obj New object. Can be NULL.
 * @return Return code. ESCH_ERROR_NOT_FOUND if key is not found.
 */
esch_error
esch_config_set_obj(esch_config* config, const char* key, esch_object* obj)
{
    esch_error ret = ESCH_OK;
    size_t i = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(key != NULL);
    ret = ESCH_ERROR_NOT_FOUND;
    for (i = 0; i < ESCH_CONFIG_ITEMS; ++i)
    {
        if (strcmp(config->config[i].key, key) == 0
                && config->config[i].type == ESCH_CONFIG_VALUE_TYPE_OBJECT)
        {
            config->config[i].data.obj_value = obj;
            ret = ESCH_OK;
            break;
        }
    }
Exit:
    return ret;
}

