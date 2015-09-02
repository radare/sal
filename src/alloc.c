/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"

void *sal_malloc(size_t size) {
	if (size<1)
		size=1;

	void *ptr = (char *)malloc(size);
	if (ptr == NULL) {
		printf("Cannot alloc %d bytes.\n", (int)size);
		exit(1);
	}
	return ptr;
}

char *sal_strdup(const char *s)
{
	char *ptr;
	if (s == NULL)
		return NULL;
	ptr = strdup(s);
	if (ptr == NULL) {
		printf("Cannot alloc %d bytes.\n", (int)strlen (s));
		exit(1);
	}
	return ptr;
}

void *sal_realloc(void *chunk, size_t size)
{
	void *ptr = realloc(chunk, size);
	if (ptr == NULL) {
		printf("Cannot alloc %d bytes.\n", (int) size);
		exit(1);
	}
	return ptr;
}

void sal_free(void *ptr) {
	//printf("[alloc:free] %p\n", ptr);
	free (ptr);
}

void alloc_report() {
	printf ("Allocated chunks: 0\n");
}
