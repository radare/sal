/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "vm.h"
#include "interrupt.h"

void print_usage() {
	printf("Usage: sal [-V] [-dv] [-IL path] [-s stacksize] [-o file] [filename|-]\n");
	printf("  -d           debug prompt foreach opcode. ENV('DEBUG');\n");
	printf("  -v           show verbose information for execution. ENV('VERBOSE');\n");
	printf("  -V           print version number.\n");
	printf("  -I [ipath]   include directories.\n");
	printf("  -L [lpath]   library path.\n");
	printf("  -l [libname] link against a sal library. (vm.loadlib)\n");
	//printf("  -s [size]    set the stack size.\n");
	printf("  -o [file]    write opcode to opcode to a file.\n");
}

int main(int argc, const char **argv, const char **environ) {
	const char *file = NULL;
	vm_t *vm = NULL;
	//label_t *eip;
	int c;

	if (argc<2) {
		print_usage ();
		return 1;
	}

	vm = vm_new (argc, argv, environ, 256);

	while ((c = getopt (argc, (char*const*)argv, "s:ho:vVdI:L:l:")) != -1) {
		switch (c) {
		case 'l':
			{
			char buf[1024];
			strcpy(buf, "libsal");
			strcat(buf, optarg);
			strcat(buf, ".so");
			vm_loadlib(vm, buf);
			}
			break;
		case 'I':
			vm->ipath = list_add(&vm->ipath, optarg);
			break;
		case 'L':
			vm->lpath = list_add(&vm->lpath, optarg);
			break;
		case 'h':
			print_usage();
			return 0;
		/* TODO: Implement a binary form */
		case 'o':
			vm->output = fopen(optarg,"w");
			if (vm->output == NULL) {
				fprintf(stderr, "Cannot open %s for writing.\n", optarg);
				vm_exit(vm, 1);
			}
			break;
		case 'd':
			vm->debug = 1;
			break;
		case 's':
			// TODO
			break;
		case 'v':
			vm->verbose = 1;
			break;
		case 'V':
			printf("%s\n", VERSION);
			exit(0);
			break;
		}
	}

	vm_set_args (vm, argc, argv);

	if (getenv ("DEBUG"))
		vm->debug = 1;

	if (getenv ("VERBOSE"))
		vm->verbose = 1;

	if (optind >= argc) {
		print_usage();
		return 0;
	}
	file = argv[optind++];

	if (vm->debug)
		printf("\nSAL debugger mode. Type '?' for help.\n\n");

	if (!strcmp(file, "-")) {
		c = vm_run(vm, file);
		return -c;
	} else {
		label_t *eip;
		c = vm_load(vm, file);
		if (c == -1) {
			printf("Cannot load file: %s\n", file);
			vm_exit(vm, 1);
		}
		eip = (label_t *)label_get(vm, "main");
		if (eip == NULL)
			file_seek(vm->file, 0);
		else
			label_call(vm, eip);
	}

#if defined(PROFILER)
	profiler_init();
#endif

	while(vm_fetch(vm));

	if (interrupt_call(vm, INT_SHDW))
		while(vm_fetch(vm));

	vm_free(vm);

	return 0;
}
