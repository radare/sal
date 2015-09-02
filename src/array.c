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
#include <stdlib.h>
#include "vm.h"
#include "alloc.h"
#include "array.h"

obj_t *array_new(int size)
{
	obj_t *obj   = object_new("array");
	array_t *arr = (array_t *)  sal_malloc(sizeof(array_t));
	arr->cells   = (object_t **)sal_malloc(size * sizeof(object_t*));

	obj->length  = size;
	arr->size    = size;
	memset(arr->cells, 0, size);

	obj->type    = OBJECT_ARRAY;
	obj->data    = arr;

	return obj;
}

obj_t *array_get(obj_t *obj, int pos)
{
	array_t *arr = obj->data;

	if (obj->type!=OBJECT_ARRAY)
		return NULL;

	if (pos<0 || pos>=arr->size)
		return NULL;

	return arr->cells[pos];
}

int array_set(obj_t *obj, int pos, obj_t *set)
{
	array_t *arr = obj->data;

	if (obj->type!=OBJECT_ARRAY)
		return -1;

	if (pos<0 || pos>arr->size)
		return -1;

//	object_free(arr->cells[pos]);
	arr->cells[pos] = set;

	return 0;
}

int array_dset(obj_t *obj, int pos, obj_t *set)
{
	if (pos<0)
		return -1;

	if (pos >= obj->length)
		array_resize(obj, pos+1);

	array_set(obj, pos, set);

	return 0;
}

void array_free(array_t *arr)
{
	arr->size = 0;
	free(arr->cells);
	arr->cells = 0;
}

int array_size(obj_t *obj)
{
	array_t *arr = obj->data;

	if (obj->type!=OBJECT_ARRAY)
		return 0;

	if (arr->size != obj->length)
		return 0;

	return obj->length;
}

// XXX we must care for the real destruction of the array if fail
int array_resize(obj_t *obj, int size)
{
	register int i;
	array_t *arr = obj->data;

	if (obj->type!=OBJECT_ARRAY)
		return -1;

	if (size<0)
		return -1;

	if (size<0)
		return -1;

	arr->cells = (object_t **)realloc(arr->cells, size*sizeof(object_t *));
	if (arr->cells == NULL)
		return -1;

	if (size > arr->size) {
		for (i = arr->size; i<size; i++)
			arr->cells[i] = NULL;
	}

	arr->size = size;
	obj->length = size;

	return 0;
}
