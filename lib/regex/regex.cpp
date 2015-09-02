#define regex_init() push "libsalregex.so" vm.loadlib
#define regex_new(x) push x regex.new
#define regex_match(x,y) push y push x regex.match
#define regex_free(x) push x regex.free
