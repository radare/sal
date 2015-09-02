/*
 * Copyright (C) 2006
 *       pancake <pancake@phreaker.net>
 *
 * sal is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * sal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sal; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

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
