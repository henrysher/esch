/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_TYPE_H_
#define _ESCH_TYPE_H_

#include "esch.h"
#include "esch_object.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * The public type info structure. This is the header of all structures
 * used in Esch. All objects can be casted to esch_object to get type
 * information.
 */
struct esch_type
{
    unsigned int               version;
    size_t                     object_size;
    esch_object_new_f          object_new;
    esch_object_delete_f       object_delete;
    esch_object_copy_f         object_copy;
    esch_object_to_string_f    object_to_string;
    esch_object_get_doc_f      object_get_doc;
    esch_object_get_iterator_f object_get_iterator;
};

/*
 * This structure is to help built-in data type create they own static
 * types by manual creation.
 */
struct esch_builtin_type
{
    esch_object header;
    esch_type   type;
};

extern esch_type esch_meta_type;
#define ESCH_TYPE_GET_VERSION(ti) ((ti)->version)
#define ESCH_TYPE_GET_OBJECT_SIZE(ti) ((ti)->object_size)
#define ESCH_TYPE_GET_OBJECT_NEW(ti) ((ti)->object_new)
#define ESCH_TYPE_GET_OBJECT_DELETE(ti) ((ti)->object_delete)
#define ESCH_TYPE_GET_OBJECT_COPY(ti) ((ti)->object_copy)
#define ESCH_TYPE_GET_OBJECT_TO_STRING(ti) ((ti)->object_to_string)
#define ESCH_TYPE_GET_OBJECT_GET_DOC(ti) ((ti)->object_get_doc)
#define ESCH_TYPE_GET_OBJECT_GET_ITERATOR(ti) ((ti)->object_get_iterator)

#define ESCH_IS_VALID_TYPE(ti) \
    (((ti)->version == ESCH_VERSION) && \
     (ESCH_OBJECT_GET_TYPE(ESCH_CAST_TO_OBJECT(ti)) == &esch_meta_type) && \
     ((ti)->object_size > sizeof(esch_object)) && \
     ((ti)->object_new != NULL) && \
     ((ti)->object_delete != NULL) && \
     ((ti)->object_copy != NULL) && \
     ((ti)->object_to_string != NULL) && \
     ((ti)->object_get_doc != NULL))

/* ----------------------------------------------------------------- */
/*                       Internal functions                          */
/* ----------------------------------------------------------------- */
/**
 * Default copy constructor.
 * @return Always return ESCH_ERROR_NOT_SUPPORTED.
 */
esch_error esch_type_default_non_copiable(esch_object*, esch_object**);
/**
 * Default string form convertor.
 * @return Always return ESCH_ERROR_NOT_SUPPORTED.
 */
esch_error esch_type_default_no_string_form(esch_object*, esch_string**);
/**
 * Default doc string generator.
 * @return Always return ESCH_ERROR_NOT_SUPPORTED.
 */
esch_error esch_type_default_no_doc(esch_object*, esch_string**);

/**
 * Default iterator retrieving function.
 * @return Always return ESCH_ERROR_NOT_SUPPORTED
 */
esch_error esch_type_default_no_iterator(esch_object*, esch_iterator*);

esch_error esch_type_new_i(esch_config* config, esch_type** type);
esch_error esch_type_delete_i(esch_type* type);

#define ESCH_TYPE_IS_PRIMITIVE(ti) \
    ((ti)->object_get_iterator == esch_type_default_no_iterator)
#define ESCH_TYPE_IS_CONTAINER(ti) \
    ((ti)->object_get_iterator != esch_type_default_no_iterator)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_TYPE_H_ */

