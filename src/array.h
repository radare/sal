#include "object.h"

typedef struct {
	int size;
	object_t **cells;
} array_t;

obj_t *array_new(int size);
obj_t *array_get(obj_t *obj, int pos);
int array_set(obj_t *obj, int pos, obj_t *set);
int array_dset(obj_t *obj, int pos, obj_t *set);
void array_free(array_t *arr);
int array_size(obj_t *obj);
#define array_length(x) array_size(x)
int array_resize(obj_t *obj, int size);
