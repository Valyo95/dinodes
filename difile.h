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

int di_createfile(char * filename, listofdirs * dirlist, int compress);
int di_add_dir(int fd, char *dirname, int parent_num, metadata * md, int compress);
int di_find_dirlist(int fd, listofdirs * dirlist);
int di_find_dir(int fd, char *dirname,int blockNum, node *arr);
int di_append(char * filename, listofdirs * dirlist, int compress);
int di_append_dir(int fd, char *dirname, int parent_num, metadata * md, int compress, int * metadata_block);


Header di_getHeader(int fd);

int printMetadata(int fd);
void printArrayNode(node inode);
int dirTraverse(int blockNum, int fd, node *arr, int depth);
node * getInodesArray(int fd);
int freeNodeArray(node **arr, int fd);
int printHierarchy(int fd);
int dirTraverseMetaData(int blockNum, int fd, node *arr, int depth);

int extractDiFile(int fd, char *fileName,listofdirs *list);
int extractDir(int blockNum, int fd, node *arr, int depth);
int SearchNode(int blockNum, int fd, node *arr, int depth, dirNode *list);

char * getSymLinkPath(char *argv);
char * relative_string(char * path1, char * path2, int max_len);

#endif