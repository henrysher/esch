/* vim:ft=c expandtab tw=72 sw=4
 */
/*
 * +=================================================================+
 *
 * From: http://opensource.org/licenses/BSD-3-Clause 
 *
 * Copyright (c) 2013, Fuzhou Chen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Fuzhou Chen nor other names of the contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * +=================================================================+
 */
#ifndef _ESCH_ESCH_H_
#define _ESCH_ESCH_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * This is the only public include file for Esch interpreter. All
 * developers who write code to interact with esch interpreter must
 * include this header.
 */

#define ESCH_VERSION 1
#define ESCH_TYPE_MAGIC 0xEC00

#define ESCH_FALSE 0
#define ESCH_TRUE (~(ESCH_FALSE))

/*
 * The configuration keys are pre-defined within configuration. List:
 * - key = "common:alloc", value = esch_alloc
 * - key = "common:log", value = esch_log
 * - key = "common:gc", value = esch_gc
 * - key = "vector:value_type", value = enum_type
 * - key = "vector:length", value = int
 * - key = "gc:naive:slots", value = int
 * - key = "gc:naive:root", value = int
 */
extern const char* ESCH_CONFIG_KEY_ALLOC;
extern const char* ESCH_CONFIG_KEY_LOG;
extern const char* ESCH_CONFIG_KEY_GC;
extern const char* ESCH_CONFIG_KEY_VECTOR_ELEMENT_TYPE;
extern const char* ESCH_CONFIG_KEY_VECTOR_LENGTH;
extern const char* ESCH_CONFIG_KEY_VECTOR_ENLARGE;
extern const char* ESCH_CONFIG_KEY_GC_NAIVE_SLOTS;
extern const char* ESCH_CONFIG_KEY_GC_NAIVE_ROOT;
extern const char* ESCH_CONFIG_KEY_GC_NAIVE_ENLARGE;

typedef enum esch_error {
    ESCH_OK = 0,
    ESCH_ERROR_NOT_IMPLEMENTED,
    ESCH_ERROR_OUT_OF_MEMORY,
    ESCH_ERROR_INVALID_PARAMETER,
    ESCH_ERROR_INVALID_STATE,
    ESCH_ERROR_NOT_FOUND,
    ESCH_ERROR_BAD_VALUE_TYPE,
    ESCH_ERROR_OUT_OF_BOUND,
    ESCH_ERROR_NOT_SUPPORTED,
    ESCH_ERROR_DELETE_MANAGED_OBJECT,
    ESCH_ERROR_OBJECT_ALLOC_MISSING,
    ESCH_ERROR_OBJECT_LOG_MISSING,
    ESCH_ERROR_OBJECT_GC_MISSING,
    ESCH_ERROR_OBJECT_UNEXPECTED_GC_ATTACHED,
    ESCH_ERROR_GC_ROOT_MISSING,
    ESCH_ERROR_GC_ROOT_NOT_CONTAINER,
    ESCH_ERROR_CONTAINER_FULL,
} esch_error;

typedef enum esch_config_value_type {
    ESCH_CONFIG_VALUE_TYPE_UNKNOWN = 0,
    ESCH_CONFIG_VALUE_TYPE_INTEGER,
    ESCH_CONFIG_VALUE_TYPE_STRING,
    ESCH_CONFIG_VALUE_TYPE_OBJECT,
} esch_config_value_type;

/**
 * The data structure to allow contianer type define an iterator. It's
 * used to access element of container type.
 */
typedef enum esch_value_type
{
    ESCH_VALUE_TYPE_UNKNOWN = 0,
    ESCH_VALUE_TYPE_BYTE,
    ESCH_VALUE_TYPE_UNICODE,
    ESCH_VALUE_TYPE_INTEGER,
    ESCH_VALUE_TYPE_FLOAT,
    ESCH_VALUE_TYPE_OBJECT,
    ESCH_VALUE_TYPE_NIL,
    ESCH_VALUE_TYPE_END,
} esch_value_type;

/* Basic types */
typedef struct esch_type            esch_type;
typedef struct esch_object          esch_object;
typedef struct esch_iterator        esch_iterator;
typedef struct esch_value           esch_value;
typedef struct esch_config          esch_config;
typedef struct esch_alloc           esch_alloc;
typedef struct esch_log             esch_log;
typedef struct esch_gc              esch_gc;
typedef struct esch_parser          esch_parser;
typedef struct esch_parser_callback esch_parser_callback;
typedef struct esch_ast             esch_ast;
typedef struct esch_string          esch_string;
typedef struct esch_vector          esch_vector;
typedef struct esch_pair            esch_pair;
typedef char                        esch_utf8;
typedef int32_t                     esch_unicode;
typedef unsigned char               esch_bool;
typedef unsigned char               esch_byte;

/* Prototype of object/type methods */
typedef esch_error (*esch_object_new_f)(esch_config*, esch_object**);
typedef esch_error (*esch_object_destructor_f)(esch_object*);
typedef esch_error (*esch_object_copy_f)(esch_object*, esch_object**);
typedef esch_error (*esch_object_to_string_f)(esch_object*, esch_string**);
typedef esch_error (*esch_object_get_doc_f)(esch_object*, esch_string**);
typedef esch_error (*esch_object_get_iterator_f)(esch_object*,
                                                 esch_iterator*);
typedef esch_error (*esch_iterator_get_value_f)(esch_iterator*,
                                                esch_value*);
typedef esch_error (*esch_iterator_get_next_f)(esch_iterator*);
typedef esch_error (*esch_alloc_realloc_f)(esch_alloc*, void*, size_t,
                                           void**);
typedef esch_error (*esch_alloc_free_f)(esch_alloc*, void*);


/* ----------------------------------------------------------------- */
/*                        Type definition                            */
/* ----------------------------------------------------------------- */

/**
 * Create a new type.
 * @param config Given config object.
 * @param type Returned parameter. Created type.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_new(esch_config* config, esch_type** type);
/**
 * Set object size for given type.
 * @param Given type.
 * @param size Object size. The size must larger than sizeof(esch_object).
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_size(esch_type* type, size_t object_size);
/**
 * Set object creation method for given type.
 * @param Given type.
 * @param object_new Function pointer to create a new object.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_new(esch_type* type,
                                    esch_object_new_f object_new);
/**
 * Set object deleting method for given type.
 * @param Given type.
 * @param object_destructor Function pointer to delete an object.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_destructor(esch_type* type,
                           esch_object_destructor_f object_destructor);
/**
 * Set object copying method for given type.
 * @param Given type.
 * @param object_copy Function pointer to copy an object.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_copy(esch_type* type,
                                     esch_object_copy_f object_copy);
/**
 * Set string form conversion method for given type.
 * @param Given type.
 * @param object_to_string Function pointer to convert to string.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_to_string(esch_type* type,
                             esch_object_to_string_f object_to_string);
/**
 * Set document string generation method for given type.
 * @param Given type.
 * @param object_get_doc Function pointer to get document string.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_get_doc(esch_type* type,
                                 esch_object_get_doc_f object_get_doc);
/**
 * Set iterator generation method for given type.
 * @param Given type.
 * @param object_get_iterator Function pointer to get container iterator.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_set_object_get_iterator(esch_type* type,
                       esch_object_get_iterator_f object_get_iterator);
/**
 * Verify if given type is valid.
 * @param Given type.
 * @param valid Returned paramter. ESCH_TRUE if valid, ESCH_FALSE if not.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_type_is_valid_type(esch_type* type, esch_bool* valid);
/* ----------------------------------------------------------------- */
/*                        Object model                               */
/* ----------------------------------------------------------------- */
/**
 * Data structure to represent general value. Support different types
 * of values.
 */
struct esch_value
{
    enum esch_value_type type;
    union
    {
        esch_byte    b; /**< byte value */
        esch_unicode u; /**< unicode value */
        int          i; /**< integer value */
        double       f; /**< float value (implemented as double) */
        esch_object* o; /**< object value */
    } val;
};

struct esch_iterator
{
    esch_object* container;
    void* iterator; /**< Set to NULL when end of iterator is reached.*/
    esch_iterator_get_value_f get_value;
    esch_iterator_get_next_f get_next;
};

/**
 * Common Public interface to create an object and do basic initialization.
 * @param config Config object to get shared configuration between types.
 * @param type Type specifier structure.
 * @param esch_object Returned object.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_object_new(esch_config* config, esch_type* type,
                           esch_object** obj);
/**
 * Common public interface to delete an object, honoring GC system.
 * @param obj Given object to delete.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_object_delete(esch_object* obj);

/**
 * Get type reference of given object.
 * @param obj Given object.
 * @param type Returned parameter. The pointer of type object.
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_object_get_type(esch_object* obj, esch_type** type);

/**
 * Check if given object is primitive.
 * @param obj Given object.
 * @param primitive Returned parameter. True if object is primitive. 
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_object_is_primitive(esch_object* obj,
                                    esch_bool* primitive);
/**
 * Check if given object is primitive.
 * @param obj Given object.
 * @param primitive Returned parameter. True if object is container. 
 * @return Returned code. ESCH_OK if success.
 */
esch_error esch_object_is_container(esch_object* obj,
                                    esch_bool* container);

/**
 * Return an iterator structure for container object.
 * @param obj Given container object.
 * @param iter In-out parameter. Must point to a esch_iterator structure.
 * @return Returned code. ESCH_OK if success. For primitive object, it
 *         returns ESCH_ERROR_NOT_SUPPORTED.
 */
esch_error esch_object_get_iterator(esch_object* obj, esch_iterator* iter);

/**
 * Cast a concrete object to esch_object.
 * @param data A concrete object, for example, esch_string.
 * @param obj Returned paramter. The casted esch_object pointer.
 * @return Returned code. ESCH_OK if success.
 */
esch_error
esch_object_cast_to_object(void* data, esch_object** obj);

/**
 * Cast a concrete object to esch_object.
 * @param obj Given esch_object.
 * @param data Returned paramter. A casted concrete object.
 * @return Returned code. ESCH_OK if success.
 */
esch_error
esch_object_cast_from_object(esch_object* obj, void** data);
/* ----------------------------------------------------------------- */
/*                         Configuration                             */
/* ----------------------------------------------------------------- */

/* --- Configuration --- */
esch_error esch_config_new(esch_log* log, esch_alloc* alloc,
                           esch_config** config);
esch_error esch_config_get_int(esch_config* config,
                               const char* key, int* value);
esch_error esch_config_set_int(esch_config* config,
                               const char* key, int value);
esch_error esch_config_get_str(esch_config* config,
                               const char* key, char** value);
esch_error esch_config_set_str(esch_config* config,
                               const char* key, char* value);
esch_error esch_config_get_obj(esch_config* config,
                               const char* key, esch_object** obj);
esch_error esch_config_set_obj(esch_config* config,
                               const char* key, esch_object* obj);

/* --- Memory allocator --- */
/* Default C alloc does not depend on esch_config. */
/**
 * Create a new esch_alloc object. With underlying allocator 
 * @param config Config object.
 * @param alloc Returned allocator object.
 * @return Error code.
 */
esch_error esch_alloc_new_c_default(esch_config* config,
                                    esch_alloc** alloc);
/**
 * Allocate a new buffer or resize an existing buffer.
 * @param alloc The esch_alloc object to allocate buffer.
 * @param in    Input memory buffer. Can be NULL.
 * @param size  Size of wanted buffer.
 * @param out   Returned pointer to allocated buffer.
 * @return      Error code.
 */
esch_error esch_alloc_realloc(esch_alloc* alloc, void* in,
                              size_t size, void** out);
esch_error esch_alloc_free(esch_alloc* alloc, void* ptr);

/* TODO A buddy memory allocator */
esch_error esch_alloc_new_buddy(esch_config* config, esch_alloc** alloc);

/* --- Logger objects -- */
/* Do nothing log and printf log do not depend on esch_config. */
extern esch_log* esch_global_log;
esch_error esch_log_new_do_nothing(esch_config* config, esch_log** log);
esch_error esch_log_new_printf(esch_config* config, esch_log** log);
esch_error esch_log_delete(esch_log* log);
/**
 * Public function to log error.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @return Error code;
 */
esch_error esch_log_error(esch_log* log, const char* fmt, ...);
/**
 * Public function to log warning.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @return Error code;
 */
esch_error esch_log_warn(esch_log* log, const char* fmt, ...);
/**
 * Public function to log general information.
 * @param log The log object.
 * @param fmt Format string follos variable arguments.
 * @return Error code;
 */
esch_error esch_log_info(esch_log* log, const char* fmt, ...);

/* --- Garbage collector -- */
/**
 * Create a new mark-and-sweep GC object.
 * @param config Given config object.
 * @param gc Returned GC object.
 * @return Return code. ESCH_OK for OK.
 */
esch_error esch_gc_new_naive_mark_sweep(esch_config* config, esch_gc** gc);
esch_error esch_gc_recycle(esch_gc* gc);


/* --- Runtime --- */
typedef struct esch_runtime esch_runtime;

esch_error esch_runtime_new(esch_config* config, esch_runtime** runtime);
esch_error esch_runtime_delete(esch_runtime* runtime);
esch_error esch_runtime_register_type(esch_runtime* runtime,
                                       const char* name,
                                       esch_type* type);
/* --- String objects --- */
/**
 * Create a new string from UTF-8 string. It also converts
 * @param config Config object to set alloc and log.
 * @param utf8 Input UTF-8 string.
 * @param str Returned string object.
 * @return Error code.
 */
esch_error esch_string_new_from_utf8(esch_config* config, const char* utf8,
                                     int begin, int end,
                                     esch_string** str);
/**
 * Duplicate a string with same configuration.
 * @param input Original input string.
 * @param output Output string. Must be freed with esch_object_delete()
 * @return Return code.
 */
esch_error esch_string_duplicate(esch_string* input, esch_string** output);
/**
 * Get a reference to UTF-8 string of given string.
 * @param str Give esch_string object
 * @return The internal UTF-8 c-string, ending with '\0'.
 */
char* esch_string_get_utf8_ref(esch_string* str);
/**
 * Get a reference to UTF-8 string of given string.
 * @param str Give esch_string object
 * @return The internal Unicode c-string, ending with '\0'.
 */
esch_unicode* esch_string_get_unicode_ref(esch_string* str);
/**
 * Get length of given string in UTF-8 representation.
 * @param str Give esch_string object
 * @return Length of string, in UTF-8 representation, '\0' not included.
 */
size_t esch_string_get_utf8_length(esch_string* str);
/**
 * Get length of given string in Unicode representation.
 * @param str Give esch_string object
 * @return Length of string, in Unicode representation, '\0' not included.
 */
size_t esch_string_get_unicode_length(esch_string* str);

int esch_unicode_string_is_valid_identifier(const esch_unicode* unicode);

#define esch_unicode_is_ascii(ch) \
    (((esch_unicode)(ch)) >= 0 && ((esch_unicode)(ch)) < 127 )
#define esch_unicode_is_digit(ch) ((ch) >= '0' && (ch) <= '9')
#define esch_unicode_is_alpha(ch) \
    (((ch) >= 'A' && (ch) <= 'Z') || ((ch) >= 'a' && (ch) <= 'z'))
#define esch_unicode_is_extended_alphabetic(ch) \
    ((ch) == '!' || (ch) == '$' || (ch) == '%' || (ch) == '&' || \
     (ch) == '*' || (ch) == '+' || (ch) == '-' || (ch) == '.' || \
     (ch) == '/' || (ch) == ':' || (ch) == '<' || (ch) == '=' || \
     (ch) == '>' || (ch) == '?' || (ch) == '@' || (ch) == '^' || \
     (ch) == '_' || (ch) == '~')
int esch_unicode_is_range_lu(esch_unicode ch);
int esch_unicode_is_range_ll(esch_unicode ch);
int esch_unicode_is_range_lt(esch_unicode ch);
int esch_unicode_is_range_lm(esch_unicode ch);
int esch_unicode_is_range_lo(esch_unicode ch);
int esch_unicode_is_range_mn(esch_unicode ch);
int esch_unicode_is_range_mc(esch_unicode ch);
int esch_unicode_is_range_nd(esch_unicode ch);
int esch_unicode_is_range_nl(esch_unicode ch);
int esch_unicode_is_range_no(esch_unicode ch);
int esch_unicode_is_range_pd(esch_unicode ch);
int esch_unicode_is_range_pc(esch_unicode ch);
int esch_unicode_is_range_po(esch_unicode ch);
int esch_unicode_is_range_sc(esch_unicode ch);
int esch_unicode_is_range_sm(esch_unicode ch);
int esch_unicode_is_range_sk(esch_unicode ch);
int esch_unicode_is_range_so(esch_unicode ch);
int esch_unicode_is_range_co(esch_unicode ch);

/* --- Vector --- */
/* XXX
 * This is a vector to represent Scheme pair, list and array. It's used
 * to represent a non-typed array.
 * The vector accepts only valid esch_objects.
 */

/**
 * Create a new vector.
 * @param config Configuration.
 * @param vec Returned vector object.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_new(esch_config* config, esch_vector** vec);

/**
 * Append a value at the end of vector.
 * @param vec Given vector object.
 * @param obj A new object. Can't be NULL.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_append_value(esch_vector* vec, esch_value* value);

/**
 * Append an object at the end of vector.
 * @param vec Given vector object.
 * @param obj A new object. Can't be NULL.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_append_object(esch_vector* vec, esch_object* obj);
/**
 * Append a byte value at the end of vector.
 * @param vec Given vector object.
 * @param b A new byte.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_append_byte(esch_vector* vec, esch_byte b);
/**
 * Append a unicode value at the end of vector.
 * @param vec Given vector object.
 * @param u A new unicode value.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_append_unicode(esch_vector* vec, esch_unicode u);
/**
 * Append an integer value at the end of vector.
 * @param vec Given vector object.
 * @param i A new integer value.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_append_integer(esch_vector* vec, int i);
/**
 * Append a float value at the end of vector.
 * @param vec Given vector object.
 * @param f A new float value.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_append_float(esch_vector* vec, double f);
/**
 * Get length of given vector.
 * @param vec Given vector object.
 * @param length Length of given vector.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_length(esch_vector* vec, size_t* length);

/**
 * Get arbitary element from vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param expected_type Type, or ignore if set to ESCH_VALUE_TYPE_END.
 * @param value Returned value. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_value(esch_vector* vec, int index,
                                 esch_value_type expected_type,
                                 esch_value* value);
/**
 * Set element to vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param value New value.
 * @return Return code. ESCH_OK if success.
 */

esch_error esch_vector_set_value(esch_vector* vec, int index,
                                 esch_value* value);

/**
 * Get object element from vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param obj Returned object. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_object(esch_vector* vec, int index,
                                  esch_object** obj);
/**
 * Get byte element from vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param i Returned object. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_byte(esch_vector* vec, int index,
                                esch_byte* b);
/**
 * Get unicode element from vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param u Returned unicode. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_unicode(esch_vector* vec, int index,
                                   esch_unicode* u);
/**
 * Get integer element from vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param i Returned integer. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_integer(esch_vector* vec, int index, int* i);
/**
 * Get float element from vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param f Returned float. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_get_float(esch_vector* vec, int index, double* f);

/**
 * Set object element in vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param obj New object. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_set_object(esch_vector* vec, int index,
                                  esch_object* obj);
/**
 * Set byte element in vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param b New byte. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_set_byte(esch_vector* vec, int index,
                                esch_byte b);
/**
 * Set unicode element in vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param u New unicode. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_set_unicode(esch_vector* vec, int index,
                                   esch_unicode u);
/**
 * Set integer element in vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param i New integer. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_set_integer(esch_vector* vec, int index, int i);
/**
 * Set float element in vector.
 * @param vec Given vector object.
 * @param index Given index. Negative index means starting from end.
 * @param f New float. Unchanged if an error is raised.
 * @return Return code. ESCH_OK if success.
 */
esch_error esch_vector_set_float(esch_vector* vec, int index, double f);

/* --- Pair --- */
/**
 * Create a new pair.
 * @param config Given config object.
 * @param head Head object.
 * @param tail Tail object.
 * @param pair Out parameter of created pair.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_new(esch_config* config,
                         esch_value* head, esch_value* tail,
                         esch_pair** pair);
/**
 * Create an empty list.
 * @param config Given config object.
 * @param head Head object.
 * @param tail Tail object.
 * @param pair Out parameter of created pair.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_new_empty(esch_config* config, esch_pair** pair);

/**
 * Get head of pair.
 * @param pair Given pair object.
 * @param value Returned value.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_get_head(esch_pair* pair, esch_value* value);
/**
 * Get tail of pair.
 * @param pair Given pair object.
 * @param value Returned value.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_get_tail(esch_pair* pair, esch_value* value);
/**
 * Set head of pair.
 * @param pair Given pair object.
 * @param value New value to set.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_set_head(esch_pair* pair, esch_value* value);
/**
 * Set tail of pair.
 * @param pair Given pair object.
 * @param value New value to set.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_set_tail(esch_pair* pair, esch_value* value);

/**
 * Check if given pair is a list.
 * @param pair Given pair object.
 * @param is_list Bool value to indicate if it's a list.
 * @return ESCH_OK if correct.
 */
esch_error esch_pair_is_list(esch_pair* pair, esch_bool* is_list);

/* --- Number -- */

/* --- Parser --- */
esch_error esch_parser_new(esch_config* config, esch_parser** parser);
esch_error esch_parser_delete(esch_parser* parser);
esch_error esch_parser_register_callback(esch_parser* parser,
                                         esch_parser_callback* cb);
esch_error esch_parser_remove_callback(esch_parser* parser,
                                       esch_parser_callback* cb);
esch_error esch_parser_read_line(esch_parser* parser, char* input);
esch_error esch_parser_read_file(esch_parser* parser, char* file);

/* --- 
 * TODO Add more functions as part of configurations.
 * 1. Message callback function to keep error information.
 * 2. Standard library
 * 3. Search path
 * --- */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _ESCH_ESCH_H_ */
