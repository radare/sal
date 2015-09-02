#include "vm.h"
#include "hash.h"
#include "flags.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

char *salsyms = "syscall.0,syscall.1,syscall.2,syscall.3";

void sal_syscall_0(vm_t *vm)
{
	int num = 0;
	obj_t *sn = sal_stack_pop (vm->stack);
	if (sn && sn->type == OBJECT_INTEGER) {
		num = integer_get_value (sn);
	} else {
		printf ("Invalid value in stack\n");
		return;
	}
	syscall (num);
}

static unsigned long long getArg(obj_t *o) {
	if (o->type == OBJECT_INTEGER) {
		return integer_get_value (o);
	}
	if (o->type == OBJECT_STRING) {
		return (unsigned long long) \
			(size_t)o->data;
	}
	return 0L;
}

void sal_syscall_1(vm_t *vm)
{
	int num = 0, na0 = 0;
	obj_t *sn = sal_stack_pop (vm->stack);
	obj_t *a0 = sal_stack_pop (vm->stack);
	num = getArg (sn);
	na0 = getArg (a0);
	syscall (num, na0);
}

void sal_syscall_2(vm_t *vm)
{
	fprintf (stderr, "syscall.2 TODO\n");
}

void sal_syscall_3(vm_t *vm)
{
	int ret;
	unsigned long long num = 0, na0 = 0, na1 = 0, na2 = 0;
	obj_t *sn = sal_stack_pop (vm->stack);
	obj_t *a0 = sal_stack_pop (vm->stack);
	obj_t *a1 = sal_stack_pop (vm->stack);
	obj_t *a2 = sal_stack_pop (vm->stack);
	num = getArg (sn);
	na0 = getArg (a0);
	na1 = getArg (a1);
	na2 = getArg (a2);
#if 0
	printf ("SN  = %x\n", num);
	printf ("a0  = %x\n", na0);
	printf ("a1  = %x\n", na1);
	printf ("a2  = %x\n", na2);
  ret = syscall (4, 1, "HELLO WORLD\n", 12);
#endif
	ret = syscall (num, na0, na1, na2);
	printf ("RET %d\n", ret);
}
