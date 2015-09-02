#ifndef _INCLUDE_LIST_H_
#define _INCLUDE_LIST_H_

#include "object.h"

typedef struct ns {
        void *data;
        struct ns *next;
} list_t;

#define list_new() NULL
list_t *list_add(list_t **p, void *data);
void list_remove(list_t **p);
list_t **list_search(list_t **n, int i);
list_t *list_node_get(list_t **l, char *name);
obj_t *list_obj_get(list_t **l, char *name);
list_t *list_obj_set(list_t **p, obj_t *obj);
void list_print(list_t *n);

#endif
