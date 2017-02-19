#ifndef DIFILE_H
#define DIFILE_H

#include "dirlist.h"
#include "metadata.h"

struct Header
{
	int file_size;
	int dinodes;
	int metadata_block;
};

typedef struct Header Header;

int di_createfile(char * filename, listofdirs * dirlist);
int di_add_dir(int fd, char *dirname, int parent_num, metadata * md);
#endif