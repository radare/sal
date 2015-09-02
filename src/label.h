#ifndef _INCLUDE_LABEL_H_
#define _INCLUDE_LABEL_H_

#include "file.h"

typedef struct label_t {
	char magic;
	char name[128];
	int argc;
	off_t offset;
	void *native;
	file_t *file;
} label_t;

#define LABEL_MAGIC (char)0x2e

// NOTE : vm_t has a deadlock definition: moved to vm.h
//void label_call(vm_t *vm, label_t *l);
//label_t *label_get(vm_t *vm, char *name);
//label_t *label_new_native(vm_t *vm, char *name, void *ptr);
//label_t *label_new(vm_t *vm, char *name);

#endif
