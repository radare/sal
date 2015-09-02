#ifndef _INCLUDE_REGISTER_H_
#define _INCLUDE_REGISTER_H_

int is_static_register(char *name);
int is_register(char *name);
void set_register_object(vm_t *vm, char *name, obj_t *obj);
obj_t *get_register_object(vm_t *vm, char *name);

#endif
