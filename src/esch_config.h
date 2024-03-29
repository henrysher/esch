/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_CONFIG_H_
#define _ESCH_CONFIG_H_

#include "esch.h"
#include "esch_object.h"
#include "esch_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ESCH_CONFIG_KEY_LENGTH 32
#define ESCH_CONFIG_VALUE_STRING_LENGTH 255
#define ESCH_CONFIG_ITEMS 8
struct esch_config
{
    /*
     * TODO We may consider changing to hashtable when configurable
     * items keep growing.
     */
    struct
    {
        esch_config_value_type type;
        char key[ESCH_CONFIG_KEY_LENGTH + 1];
        union
        {
            int int_value;
            char str_value[ESCH_CONFIG_VALUE_STRING_LENGTH + 1];
            esch_object* obj_value;
        } data;
    } config[ESCH_CONFIG_ITEMS];
};

extern struct esch_builtin_type esch_config_type;

#define ESCH_IS_VALID_CONFIG(cfg) \
    ((cfg) != NULL && \
     ESCH_IS_VALID_OBJECT(ESCH_CAST_TO_OBJECT(cfg)) && \
     (ESCH_OBJECT_GET_TYPE(ESCH_CAST_TO_OBJECT(cfg)) == \
          &(esch_config_type.type)))

#define ESCH_CONFIG_GET_ALLOC(cfg) \
    ((esch_object*)(cfg->config[0].data.obj_value))
#define ESCH_CONFIG_GET_LOG(cfg) \
    ((esch_object*)(cfg->config[1].data.obj_value))
#define ESCH_CONFIG_GET_GC(cfg) \
    ((esch_object*)(cfg->config[2].data.obj_value))
#define ESCH_CONFIG_GET_VECOTR_LENGTH(cfg) \
    ((int)(cfg->config[3].data.int_value))
#define ESCH_CONFIG_GET_VECTOR_ENLARGE(cfg) \
    ((int)(cfg->config[4].data.int_value))
#define ESCH_CONFIG_GET_GC_NAIVE_SLOTS(cfg) \
    ((int)(cfg->config[5].data.int_value))
#define ESCH_CONFIG_GET_GC_NAIVE_ROOT(cfg) \
    ((esch_object*)(cfg->config[6].data.obj_value))
#define ESCH_CONFIG_GET_GC_NAIVE_ENLARGE(cfg) \
    ((int)(cfg->config[7].data.int_value))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_CONFIG_H_ */

