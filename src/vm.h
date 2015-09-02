#ifndef _INCLUDE_VM_H_
#define _INCLUDE_VM_H_

#include <stdio.h>
#include <time.h>
#include "config.h"
#include "label.h"
#include "flags.h"
#include "list.h"
#include "object.h"
#include "pointer.h"
#include "integer.h"
#include "string.h"
#include "alloc.h"
#include "stack.h"
#include "file.h"

#ifndef VERSION
#define VERSION "no-version"
#endif

int string_length(obj_t *obj);
obj_t *string_new(const char *string);
obj_t *string_bin_new(char *data, int len);
obj_t *string_new_t(obj_t *obj);
#define string_get_length(obj) obj->length
#define string_get_value(obj) ((char *)obj->data)
#define string_free(obj) free(obj->data);
#define string_is_binary(obj) (obj->name[1]=='b')

typedef struct vm_t {
	time_t uptime;
	int debug;
	int callback; /* flag to indicate if we're doing a callback or not (native->sal->native) */
	int verbose;
	int flags;
	FILE *output; /* output file to dump execution */
	file_t *file;
	sal_stack_t *stack;
	label_t *entry;
	list_t *ipath;
	list_t *lpath;
	list_t *files;     /* list of source files opened */
	list_t *labels;    /* list of labels */
	list_t *regs;      /* list of objects aka variables */
	// XXX maybe objects and labels must be together :?, if I want source polimorhing
	list_t *libraries; /* list of native libraries opened, maybe global :? */
	obj_t *argv;       /* %av : program arguments vector register */
	obj_t *intv;       /* %iv : interrupt vector register */
	obj_t *thrv;       /* %tv : thread vector */
	obj_t *envr;       /* %ev : environment vector register */
} vm_t;

/* label.h */
label_t *label_new_native(vm_t *vm, char *name, void *ptr);
int label_call(vm_t *vm, label_t *l);
label_t *label_get(vm_t *vm, char *name);
label_t *label_new(vm_t *vm, char *name);
label_t *label_push_ret(vm_t *vm, char *word);

void vm_set_args(vm_t *vm, int argc, const char **argv);
void vm_execute(vm_t *vm, char *opcode);
void vm_free(vm_t *vm);
void vm_exit(vm_t *vm, int ret);
int vm_run(vm_t *vm, const char *filename);
int vm_load(vm_t *vm, const char *filename);
int vm_loadlib(vm_t *vm, const char *libname);
int vm_fetch(vm_t *vm);
void vm_init(vm_t *vm);

// fork   - duplica l'estructura vm per complert
// thread - crea una nova struct vm amb pila nova, pero pool d'objectes compartit
vm_t *vm_new(int argc, const char **argv, const char **environ, int sal_stack_size);
void vm_shutdown(vm_t *vm);

extern int main_interrupt;
extern int exit_value;

#define vm_callback *(void *)(vm_t *)

#define VM_DEBUG if (vm->debug)
#define VM_VERBOSE if (vm->verbose)
#define VM_OUTPUT if (vm->output)
#define VM_PRINT(x,y) printf(" [%s:0x%04x] "x, (char *)vm->file->name, (unsigned int)file_offset(vm->file), y)

#include "register.h"

object_type value_type(vm_t *vm, char *name);
object_type pointer_type(vm_t *vm, char *name);

// XXX hardcoded value ???
#define SIZE_OFF_T 4

#if SIZEOF_OFF_T == 8
#define OFF_FMT "0x%08llX"
#define OFF_FMTs "0x%08llX"
#define OFF_FMTx "%llX"
#define OFF_FMTd "%lld"
#define offtd long long
#define offtx unsigned long long
#else
#define OFF_FMT "%08X"
#define OFF_FMTx "%X"
#define OFF_FMTd "%d"
#define offtd int
#define offtx unsigned int
#endif

#endif



