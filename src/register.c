/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "interrupt.h"
#include "list.h"
#include "integer.h"

int is_static_register(char *name)
{
	if (!strcmp(name, "%sp")
	||  !strcmp(name, "%pc")
	||  !strcmp(name, "%av")
	||  !strcmp(name, "%iv")
	||  !strcmp(name, "%ev"))
		return 1;

	return 0;
}

int is_register(char *name)
{
	if (is_static_register(name))
		return 1;
	if ( (name[0]=='%' && name[1]=='r' && name[2]>='0' && name[2]<='9' )
	||   (name[0]=='r' && name[1]>='0' && name[1]<='9'))
		return 1;
	return 0;
}

void set_register_object(vm_t *vm, char *name, obj_t *obj)
{
	obj_t *obj2;

	if (is_static_register(name)) {
		switch(name[1]) {
		case 'i': // interrupt vector
			printf("TODO push %%iv\n");
			break;
		case 'p':
			if (obj->type == OBJECT_INTEGER) {
				int value = integer_get_value(obj);
				if (value < 0) value = 0;
				file_seek(vm->file, (long)value);
				object_free(obj);
			} else {
				interrupt_call(vm, INT_INVO);
				fprintf(stderr, "%%pc only accepts integer objects.\n");
			}
			break;
		case 's':
			if (obj->type == OBJECT_INTEGER) {
				int value = integer_get_value(obj);
				if (value < 0) value = 0;
				if (value > vm->stack->maxSize)
					value = vm->stack->maxSize;

				vm->stack->top = value;
			} else {
				fprintf(stderr, "%%sp only accepts integer objects.\n");
				interrupt_call(vm, INT_INVO);
			}
			break;
		}
	} else {
		//obj2 = object_ref(obj);
		obj2 = object_copy(obj);
	//	object_free(obj);
		obj2->type = OBJECT_REGISTER;
		strcpy(obj2->name, name);
		obj2->data = obj;
		vm->regs = (list_t *)list_obj_set(&vm->regs, (obj_t *)obj2);
	}
}

obj_t *get_register_object(vm_t *vm, char *name)
{
	obj_t *obj = NULL;
	obj_t *obj2 = NULL;

	if (is_static_register(name)) {
		switch(name[1]) {
		case 'a': // %av
			obj = object_ref(vm->argv);
			break;
		case 'e':
			obj = object_ref(vm->envr);
			break;
		case 'i': // %iv
			obj = object_ref(vm->intv);
			break;
		case 's': // %sp
			obj = object_new(name);
			obj->type = OBJECT_INTEGER;
			obj->length = vm->stack->top + 1;
			break;
		case 'p': // %pc
			obj = object_new(name);
			obj->length = (unsigned long) file_offset(vm->file);
			obj->type = OBJECT_INTEGER;
			break;
		default:
			printf("INVLIA REAIGSTEX\n");
			break;
		}
	} else {
		obj = (obj_t *)list_obj_get(&vm->regs, name);
		if (obj == NULL) {
			printf("Register '%s' not yet allocated.\n", name);
			exit(1);
		}
		if (obj->type == OBJECT_REGISTER) {
			obj2 = obj->data;
			if (obj2 == NULL) {
				printf("Register has null contents\n");
				vm_exit(vm, 1);
			}
			//obj->type = obj2->type;
			switch(obj2->type) {
			case OBJECT_REGISTER:
				printf("Nested registers are nasty.\n");
				vm_exit(vm, 1);
	/*
				{
				obj = object_new(name);
				obj->name[0]='\0';
				obj->type = ((obj_t *)obj2->data)->type;
				obj->length = ((obj_t *)obj2->data)->length;
				switch(obj->type) {
				case OBJECT_STRING:
					obj->data = strdup(((obj_t *)obj2->data)->data);
					break;
				default:
					obj->data = obj2->data;
					break;
				}
				}
	*/
				break;
			case OBJECT_STRING:
				obj = string_new_t(obj2);
				break;
			case OBJECT_INTEGER:
				obj = integer_new_i(integer_get_value(obj2));
				break;
			case OBJECT_POINTER:
			case OBJECT_HASH:
			case OBJECT_LABEL:
			case OBJECT_ARRAY:
			case OBJECT_NATIVE:
				obj = object_ref(obj2);
				break;
			default:
				printf("Invalid object type. TODO! %d\n", obj2->type);
				exit(1);
				break;
			}
		} else {
			printf("This is not a register!\n");
			interrupt_call(vm, INT_INVO);
			vm_exit(vm, 1);
		}
	}
	return obj;
}

