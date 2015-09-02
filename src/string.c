/* Copyright (C) 2006-2015 - BSD - pancake */

#include <string.h>
#include <stdlib.h>
#include "vm.h"

obj_t *string_new(const char *string)
{
	obj_t *obj = (obj_t *)object_new("_string");
	obj->type = OBJECT_STRING;
	if (string == NULL) {
		obj->length = 0;
		obj->data = strdup("");
	} else {
		obj->length = strlen(string);
		if (string[0]=='"')
			obj->data = strdup(string+1);
		else
			obj->data = strdup(string);
	}

	return obj;
}

obj_t *string_bin_new(char *data, int len)
{
	obj_t *obj = (obj_t *)object_new("_bstring");

	obj->type = OBJECT_STRING;
	if (data == NULL) {
		obj->length = 0;
		obj->data = strdup("");
	} else {
		obj->length = len;
		obj->data = (char *)malloc( len );
		memcpy(obj->data, data, len);
	}

	return obj;
}


obj_t *string_new_t(obj_t *obj)
{
	if (string_is_binary(obj))
		return string_bin_new(obj->data, obj->length);

	// TODO: use length, for optimization
	return string_new(obj->data);
}

int string_length(obj_t *obj)
{
	return strlen(string_get_value(obj));
}

#define string_new_t(obj) string_new(obj->data)
