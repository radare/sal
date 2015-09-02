
	//interrupt_call(vm, INT_SHDW);
enum {
	INT_SHDW=0, // exit interrupt
	INT_ESTK=1, // empty stack
	INT_FSTK=2, // full stack
	INT_INVO=3, // invalid object
	INT_INTS=4, // interrupt signal received (^C) aka SIGINT
	INT_ZERO=5, // division by 0
	INT_INVL=6, // invalid label
} interrupt_t;


obj_t *interrupt_get(vm_t *vm, int interrupt);
void interrupt_set(vm_t *vm, int interrupt, label_t *lab);
int interrupt_call(vm_t *vm, int integer);
void int_shutdown(vm_t *vm);
void int_interrupt_signal(vm_t *vm);
void int_empty_stack(vm_t *vm);
void int_invalid_object(vm_t *vm);
void int_zero_division(vm_t *vm);

// check if is null too

// TODO: check if 'y' is a label

