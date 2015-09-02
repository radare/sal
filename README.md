```
                 ___,__ ,_
                / _/\_ \| |
                \_ \/ _ | |_,
                /__/|___|___|

         Scripting Assembly Language
```

* Author: pancake <pancake@nopcode.org>
* ProjectStartsDate: 2006
* License: 4-Clause BSD

SAL is a portable and lightweight virtual machine
that executes a dynamic assembly language useful
for simple interfacing with native code.

You can embed SAL easily by using libsal, inject
some new labels and load files or streams.

The language itself is structured by objects of
different types:
```
typedef enum {
  OBJECT_STRING   = 0,// zero-terminated char arrays
  OBJECT_INTEGER  = 1, // 32bit numeric values
  OBJECT_FLOAT    = 2, // float/double
  OBJECT_LABEL    = 3, // source labels
  OBJECT_REGISTER = 4, // virtual registers
  OBJECT_POINTER  = 5, // pointers and register refs
  OBJECT_ARRAY    = 6, // array of objects
  OBJECT_HASH     = 7, // hashtable
  OBJECT_NATIVE   = 8, // native code
  OBJECT_INVALID  = -1
} object_type;
```

EXAMPLES
========

A simple hello world with SAL:

```
$ echo 'push "Hello World!" println 1' | sal -
Hello World!
```

But you can also run some syscalls, note that '.' is an alias for push:

```
$ echo '. 12 . "Hello World\n" . 1 . 4 syscall.3' | sal -
```

There are unit tests, examples and libraries to demonstrate how to write graphical applications, network clients, manage arrays, parse XML and other common tasks.

The SAL VM supports interrupt events and hooking system to handle division by zero, and other kind of errors.

The CPP preprocessor (or any other) can be used to provide a higher-level abstraction:
```
#include "sal.cpp"

eof:
  println("EOF");
  close(%r0)
  exit(0)

cannot_connect:
  println("Cannot connect to remote host")
  exit(1)

main:
  connect("radare.org",80)
    on_error(cannot_connect)
      is(%r0)
  write(%r0,"GET /r/ HTTP/1.0\r\nHost: www.radare.org\r\n\r\n")

again:
  readln(%r0)
    on_error(eof)
      is(%r1)
  print(%r1)
  again
```

### Future

At some point, SAL can be reimplemented to support JIT or just be translated to native code, using the `ragg2` emiters for example. But this will drop some dynamism and needs some more review.

SAL can be injected into processes in order to have an interactive shell inside the target program.
