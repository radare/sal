#ifndef _INCLUDE_INTEGER_H_
#define _INCLUDE_INTEGER_H_

#define integer_set_value(x,y) x->length = y
int integer_get_value(obj_t *obj);
int integer_get_value_i(char *string);
obj_t *integer_new_i(int number);
obj_t *integer_new(char *string);
#define integer_new_t(x) integer_new_i(integer_get_value(x))

#endif
