#ifndef _INCLUDE_FLAGS_H_
#define _INCLUDE_FLAGS_H_

#define FLAGS_CLEAR 0x00000000
#define FLAGS_ERROR 0x00000001
#define FLAGS_CARRY 0x00000002
#define FLAGS_EXIST 0x00000004
#define FLAGS_NOPER 0x00000008

#define vm_flags_set(x) vm->flags = x
#define vm_flags_get(x) vm->flags

#endif
