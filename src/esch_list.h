/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_LIST_H_
#define _ESCH_LIST_H_
#include "esch.h"
#include "esch_config.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct esch_list_node
{
    struct esch_list* owner;
    esch_object* data;
    esch_list_node* prev;
    esch_list_node* next;
};

/* 
 * This data structure, esch_list, is used to represent three data
 * structures in Scheme: pair, list, vector. It's implemented as a
 * dual-ended vector.
 *
 * Goals for time complexity
 * - Append operation: O(1)
 * - Prepend operation: O(1)
 * */
struct esch_list
{
    ESCH_COMMON_HEADER
    size_t length; /* Length of assigned nodes */
    esch_list_node* node_block_head;
    esch_list_node* first_node;
    esch_list_node* last_node;
};

#define ESCH_IS_VALID_LIST(obj) \
    (ESCH_GET_TYPE(obj) == ESCH_TYPE_LIST && \
     ESCH_IS_VALID_OBJECT(obj) && \
     ((esch_list*)obj)->node_block_head != NULL)

/*
 * To make memory allocation efficient, we create 16 nodes by default.
 * XXX The settings may be configurable in the furture.
 */
#define ESCH_LIST_NODES_PER_BLOCK 16

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_LIST_H_ */
