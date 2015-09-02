/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdio.h>
#include <string.h>
#include "vm.h"

obj_t *float_new(char *string)
{
	object_t *obj = object_new("_float");
	obj->type = OBJECT_FLOAT;
	obj->length = 0;
	sscanf(string, "%f", (float *)&obj->data);

	return obj;
}

obj_t *float_new_f(float f)
{
	object_t *obj = object_new("_float");
	obj->type = OBJECT_FLOAT;
	obj->length = 0;
	memcpy((void *)&obj->data, (const void *)&f, sizeof(float));

	return obj;
}

float float_get_value(obj_t *obj)
{
	float ret;
	memcpy(&ret, &obj->data, sizeof(float));
	return ret;
}
