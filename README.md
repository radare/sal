```
                 ___,__ ,_
                / _/\_ \| |
                \_ \/ _ | |_,
                /__/|___|___|

         Scripting Assembly Language
```

Author: pancake <pancake@nopcode.org>
ProjectStartsDate: 2006
License: BSD

SAL is a portable and lightweight virtual machine
that executes a dynamic assembly language useful
for simple interfacing with native code.

You can embed SAL easily by using libsal, inject
some new labels and load files or streams.

The language itself is structured by objects of
different types:

# Labels:

    Labels are used to mark some points of the
    execution code where you'll be able to jump
    there later.

    Labels can be native or virtual.

    Virtual labels are defined by a loaded file
    with certain offset.

    Native labels could be defined internally by
    calling
 
# Strings:

    Strings are objects that store a dynamic
    zero-terminated array of characters.

# Integers:

    Integer objects are handled as simple 32/64
    integer values.

# Floating point:

    Same as above, but for float/double types.

# Pointers:

    You can use pointers and register references.
    See examples fmi.


EXAMPLES
========

A simple hello world with SAL:

```
$ echo 'push "Hello World!" println 1' | sal -
Hello World!
```
