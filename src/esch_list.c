#include <math.h>
#include <assert.h>
#include "esch.h"
#include "esch_list.h"
#include "esch_debug.h"
#include "esch_object.h"

/**
 * Create a new list.
 * @param config Configuration object to pass log and alloc.
 * @param list Newly create list.
 * @return Return code. ESCH_OK for success, others for errors.
 */
esch_error
esch_list_new(esch_config* config, size_t initial_length, esch_list** list)
{
    esch_error ret = ESCH_OK;
    esch_list* new_list = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    size_t node_idx = 0;
    size_t nblocks = 0;
    size_t blocks = 0;
    esch_list_node* new_block = NULL;

    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_INTERNAL_CONFIG_GET_ALLOC(config) != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_INTERNAL_CONFIG_GET_LOG(config) != NULL);
    ESCH_CHECK_PARAM_PUBLIC(list != NULL);

    alloc = ESCH_INTERNAL_CONFIG_GET_ALLOC(config);
    log = ESCH_INTERNAL_CONFIG_GET_LOG(config);
    ret = esch_alloc_malloc(alloc, sizeof(esch_list), (void**)&new_list);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc list", ret);
    ESCH_GET_VERSION(new_list) = ESCH_VERSION;
    ESCH_GET_TYPE(new_list) = ESCH_TYPE_LIST;
    ESCH_GET_ALLOC(new_list) = alloc;
    ESCH_GET_LOG(new_list) = log;
    new_list->length = 0; /* No element is really assigned */
    new_list->node_block_head = NULL;
    new_list->first_node = NULL; /* No node is allocated */
    new_list->last_node = NULL;

    if (initial_length > 0)
    {
        blocks = ceil(initial_length / ESCH_LIST_NODES_PER_BLOCK);
    }
    else
    {
        // Default blocks
        blocks = 1;
    }

    /* Now, create a linked list. */
    new_block = NULL;
    for(nblocks = 0; nblocks < blocks; ++nblocks)
    {
        ret = esch_alloc_malloc(alloc,
                ESCH_LIST_NODES_PER_BLOCK * sizeof(esch_list_node),
                (void*)(&new_block));
        ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc list nodes", ret);
        new_block[0].owner = new_list;
        new_block[0].data = NULL;
        new_block[0].prev = NULL;
        new_block[0].next = &new_block[1];
        for(node_idx = 1; node_idx < ESCH_LIST_NODES_PER_BLOCK - 1; ++node_idx)
        {
            new_block[node_idx].owner = new_list;
            new_block[node_idx].data = NULL;
            new_block[node_idx].prev = &new_block[node_idx - 1];
            new_block[node_idx].next = &new_block[node_idx + 1];
        }
        new_block[node_idx].owner = new_list;
        new_block[node_idx].data = NULL;
        new_block[node_idx].prev = &new_block[node_idx -1];
        /* Add to linked list. */
        new_block[node_idx].next = new_list->node_block_head;
        new_list->node_block_head = new_block;
    }

    (*list) = new_list;
    new_list = NULL;
Exit:
    if (new_list)
    {
        esch_list_delete(new_list, ESCH_FALSE);
    }
    return ret;
}

/**
 * Delete a list.
 * @param list List to be deleted.
 * @param delete_data Determine if data should be deleted as well.
 * @return Return code.
 */
esch_error
esch_list_delete(esch_list* list, int delete_data)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    esch_list_node* node_head = NULL;
    esch_list_node* next_head = NULL;
    size_t block_idx = 0;
    if (list == NULL)
    {
        return ret;
    }
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LIST(list));
    alloc = ESCH_GET_ALLOC(list);
    node_head = list->node_block_head;
    while(node_head != NULL)
    {
        next_head = node_head[ESCH_LIST_NODES_PER_BLOCK - 1].next;
        if (delete_data)
        {
            for(block_idx = 0;
                    block_idx < ESCH_LIST_NODES_PER_BLOCK; ++block_idx)
            {
                (void)esch_object_delete(node_head[block_idx].data,
                                         delete_data);
            }
        }
        esch_alloc_free(alloc, node_head);
        node_head = next_head;
    }
    esch_alloc_free(alloc, list);
Exit:
    return ret;
}

/**
 * Get element count of specified list.
 * @param list Given list.
 * @param size Size of list. 
 * @return Return code. ESCH_OK for success, others for errors.
 */
esch_error
esch_list_get_length(esch_list* list, size_t* length)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(list != NULL);
    ESCH_CHECK_PARAM_PUBLIC(length != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LIST(list));
    (*length) = list->length;
Exit:
    return ret;
}

/**
 * Get data held in list, by index.
 * @param list Given list.
 * @param index Index of object. If negative is given, search from tail.
 * @data Returned data.
 * @return Return code. ESCH_OK for success, others for errors.
 */
esch_error
esch_list_get_data(esch_list* list, int index, esch_object** data)
{
    esch_error ret = ESCH_OK;
    esch_list_node* node = NULL;
    ret = esch_list_get_node(list, index, &node);
    if (ret == ESCH_OK)
    {
        (*data) = (esch_object*)(node->data);
    }
    return ret;
}

/**
 * Get data held in list, by index.
 * @param list Given list.
 * @param index Index of object. If negative is given, search from tail.
 * @node Returned data node. No need to free.
 * @return Return code. ESCH_OK for success, others for errors.
 */
esch_error
esch_list_get_node(esch_list* list, int index, esch_list_node** node)
{
    esch_error ret = ESCH_OK;
    esch_list_node* returned_node = NULL;
    ESCH_CHECK_PARAM_PUBLIC(list != NULL);
    ESCH_CHECK_PARAM_PUBLIC(index >= 0);
    ESCH_CHECK_PARAM_PUBLIC(node != NULL);

    /* Search from head. */
    ESCH_CHECK_2((size_t)index >= list->length, list,
                 "List index overflow, length < %d, get %d",
                 list->length, index, ESCH_ERROR_OUT_OF_BOUND);
    /* As deleting element is not allowed in esch_list, a rule is
     * implicitly applied: first_node always points to the first
     * buffer. */
    if ((size_t)index + (list->first_node - list->node_block_head)
            < ESCH_LIST_NODES_PER_BLOCK)
    {
        returned_node = list->node_block_head + index;
    }
    else
    {
        esch_list_node* iter = NULL;
        size_t i, j = 0;
        size_t adjusted_index = 0;
        adjusted_index = index - (ESCH_LIST_NODES_PER_BLOCK - 
                         (list->first_node - list->node_block_head)) + 1;
        i = adjusted_index / ESCH_LIST_NODES_PER_BLOCK;
        j = adjusted_index % ESCH_LIST_NODES_PER_BLOCK;
        iter = &(list->node_block_head[ESCH_LIST_NODES_PER_BLOCK - 1]);
        --i;
        while (i != 0)
        {
            iter = &(iter[ESCH_LIST_NODES_PER_BLOCK - 1]);
            assert(iter != NULL);
            --i;
        }
        returned_node = &(iter[j]);
    }
    (*node) = returned_node;
Exit:
    return ret;
}

/**
 * Append a new element to the tail of list.
 * @param list Given esch_list object.
 * @param data A data object to insert.
 * @return Return code. ESCH_OK for success, others for errors.
 */
esch_error
esch_list_append(esch_list* list, esch_object* data)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(list != NULL);
    ESCH_CHECK_PARAM_PUBLIC(data != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_OBJECT(data));

    if (list->last_node == NULL)
    {
        /* No node is allocated. */
        assert(list->node_block_head != NULL);
        assert(list->first_node == NULL);
    }
    else
    {
        /* Append a new node. */
    }
Exit:
    return ret;
}
