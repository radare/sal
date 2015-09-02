#include "vm.h"
#include "hash.h"
#include <expat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void label_call_back(vm_t *vm, label_t *l);

char *salsyms = "expat.new,expat.parse,expat.free";

struct sal_expat {
	char magic[4];
	vm_t *vm;
	XML_Parser parser;
	label_t *s, *e, *c;
};

#define MAGIC "\x22\x33\x44\x55"

void startElement(void *userData, const char *name, const char **atts)
{
	register int i;
	struct sal_expat *se = userData;
	obj_t *att = hash_new("xml-attributes");

	for(i = 0; atts[i]; i+=2) {
		char *key = (char *)atts[i+0];
		char *val = (char *)atts[i+1];

		hash_set(att, key, string_new(val));
		//printf(" * %s = %s\n", key, val);
	}
	sal_stack_push(se->vm->stack, att);
	sal_stack_push(se->vm->stack, (void *)string_new((char*)name));
	label_call_back(se->vm, se->s);
}

void endElement(void *userData, const char *name)
{
	struct sal_expat *se = userData;

	sal_stack_push(se->vm->stack, (void *)string_new((char *)name));
	label_call_back(se->vm, se->e);
}

void charElement(void *userData, const XML_Char *s, int len)
{
	struct sal_expat *se = userData;
	char *buf = malloc (len+1);

	strcpy(buf, (const char *)s);
	buf[len]='\0';

	sal_stack_push(se->vm->stack, (void *)string_new((char*)buf));
	label_call_back(se->vm, se->c);
	free (buf);
}

void sal_expat_new(vm_t *vm)
{
	obj_t *s = sal_stack_pop(vm->stack);
	obj_t *e = sal_stack_pop(vm->stack);
	obj_t *c = sal_stack_pop(vm->stack);
	struct sal_expat *se;
	obj_t *obj;

	if (s->type!=OBJECT_LABEL
	||  e->type!=OBJECT_LABEL
	||  c->type!=OBJECT_LABEL) {
		printf("Expecting 3 labels\n");
		exit(1);
	}

	se = (struct sal_expat *)malloc(sizeof(struct sal_expat));
	memcpy (&se->magic, MAGIC, 4);
	se->parser = XML_ParserCreate(NULL);
	se->vm = vm;
	se->s = s->data;
	se->e = e->data;
	se->c = c->data;
	XML_SetUserData(se->parser, se);
	XML_SetElementHandler(se->parser, startElement, endElement);
	XML_SetCharacterDataHandler(se->parser, charElement);

	obj = pointer_new("expat-handler", NULL);
	obj->length = sizeof(struct sal_expat);
	obj->data   = se;
	sal_stack_push(vm->stack, (void *)obj);
}

void sal_expat_parse(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *str = sal_stack_pop(vm->stack);
	struct sal_expat *se = pointer_get_value(obj);
	file_t *tmp;
	int done;

	tmp = vm->file;
	
	do {
		done = 0;
		if (!XML_Parse(se->parser, string_get_value(str), string_get_length(str), done)) {
		done = 1;
		//handle parse error
		}
	} while (!done);

	vm->file = tmp;

//	printf("JMPTO%d\n", file_offset(vm->file));
}

void sal_expat_free(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);

	if (obj->type == OBJECT_POINTER) {
		struct sal_expat *se = pointer_get_value(obj);
		if (!memcmp(obj->data, MAGIC, 4)) {
			se = pointer_get_value(obj);
			XML_ParserFree(se->parser);
			free(obj->data);
		}
	}
}
