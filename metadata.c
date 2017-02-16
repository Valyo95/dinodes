#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "metadata.h"

int md_create(metadata ** md, int block_size)
{
	*md = malloc(sizeof(metadata));

	if (*md == NULL) return 10;

	metadata * mdp = *md;

	mdp->block_size = block_size;
	mdp->firstBlock = NULL;
	mdp->lastBlock = NULL;
	mdp->block_count = 0;
	mdp->lastListBlock = 0;
	mdp->lastDirBlock = 0;

	mdp->last_dinodelist = NULL;
	mdp->listIndex = 0;
	mdp->listMax = (block_size - 2*sizeof(int)) / sizeof(dinode);
	
	mdp->last_dirInfo = NULL;
	mdp->dirIndex = 0;
	mdp->dirMax = (block_size - 2*sizeof(int)) / sizeof(dirEntry);

	printf("metadata created for block size %d\n",block_size);
	printf("size of stat is %d\n",sizeof(struct stat));
	printf("maxList %d, maxDirinfo %d\n\n",mdp->listMax,mdp->dirMax);

	return 0;
}



int md_add_block(metadata * md, int block_type)
{
	block * new_block;
	dinodelist * dList = NULL;
	dirInfo * dInfo = NULL;

	md->block_count++;

	if (block_type == 1)
	{
		dList = malloc(sizeof(dinodelist));
		dList->prev = (md->block_count - md->lastListBlock);
		dList->next = -1;
		dList->dinodes = malloc(md->listMax*sizeof(dinode));

		md->lastListBlock = md->block_count;
		md->last_dinodelist = dList;
		md->listIndex = 0;
	}
	else
	{
		dInfo = malloc(sizeof(dirInfo));
		dInfo->count = 0;
		dInfo->next = -1;
		dInfo->entries = malloc(md->dirMax*sizeof(dirEntry));

		md->lastDirBlock = md->block_count;
		md->last_dirInfo = dInfo;
		md->dirIndex = 0;
	}

	new_block = malloc(sizeof(block));
	new_block->type = block_type;
	new_block->dList = dList;
	new_block->dInfo = dInfo;
	new_block->next = NULL;

	if (md->firstBlock == NULL)
	{
		md->firstBlock = new_block;
		md->lastBlock = new_block;
	}
	else
	{
		md->lastBlock->next = new_block;
		md->lastBlock = new_block;
	}

	return 0;
}



int md_add_dinode(metadata * md, struct stat node_info, int pointer)
{
	if (md->last_dinodelist == NULL)/*this is the first dinode*/
	{
		md_add_block(md,1);
	}
	else if (md->listIndex == md->listMax)
	{
		md->last_dinodelist->next = md->block_count + 1;
		md_add_block(md,1);
		md->listIndex = 0;	
	}

	md->last_dinodelist->dinodes[md->listIndex].node_info = node_info;
	md->last_dinodelist->dinodes[md->listIndex].pointer = pointer;

	md->listIndex++;
}



int md_create_dirInfo(metadata *md)
{
	if (md->last_dirInfo != NULL)
	{
		md->last_dirInfo->next = -1;
	}

	md_add_block(md,2);

	return 0;
}



int md_add_dirEntry(metadata *md, char name[30], int dinode_num)
{
	if (md->dirIndex == md->dirMax)
	{
		md->last_dirInfo->next = md->block_count + 1;
		md->dirIndex = 0;
		md_add_block(md,2);
		/*next block to be created will be the continuation*/
	}

	strcpy(md->last_dirInfo->entries[md->dirIndex].name,name);
	md->last_dirInfo->entries[md->dirIndex].dinode_num = dinode_num;

	md->dirIndex++;
	md->last_dirInfo->count++;

	return 0;
}



int md_free(metadata ** md)
{
	block * current = (*md)->firstBlock;
	block * next = NULL;

	while (current != NULL)
	{
		if (current->type == 1)
		{
			free(current->dList->dinodes);
			free(current->dList);
		}
		else
		{
			free(current->dInfo->entries);
			free(current->dInfo);
		}

		next = current->next;
		free(current);
		current = next;
	}

	free(*md);

	return 0;
}

int md_printall(metadata *md)
{
	block * current = md->firstBlock;
	int count = 0;
	int i;

	while (current !=NULL)
	{
		count++;
		printf(">Block : %d - ",count);
		if (current->type == 1)
		{
			printf("Type: DinodeList -");
			printf("Previous: %d, Next: %d \nDinodes:\n",current->dList->prev, current->dList->next);
			
			for (i=0;i<md->listMax;i++)
			{
				printf("%d: stat:..., pointer: %d\n",i, current->dList->dinodes[i].pointer);
			}
		}
		else
		{
			printf("Type: dirInfo -");
			printf("Next: %d, Count: %d\n",current->dInfo->next, current->dInfo->count);
			for (i=0;i<current->dInfo->count;i++)
			{
				printf("Name: %s InodeNum: %d\n",current->dInfo->entries[i].name, current->dInfo->entries[i].dinode_num);
			}			
		}

		putchar('\n');
		current = current->next;
	}
}