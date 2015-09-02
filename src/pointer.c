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

#include "vm.h"

/*
 * length = size of the containing chunk
 * data   = pointer
 */

obj_t *pointer_new(char *string, obj_t *src)
{
	char buf[1024];

	sprintf(buf, "ptr_%.900s", string);
	obj_t *obj  = (obj_t *)object_new( buf );
	obj->type   = OBJECT_POINTER;
	obj->data   = src;

	return obj;
}
