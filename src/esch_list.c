#include "esch.h"
#include "esch_list.h"
#include "esch_debug.h"

/**
 * Create a new list.
 * @param config Basic configuration.
 * @param lst Newly create list.
 * @return Return code.
 */
esch_error
esch_list_new(esch_config* config, esch_list** lst)
{
    esch_error ret = ESCH_OK;
    esch_list* new_lst = NULL;
    esch_alloc* alloc = NULL;
    esch_log* log = NULL;
    int idx = 0;
    ESCH_CHECK_PARAM_PUBLIC(config != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config->alloc != NULL);
    ESCH_CHECK_PARAM_PUBLIC(config->log != NULL);
    ESCH_CHECK_PARAM_PUBLIC(lst != NULL);

    alloc = config->alloc;
    log = config->log;
    ret = esch_alloc_malloc(alloc, sizeof(esch_list), (void**)&new_lst);
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc list", ret);
    ESCH_GET_TYPE(new_lst) = ESCH_TYPE_LIST;
    ESCH_GET_ALLOC(new_lst) = alloc;
    ESCH_GET_LOG(new_lst) = log;
    new_lst->slots = ESCH_LIST_INITIAL_ELEMENTS;
    new_lst->size = 0;
    new_lst->first = NULL;
    new_lst->last = NULL;

    ret = esch_alloc_malloc(alloc,
            ESCH_LIST_INITIAL_ELEMENTS * sizeof(esch_list_element),
            (void**)&(new_lst->elements));
    ESCH_CHECK(ret == ESCH_OK, log, "Can't malloc list elements", ret);
    for(idx = 0; idx < new_lst->slots; ++idx)
    {
        new_lst->elements[idx].owner = new_lst;
        new_lst->elements[idx].data = NULL;
        new_lst->elements[idx].prev_index = 0;
        new_lst->elements[idx].next_index = 0;
    }
    (*lst) = new_lst;
    new_lst = NULL;
Exit:
    if (new_lst)
    {
        esch_list_delete(new_lst);
    }
    return ret;
}

/**
 * Delete a list.
 * @param lst List to be deleted.
 * @return Return code.
 */
esch_error
esch_list_delete(esch_list* lst)
{
    esch_error ret = ESCH_OK;
    esch_alloc* alloc = NULL;
    if (lst == NULL)
    {
        return ret;
    }
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LIST(lst));
    alloc = ESCH_GET_ALLOC(lst);
    esch_alloc_free(alloc, lst->elements);
    esch_alloc_free(alloc, lst);
Exit:
    return ret;
}
/**
 * Get element count of specified list.
 * @param lst Given list.
 * @param size Size of list. 
 */
esch_error
esch_list_get_size(esch_list* lst, size_t* size)
{
    esch_error ret = ESCH_OK;
    ESCH_CHECK_PARAM_PUBLIC(lst != NULL);
    ESCH_CHECK_PARAM_PUBLIC(size != NULL);
    ESCH_CHECK_PARAM_PUBLIC(ESCH_IS_VALID_LIST(lst));
    (*size) = lst->size;
Exit:
    return ret;
}
esch_error esch_list_get_first(esch_list** lst, esch_list_element** first);
esch_error esch_list_get_by_index(esch_list** lst, int index,
                                  esch_list_element** first);
esch_error esch_list_insert_element(esch_list_element node,
                                    void* element,
                                    esch_list_element** new_element);
esch_error esch_list_insert_element_array(esch_list** lst,
                                          void** data, int size,
                                          esch_list_element** node);
esch_error esch_list_get_next(esch_list_element* node,
                              esch_list_element** next);
esch_error esch_list_get_prev(esch_list_element* node, 
                              esch_list_element** prev);
esch_error esch_list_remove_element(esch_list_element* element);

