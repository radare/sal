/*

calltables are this:
 - 4 byte arrays of N size
 - first 4 bytes are a big endian int value of items in array

*/

vm {
	obj_t objects[256];
}

struct obj_t {
	int objid;
	char name[8]; // v*,memcpy,,
}

struct class_t {
	int class_id = CLASS_INTEGER;
	char name[8];
	void *data = 3;
	ctable_t ct[256];
}

// integer, float have methods for internalize or externalize the value
// integer_get_value()
// integer_get_external_value()

#define ctable_t void *
#define ct_get_size(x) (int)calltable[0]
#define ct_set_size(x) (int)calltable[0]=x

void sal2_calltable_add(calltable_t *ct, void *ptr)
{
	ct_set_size(ct_get_size(ct)+1);
	ct[ct_get_size(ct)] = ptr;
}

void sal2_calltable_new()
{
	void *calltable[256*sizeof(void *)];

	memset(&calltable, '\0', sizeof(calltable));

	calls++
	calltable[0] = (void *)&libvm_int;
	calltable[1] = (void *)&libvm_nop;
}
