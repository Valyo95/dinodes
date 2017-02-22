#ifndef DIRLIST_H
#define DIRLIST_H

typedef struct dirNode dirNode;
struct dirNode
{
	char dir[30];
	dirNode * next;
};

typedef struct listofdirs listofdirs;
struct listofdirs
{
	dirNode * first;
	dirNode * last;
};

int dirlist_create(listofdirs ** dirlist);
int dirlist_add_dir(listofdirs * dirlist, char dir[30]);
int dirlist_free(listofdirs ** dirlist);

listofdirs * path_to_list(char * path);

#endif