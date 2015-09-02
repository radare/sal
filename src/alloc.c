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
