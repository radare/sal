#include "vm.h"
#include "hash.h"
#include "flags.h"
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *salsyms = "regex.new,regex.match,regex.free";

#define MAGIC "\x19\x24\x81\x05"

struct sal_regex {
	char magic[4];
	regex_t preg;
	vm_t *vm;
};

void sal_regex_new(vm_t *vm)
{
	obj_t *s = sal_stack_pop(vm->stack);
	struct sal_regex *se;
	obj_t *obj;
	int ret;

	if (s->type!=OBJECT_STRING) {
		printf("libsalregex: Expecting 1 string on sal_stack.\n");
		vm_exit(vm, 1);
	}

	se = (struct sal_regex *)malloc(sizeof(struct sal_regex));
	memcpy(se->magic, MAGIC, 4);

	vm->flags = FLAGS_CLEAR;
	if (regcomp(&se->preg, s->data, REG_EXTENDED)) {
		vm->flags=FLAGS_ERROR;
		return;
	}

	obj = object_new("regex-handler");
	obj->length = sizeof(struct sal_regex);
	obj->type = OBJECT_NATIVE;
	obj->data   = se;
	sal_stack_push(vm->stack, (void *)obj);
}

void sal_regex_match(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *str = sal_stack_pop(vm->stack);
	struct sal_regex *se = pointer_get_value(obj);

	vm->flags = FLAGS_CLEAR;
	if (str->type == OBJECT_STRING) {
		if (obj->type == OBJECT_NATIVE) {
			struct sal_regex *se = pointer_get_value(obj);
			if (memcmp(se->magic, MAGIC, 4) == 0) {
				if (!regexec(&se->preg, string_get_value(str), (size_t)0, NULL, REG_NOTBOL|REG_NOTEOL)) {
					vm->flags=FLAGS_EXIST;
				}
			} else {
				printf("Invalid magic for native object\n");
			}
		} else {
			printf("Invalid object. Native object expected.\n");
		}
	} else {
		printf("Invalid object to match\n");
		vm_exit(vm, 1);
	}
}

void sal_regex_free(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);

	if (obj->type == OBJECT_POINTER) {
		struct sal_regex *se = pointer_get_value(obj);
		if (!memcmp(obj->data, MAGIC)) {
			regfree(&se->preg);
			free(obj->data);
		}
	}
}
