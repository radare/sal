#ifndef _INCLUDE_OBJECT_H_
#define _INCLUDE_OBJECT_H_

#include "vm.h"

typedef enum {
	OBJECT_STRING   = 0,
	OBJECT_INTEGER  = 1,
	OBJECT_FLOAT    = 2,
	OBJECT_LABEL    = 3,
	OBJECT_REGISTER = 4,
	OBJECT_POINTER  = 5,
	OBJECT_ARRAY    = 6,
	OBJECT_HASH     = 7,
	OBJECT_NATIVE   = 8,
	OBJECT_INVALID  = -1
} object_type;

#define OBJECT_MAGIC (char)0x86

typedef struct object_t {
	char magic;
	char name[128];
	int refs;
	object_type type;
	int length;
	void *data;
} object_t;

#define pointer_get_value(x) x->data

#define obj_t object_t
obj_t *object_new(char *name);
obj_t *object_copy(obj_t *);
obj_t *object_ref(obj_t *);
obj_t *object_new_label(char *name, label_t *label);
void object_dump(object_t *obj, int level);
void object_free(object_t *obj);

#endif
