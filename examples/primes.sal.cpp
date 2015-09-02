#include <sal.cpp>

#define LASTPRIME 32
#define ARRAYSIZE 1024
 // r0 = array
 // r1 = root iter
 // r2 = arrayidx
main:
	push ARRAYSIZE anew pop %r0
	push 2 pop %r1 // iterator
	push 1 pop %r2 // r2 = arraysize
	push 1 push 0 push %r0 aset // r0[0] = 1


_mainloop:
	call findprime
	push %r1 push 1 add 2 pop %r1 // r1++
	push %r1
	push LASTPRIME
	cmp
	jz done
	_mainloop
done:
	push "Found "
	push %r2
	push "primes"
	println 3

	push 0
	vm.exit

findprime:
	push %r2 pop %r8 // r8 = r2
	push 0 pop %r9 // r9 = 0
	push %r0 asz pop %r10 // r10 = sizeof(array)
	doloop:
		push %r9 push %r2 neg add 2 // if (r9 == r2)
		jz doloop_end           //	break;

		push %r9 push %r0 aget pop %r7 // r7 = r0 [r9]
		// if (%r7 %i
		// %r1 is n
	push "--- " println 1
		push %r7 push %r9 div push %r7 neg add 2
		jnz nonprime
		array_set (%r0, %r2, %r7)
		push %r2 push 1 add 2 pop %r2

		push "added prime "
		push %r7
		println 2

		nonprime:

		push %r9 push 1 add 2 pop %r9 // r9++
		doloop
	doloop_end:
	ret
