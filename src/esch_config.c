#include "esch.h"
#include "esch_debug.h"
#include "esch_config.h"
#include "esch_log.h"
#include <assert.h>
#include <string.h>

const char* ESCH_CONFIG_KEY_ALLOC = "common:alloc";
const char* ESCH_CONFIG_KEY_LOG = "common:log";
const char* ESCH_CONFIG_KEY_GC = "common:gc";
const char* ESCH_CONFIG_KEY_VECTOR_INITIAL_LENGTH = "vector:initial_length";
const char* ESCH_CONFIG_KEY_GC_NAIVE_INITIAL_CELLS = "gc:naive:initial_cells";

static esch_error esch_config_destructor(esch_object* obj);
static esch_error esch_config_new_as_object(esch_config*, esch_object** obj);

struct esch_builtin_type esch_config_type =
{
    {
        &(esch_meta_type.type),
        NULL,
        &(esch_log_do_nothing.log),
        NULL,
        NULL,
    },
    {
        ESCH_VERSION,
        sizeof(esch_config),
        esch_config_new_as_object,
        esch_config_destructor,
        esch_type_default_non_copiable,
        esch_type_default_no_string_form,
        esch_type_default_no_doc,
        esch_type_default_no_iterator
    }
};

/**
 * Create a new config object.
 * @param config Returned config object.
 * @return error code. ESCH_OK if success, others on error.
 */
esch_error
esch_config_new(esch_log* log, esch_alloc* alloc, esch_config** config)
{
    esch_error ret = ESCH_OK;
    esch_object* new_obj = NULL;
    esch_config* new_config = NULL;
    size_t size = 0;
    ESCH_CHECK_PARAM_PUBLIC(log != NULL);
    ESCH_CHECK_PARAM_PUBLIC(alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);

    size = sizeof(esch_object) + sizeof(esch_config);
    ret = esch_alloc_realloc(alloc, NULL, size, (void**)&new_obj);
    ESCH_CHECK(ret == ESCH_OK, esch_global_log,
            "Can't create config", ESCH_ERROR_INVALID_PARAMETER);
    new_config = ESCH_CAST_FROM_OBJECT(new_obj, esch_config);

    ESCH_OBJECT_GET_TYPE(new_obj)    = &(esch_config_type.type);
    ESCH_OBJECT_GET_ALLOC(new_obj)   = alloc;
    ESCH_OBJECT_GET_LOG(new_obj)     = log;
    ESCH_OBJECT_GET_GC(new_obj)      = NULL;
    ESCH_OBJECT_GET_GC_ID(new_obj)   = NULL; /* Can't get managed. */

    /* Fill preset keys */
    strncpy(new_config->config[0].key,
            ESCH_CONFIG_KEY_ALLOC, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[0].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;
    strncpy(new_config->config[1].key,
            ESCH_CONFIG_KEY_LOG, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[1].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;
    strncpy(new_config->config[2].key,
            ESCH_CONFIG_KEY_GC, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[2].type = ESCH_CONFIG_VALUE_TYPE_OBJECT;

    strncpy(new_config->config[3].key,
            ESCH_CONFIG_KEY_VECTOR_INITIAL_LENGTH, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[3].type = ESCH_CONFIG_VALUE_TYPE_INTEGER;
    new_config->config[3].data.int_value = 1;

    strncpy(new_config->config[4].key,
            ESCH_CONFIG_KEY_GC_NAIVE_INITIAL_CELLS, ESCH_CONFIG_KEY_LENGTH);
    new_config->config[4].type = ESCH_CONFIG_VALUE_TYPE_INTEGER;
    new_config->config[4].data.int_value = 4096;

    (*config) = new_config;
    new_config = NULL;
Exit:
    if (new_config != NULL)
    {
        (void)esch_alloc_free(alloc, (void*)new_obj);
    }
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

static esch_error
esch_config_new_as_object(esch_config* config, esch_object** obj)
{
    esch_error ret = ESCH_OK;
    esch_config* new_config = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    esch_object* alloc_obj = NULL;
    esch_object* log_obj = NULL;

    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    log_obj = ESCH_CONFIG_GET_LOG(config);
    alloc_obj = ESCH_CONFIG_GET_ALLOC(config);
    ESCH_CHECK_PARAM_PUBLIC(log_obj != NULL);
    ESCH_CHECK_PARAM_PUBLIC(alloc_obj != NULL);
    log = ESCH_CAST_FROM_OBJECT(log_obj, esch_log);
    alloc = ESCH_CAST_FROM_OBJECT(alloc_obj, esch_alloc);

    ret = esch_config_new(log, alloc, &new_config);
    if (ret == ESCH_OK)
    {
        (*obj) = ESCH_CAST_TO_OBJECT(new_config);
    }
Exit:
    return ret;
}

static esch_error
esch_config_destructor(esch_object* obj)
{
    (void)obj;
    /* Just do nothing */
    return ESCH_OK;
}

