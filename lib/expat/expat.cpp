#define expat_init() push "libsalexpat" vm.loadlib pop %r0
#define expat_new(x,y,z) push z push y push x expat.new
#define expat_parse_string(x,y) push y push x expat.parse
#define expat_free(x) push x expat.free
