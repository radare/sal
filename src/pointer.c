/* Copyright (C) 2006-2015 - BSD - pancake */

#include "vm.h"

/*
 * length = size of the containing chunk
 * data   = pointer
 */

obj_t *pointer_new(char *string, obj_t *src)
{
	char buf[1024];

	sprintf(buf, "ptr_%.900s", string);
	obj_t *obj  = (obj_t *)object_new( buf );
	obj->type   = OBJECT_POINTER;
	obj->data   = src;

	return obj;
}
