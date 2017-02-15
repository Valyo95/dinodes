#ifndef DINODE_H
#define DINODE_H

struct metadata
{
	char owner[30];
	char group[30];
	char access_rights[20];
	char timestamp[30];
};

typedef struct metadata metadata;


struct dinode_entry
{
	char name[30];
	int dinode_num;
};

typedef struct dinode_entry dinode_entry;


struct dinode
{
	metadata mdata;
	int link_count;/*num of hard links point to the dinode*/
	short int node_type;/*0: directory - 1: file*/
	int directPtr;/*pointer to block with file (for file) or dinode entries(for dir)*/

};

typedef struct dinode dinode;

#endif