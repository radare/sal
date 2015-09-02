/* Array's hello world */
#include "sal.cpp"

/* Create a new array of size 3 */
array_new(3) is(%r0)
array_set(%r0, 0, "Hello")
array_set(%r0, 1, "World")

/* */
array_size(%r0)
push "Array size: "
println 2

array_resize(%r0, 2) is(%r0)

array_size(%r0) is(%r1)
push %r1
push "Array size: "
println 2

; print all elements of the array (size..0)
push %r1
loop:
	push -1
	add 2
	dup 1
	; Elements to print
	dup 1
	push " : " swap 2
	push %r0 aget
	swap 3  ; swap elements order
	println 3
	;--
	dup 1
	jnz loop

