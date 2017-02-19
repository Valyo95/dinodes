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
int di_find_dirlist(int fd, listofdirs * dirlist);
int di_find_dir(int fd, char *dirname,int blockNum, node *arr);

Header di_getHeader(int fd);

int printMetadata(int fd);
void printArrayNode(node inode);
int dirTraverse(int blockNum, int fd, node *arr, int depth);
node * getInodesArray(int fd);


#endif