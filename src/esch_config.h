/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_CONFIG_H_
#define _ESCH_CONFIG_H_
#include "esch.h"
#include "esch_object.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ESCH_CONFIG_KEY_LENGTH 32
#define ESCH_CONFIG_VALUE_STRING_LENGTH 255
#define ESCH_CONFIG_ITEMS 2
struct esch_config
{
    ESCH_COMMON_HEADER;
    struct
    {
        char key[ESCH_CONFIG_KEY_LENGTH + 1];
        union
        {
            int int_value;
            char str_value[ESCH_CONFIG_VALUE_STRING_LENGTH + 1];
            void* ptr_value;
        } data;
    } config[ESCH_CONFIG_ITEMS];
};

#define ESCH_IS_VALID_CONFIG(obj) \
    (((ESCH_GET_TYPE(obj) == ESCH_TYPE_CONFIG)) \
     && \
     ESCH_GET_ALLOC(obj) != NULL && \
     ESCH_GET_LOG(obj) != NULL && \
     ESCH_GET_ALLOC(obj) == (obj))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_CONFIG_H_ */

