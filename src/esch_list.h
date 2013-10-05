/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_LIST_H_
#define _ESCH_LIST_H_
#include "esch.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_list_element
{
    struct esch_list* owner;
    void* object;
    int prev_index;
    int next_index;
};

struct esch_list
{
    ESCH_COMMON_HEADER;
    enum esch_type element_type;
    bool enforce_same_type;
    int size;
    esch_list_element** elements;
    esch_list_element* first;
    esch_list_element* last;
};

#define ESCH_IS_VALID_LIST(obj) \
    (ESCH_GET_TYPE(obj) == ESCH_TYPE_LIST && \
     ESCH_IS_VALID_OBJECT(OBJ) && \
     ((esch_list*)obj)->elements != NULL)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_LIST_H_ */
