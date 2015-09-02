/* Copyright (C) 2006-2015 - BSD - pancake */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alloc.h"
#include "file.h"
#include "vm.h"

struct file_t *file_open(const char *filename)
{
	FILE *fd;
	struct file_t *ft = NULL;

	if (filename[0]=='-')
		fd = stdin;
	else
		fd = fopen(filename,"rb");

	if (fd == NULL) {
		printf("Cannot open \"%s\"\n", filename);
		return NULL;
	} else {
		ft = (file_t *)sal_malloc(sizeof(file_t));
		ft->fd = fd;
		ft->name = (char *)sal_strdup(filename);
	}

	return ft;
}

struct file_t *file_reopen(file_t *ft)
{
	long offset = file_offset(ft);
	fclose(ft->fd);
	ft->fd = fopen(ft->name, "rb");
	if (ft->fd == NULL)
		return NULL;

	file_seek(ft, offset);

	return ft;
}


/* XXX: instead of strdup, just provide a global pointer buffer */
char *file_read(file_t *ft, FILE *output)
{
	char buf[1024];
	char *ptr = buf;

	if (feof(ft->fd))
		return NULL;

	buf[0]='\0';
	fscanf(ft->fd, "%1024s", buf);
	
	switch (*buf) {
	case '\0':
		return NULL;
	case '"':
		/* string */
	//	memcpy(buf, buf+1, strlen(buf));
		ptr = strchr(buf+1,'"');
		if (ptr) {
			if ((ptr[0]-1) != '\\') {
				*ptr = '\0';
				return strdup(buf);
			}
			ptr++;
		}
		ptr = buf + strlen (buf);
		for (;;) {
			fread(ptr, 1, 1, ft->fd);
			if (ptr[0]=='\\') {
				// TODO: special char
				fread(ptr, 1, 1, ft->fd);
				switch(ptr[0]) {
				case 'n':
					ptr[0]='\n';
					break;
				case 'r':
					ptr[0]='\r';
					break;
				}
			} else
			if (ptr[0]=='"') {
				ptr[0]='\0';
				break;
			}
			ptr++;
		}
		break;
	case '/':
		if (buf[1]=='/') {
			/* skip comment */
			fgets(buf, sizeof(buf), ft->fd);
			return file_read(ft, 0);
		}
		break;
	case '#':
	case ';':
		/* skip comment */
		fgets(buf, sizeof(buf), ft->fd);
		return file_read(ft, 0);
	}

	if (output)
		fprintf(output, "%s ", buf);

	return strdup(buf);
}


off_t file_offset(file_t *ft)
{
	return ftell(ft->fd);
}

void file_seek(file_t *ft, off_t offset)
{
	fseek(ft->fd, offset, SEEK_SET);
}

void file_close(struct file_t *ft)
{
	if (ft == NULL) return;
	free(ft->name);
	fclose(ft->fd);
	free(ft);
}

int file_eof(struct file_t *ft)
{
	return feof(ft->fd);
}
