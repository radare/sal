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

#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "array.h"
#include "label.h"

label_t *label_new(vm_t *vm, char *name)
{
	label_t *label = (label_t *)sal_malloc(sizeof(label_t));

	if (label == NULL)
		return NULL;

	label->native  = 0;
	strcpy(label->name,name);
	label->file    = vm->file;
	label->argc    = 0;
	label->offset  = file_offset(vm->file);

	return label;
}

label_t *label_new_native(vm_t *vm, char *name, void *ptr)
{
	label_t *label = (label_t *)sal_malloc(sizeof(label_t));

	strcpy(label->name,name);
	label->native  = ptr;
	label->argc    = 0; // XXX not yet implemented: XREF: label.c:73 

	return label;
}

label_t *label_get(vm_t *vm, char *name)
{
	list_t *p = vm->labels;

	while(p != NULL) {
		label_t *l = (label_t *)p->data;
		if (!strcmp(l->name, name))
			return l;
		p = p->next;
	}

	return NULL;
}

// WARNING: This function does not calls the label, just prepares the return address
label_t *label_push_ret(vm_t *vm, char *word)
{
	list_t *p = vm->labels;

	while(p != NULL) {
		label_t *l = (label_t *)p->data;
		if (!strcmp(l->name, word)) {
			obj_t *obj = object_new("_label");
			obj->type = OBJECT_LABEL;
			obj->data = (void *)label_new(vm, "_ret");
			if (obj->data == NULL) {
				printf("Invalid label name. '%s'\n",word);
				vm_exit(vm, 1);
			}
			sal_stack_push(vm->stack, obj);
			return l; // label_call(vm, l);
		}
		p = p->next;
	}
	// TODO: interrupt here?
	if (p == NULL) {
		printf("call: Invalid label '%s'.\n", word);
		vm_exit(vm, 1);
	}

	return NULL;
}

int label_call(vm_t *vm, label_t *l)
{
	if (l == NULL) {
		printf("Entry not defined.\n");
		return -1;
	}

	if (l->native) {
		// XXX argc not yet implemented. XREF: label: label.c:48
		void (*cb)(vm_t *) = l->native;
		cb(vm);
	} else {
		int i, end = 0;
		char *ptr = strchr(l->name, '@');
		obj_t *dst;

		if (ptr) {
			end = atoi(ptr+1);
		}

		if (end>0) {
			dst = array_new(end);
			
		for(i=0;i<end;i++) {
			char *name = file_read(vm->file, vm->output);
			object_type type = value_type(vm, name);
			object_t *obj;
			VM_VERBOSE printf("arg: %s\n", name);

			switch(type) {
			case OBJECT_POINTER:
				obj = pointer_new(name+1, NULL);
				array_set(dst, i, obj);
				break;
			case OBJECT_REGISTER:
				obj = object_new("%av");
				obj->type=OBJECT_REGISTER;
				array_set(dst, i, obj);
				break;
			case OBJECT_STRING:
				//set_register_object(vm, , string_new(name));
				array_set(dst, i, string_new(name));
				break;
			default:
				printf("notyet supported arg type %d\n", type);
				break;
			}
			//free(name);
		}
			vm->argv = dst;
			// TODO: restore dst when 'ret'
			//libvm_dump(vm);
		}

		//vm->callback++;
		vm->file = l->file;
		if (end)
			label_push_ret(vm, l->name);

		file_seek(vm->file, l->offset);

		return 0;
	}

	return -1;
}

void label_call_back(vm_t *vm, label_t *l)
{
	if (l->native) {
		void (*cb)(vm_t *) = l->native;
		cb(vm);
	} else {
		file_t *bak = vm->file;
		off_t offset = file_offset(vm->file);

		vm->file = l->file;
		file_seek(vm->file, l->offset);

		vm->callback++;
		while(vm->callback>0 && vm_fetch(vm));
		//printf(">> back from the call. %x\n", offset);
		vm->file = bak;
		file_seek(vm->file, offset);
	}
}
