/* Copyright (C) 2006-2015 - BSD - pancake */

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "vm.h"
#include "hash.h"
#include "interrupt.h"
#include "stack.h"
#include "label.h"
#include "object.h"
#include "flags.h"
#include "integer.h"
#include "float.h"
#include "register.h"
#include "libvm.h"
#include "array.h"

/* TODO: opcodes
pusha, popa
div - push 2 push 6 div 2 ; = 3
shr - shift right
shl - shift left

split ; push "," push "This,is,new" split
join  ; 

icast, scast, fcast - cast for different kind of objects
get/setf -> gf/sf?
*/

void libvm_loadlib(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	if (obj->type != OBJECT_STRING) {
		fprintf(stderr, "Invalid object in stack. Expecting string.\n");
		exit(1);
	}
	vm_loadlib(vm, string_get_value(obj));
	object_free(obj);
}

/**
 * @syntax: vm.version
 * @descr.: pushes the string of the sal version
 * @exmple: vm.version println
 */
void libvm_version(vm_t *vm)
{
	VM_VERBOSE VM_PRINT("%s\n", "vm.version");
	sal_stack_push(vm->stack, (void *)string_new(VERSION));
}

/**
 * @syntax: vm.system
 * @descr.: executes the last string the stack
 * @exmple: push "ls /" vm.system
 */
void libvm_system(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);

	VM_VERBOSE VM_PRINT("%s\n", "vm.system");
	if (obj->type!= OBJECT_STRING) {
		printf("String expected in stack\n");
		interrupt_call(vm, INT_INVO);
		exit(1);
	}

	system(string_get_value(obj));
}

/**
 * @syntax: drop <times>
 * @descr.: pops <times> elements from stack and destroy't
 * @exmple: push 3 push 2 drop 2 ; does nothing
 */
void libvm_drop(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	int times = atoi(word);
	VM_VERBOSE VM_PRINT("drop %d\n", times);
	
	// TODO do not crash when stack is empty
	for(;times;times--) {
		object_t *obj = sal_stack_pop(vm->stack);
		object_free(obj);
	}
}

/**
 * @syntax: getf
 * @descr.: pushes an integer object with the register flag value and clears the flag to 0.
 * @exmple: getf jnz error
 */
void libvm_getf(vm_t *vm)
{
	obj_t *obj = integer_new_i(vm->flags);
	VM_VERBOSE VM_PRINT("%s\n", "getf");

	vm->flags = FLAGS_CLEAR;

	sal_stack_push(vm->stack, obj);
}

/**
 * @syntax: setf
 * @descr.: sets the flag register to the interger previously pushed
 * @exmple: push 0 setf; clears the flag register (sf), (gf) ?
 */
void libvm_setf(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "setf");

	if (obj->type != OBJECT_INTEGER) {
		printf("No integer in stack!\n");
		return;
	}

	vm->flags = integer_get_value(obj);
}

/**
 * jf: jump if flags != 0 (FLAGS_CLEAR)
 *
 */
void libvm_jf(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	label_t *l = (label_t *)label_get(vm, word);
	VM_VERBOSE VM_PRINT("%s\n", "jf");
	if (vm->flags != FLAGS_CLEAR)
		label_call(vm, l);
		
	vm->flags = FLAGS_CLEAR;
}

/**
 * @syntax: shift N
 * @descr.: shifts the poped value and pushes the result
 * @exmple: push 10 shift 1 println 1 ; 10>>1 = 5
 *          push 10 shift -1 println 1 ; 10<<1 = 
 */
void libvm_shift(vm_t *vm)
{
	int n,i;
	obj_t *obj = sal_stack_pop(vm->stack);
	char *word = file_read(vm->file, vm->output);
	object_type type = value_type(vm, word);

	if (type != OBJECT_INTEGER) {
		printf("Expecting integer here\n");
		exit(1);
	}

	switch(obj->type) {
	case OBJECT_INTEGER:
		n = integer_get_value(obj);
		i = integer_get_value_i(word);
		if (i>0) {
			integer_set_value(obj, n>>i);
		} else
		if (i<0) {
			integer_set_value(obj, n<<-i);
		}
		break;
	case OBJECT_REGISTER:
		break;
	case OBJECT_STRING:
		n = integer_get_value_i(word);
		if (n>0) {
			char *str;
			if (n>obj->length) n = obj->length;
			str = string_get_value(obj);
			for(i = obj->length; i >= obj->length-n-1; i--)
				str[i]='\0';
		} else
		if (n<0) {
			char *str;
			n = -n;
			if (n>obj->length) n = obj->length;
			str = string_get_value(obj);
			for(i=0;i<obj->length;i++)
				str[i] = str[i+n];
			obj->length = obj->length - n;
		}
		break;
	default:
		fprintf(stderr, "Unknown type here\n");
		vm_exit(vm, 1);
		break;
	}
	
	sal_stack_push(vm->stack, obj);
}

/**
 * @syntax: int int
 * @descr.: reads N bytes from the filedescriptor
 * @exmple: push 128 push %r0 read ; reads 128 bytes from the %r0 fd
 * @uses  : cpu flags
 */
void libvm_read(vm_t *vm)
{
	char *buf;
	int fd, len;
	obj_t *fdo = sal_stack_pop(vm->stack);
	obj_t *obj = sal_stack_pop(vm->stack);

	VM_VERBOSE VM_PRINT("%s\n", "read");

	if (fdo->type != OBJECT_INTEGER) {
		printf("No integer in stack! (filedescriptor expected)\n");
		return;
	}

	if (obj->type != OBJECT_INTEGER) {
		fprintf(stderr, "No integer in stack!\n");
		return;
	}

	fd  = integer_get_value(fdo);
	len = integer_get_value(obj);

	if ( len < 0 ) len = 0;

	vm->flags = FLAGS_CLEAR;
	buf = (char *)malloc( len );
	len = read(fd, buf, len);
	if (len < 1)
		vm->flags = FLAGS_ERROR;

	sal_stack_push( vm->stack, string_bin_new(buf, len) );
}

/**
 * @syntax: int
 * @descr.: calls the interrupt #(integer) inside the interrupt vector register
 * @exmple: push 0 int ; calls the EXIT interrupt
 */
void libvm_int(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "int");

	if (obj->type != OBJECT_INTEGER) {
		printf("No integer in stack!\n");
		return;
	}

	interrupt_call(vm, integer_get_value(obj));
	//sal_stack_push( vm->stack, (void *)integer_new_i( array_size(obj) ) );
}

/**
 * @syntax: anews
 * @descr.: creates and initializes a new array object with pop elements
 * @exmple: push "one" push "two" push 2 anews
 */
void libvm_anews(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	int i, times = integer_get_value(obj);
	obj_t *arr;

	VM_VERBOSE VM_PRINT("%s\n", "anews");
	
	arr = array_new(times);

	for(i=0;i<times;i++) {
		obj_t *obj2 = sal_stack_pop(vm->stack);
		array_set(arr, i, obj2);
	}

	sal_stack_push(vm->stack, arr);
}

/**
 * @syntax: anews
 * @descr.: creates a new array object with pop elements
 * @exmple: push "one" push "two" push 2 anews
 */
void libvm_anew(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *arr;
	VM_VERBOSE VM_PRINT("%s\n", "anew");

	if (obj->type != OBJECT_INTEGER) {
		printf("Unexpected object found!\n");
		return;
	}
	
	arr = array_new(integer_get_value(obj));
	if (arr == NULL) {
		printf("Cannot create array!\n");
		return;
	}
	sal_stack_push(vm->stack, (void *)arr);
}

/**
 * @syntax: asz
 * @descr.: pushes the size of the array or string poped from the stack
 * @exmple: push %av asz pop %r0
 */
void libvm_asz(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "asz");

	switch(obj->type) {
	case OBJECT_ARRAY:
		sal_stack_push( vm->stack, (void *)integer_new_i( array_size(obj) ) );
		break;
	case OBJECT_STRING:
		sal_stack_push( vm->stack, (void *)integer_new_i( obj->length ) );
		break;
	default:
		fprintf(stderr, "No array in stack!\n");
		interrupt_call(vm, INT_INVO);
		return;
	}
}

/**
 * @syntax: aget
 * @descr.: pushes the element found in the poped element in the poped array
 * @exmple: push 0 push %av aget println 1 ; gets the program name
 */
void libvm_aget(vm_t *vm)
{
	obj_t *tmp;
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *pos = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "aget");

	if (obj->type == OBJECT_ARRAY && pos->type == OBJECT_INTEGER) {
		tmp = array_get(obj, integer_get_value(pos));

		if (tmp == NULL) {
			//printf("NULL REF\n");
			vm->flags = FLAGS_EXIST; // does not exist!
		} else {
			if (tmp->type == OBJECT_STRING)
				tmp->data = strdup(tmp->data);
			//tmp = tmp->data;
			sal_stack_push (vm->stack, (void *)tmp);
		}
		return;
	}

//printf("obj %d pos %d\n", obj->type, pos->type);

	if (obj->type == OBJECT_HASH &&  pos->type == OBJECT_STRING) {
		tmp = hash_get(obj, pos);
		if (tmp) {
			sal_stack_push (vm->stack, (void *)tmp);
		} else {
			vm->flags = FLAGS_EXIST;
		}
		return;
	}

	interrupt_call(vm, INT_INVO);
}

/**
 * @syntax: aset
 * @descr.: sets the poped object to the poped position in the poped array
 * @exmple: push "obj" push 0 push %r0 aset ; %r0[0]="obj"
 */
void libvm_aset(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *pos = sal_stack_pop(vm->stack);
	obj_t *set = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "aset");

	if (obj->type != OBJECT_ARRAY) {
		printf("No array in stack!\n");
		return;
	}
	if (pos->type != OBJECT_INTEGER) {
		printf("No integer in stack!\n");
		return;
	}

	array_set(obj, integer_get_value(pos), set);
}

/**
 * @syntax: arsz
 * @descr.: resizes the poped object to the poped size
 * @exmple: push 10 push %r0 arsz ; like, arsz=realloc(arsz,10);
 */
void libvm_arsz(vm_t *vm)
{
	int err;
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *pos = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "arsz");

	if (obj->type != OBJECT_ARRAY) {
		printf("No array in stack!\n");
		return;
	}
	if (pos->type != OBJECT_INTEGER) {
		printf("No integer in stack!\n");
		return;
	}

	err = array_resize(obj, integer_get_value(pos));

	sal_stack_push(vm->stack, (void *)obj);
}

/* rmr r0 */
void libvm_rmr(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	object_type type = value_type(vm, word);
	obj_t *obj = NULL;

	VM_VERBOSE VM_PRINT("rmr %s\n", word);
	if (word == NULL)
		return;
	
	switch( type ) {
	case OBJECT_REGISTER:
		obj = get_register_object(vm, word);
		object_free(obj);
		break;
	default:
		// XXX todo
		break;
	}

	sal_free(word);
}

/**
 * @syntax: nop
 * @descr.: does theorically nothing O:)
 * @exmple: nop nop nop nop ; does nothing 4 times
 */
void libvm_nop(vm_t *vm)
{
	/* do nothing */
	int status;
	waitpid(-1, &status, 0);
}

/**
 * @syntax: dup 3
 * @descr.: pops N elements from stack and pushes N*2
 * @exmple: push 3 dup 1 add 2 println ; show 6
 */
void libvm_dup(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	int i, times = atoi(word);
	sal_stack_t *bak = sal_stack_new(100);
	VM_VERBOSE VM_PRINT("dup %s\n", word);

	for(i = 0 ; i< times ; i++) {
		int delta = vm->stack->top - i;
		if (delta<0) {
			printf("Stack underflow\n");
			vm_exit(vm, 1);
		}
		object_t *obj = vm->stack->contents[delta];

		sal_stack_push(bak, obj);
	}

	for(i = 0 ; i< times ; i++) {
		object_t *obj = object_copy(sal_stack_pop(bak));
		sal_stack_push(vm->stack, obj);
	}

	sal_stack_free(bak);
	sal_free(word);
}

/**
 * @syntax: jnz label
 * @descr.: pops the last element of the stack and jumps if is different to 0
 * @exmple: main: push 0 jnz main ; does not loops
 */
int not = 0;
void libvm_jnz(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	label_t *l = (label_t *)label_get(vm, word);
	obj_t *obj = sal_stack_pop(vm->stack);
	char *str = (obj)?obj->data:"";

	if (not==0) {
		VM_VERBOSE VM_PRINT("jnz %s\n", word);
	} else {
		VM_VERBOSE printf("%s\n",word);
	}

	if (l == NULL) {
		fprintf(stderr, "Argument '%s' for 'jnz' must be a label.\n", word);
		sal_free(word);
		exit(1);
	}
	if (obj == NULL) {
		printf("Empty stack.\n");
	}
	switch(obj->type)
	{
	case OBJECT_INTEGER:
		if (not) {
		if (obj->length==0)
			label_call(vm, l);
		} else
		if (obj->length!=0)
			label_call(vm, l);
		break;
	case OBJECT_FLOAT:
		if (not) {
		if ((float)obj->length>0.0f)
			label_call(vm, l);
		} else
		if ((float)obj->length<0.0f)
			label_call(vm, l);
		break;
	case OBJECT_STRING:
		if (str[0]=='\0')
			label_call(vm, l);
		break;
	default:
		fprintf (stderr, "jnz: type is invalid %d\n", obj->type);
		interrupt_call(vm, INT_INVO);
	}

	sal_free(word);
}

/**
 * @syntax:      jz label
 * @description: pops the last element of the stack and jumps if is equal to 0
 * @example:     main: push 0 jz main ; endless loop
 * @see:         jnz, jp, jn
 */
void libvm_jz(vm_t *vm)
{
	VM_VERBOSE VM_PRINT("jz %s", "");
	not = 1;
	libvm_jnz(vm);
	not = 0;
}

/**
 * @syntax:      jp label
 * @description: pops the last element of the stack and jumps if is positive
 * @example:     main: push 3 jp main ; endless loop
 * @see:         jn, jz, jnz
 */
void libvm_jp(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	label_t *l = (label_t *)label_get(vm, word);
	obj_t *obj = sal_stack_pop(vm->stack);
	char *str  = (obj)?obj->data:"";

	if (not==0) {
		VM_VERBOSE VM_PRINT("jp %s\n", word);
	} else {
		VM_VERBOSE printf("%s\n",word);
	}

	if (l == NULL) {
		fprintf(stderr, "Argument '%s' for 'jnz' must be a label.\n", word);
		sal_free(word);
		exit(1);
	}

	if (obj == NULL)
		printf("Empty stack.\n");

	switch(obj->type) {
	case OBJECT_INTEGER:
		if (not) {
		if (obj->length<0)
			label_call(vm, l);
		} else
		if (obj->length>0)
			label_call(vm, l);
		break;
	case OBJECT_FLOAT:
		if (not) {
		if ((float)obj->length<0.0f)
			label_call(vm, l);
		} else
		if ((float)obj->length>0.0f)
			label_call(vm, l);
		break;
	case OBJECT_STRING:
		if (not) {
		if (str[0]=='\0')
			label_call(vm, l);
		} else
		if (str[0]!='\0')
			label_call(vm, l);
		break;
	default:
		interrupt_call(vm, INT_INVO);
		sal_free(word);
		return;
	}

	sal_free(word);
}

/**
 * @syntax: jn label
 * @descr.: pops the last element of the stack and jumps if is negative
 * @exmple: main: push 3 push -6 add jn main ; endless loop
 */
void libvm_jn(vm_t *vm)
{
	VM_VERBOSE VM_PRINT("jn %s", "");
	not = 1;
	libvm_jp(vm);
	not = 0;
}

/*
 * push 3
 * push 0
 * jb bla  ; if 0 < 3
 * if (pop() < pop()) {
 * 
 */
void libvm_jb(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	label_t *l = (label_t *)label_get(vm, word);
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *obj2 = sal_stack_pop(vm->stack);

	/* obj > obj2 */
	if (obj->type == OBJECT_INTEGER && obj2->type == OBJECT_INTEGER)
		if (obj->length > obj2->length)
			
	switch(obj->type) {
	case OBJECT_INTEGER:
		switch(obj2->type) {
		case OBJECT_INTEGER:
			if (obj->length > obj2->length)
				label_call(vm, l);
			break;
		default:
			fprintf(stderr, "Invalid object.\n");
			break;
		}
		break;
	case OBJECT_STRING:
		break;
	default:
		fprintf(stderr, "Invalid object.\n");
		break;
	}

	sal_free(word);
}

/**
 * @syntax: neg
 * @descr.: pops the last element and pushes the negated value (3->-3, "foo","oof",...)
 * @exmple: push 3 neg println 1 ; prints '-3'
 */
void libvm_neg(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *tmo;
	register int i, j, k;
	char tmp;

	switch(obj->type) {
	case OBJECT_INTEGER:
		obj->length = -obj->length;
		break;
	case OBJECT_FLOAT:
		obj->length = (float)-obj->length;
		break;
	case OBJECT_STRING:
		j = strlen(string_get_value(obj))-1;
		k = j/2;
		for(i=0;i<=k;i++) {
			tmp = string_get_value(obj)[i];
			string_get_value(obj)[i] = string_get_value(obj)[j-i];
			string_get_value(obj)[j-i] = tmp;
		}
		break;
	case OBJECT_ARRAY:
		{
#if 0
			obj_t *obj2 = array_new( array_size(obj) );
			array_t *arr = (array_t*)obj2->data;
			memcpy(arr->cells, *((array_t *)obj->data)->cells, sizeof(void *)); //array_t)*(obj2->length));
			j = array_size(obj)-1;
			k = ((int)(j/2))+(j%2);
			for(i=0;i<k;i++) {
				tmo = arr->cells[i];
				arr->cells[i] = arr->cells[j-i];
				arr->cells[j-i] = tmo;
			}
			//object_free(obj);
			obj = obj2;
#else
			array_t *arr = (array_t *)obj->data;
			j = array_size(obj)-1;
			k = ((int)(j/2))+(j%2);
			for(i=0;i<k;i++) {
				tmo = arr->cells[i];
				arr->cells[i] = arr->cells[j-i];
				arr->cells[j-i] = tmo;
			}
#endif
		}
		break;
	default:
		printf("Cannot negate this kind of object.\n");
		vm_shutdown(vm);
	}

	sal_stack_push(vm->stack, obj);
}

/**
 * @syntax: readpw
 * @descr.: does theorically nothing O:)
 * @exmple: push 0 readpw push "Your password is: " println 2 ;
 */
void libvm_readpw(vm_t *vm)
{
	/* XXX: use set_raw_terminal() */
	VM_VERBOSE VM_PRINT("%s\n", "readpw");
	system("stty -echo");
	libvm_readln(vm);
	system("stty echo");
}

/**
 *
 * @syntax: cmp
 * @descr.: swaps 2 elements from the stack and pushes the result of the comparision
 * @exmple: push 1 push 2 cmp println ; will print '1'.
 *
 */
// XXX maybe return in flags not in stack here
void libvm_cmp(vm_t *vm)
{
	object_t *obj1 = sal_stack_pop(vm->stack);
	object_t *obj2 = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "cmp");

	if (obj1 && obj2) {
		if (obj1->type == obj2->type) {
			switch(obj1->type) {
			case OBJECT_POINTER:
				sal_stack_push(vm->stack,
					(void *)integer_new_i((obj1->data==obj2->data)?0:1));
				break;
			case OBJECT_STRING:
				sal_stack_push(vm->stack, (void *)integer_new_i(strcmp(obj1->data, obj2->data)));
				break;
			case OBJECT_INTEGER:
				sal_stack_push(vm->stack, (void *)integer_new_i(obj1->length-obj2->length));
				break;
			default:
				printf("Uncomparable objects in stack.\n");
				vm_exit(vm, 1);
			}
		} else {
			printf("Cannot compare two objects of different type.\n");
			vm_exit(vm, 1);
		}

		object_free(obj1);
		object_free(obj2);
	}
}

/**
 *
 * @syntax: vm.load N
 * @descr.: swaps N elements of the stack
 * @exmple: vm.load "foo"
 * TODO: loads : load binary code from stack
 *
 */
void libvm_load(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	file_t *ft = vm->file;
	object_type ot = value_type(vm, word);

	VM_VERBOSE VM_PRINT("vm.load %s\n", word);
	switch(ot) {
	case OBJECT_STRING:
		vm_load(vm, word+1);
		vm->file = ft;
		break;
	default:
		fprintf(stderr, "Invalid object on stack. Expecting string.\n");
		vm_exit(vm, 1);
	}
}

/**
 *
 * @syntax: swap N
 * @descr.: swaps N elements of the stack
 * @exmple: push 1 push 2 swap 2 println 2 ; will show 12
 *
 */
void libvm_swap(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	int i, times = atoi(word);
	sal_stack_t *s = sal_stack_new(100);
	VM_VERBOSE VM_PRINT("swap %s\n", word);
	
	for(i = 0; i<times; i++) {
		object_t *obj = sal_stack_pop(vm->stack);
		sal_stack_push(s, obj);
	}

	for(i = 0; i<times; i++) {
		object_t *obj = s->contents[i];
		sal_stack_push(vm->stack, obj);
	}

	sal_stack_free(s);
}


void libvm_mul(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *obj2 = sal_stack_pop(vm->stack);
	obj_t *out = NULL;

	switch(obj->type) {
	case OBJECT_INTEGER:
		switch(obj2->type) {
		case OBJECT_INTEGER:
			out = integer_new_i(obj->length * obj2->length);
			break;
		case OBJECT_FLOAT:
			{
			float f;
			memcpy(&f, &obj2->data, sizeof(float));
			f*=obj->length;
			out = float_new_f((float)f);
			}
			break;
		case OBJECT_STRING:
			{
			register int i;
			char buf[obj2->length*obj->length];
			buf[0]='\0';
			for(i=obj->length;i;i--)
				strcat(buf, obj2->data);
			out = string_new( buf );
			}
			break;
		default:
			fprintf(stderr, "Cannot multiply these types. %d %d\n", obj->type, obj2->type);
			break;
		}
		break;
	case OBJECT_FLOAT:
		switch(obj2->type) {
		case OBJECT_FLOAT:
			{
			float f, f2;
			memcpy(&f, &obj->data, sizeof(float));
			memcpy(&f2, &obj2->data, sizeof(float));
			f*=f2;
			out = float_new_f((float)f);
			}
			break;
		case OBJECT_INTEGER:
			{
			float f;
			int f2;
			memcpy(&f, &obj->data, sizeof(float));
			f2 = integer_get_value(obj2);
			f*=f2;
			out = float_new_f((float)f);
			}
			break;
		default:
			fprintf(stderr, "Invalid object type against float. %d\n", obj2->type);
			break;
		}
		break;
	case OBJECT_STRING:
		switch(obj2->type) {
		case OBJECT_INTEGER:
			{
			register int i;
			char buf[obj->length*obj2->length];
			buf[0]='\0';
			for(i=obj2->length;i;i--)
				strcat(buf, obj->data);
			out = string_new( buf );
			}
			break;
		default:
			fprintf(stderr, "Cannot multiply these types. %d %d\n", obj->type, obj2->type);
			break;
		}
		break;
	default:
		fprintf(stderr, "Cannot multiply these types. %d %d\n", obj->type, obj2->type);
	}

	sal_stack_push(vm->stack, out);
}

/*
 * push 5
 * push 10
 * div
 * ; push 2
 */
void libvm_div(vm_t *vm)
{
	obj_t *obj = sal_stack_pop(vm->stack);
	obj_t *obj2 = sal_stack_pop(vm->stack);
	obj_t *out = NULL;

	switch(obj->type) {
	case OBJECT_INTEGER:
		switch(obj2->type) {
		case OBJECT_INTEGER:
			if (integer_get_value(obj2)==0)
				interrupt_call(vm, INT_ZERO);
			else
				out = integer_new_i(obj->length / obj2->length);
			break;
		case OBJECT_FLOAT:
			if (float_get_value(obj2)==0)
				interrupt_call(vm, INT_ZERO);
			else {
				int f = integer_get_value(obj);
				out = float_new_f((float)(f/float_get_value(obj2)));
			}
			break;
		default:
			fprintf(stderr, "Cannot divide these types. %d %d\n", obj->type, obj2->type);
		}
		break;
	case OBJECT_FLOAT:
		switch(obj2->type) {
		case OBJECT_INTEGER:
			if (integer_get_value(obj2)==0)
				interrupt_call(vm, INT_ZERO);
			else {
				float f = float_get_value(obj);
				out = float_new_f((float)f/integer_get_value(obj2));
			}
			break;
		case OBJECT_FLOAT:
			if (float_get_value(obj2)==0)
				interrupt_call(vm, INT_ZERO);
			else {
				float f, f2;
				memcpy(&f, &obj->data, sizeof(float));
				memcpy(&f2, &obj2->data, sizeof(float));
				f /= f2;
				out = float_new_f((float)f);
			}
			break;
		default:
			fprintf(stderr, "Invalid object type against float. %d\n", obj2->type);
		}
		break;
	case OBJECT_STRING:
		switch(obj2->type) {
		case OBJECT_INTEGER: {
			// array with substrings
			char *ptr = string_get_value(obj);
			char *lst = ptr;
			char bak;
			int i, n = 0, len = strlen(ptr);
			int inc = integer_get_value(obj2);
			out = array_new(0);
			for(i=0; i<len; i+=inc ) {
				if ( i+inc > len ) inc = len - i;
				bak        = ptr[i+inc];
				ptr[i+inc] = '\0';
				array_dset(out, n, string_new(lst));
				lst       += inc;
				ptr[i+inc] = bak;
				n++;
			}
			if (i!=len)
				array_dset(out, n, string_new(lst));
			}
			break;
		case OBJECT_STRING: {
			// tokenized array
			// f.ex: "food,is,lame" / "," = [ "food", "is", "lame" ]
			char *ptr = string_get_value(obj);
			char *tok = string_get_value(obj2);
			int   n,i = 0;
			int   len = string_length(obj);
			int   inc = string_length(obj2);
			char *lst = ptr;
			char  bak;

			out = array_new(0);
			for(i=n=0;i<len && (ptr = strstr(ptr, tok));i++) {
				bak = ptr[0];
				ptr[0]='\0';
				array_dset(out, n, string_new(lst));
				ptr[0] = bak;
				lst = ptr + inc;
				ptr = ptr + inc;
				i+=inc; n++;
			}
			array_dset(out, n, string_new(lst));
			}
			break;
		default:
			fprintf(stderr, "Cannot divide these types. %d %d\n", obj->type, obj2->type);
		}
		break;
	default:
		fprintf(stderr, "Cannot divide these types. %d %d\n", obj->type, obj2->type);
	}

	if (out) {
		vm->flags = FLAGS_CLEAR;
		sal_stack_push(vm->stack, out);
	} else
		vm->flags = FLAGS_ERROR;
}

/**
 *
 * @syntax: add N
 * @descr.: pops N elements from stack and pushes the result of the sum
 * @exmple: push 1 push 2 add 2 println 1         ; will show 3
 *          push 23 push "Humans left: " println  ; will show "Humans left: 23"
 *          push "cool." push "is " push "pancake " add 3 println ; will show "pancake is cool."
 * TODO: 
 *
 */
void libvm_add(vm_t *vm)
{
	int result   = 0;
	float phloat = 0;
	char *string = NULL;
	int type     = -1;
	obj_t *oobj  = NULL;
	obj_t *obj   = NULL;
	float f, f2;
	char *word;
	int times;

	VM_VERBOSE VM_PRINT("add %s\n", word);

	word = file_read(vm->file, vm->output);
	if (!word) {
		printf("oops no word?\n");
		return;
	}
	times = integer_get_value_i(word);

	for(;times;times--) {
		obj = sal_stack_pop(vm->stack);

		if (oobj == NULL) {
			oobj = obj;
			continue;
		}
		if (type == -1)
			type = obj->type;

		switch (obj->type) {
		case OBJECT_STRING:
			switch(oobj->type) {
			case OBJECT_STRING:
				string = (char *)malloc(strlen(oobj->data)+strlen(obj->data)+1);
				strcpy(string, oobj->data);
				strcat(string, obj->data);
				break;
			case OBJECT_INTEGER:
				string = (char *)malloc(32);
				sprintf(string, "%d", oobj->length);
				strcat(string, obj->data);
				break;
			default:
				printf("strunk !! %d\n",  oobj->type);
				break;
			}
			break;
		case OBJECT_INTEGER:
			switch(oobj->type) {
			case OBJECT_INTEGER:
				result += obj->length + oobj->length;
				object_free(obj);
				type = OBJECT_INTEGER;
				break;
			case OBJECT_STRING:
				string = (char *)malloc(strlen(oobj->data)+32);
				sprintf(string, "%s%d", (char*)oobj->data, obj->length);
				type = OBJECT_STRING;
				break;
			case OBJECT_FLOAT:
				memcpy(&f, &oobj->data, sizeof(float));
				phloat+=f+integer_get_value(obj);
				type = OBJECT_FLOAT;
				break;
			default:
				printf("kasf\n");
			}
			break;
		case OBJECT_FLOAT:
			switch(oobj->type) {
			case OBJECT_INTEGER:
				memcpy(&f, &obj->data, sizeof(float));
				phloat+=f+integer_get_value(oobj);
				type = OBJECT_FLOAT;
				break;
			case OBJECT_FLOAT:
				memcpy(&f, &obj->data, sizeof(float));
				memcpy(&f2, &oobj->data, sizeof(float));
				phloat+=f+f2;
				type = OBJECT_FLOAT;
				break;
			default:
				fprintf (stderr, "Unsupported float conversion\n");
				break;
			}
			break;
		default:
			fprintf(stderr, "Don't know how to add these objects\n");
			break;
		}

		object_free(oobj);
		oobj = obj;
	}
	object_free(obj);

	vm_flags_set(FLAGS_CLEAR);
	switch(type) {
	case OBJECT_INTEGER:
		sal_stack_push(vm->stack, (void *)integer_new_i(result));
		break;
	case OBJECT_STRING:
		sal_stack_push(vm->stack, (void *)string_new(string));
		break;
	case OBJECT_FLOAT:
		sal_stack_push(vm->stack, (void *)float_new_f(phloat));
		break;
	default:
		vm_flags_set(FLAGS_ERROR);
		break;
	}

	sal_free(word);
}

void libvm_call(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	//list_t *p = vm->labels;

	VM_VERBOSE VM_PRINT("call %s\n", word);

	label_call(vm, label_push_ret(vm, word) );
}

void libvm_dump(vm_t *vm)
{
	int i;
	VM_VERBOSE VM_PRINT("%s\n", "vm.dump");
	printf("===[vm.dump]===\n");
	printf("Labels:");
	list_t *p = vm->labels;
	while(p != NULL) {
		label_t *l = (label_t *)p->data;
		printf(" %s,", l->name);
		p = p->next;
	}
	printf("\nFlags: %02x", vm->flags);
	printf("\nStack:\n");
	if (vm->stack->top == -1){
		printf("  (empty)\n");
	} else
	for(i=vm->stack->top;i>=0;i--) {
		printf("  %02x", vm->stack->top-i);
		obj_t *obj = vm->stack->contents[i];
		object_dump(obj, 2);
	}

	printf("Registers:\n");

	printf("  %%av: (Arguments)\n");
	if (vm->argv)
	object_dump(vm->argv, 5);

	printf("  %%iv: (Interrupts)\n");
	if (vm->intv)
		object_dump(vm->intv, 5);
/*
	printf("       [array]\n");
	for (i=0;i<array_size(vm->intv);i++) {
		obj_t *obj = array_get(vm->intv, i);
		printf("         [%d] = %s\n", i, obj->name);
	}
*/
	p = vm->regs;
	while(p != NULL) {
		obj_t *obj = (obj_t *)p->data;

		printf("  %s: ", obj->name);
		object_dump(obj, 2);
		p = p->next;
	}
	printf("===[vm.dump]===\n");
}

void libvm_pop(vm_t *vm)
{
	char *word  = file_read(vm->file, vm->output);
	object_type type = value_type(vm, word);
	object_type type2;
	obj_t *obj  = sal_stack_pop(vm->stack);
	obj_t *tmp;

	VM_VERBOSE VM_PRINT("pop %s\n", word);

	if (obj == NULL || word == NULL) {
		fprintf(stderr, "Cannot fetch data\n");
		sal_free(word);
		vm_exit(vm, 1);
	}

	switch(type) {
	case OBJECT_REGISTER:
		set_register_object(vm, word, obj);
		object_free(obj);
		break;
	case OBJECT_POINTER:
		type2 = pointer_type(vm, word);
		switch(type2) {
		case OBJECT_REGISTER:
			tmp = get_register_object(vm, word+1);
			set_register_object(vm, tmp->name+4, obj);
			object_free(obj);
			break;
		default:
			fprintf(stderr, "Unsupported object pointer type '%s'\n", word);
		}
		break;
	default:
		fprintf(stderr, "Cannot pop to %s (type : %d)\n", word, type);
		sal_free(word);
		vm_exit(vm, 1);
	}

	sal_free(word);
}

void libvm_fork(vm_t *vm)
{
	int pid = 0;

	VM_VERBOSE VM_PRINT("%s\n", "fork");
	/* TODO: store threads list somewhere */
	pid = fork();
	sal_stack_push(vm->stack, (void *)integer_new_i(pid));
}

void libvm_push(vm_t *vm)
{
	char *word = file_read (vm->file, vm->output);
	object_type type = value_type(vm, word);
	object_type type2;
	obj_t *obj = NULL;

	VM_VERBOSE VM_PRINT ("push %s\n", word);
	//VM_VERBOSE printf(" [%s:%p] push %s\n", vm->file->name, file_offset(vm->file), word);
	if (word == NULL) {
		printf("Missing object to push.\n");
		sal_free(word);
		return;
	}

	//VM_VERBOSE printf("ObjectType: %d\n", value_type(vm, word));

	// push &%r0
	switch (type) {
	case OBJECT_POINTER:
		type2 = pointer_type(vm, word);
		if (type2 != OBJECT_INVALID) {
			switch(type2) {
			case OBJECT_REGISTER:
				obj = pointer_new(word+1, obj);
				break;
			default:
				printf("invalid object here brbr:\n");
			}
		} else {
			printf("jksdlaf\n");
		}
		break;
	case OBJECT_REGISTER:
		obj = get_register_object(vm, word);
		break;
	case OBJECT_HASH:
	case OBJECT_ARRAY:
		obj = object_ref(obj->data);
		break;
	case OBJECT_STRING:
		obj = (obj_t *)string_new(word);
		break;
	case OBJECT_INTEGER:
		obj = (obj_t *)integer_new(word);
		break;
	case OBJECT_FLOAT:
		obj = (obj_t *)float_new(word);
		break;
	case OBJECT_LABEL:
		obj = object_new(word);
		obj->type = OBJECT_LABEL;
		obj->data = (void *)label_get(vm, word);
		if (obj->data == NULL) {
			printf("Invalid label name. '%s'\n",word);
			sal_free(word);
			object_free(obj);
			exit(1);
		}
		break;
	default:
		sal_free(word);
		fprintf (stderr, "push: invalid type: %d (%s)\n", type, word);
		interrupt_call(vm, INT_INVO);
		return;
	}

	sal_stack_push(vm->stack, obj);
	sal_free(word);
}

/* perms 
 * 0 = read
 * 1 = write
 * 2 = readwrite
*/
void libvm_open(vm_t *vm)
{
	int fd = 0;
	int flags = 0;
	object_t *file = sal_stack_pop(vm->stack);
	object_t *flo  = sal_stack_pop(vm->stack);

	VM_VERBOSE VM_PRINT("%s\n", "open");
	if (file->type != OBJECT_STRING) {
		fprintf(stderr, "String was expected\n");
		vm_exit(vm, 1);
	}

	if (flo->type != OBJECT_INTEGER) {
		fprintf(stderr, "Integer expected in stack\n");
		vm_exit(vm, 1);
	}

	switch(integer_get_value(flo)) {
	case 0: flags = O_RDONLY; break;
	case 1: flags = O_WRONLY; break;
	case 2: flags = O_RDWR|O_CREAT;   break;
	}

	fd = open(file->data, flags, 0644);
	if (fd == -1)
		vm->flags = FLAGS_ERROR;
	else	sal_stack_push(vm->stack, (void *)integer_new_i(fd) );
}

/**
 *
 * @syntax: close
 * @descr.: pops the FD from the stack and close't
 * @exmple: push 0 close
 *
 */
void libvm_close(vm_t *vm)
{
	object_t *fdo = sal_stack_pop(vm->stack);
	VM_VERBOSE VM_PRINT("%s\n", "close");
	if (fdo->type == OBJECT_INTEGER)
		close(integer_get_value(fdo));
	else	interrupt_call(vm, INT_INVO);
}

/**
 *
 * @syntax: tcp.connect
 * @descr.: pops the host and port and pushes the related socket
 * @exmple: push 80 push "localhost" tcp.connect
 *
 */
void libvm_tcpconnect(vm_t *vm)
{
	object_t *obj  = sal_stack_pop(vm->stack);
	object_t *obj2 = sal_stack_pop(vm->stack);
        struct sockaddr_in sa;
        struct hostent *he;
        int s; 
	char *host;
	int port;

	if (obj->type!=OBJECT_STRING) {
		printf("1st object is not string\n");
		return;
	}

	host = string_get_value(obj);

	switch(obj2->type) {
	case OBJECT_STRING:
		port = atoi(string_get_value(obj2));
		break;
	default:
		if (obj2->type==OBJECT_REGISTER)
			obj2 = obj2->data;

		if (obj2->type!=OBJECT_INTEGER) {
			printf("2nd object is not integer %d\n", obj2->type);
			return;
		}
		port = integer_get_value(obj2);
	}

	if (port<1 || port>65535) {
		vm->flags = FLAGS_ERROR;
		printf("Invalid port number\n");
		return;
	}

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == -1) {
		vm->flags = FLAGS_ERROR;
                return;
	}
                
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        he = (struct hostent *)gethostbyname( host );
        if (he == (struct hostent*)0) {
		vm->flags = FLAGS_ERROR;
                return;
	}

        sa.sin_addr = *((struct in_addr *)he->h_addr);
        sa.sin_port = htons( port );

        if (connect(s, (const struct sockaddr*)&sa, (socklen_t)sizeof(struct sockaddr))==0) {
		sal_stack_push(vm->stack, (void *)integer_new_i(s) );
		vm->flags = FLAGS_CLEAR;
	} else	vm->flags = FLAGS_ERROR;
}

/**
 *
 * @syntax: tcp.listen
 * @descr.: pops the host and port and pushes the related socket
 * @exmple: push 80 push "localhost" tcp.connect
 *
 */
void libvm_tcplisten(vm_t *vm)
{
	object_t *obj = sal_stack_pop(vm->stack);
        int s;
        int ret;
	int port;
        struct sockaddr_in sa;
        struct linger linger = { 0 };
        linger.l_onoff       = 1;
        linger.l_linger      = 1;

	VM_VERBOSE VM_PRINT("tcp.listen %d\n", integer_get_value(obj));
	if (obj->type == OBJECT_STRING)
		port = atoi(string_get_value(obj));
	else	port = integer_get_value(obj);

	if (port<1 || port>65535) {
		printf("Invalid port number\n");
		vm->flags = FLAGS_ERROR;
		return;
	}

        s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s <0) {
		vm->flags = FLAGS_ERROR;
                return;
	}

        setsockopt(s,
                SOL_SOCKET, SO_LINGER, (const char *) &linger, sizeof(linger));

        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        sa.sin_port = htons( port );

        ret = bind(s, (struct sockaddr *)&sa, sizeof(sa));
        if (ret < 0) {
		vm->flags = FLAGS_ERROR;
                return;
	}

        ret = listen(s, 1);
        if (ret < 0) {
		vm->flags = FLAGS_ERROR;
                return;
	}

	sal_stack_push(vm->stack, (void *)integer_new_i(s) );
}

void libvm_tcpaccept(vm_t *vm)
{
	object_t *obj = sal_stack_pop(vm->stack);

	VM_VERBOSE VM_PRINT("tcp.accept %d\n", integer_get_value(obj));
	if (obj->type != OBJECT_INTEGER) {
		printf("Object in stack is not an integer\n");
		interrupt_call(vm, INT_INVO);
		return;
	}

	// TODO: store remote host,port info
	sal_stack_push(vm->stack, (void *)integer_new_i(accept(integer_get_value(obj), NULL, NULL) ));
}

void libvm_write(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	int times = atoi (word); // XXX

	VM_VERBOSE VM_PRINT("write %s\n", word);
	object_t *fdo = sal_stack_pop(vm->stack);
	if (fdo->type == OBJECT_INTEGER) {
		for(;times;times--) {
			object_t *obj = sal_stack_pop(vm->stack);
			FILE *fd = NULL;
			switch(obj->type) {
			case OBJECT_INTEGER:
				fd = fdopen(integer_get_value(fdo), "w");
				fprintf(fd,"%d",integer_get_value(obj));
				fflush(fd);
				break;
			case OBJECT_STRING:
				write(integer_get_value(fdo),obj->data, obj->length);
				break;
			default:
				printf("ERRORR %s\n", obj->name);
				break;
			}
		}
	} else {
		printf("Stack object is not valid.\n");
		return;
	}

	sal_free(word);
}


/**
 *
 * @syntax: readln
 * @descr.: pops FD from stack and pushes a string readed from the FD
 * @exmple: push 0 readln println 1  ; echo
 *
 */
void libvm_readln(vm_t *vm)
{
	char buf[1024];
	obj_t *obj;
        int i = 0;
        int ret = 0;
	int fd;
	object_t *fdo = sal_stack_pop(vm->stack);

	VM_VERBOSE VM_PRINT("%s\n", "readln");
	fflush(stdin);
	fflush(stdout);

	if (fdo->type == OBJECT_INTEGER) {
		fd = integer_get_value(fdo);
	} else {
		fprintf(stderr, "Invalid object in stack.\n");
		vm_exit(vm, 1);
	}

        //signal(SIGPIPE,SIG_IGN);
	VM_DEBUG { printf("\n<input-line> "); fflush(stdout); }

        while(i<sizeof(buf)) {
                ret = read(fd, buf+i, 1);
                if (ret<=0) {
                        shutdown(fd, SHUT_RDWR);
                        close(fd);
			if (i==0) {
				vm->flags = FLAGS_ERROR;
				return;
			} else	break;
                }
                /*if (buf[i]==4) { // ^D
                        shutdown(config.input,SHUT_RDWR);
                        close(config.input);
                        config.input = -1;
                        break;
                }
                */
                if (buf[i]=='\r'||buf[i]=='\n')
                        break;
                i+=ret;
        }
        buf[i]='\0'; // XXX 1byte overflow

	vm->flags = FLAGS_CLEAR; // probably not needed

	obj = string_new(buf);

	sal_stack_push(vm->stack, obj);
}

/**
 *
 * @syntax: ret
 * @descr.: pops the label from the stack and jumps there.
 * @exmple: push main ret
 * @seelso: call
 *
 */
void libvm_ret(vm_t *vm)
{
	obj_t *obj;

	VM_VERBOSE VM_PRINT("%s\n", "ret");

	if (vm->callback>0) {
		vm->callback--;
		return;
	}

	obj = sal_stack_pop(vm->stack);
	if (obj->type != OBJECT_LABEL) {
		printf("return: Stack object is not a label\n");
		vm_exit(vm, 1);
	}

	label_call(vm, (label_t *)obj->data);
	object_free(obj);
}

int shutdown_process = 0;

/**
 *
 * @syntax: vm.exit
 * @descr.: pops the return value to exit the vm.
 * @exmple: push 0 vm.exit
 * @seelso: call
 *
 */
void libvm_exit(vm_t *vm)
{
	VM_VERBOSE VM_PRINT("%s\n", "vm.exit");
	obj_t *obj = sal_stack_pop(vm->stack);

	if (obj && obj->type == OBJECT_INTEGER)
		exit_value = integer_get_value(obj);

	if (shutdown_process)
		vm_shutdown(vm);
	else	interrupt_call(vm, INT_SHDW);

	shutdown_process = 1;
}

/**
 *
 * @syntax: printf #format#
 * @descr.: pops n elements from stack acording to the given format string
 * @exmple: push 23 push "world" printf "hello %s (%d)\n"
 *
 */
void libvm_printf(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	int i,times = 0;

	if (OBJECT_STRING == value_type(vm, word)) {
		char *args[10];
		for (i=1;word[i];i++)
			if (word[i] == '%')
				if (word[i+1]!='%')
					times++;

		for(i=0;i<times;i++) {
			obj_t *obj = sal_stack_pop(vm->stack);
			chk_type:
			switch(obj->type) {
			case OBJECT_INTEGER:
				args[i] = (char *)(size_t)obj->length;
				break;
			case OBJECT_REGISTER:
				obj = obj->data;
				goto chk_type;
			case OBJECT_FLOAT: // XXX broken
				args[i] = (char *)obj->data;
				break;
			case OBJECT_STRING:
				args[i] = obj->data;
				break;
			default:
#if 0
			case OBJECT_HASH:
			case OBJECT_NATIVE:
			case OBJECT_ARRAY:
			case OBJECT_POINTER:
			case OBJECT_INVALID:
			case OBJECT_LABEL:
#endif
				fprintf (stderr, "Unsupported Type Error\n");
				break;
			}
		}

		switch (times) {
		case 0: printf("%s", word+1); break;
		case 1: printf(word+1, args[0]); break;
		case 2: printf(word+1, args[0], args[1]); break;
		case 3: printf(word+1, args[0], args[1], args[2]); break;
		case 4: printf(word+1, args[0], args[1], args[2], args[3]); break;
		case 5: printf(word+1, args[0], args[1], args[2], args[3], args[4]); break;
		default: printf("printf: too many arguments. :/ (TODO)\n"); break;
		}
	} else
		fprintf(stderr, "printf: invalid argument.\n");
}


/**
 *
 * @syntax: print #times#
 * @descr.: pops #times# elements from stack and print them
 * @exmple: push "one" push "two" print 2
 *
 */
void libvm_print(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	int times = 0;
	float f;

	if (word) {
		if (is_register(word)) {
			obj_t *obj = get_register_object(vm, word);
			if (obj) {
				times = integer_get_value(obj);
			} else {
				times = integer_get_value_i(word);
			}
		} else {
			times = integer_get_value_i(word);
		}
	}

	if (not==0) {
		VM_VERBOSE VM_PRINT("print %d\n", times);
	} else {
		VM_VERBOSE VM_PRINT("println %d\n", times);
	}

	if (times == 0)
		return;

	for(;times;times--) {
		object_t *obj = sal_stack_pop(vm->stack);

		if (obj == NULL) {
			printf("Null object reference!\n");
			exit(1);
		}
		switch (obj->type) {
		case OBJECT_REGISTER: // XXX
		case OBJECT_STRING:
			printf("%s", (char *)obj->data);
			break;	
		case OBJECT_INTEGER:
			printf("%d", obj->length);
			break;
		case OBJECT_FLOAT:
			memcpy(&f, &obj->data, sizeof(float));
			printf("%.3f", f);
			break;
		case OBJECT_ARRAY:
			object_dump(obj, 10);
			//printf("[array(%d)]", obj->length);
			break;
		default:
			printf("Don't know how to pretty print this kind of object\n");
		}
	}

	sal_free(word);
}

void libvm_println(vm_t *vm)
{
	not = 1;
	libvm_print(vm);
	not = 0;
	printf("\n");
}

/**
 * // XXX vm.sleep 5 or push 5 vm.sleep O_o!!
 * @syntax: vm.sleep
 * @exmple: push 6 vm.sleep ; sleeps 6 seconds
 *
 */
void libvm_sleep(vm_t *vm)
{
	char *word = file_read(vm->file, vm->output);
	object_type type = value_type(vm, word);

	VM_VERBOSE VM_PRINT("vm.sleep %s\n", word);
	switch(type) {
	case OBJECT_INTEGER:
		sleep(integer_get_value_i(word));
		break;
	case OBJECT_FLOAT:
		usleep((int)(atof(word)*1000000));
		break;
	default:
		fprintf(stderr, "Invalid sleep object\n");
	}

	sal_free(word);
}
