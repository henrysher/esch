/* vim:ft=c expandtab tw=72 sw=4
 */
/* See Copyright notice in esch.h */
#ifndef _ESCH_VALUE_H_
#define _ESCH_VALUE_H_

#include "esch.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*esch_value_assign_f)(esch_value*, esch_value*);
typedef esch_error (*esch_value_check_f)(esch_value*);

extern esch_error esch_value_type_check[8][8]; /* match every value */
extern esch_value_check_f esch_value_check[8];
extern esch_value_assign_f esch_value_assign[2];

extern void esch_value_get_object(void* data, esch_value* value);
extern void esch_value_get_integer(void* data, esch_value* value);
extern void esch_value_get_float(void* data, esch_value* value);
extern void esch_value_get_byte(void* data, esch_value* value);
extern void esch_value_get_unicode(void* data, esch_value* value);

extern void esch_value_set_integer(esch_value* value, void* data);
extern void esch_value_set_float(esch_value* value, void* data);
extern void esch_value_set_byte(esch_value* value, void* data);
extern void esch_value_set_unicode(esch_value* value, void* data);
extern void esch_value_set_object(esch_value* value, void* data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESCH_VALUE_H_ */
