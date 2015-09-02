#include "vm.h"
#include "hash.h"
#include "flags.h"
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static unsigned long long getArg(obj_t *o) {
	if (!o) return 0LL;
	if (o->type == OBJECT_INTEGER) {
		return integer_get_value (o);
	}
	if (o->type == OBJECT_STRING) {
		return (unsigned long long) \
			(size_t)o->data;
	}
	return 0L;
}

static uint64_t *popNums (vm_t *vm, int n) {
	uint64_t *r = calloc (n, sizeof (uint64_t));
	if (r) {
		int i;
		for (i=0; i<n; i++) {
			r[i] = getArg (sal_stack_pop (vm->stack));
		}
	}
	return r;
}

// ------------------------------------------- //

char *salsyms = "syscall.0,syscall.1,syscall.2,syscall.3";

void libvm_syscall_0(vm_t *vm)
{
	uint64_t *a = popNums (vm, 1);
	if (a) {
		int ret = syscall (a[0]);
		sal_stack_push (vm->stack, integer_new_i (ret));
		free (a);
	}
}

void libvm_syscall_1(vm_t *vm) {
	uint64_t *a = popNums (vm, 2);
	if (a) {
		int ret = syscall (a[0], a[1]);
		sal_stack_push (vm->stack, integer_new_i (ret));
		free (a);
	}
}

void libvm_syscall_2(vm_t *vm) {
	uint64_t *a = popNums (vm, 3);
	if (a) {
		int ret = syscall (a[0], a[1], a[2]);
		sal_stack_push (vm->stack, integer_new_i (ret));
		free (a);
	}
}

void libvm_syscall_3(vm_t *vm) {
	uint64_t *a = popNums (vm, 4);
	if (a) {
		int ret = syscall (a[0], a[1], a[2], a[3]);
		sal_stack_push (vm->stack, integer_new_i (ret));
		free (a);
	}
}
