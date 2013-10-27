#include "esch.h"

typedef esch_error (*esch_object_delete_func)(esch_object*);

static esch_object_delete_func
primitive_delete_func_table[] =
{
    NULL,
    (esch_object_delete_func)esch_string_delete, /* ESCH_TYPE_CHAR_AS_STRING */
    (esch_object_delete_func)esch_string_delete, /* ESCH_TYPE_STRING */
    (esch_object_delete_func)esch_string_delete, /* ESCH_TYPE_SYMBOL */
    NULL, /* ESCH_TYPE_NUMBER */
    NULL
};
static esch_object_delete_func
container_delete_func_table[] =
{
    NULL,
    (esch_object_delete_func)esch_list_delete, /* ESCH_TYPE_LIST */
    NULL
};

esch_error esch_object_delete(esch_object* data)
{
    esch_error ret = ESCH_OK;
    size_t idx = 0;
    if (data == NULL)
        return ret;

    if (!ESCH_IS_VALID_OBJECT(data))
    {
        return ESCH_ERROR_INVALID_PARAMETER;
    }
    else if (ESCH_IS_NO_DELETE(data))
    {
        return ESCH_ERROR_INVALID_PARAMETER;
    }
    else if (ESCH_IS_PRIMITIVE(data))
    {
        idx = (ESCH_GET_TYPE(data) & 0xF);
        return primitive_delete_func_table[idx](data);
    }
    else if (ESCH_IS_CONTAINER(data))
    {
        idx = (ESCH_GET_TYPE(data) & 0xF);
        return container_delete_func_table[idx](data);
    }
    else
    {
        return ESCH_ERROR_INVALID_PARAMETER;
    }
    return ret;
}