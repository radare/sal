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
#include <getopt.h>
#include <dlfcn.h>
#include <stdlib.h>
#undef __USE_BSD
#undef __USE_XOPEN_EXTENDED
#include <signal.h>
#include "vm.h"
#include "file.h"
#include "flags.h"
#include "interrupt.h"
#include "libvm.h"
#include "array.h"

#define ADD_VMLABEL(x,y) \
	sym = (label_t *)label_new_native((vm_t *)vm, (char *)x, (void *)&libvm_##y); \
	vm->labels = (list_t *)list_obj_set((list_t **)&vm->labels, (object_t *)sym);

#if defined(PROFILER)
struct profiler {
	int opcodes;
	time_t timebase;
};
struct profiler profiler;

void profiler_init()
{
	profiler.opcodes = 0;
	profiler.timebase = time(NULL);
}

void profiler_dump(vm_t *vm)
{
	time_t vmuptime = profiler.timebase; //(vm)?vm->uptime:0;
	time_t uptime = time(NULL);
	uptime-=profiler.timebase; //(int)((vm)?vm->uptime:time(NULL));

	printf("profiler:\n");
	printf(" %d ops\n", profiler.opcodes);
	if (uptime) {
		printf(" %d secs %d, %d\n", uptime, vmuptime, time(NULL));
		printf(" %d op/sec\n", profiler.opcodes/uptime);
	}
}
#endif

void vm_init(vm_t *vm)
{
	label_t *sym;

	/* io */
	ADD_VMLABEL("print", print)
	ADD_VMLABEL("printf", printf)
	ADD_VMLABEL("println", println);
	ADD_VMLABEL("open", open)
	ADD_VMLABEL("close", close)
	ADD_VMLABEL("read", read)
	ADD_VMLABEL("readln", readln)
	ADD_VMLABEL("readpw", readpw)
	ADD_VMLABEL("write", write)
	ADD_VMLABEL("tcp.connect", tcpconnect)
	ADD_VMLABEL("tcp.listen", tcplisten)
	ADD_VMLABEL("tcp.accept", tcpaccept)

	ADD_VMLABEL("rmr", rmr);

	/* flags */
	ADD_VMLABEL("getf", getf);
	ADD_VMLABEL("setf", setf);

	/* array */
	ADD_VMLABEL("anew", anew);
	ADD_VMLABEL("anews", anews);
	ADD_VMLABEL("asz", asz);
	ADD_VMLABEL("arsz", arsz);
	ADD_VMLABEL("aget", aget);
	ADD_VMLABEL("aset", aset);

	/* stack */
	ADD_VMLABEL("drop", drop)
	ADD_VMLABEL("push", push)
	ADD_VMLABEL("pop", pop)
	ADD_VMLABEL("dup", dup)
	ADD_VMLABEL("swap", swap)

	/* jumps */
	ADD_VMLABEL("jn", jn)
	ADD_VMLABEL("jp", jp)
	ADD_VMLABEL("jf", jf)
	ADD_VMLABEL("jz", jz)
	ADD_VMLABEL("jnz", jnz)
	ADD_VMLABEL("call", call)
	ADD_VMLABEL("ret", ret)

	/* arithmetic */
	ADD_VMLABEL("shift", shift);
	ADD_VMLABEL("add", add)
	ADD_VMLABEL("mul", mul)
	ADD_VMLABEL("div", div)
	ADD_VMLABEL("nop", nop)
	ADD_VMLABEL("neg", neg)
	ADD_VMLABEL("int", int)
	ADD_VMLABEL("cmp", cmp)

	/* vm namespace */
	ADD_VMLABEL("vm.sleep", sleep)
	ADD_VMLABEL("vm.system", system)
	ADD_VMLABEL("vm.version", version)
	ADD_VMLABEL("vm.fork", fork)
	ADD_VMLABEL("vm.exit", exit)
	ADD_VMLABEL("vm.dump", dump)
	ADD_VMLABEL("vm.load", load)
	ADD_VMLABEL("vm.loadlib", loadlib)
}

vm_t *main_vm = NULL;

void signal_handler(int signal)
{
	switch (signal) {
	case SIGINT:
		system("stty echo");
		if (main_vm)
		interrupt_call(main_vm, INT_INTS);
#if defined(PROFILER)
		profiler_dump(NULL);
#endif
		break;
	}
}

char *hardcoded_ipath[] = {
	"./",
	"/usr/include/",
	NULL
};
char *hardcoded_lpath[] = {
	"./",
	LIBDIR"/",
	"/usr/lib/",
	NULL
};

void vm_set_args(vm_t *vm, int argc, const char **argv) {
	int i;
	int idx = argc-optind-1;
	int delta = argc-idx;

	if (idx<0) {
		printf ("Missing arguments\n");
		exit(1);
	}
	vm->argv = array_new(idx);
	for(i=0;i<idx;i++)
		array_set(vm->argv, i, string_new(argv[i+delta]));
}

vm_t *vm_new(int argc, const char **argv, const char **environ, int sal_stack_size) {
	register int i;
	vm_t *vm = (vm_t *)sal_malloc(sizeof(vm_t));

	vm->file     = NULL;
	vm->verbose  = 0;
	vm->debug    = 0;
	vm->output   = NULL;
	vm->uptime   = time(NULL);
	vm->entry    = NULL;
	vm->flags    = FLAGS_CLEAR;
	vm->files    = list_new();
	vm->stack    = sal_stack_new( sal_stack_size );
	vm->callback = 0;
	vm->labels   = list_new();
	vm->regs     = list_new();
	vm->argv     = NULL; // vm_set_args(vm, argc, argv);

	/* paths */
	vm->ipath    = list_new();
	for(i = 0; hardcoded_ipath[i]; i++)
		vm->ipath = list_add(&vm->ipath, hardcoded_ipath[i]);
	vm->lpath = list_new();
	for(i = 0; hardcoded_lpath[i]; i++)
		vm->lpath = list_add(&vm->lpath, hardcoded_lpath[i]);

	vm->intv = array_new(10);
	interrupt_set(vm, INT_SHDW,
		label_new_native(vm, "exit", int_shutdown));
	interrupt_set(vm, INT_ESTK,
		label_new_native(vm, "empty-stack", int_empty_stack));
	interrupt_set(vm, INT_INVO,
		label_new_native(vm, "invalid-object", int_invalid_object));
	interrupt_set(vm, INT_INTS,
		label_new_native(vm, "interrupt-singal", int_interrupt_signal));
	interrupt_set(vm, INT_ZERO,
		label_new_native(vm, "zero-division", int_zero_division));

	vm->envr = array_new(1);
	for(i = 0; environ[i] != NULL;i++) {
		array_resize(vm->envr, i+1);
		array_set(vm->envr, i, string_new(environ[i]));
	}

	vm_init( vm );
	signal(SIGINT, signal_handler);
	main_vm = vm;

	return vm;
}

void vm_execute(vm_t *vm, char *opcode)
{
	list_t *p = vm->labels;

	while(p != NULL) {
		label_t *l = (label_t *)p->data;
		if (!strcmp(l->name, opcode)) {
			label_call(vm, l);
			break;
		}
		p = p->next;
	}

	if (p == NULL) {
		fprintf(stderr, "WARNING: Invalid symbol '%s'.\n", opcode);
		if (!vm->debug)
			vm_exit(vm, 1);
	}

#if defined(PROFILER)
	profiler.opcodes++;
#endif
}

static void vm_debug(vm_t *vm)
{
	char buf[1024];
	if (vm->debug == 0)
		return;

	while(1) {
		printf("debug> ");
		fflush(stdout);
		buf[0]='\0';
		// TODO: use readline
		fgets(buf, 1024, stdin);
		if (buf[0]==0) {
			printf("Bye!\n");
			vm_exit(vm, 1);
		}
		buf[strlen(buf)-1]='\0';

		// TODO: Commands todo
		// help/h/?
		// <intro> next opcode
		// <seek> <offset>
		if (buf[0]=='\0') {
			// TODO: print next opcode and execute (return)
			return;
		} else
		if (!strcmp(buf,"?")) {
			libvm_dump(vm);
		} else
			vm_execute(vm, buf);
	}
}

int vm_fetch(vm_t *vm)
{
	char *keyword = NULL;

	vm_debug(vm);

	keyword = file_read(vm->file, 0);
	if (keyword == NULL)
		return 0;

	if (keyword[strlen(keyword)-1]==':') {
		keyword[strlen(keyword)-1]='\0';
		vm->labels = list_add(&vm->labels, (void *)label_new(vm, keyword));
		VM_VERBOSE VM_PRINT("%s:              ; label\n", keyword);
	} else
	if (keyword[0]=='/' && keyword[1]=='*') {
		while (1) {
			sal_free(keyword);
			keyword = file_read(vm->file, vm->output);
			if (keyword==NULL) {
				sal_free(keyword);
				return 1;
			}
			if (strstr(keyword,"*/")) // this will fail when */bla
				break;
		}
	} else {
		// XXX:  only show when no branch or label
		VM_OUTPUT fprintf(vm->output, "%s ", keyword);
		vm_execute(vm, keyword);
		VM_OUTPUT fprintf(vm->output,"\n");
	}

	sal_free(keyword);

	return 1;
}

int vm_loadlib(vm_t *vm, const char *libname) {
	register int i;
	list_t *p = vm->lpath;

	VM_VERBOSE VM_PRINT("%s\n", "vm.loadlib");
	while(p != NULL) {
		char *name;
		char *path = p->data;
		void *handle;
		void *ptr;
		char libpath[1024];

		//printf("LPATH: %s\n", path);
		strcpy(libpath, path);
		strcat(libpath, libname);
		handle = dlopen(libpath, RTLD_LAZY);
		if (handle == NULL) {
			char *s = malloc (strlen (libpath)+10);
			sprintf (s, "%s.so", libpath);
			handle = dlopen(s, RTLD_LAZY);
			free (s);
		}
		if (handle == NULL) {
			char *s = malloc (strlen (libpath)+10);
			sprintf (s, "%s.dylib", libpath);
			handle = dlopen(s, RTLD_LAZY);
			free (s);
		}
		if (handle == NULL) {
			VM_VERBOSE printf("vm.loadlib: %s\n", dlerror());
		} else {
			char **salsyms = dlsym(handle, "salsyms");
			if (salsyms!=NULL) {
				char buf[strlen(*salsyms)+100];
				char nam[1024];

				strcpy(buf, *salsyms);
				name = strtok(buf, ",");
				while(name) {
					strcpy(nam, "sal_"); strcat(nam, name);
					for(i=0;nam[i];i++) if (nam[i]=='.') nam[i]='_';
					ptr = dlsym(handle, nam);
					if (ptr == NULL)
						printf("cannot resolve symbol: %s (%s).\n", nam, name);
					else	vm->labels = list_add(&vm->labels,
							(void *)label_new_native(vm, name, ptr));
					name = strtok(NULL, ",");
				}
			} else {
				printf("This is not a sal library.\n");
			}

			sal_stack_push (vm->stack, integer_new_i (1));
			//dlclose(handle); // IMHO no
			return 1;
		}
		p = p->next;
	}

	fprintf(stderr, "vm.loadlib: Library '%s' not found!\n", libname);

	sal_stack_push (vm->stack, integer_new_i (0));
	return 0;
}

// TODO: ipath usage!!
int vm_load(vm_t *vm, const char *filename) {
	char *word;
	char realname[1024];
	struct file_t *ft = NULL;

	/* CPP support */
	strncpy(realname, filename, 1000);
	realname[strlen(realname)]='\0';
	strcat(realname, "\2\3");

	if ((word = strstr(realname,".gpp\2\3"))) {
		char buf[1024];
		word[0]='\0';
		sprintf(buf, "gpp -I"DATADIR"/sal/include %s > %s", filename, realname);
		system(buf);
		filename = realname;
	} else
	if ((word = strstr(realname,".m4\2\3"))) {
		char buf[1024];
		word[0]='\0';
		sprintf(buf, "m4 -I"DATADIR"/sal/include %s > %s", filename, realname);
		system(buf);
		filename = realname;
	} else
	if ((word = strstr(realname,".cpp\2\3"))) {
		char buf[1024];
		word[0]='\0';
		sprintf(buf, "cpp -I"DATADIR"/sal/include %s > %s", filename, realname);
		system(buf);
		filename = realname;
	}

	ft = file_open (filename);
	if (ft == NULL) {
		printf("Cannot open file %s\n", filename);
		return -1;
	}

	vm->files = list_add(&vm->files, (void *)ft);
	vm->file  = ft;

	for(;;)
	{
		char *keyword = file_read(vm->file, NULL);
		if (keyword == NULL)
			break;
		if (keyword[strlen(keyword)-1]==':') {
			keyword[strlen(keyword)-1]='\0';
			VM_DEBUG VM_PRINT("%s:              ; label\n", keyword);
			vm->labels = list_add(&vm->labels, (void *)label_new(vm, keyword));
		} 
		sal_free(keyword);
	}

	VM_DEBUG printf("vm_load: %s\n", filename);
	/* sure ? */
	//file_close(ft);

	return 0;
}

int vm_run(vm_t *vm, const char *filename)
{
	struct file_t *ft = NULL;

	VM_DEBUG printf("Debugger mode. Type '?' for help.\n");

	ft = file_open(filename);
	if (ft == NULL) return -1;

	vm->files = list_add(&vm->files, (void *)ft);
	vm->file = ft;
	if (vm->entry == NULL)
		vm->entry = (label_t *)label_new(vm,"_entry");

	while(vm_fetch(vm));

	interrupt_call(vm, INT_SHDW);

	/* sure ? */
	//file_close(ft);

	return 0;
}

void vm_exit(vm_t *vm, int ret)
{
	vm_free(vm);

#if defined(PROFILER)
	profiler_dump(vm);
	printf("vm_exit\n");
#endif

	exit(ret);
}

int exit_value = 0;

void vm_shutdown(vm_t *vm)
{
	VM_VERBOSE printf(">> vm shutdown!\n");

	//interrupt_call(vm, INT_SHDW);
	vm_exit(vm, exit_value);
}

void vm_free(vm_t *vm)
{
	// TODO: Implement a foreach function for list_foreach
	file_close(vm->file);
	if (vm->output)
		fclose(vm->output);
	sal_stack_free(vm->stack);
/*
	list_free(vm->files);
	list_free(vm->labels);
	list_free(vm->objects);
*/
	sal_free(vm);
}

