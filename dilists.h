#ifndef DILISTS_H
#define DILISTS_H

#include "dinode.h"

typedef struct dirInfoNode dirInfoNode;

struct dirInfoNode
{
	dirEntry entry;/*name of entity + dinode # of that entity*/
	dirInfoNode * next;
};



struct dirInfoHead 
{
	int count;
	int current;
	int parent;
	dirInfoNode * first;
	dirInfoNode * last;
};

typedef struct dirInfoHead dirInfoHead;

int dirInfo_create(dirInfoHead ** head, int current, int parent);
int dirInfo_add(dirInfoHead * head, char name[30], int dinode_num);
int dirInfo_delete(dirInfoHead * head, char name[30]);
void dirInfo_printall(dirInfoHead * head);
int dirInfo_free(dirInfoHead ** head);

#endif