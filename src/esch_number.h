/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_NUMBER_H_
#define _ESCH_NUMBER_H_
#include "esch.h"
#include "esch_object.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct esch_integer esch_integer;
typedef struct esch_fraction esch_fraction;

struct esch_integer
{
    ESCH_COMMON_HEADER;
    union
    {
        unsigned int ival;
        struct
        {
            size_t length;
            unsigned char* digit;
        } bval;
    } value;
    unsigned char big: 1;
    unsigned char negative: 1;
};

struct esch_fraction
{
    ESCH_COMMON_HEADER;
    ESCH_BOOL precise: 1;
    esch_integer* numerator;
    esch_integer* denorminator;
};

struct esch_complex
{
    ESCH_COMMON_HEADER;
    esch_fraction* real;
    esch_fraction* imaginary;
};

/* TODO: Not complete. */
#define ESCH_IS_VALID_INTEGER(obj) \
    (ESCH_IS_VALID_OBJECT(obj) && \
     ((!((esch_integer*)obj)->big) || \
      ((esch_integer*)obj)->value.bval.digit != NULL))

#define ESCH_IS_VALID_COMPLEX(obj) \
    (ESCH_IS_VALID_OBJECT(obj))

/*
 * The only public interface should be esch_complex.
 * Integer and fraction should be kept private.
 */

esch_error esch_integer_delete(esch_integer* val);
esch_error esch_integer_new_from_base10(esch_config* config,
                                        const char* begin,
                                        const char* end,
                                        esch_integer** val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_NUMBER_H_ */

