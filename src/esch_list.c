#include "esch.h"

esch_error esch_list_new(esch_config* config, int enforce_same_type,
                         esch_list** lst);
esch_error esch_list_delete(esch_list** lst);
esch_error esch_list_get_size(esch_list** lst, int** size);
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

