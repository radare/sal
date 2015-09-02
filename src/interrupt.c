/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdlib.h>
#include "vm.h"
#include "label.h"
#include "array.h"
#include "interrupt.h"

obj_t *interrupt_get(vm_t *vm, int interrupt)
{
	return array_get(vm->intv, interrupt);
}

void interrupt_set(vm_t *vm, int interrupt, label_t *lab)
{
	obj_t *obj = object_new_label(lab->name, lab);
	array_dset(vm->intv, interrupt, obj);
}

int main_interrupt = -1;

int interrupt_call(vm_t *vm, int interrupt)
{
	obj_t *obj = interrupt_get(vm, interrupt);
	label_t *ptr;

	/* cannot nest interrupts */
	if (main_interrupt == interrupt)
		return 0;

	main_interrupt = interrupt;

	if (obj == NULL) {
		main_interrupt = -1;
		return 0;
	}

	ptr = obj->data;

	VM_VERBOSE printf("[int] %d : %s @ %p\n",interrupt, ptr->name, ptr->native);
	if (ptr == NULL) {
		printf("No hook on interrupt %d\n", interrupt);
		main_interrupt = -1;
		return 0;
	}

	obj = object_new_label("_ret", label_new(vm, "_ret"));
	sal_stack_push(vm->stack, (void *)obj);

	label_call(vm, ptr);

	main_interrupt = -1;

	return 1;
}

void int_interrupt_signal(vm_t *vm)
{
	printf("^C\n");

	vm_exit(vm, 0);
}

void int_shutdown(vm_t *vm)
{
	//object_t *obj = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT(">> interrupt %s\n","shutdown");
	vm_shutdown(vm);
}

void int_empty_stack(vm_t *vm)
{
	fprintf(stderr, "interrupt: Stack is empty!\n");
	exit(1);
}

void int_invalid_object(vm_t *vm)
{
	fprintf(stderr, "interrupt: Invalid object found!\n");
	exit(1);
}

void int_zero_division(vm_t *vm)
{
	fprintf(stderr, "interrupt: Division by 0 found!\n");
	exit(1);
}
