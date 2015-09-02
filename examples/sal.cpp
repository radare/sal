// Language
#define reg0 %r0
#define reg1 %r1
#define reg2 %r2
#define reg3 %r3
#define reg4 %r4
#define reg5 %r5
#define reg6 %r6
#define string(x) x
#define integer(x) x
#define float(x) x
#define label(x) x
#define is(x) pop x
#define on_error(x) getf jnz x
#define on_zero(x) jz x
#define on_non_zero(x) jnz x
#define include(x) push x vm.load
#define load(x) push x vm.load
#define loadlib(x) push x vm.loadlib
#define set(x, y) push y pop x

// Loops
#define fordown(x) push x dup 1
#define endfor(x) push -1 add 2 dup 1 dup 1 jnz x drop 2

// File descriptors
#define open(filename,perms) push perms push filename open
#define readln(x) push x readln
#define write(x,y) push y push x write 1
#define println(x) push x println 1
#define print(x) push x print 1
#define close(x) push x close
#define exit(x) push x vm.exit

// Sockets
#define connect(host,port) push port push host tcp.connect
#define listen(port) push port listen
#define accept(socket) push socket accept

// Arrays
#define array_new(x) push x anew
#define array_set(x,y,z) push z push y push x aset
#define array_get(x,y) push y push x aget
#define array_size(x) push x asz
#define array_resize(x,y) push y push x arsz
#define array_free(x) rmr x

// Arguments
#define argv(x) push x push %av aget
#define argc() push %av asz

// Stack
#define push(x) push x
#define pop(x) pop x

// Math
#define mod(x,y) push y push x div push x sub 2

// Interrupts
#define INT_SHDW 0
#define INT_ESTK 1
#define INT_FSTK 2
#define INT_INVO 3
#define INT_INTS 4
#define INT_ZERO 5
#define int(x) push x int
#define interrupt(x) push x int
