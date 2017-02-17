#ifndef METADATA_H
#define METADATA_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*DINODE LIST*/
struct dinode
{
	struct stat node_info;
	int pointer;/*block offset of dir info for dirs,block of file for files*/
};
typedef struct dinode dinode;


struct dinodelist
{
	int prev;
	int next;
	dinode * dinodes;
};
typedef struct dinodelist dinodelist;


/*DIR INFO*/
struct dirEntry
{
	char name[30];
	int dinode_num;
};
typedef struct dirEntry dirEntry;


struct dirInfo
{
	int count;	
	int next;
	dirEntry * entries;

	/*following is not to be saved at file*/
	int dirIndex;
	int block_num;
};
typedef struct dirInfo dirInfo;


/*METADATA "BLOCKS"*/
typedef struct block block;
struct block
{
	int type;
	/*a block will either be a dinodelist (1) or a dirInfo (2)*/
	void * content;
	block * next;
};


struct metadata
{
	int block_size;
	block * firstBlock;
	block * lastBlock;
	int block_count;
	int dinode_count;
	int lastListBlock;

	dinodelist * last_dinodelist;
	int listIndex;
	int listMax;
	int dirMax;
};
typedef struct metadata metadata;

int md_create(metadata ** md, int block_size);
int md_add_block(metadata * md, int block_type);

int md_add_dinode(metadata * md, struct stat node_info, char type, int pointer);/*'f' file,'d' dir*/
int md_create_dirInfo(metadata *md, dirInfo ** dInfo);
int md_add_dirEntry(metadata *md, dirInfo ** dInfo, char name[30], int dinode_num);

int md_free(metadata ** md);
int md_printall(metadata *md);

#endif