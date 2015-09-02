/*
 * CPP stuff for sal
 * cpp xx | sal -
 */

#include "sal.cpp"

#define filename reg2

main:
	argc() push 1
		cmp on_non_zero(usage)

	argv(0) on_error(cannot_open) is(filename)

	open(filename, 0)
		on_error(cannot_open)
			is(reg0)

	/* Read one line */
	readln(reg0) is(reg1)
	println(reg1)

	close(reg0)

	exit(0)

cannot_open:
	println("Cannot open this file.")
	exit(1)

usage:
	println("Usage: sal files.sal [file]")
	exit(0)
