/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "float.h"
#include "list.h"
#include "hash.h"
#include "array.h"
#include "integer.h"

obj_t *object_new(char *name)
{
	object_t *obj = (object_t *)sal_malloc(sizeof(object_t));
	obj->magic = OBJECT_MAGIC;
	obj->refs  = 0;
	strncpy (obj->name, name, sizeof (obj->name)-1);
	return obj;
}

obj_t *object_copy(object_t *tmp)
{
	register int i,j;
	obj_t *obj = NULL;
	
	if (tmp == NULL)
		return NULL;

	switch(tmp->type) {
	case OBJECT_STRING:
		obj = string_new_t(tmp);
		break;
	case OBJECT_INTEGER:
		obj = integer_new_i(integer_get_value(tmp));
		break;
	case OBJECT_FLOAT:
		{
		float f;
		memcpy(&f, &tmp->data, sizeof(float));
		obj = float_new_f(f);
		}
		break;
	case OBJECT_ARRAY:
		j = array_size(tmp);
		obj = array_new (j);
		for (i = 0; i<j; i++)
			array_set(obj, i, object_copy( array_get(tmp, i) ));
		break;
	default: // XXX clone object with memleak risk
		obj = object_new(tmp->name);
		memcpy(obj, tmp, sizeof(obj_t));
		break;
	}
	//obj->name = tmp->name;
	obj->refs = 0;

	return obj;
}

obj_t *object_ref(obj_t *obj)
{
	obj->refs++;
	return obj;
}

obj_t *object_new_label(char *name, label_t *label)
{
	obj_t *obj = object_new(name);

	strncpy(obj->name, name, sizeof (obj->name)-1);
	obj->type = OBJECT_LABEL;

	if (label == NULL) {
		fprintf(stderr, "Null label\n");
	}

	obj->data = label;

	return obj;
}

void object_dump(object_t *obj, int level)
{
	int l = 0;

	for(;l<level;l++) printf(" ");

	if (obj == NULL) {
		printf(" (null)\n");
		return;
	}

	if (obj->magic!=OBJECT_MAGIC) {
		fprintf(stderr, "\nobject_dump: Invalid MAGIC found in object. (%08x)\n", obj->magic);
		exit(1);
	}
	if (obj->magic==LABEL_MAGIC) {
		printf("This is a label\n");
		return;
	}

	switch(obj->type) {
	case OBJECT_ARRAY: {
		register int i;
		int size = array_size(obj);
		printf("[array] size=%d %d\n", obj->length, size);
		for(i = 0; i< size; i++)
			object_dump(array_get(obj,i), level+5); }
		break;
	case OBJECT_HASH:
		printf("[hash] size=%d\n", hash_size(obj));
		break;
	case OBJECT_STRING:
		if (string_is_binary(obj))
			printf("[bstring] %s: %d bytes\n", obj->name, obj->length);
		else
			printf("[string] %s: %s\n", obj->name, (char *)obj->data);
		break;
	case OBJECT_INTEGER:
		printf("[integer] %d\n", obj->length);
		break;
	case OBJECT_FLOAT: {
		float f;
		memcpy(&f, &obj->data, sizeof(float));
		printf("[float] %f\n", f); }
		break;
	case OBJECT_LABEL: {
		label_t *l = obj->data;
		printf("[label] %s\n", l->name); }
		break;
	case OBJECT_REGISTER:
		printf("[reg] %s contains:\n", obj->name );
		object_dump(obj->data, level+5);
		break;
	case OBJECT_POINTER:
		printf("[pointer] %s contains:\n", obj->name );
		object_dump(obj->data, level+5);
		break;
	case OBJECT_NATIVE:
		printf("[native] %s (size %d):\n", obj->name, obj->length);
		break;
	default:
		printf("[unknown] type : %d\n", obj->type);
		break;
	}
}

object_type value_type(vm_t *vm, char *name)
{
	int i = atoi(name);
	list_t *p = vm->labels;

	if (name[0]=='*')
		return OBJECT_POINTER;

	if (name[0]=='&')
		return OBJECT_POINTER;

	if (i==0 && name[0]!='0') {
		while(p != NULL) {
			label_t *l = (label_t *)p->data;
			if (!strcmp(l->name, name))
				return OBJECT_LABEL;
			p = p->next;
		}

		p = vm->regs;
		while(p != NULL) {
			obj_t *l = (obj_t *)p->data;
			if (!strcmp(l->name, name))
				return OBJECT_REGISTER;
			p = p->next;
		}
	} else {
		if (!strchr(name, '.'))
			return OBJECT_INTEGER;

		return OBJECT_FLOAT;
	}

	if ( is_register(name) ) 
		return OBJECT_REGISTER;

	if (name[0]=='"')
		return OBJECT_STRING;

	return -1;
}

object_type pointer_type(vm_t *vm, char *name)
{
	object_type type;

	switch(name[0]) {
	case '*':
		type = value_type(vm, name+1);

		switch(type) {
		case OBJECT_REGISTER:
		case OBJECT_POINTER:
			return type;
			break;
		case OBJECT_ARRAY:
		case OBJECT_LABEL:
			return type;
			break;
		default:
			fprintf(stderr, "this object cannot be pointed");
			vm_exit(vm, 1);
			break;
		}
		break;
	case '&':
		type = value_type(vm, name+1);

		switch(type) {
		case OBJECT_REGISTER:
		case OBJECT_POINTER:
			return type;
			break;
		default:
			fprintf(stderr, "this object cannot be referenced");
			vm_exit(vm, 1);
			break;
		}
	}

	return -1;
}

// XXX is this a dupped functionality??
obj_t *object_new_t(vm_t *vm, char *name)
{
	obj_t *obj = object_new(name);
	obj->type = value_type(vm, name);
	
	return obj;
}

void object_free(object_t *obj)
{
	if (obj == NULL)
		return;

	//printf("REF%d\n", obj->refs);
	if (obj->refs>=0) {
		obj->refs--;
		return;
	}

	switch(obj->type) {
	case OBJECT_ARRAY:
		array_free(obj->data);
		break;
	case OBJECT_STRING:
	case OBJECT_REGISTER:
		free(obj->data);
		obj->data = NULL;
		break;
	case OBJECT_INTEGER:
	case OBJECT_FLOAT:
		break;
	default:
		printf("XXX: Cannot free %d\n", obj->type);
	}

	obj->length = 0;

	free(obj);
}
