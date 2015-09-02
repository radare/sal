#define syscall_init() push "libsalsyscall.so" vm.loadlib
#define syscall_0(x) push x syscall.0
#define syscall_1(x,y) push y push x syscall.1
#define syscall_2(x,y,z) push z push y push x syscall.2
#define syscall_3(w,x,y,z) push z push y push x push w syscall.3
