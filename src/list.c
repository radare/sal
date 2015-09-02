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
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "list.h"

list_t *list_add(list_t **p, void *data)
{
    list_t *n = sal_malloc(sizeof(list_t));
    n->next = *p;
    *p = n;
    n->data = data;
    return n;
}

/* remove head */
void list_remove(list_t **p)
{
    if (*p != NULL) {
        list_t *n = *p;
        *p = (*p)->next;
        free(n);
    }
}

#if 0
list_t **list_search(list_t **n, int i) {
    while (*n != NULL) {
        if (((int)(*n)->data) == i) {
            return n;
        }
        n = &(*n)->next;
    }
    return NULL;
}
#endif

void list_print(list_t *n) {
    if (n == NULL) {
        printf("list is empty\n");
    }
    while (n != NULL) {
        printf("print %p %p %p\n", (void *)n, (void *)n->next, n->data);
        n = n->next;
    }
}

list_t *list_node_get(list_t **l, char *name)
{
    list_t **n = l;
    while (*n != NULL) {
	obj_t *obj = (obj_t *)(*n)->data;
	if (!strcmp(obj->name, name))
            return (*n);
        n = &(*n)->next;
    }
    return NULL;
}

obj_t *list_obj_get(list_t **l, char *name)
{
    list_t **n = l;
    while (*n != NULL) {
	obj_t *obj = (obj_t *)(*n)->data;
//obj_t *obj = obj->data;
//printf("NAME: %s\n",name);
//pritf(" %p %p\n", obj->name, obj->data);
//printf("==> O %p OName %s Name %s\n", obj, obj->name, name);
	if (!strcmp(obj->name, name))
            return obj;
        n = &(*n)->next;
    }
    return NULL;
}

list_t *list_obj_set(list_t **p, obj_t *obj)
{
    list_t *f = list_node_get(p, obj->name);
    list_t *n = NULL;

    if (f != NULL) {
	obj_t *tmp = f->data;
	object_free(tmp);
	//free(f->data);
	f->data = obj;
	n = *p;
    } else {
	n = sal_malloc(sizeof(list_t));
	n->next = *p;
	*p = n;
	n->data = (void *)obj;
    }

    return n;
}
