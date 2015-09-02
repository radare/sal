/*
 * Copyright (C) 2006
 *       pancake <pancake@phreaker.net>
 *
 * sal is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * sal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sal; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"

int integer_get_value_i(char *arg)
{
	// from radare:get_offset
	int ret = 0;
	int i;

	if (arg == NULL) return 0;
	for(;*arg==' ';arg=arg+1) {}
	for(i=0;arg[i]==' ';i++) {}
	for(;arg[i]=='\\';i++) {}
	i++;

	if (arg[i] == 'x' && i>0 && arg[i-1]=='0')
		sscanf(arg, "0x"OFF_FMTx, &ret);
	else	sscanf(arg, OFF_FMTd, &ret);

	switch(arg[strlen(arg)-1]) {
	case 'o': // octal
		sscanf(arg, "%o", &ret);
		break;
	case 'b': { // binary
		int i,j,f=1;
		for(j=0,i=strlen(arg)-2;i>=0;i--,j++) {
			if (arg[i]=='b') break;
			if (arg[i]=='1') { if (f--) ret = 0; ret|=1<<j; } else
			if (arg[i]!='0') break;
		}}
		break;
	case 'K': case 'k':
		ret*=1024;
		break;
	case 'M': case 'm':
		ret*=1024*1024;
		break;
	case 'G': case 'g':
		ret*=1024*1024*1024;
		break;
	}

	return ret;
}

obj_t *integer_new(char *string)
{
	object_t *obj = object_new("_integer");
	obj->type   = OBJECT_INTEGER;
	obj->length = integer_get_value_i(string);
	obj->data   = NULL;

	return obj;
}

obj_t *integer_new_i(int number)
{
	obj_t *obj  = object_new("_integer");
	obj->type   = OBJECT_INTEGER;
	obj->length = number;
	obj->data   = NULL;

	return obj;
}

int integer_get_value(obj_t *obj)
{
	if (obj->type!=OBJECT_INTEGER) {
		// TODO handle exception
		return 0;
	}
	return obj->length;
}
