#ifndef _INCLUDE_FILE_H_
#define _INCLUDE_FILE_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct file_t {
	FILE *fd;
	char *name;
} file_t;

file_t *file_open(const char *filename);
file_t *file_reopen(file_t *ft);
char *file_read(file_t *ft, FILE *output);
off_t file_offset(file_t *ft);
void file_seek(file_t *ft, off_t offset);
void file_close(struct file_t *ft);

#endif
