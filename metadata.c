#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "metadata.h"
#include "blocks.h"

int md_create(metadata ** md, int block_size)
{
	*md = malloc(sizeof(metadata));

	if (*md == NULL) return 10;

	metadata * mdp = *md;

	mdp->block_size = block_size;
	mdp->firstBlock = NULL;
	mdp->lastBlock = NULL;
	mdp->block_count = 0;
	mdp->dinode_count = 0;
	mdp->lastListBlock = 1;

	mdp->last_dinodelist = NULL;
	mdp->listIndex = 0;
	mdp->listMax = (block_size - 2*sizeof(int)) / sizeof(dinode);
	mdp->dirMax = (block_size - 2*sizeof(int)) / sizeof(dirEntry);

	printf("metadata created for block size %d\n",block_size);
	printf("size of stat is %ld\n",sizeof(struct stat));
	printf("maxList %d, maxDirinfo %d\n\n",mdp->listMax,mdp->dirMax);

	return 0;
}



int md_add_block(metadata * md, int block_type)
{
	block * new_block;
	dinodelist * dList = NULL;
	dirInfo * dInfo = NULL;

	new_block = malloc(sizeof(block));
	new_block->type = block_type;
	new_block->next = NULL;

	md->block_count++;

	if (block_type == 1)
	{
		dList = malloc(sizeof(dinodelist));
		dList->count = 0;
		dList->next = -1;
		dList->dinodes = malloc(md->listMax*sizeof(dinode));

		md->lastListBlock = md->block_count;
		md->last_dinodelist = dList;
		md->listIndex = 0;

		new_block->content = dList;
	}
	else
	{
		dInfo = malloc(sizeof(dirInfo));
		dInfo->count = 0;
		dInfo->next = -1;
		dInfo->entries = malloc(md->dirMax*sizeof(dirEntry));
		dInfo->dirIndex = 0;
		dInfo->block_num = md->block_count;

		new_block->content = dInfo;
	}

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



int md_add_dinode(metadata * md, struct stat node_info, char type, int pointer, int compression_size)
{
	if (md->last_dinodelist == NULL)/*this is the first dinode*/
	{
		md_add_block(md,1);
	}
	else if (md->listIndex == md->listMax)
	{
		md->last_dinodelist->next = md->block_count + 1 - md->lastListBlock;
		md_add_block(md,1);
		md->listIndex = 0;	
	}

	md->last_dinodelist->dinodes[md->listIndex].node_info = node_info;
	md->last_dinodelist->dinodes[md->listIndex].compression_size = compression_size;
	md->last_dinodelist->count++;

	if (type == 'f')
		md->last_dinodelist->dinodes[md->listIndex].pointer = pointer;
	else
		md->last_dinodelist->dinodes[md->listIndex].pointer = md->block_count + 1 - md->lastListBlock;

	md->listIndex++;
	md->dinode_count++;

	return 0;
}



int md_create_dirInfo(metadata *md, dirInfo ** dInfo)
{
	md_add_block(md,2);

	*dInfo = md->lastBlock->content;

	return 0;
}



int md_add_dirEntry(metadata *md, dirInfo ** dInfo, char name[30], int dinode_num)
{
	dirInfo * dInfop = *dInfo;

	if ( dInfop->dirIndex == md->dirMax )
	{
		md_add_block(md,2);
		(*dInfo)->next = (md->block_count - dInfop->block_num);
		dInfop->dirIndex = 0;
		*dInfo = md->lastBlock->content;
		dInfop = *dInfo;
		/*next block to be created will be the continuation*/
	}

	strcpy(dInfop->entries[dInfop->dirIndex].name,name);
	dInfop->entries[dInfop->dirIndex].dinode_num = dinode_num;

	dInfop->dirIndex++;
	dInfop->count++;

	return 0;
}



int md_find_dinode(metadata *md, ino_t inode_num)
{
	int i,next,count;
	int dinode_num = 1;
	block * current_block = md->firstBlock;
	dinodelist * current;
	dinode * dinodes;

	do
	{
		current = current_block->content;
		next = current->next;
		count = current->count;
		dinodes = current->dinodes;

		for (i=0;i<count;i++)
		{
			if (dinodes[i].node_info.st_ino == inode_num)
				return dinode_num;
			else
				dinode_num++;
		}

		for (i=0;i<next;i++)
			current_block = current_block->next;
	}
	while (next != -1);

	return -1;
}



int md_free(metadata ** md)
{
	block * current = (*md)->firstBlock;
	block * next = NULL;
	dinodelist * dList;
	dirInfo * dInfo;

	while (current != NULL)
	{
		if (current->type == 1)
		{
			dList = current->content;
			free(dList->dinodes);
			free(dList);
		}
		else
		{
			dInfo = current->content;
			free(dInfo->entries);
			free(dInfo);
		}

		next = current->next;
		free(current);
		current = next;
	}

	free(*md);

	return 0;
}

void md_printall(metadata *md)
{
	block * current = md->firstBlock;
	int count = 0;
	int dinode = 0;
	int i;
	dinodelist * dList;
	dirInfo * dInfo;

	while (current !=NULL)
	{
		count++;
		printf(">Block : %d - ",count);
		if (current->type == 1)
		{
			dList = current->content;
			printf("Type: DinodeList -");
			printf("Count: %d, Next: %d \nDinodes:\n",dList->count, dList->next);
			
			for (i=0;i<dList->count;i++)
			{
				dinode++;
				printf("%d: stat:..., compression_size: %ld, pointer: %d, inodenum: %ld\n",dinode, dList->dinodes[i].compression_size, dList->dinodes[i].pointer, dList->dinodes[i].node_info.st_ino);
			}
		}
		else
		{
			dInfo = current->content;
			printf("Type: dirInfo -");
			printf("Count: %d, Next: %d\n", dInfo->count, dInfo->next);
			for (i=0;i<dInfo->count;i++)
			{
				printf("Name: %s InodeNum: %d\n",dInfo->entries[i].name, dInfo->entries[i].dinode_num);
			}			
		}

		putchar('\n');
		current = current->next;
	}
}



int md_writetofile(metadata *md, int fd)
{
	block * current = md->firstBlock;
	void * writeblock = malloc(md->block_size);
	void * start = writeblock;

	dinodelist * dList;
	dirInfo * dInfo;
	int count = 0;


	while (current !=NULL)
	{
		count ++;
		if (current->type == 1)
		{
			dList = current->content;

			memcpy(writeblock, &dList->count, sizeof(int));
			writeblock+= sizeof(int);

			memcpy(writeblock, &dList->next, sizeof(int));
			writeblock+= sizeof(int);

			memcpy(writeblock, dList->dinodes, dList->count*sizeof(dinode));

			writeblock = start;

			WriteBlock(fd, -1, writeblock);
		}
		else
		{
			dInfo = current->content;

			memcpy(writeblock, &dInfo->count, sizeof(int));
			writeblock+= sizeof(int);

			memcpy(writeblock, &dInfo->next, sizeof(int));
			writeblock+= sizeof(int);

	//		printf("Write %d entries from block %d!\n",dInfo->count,count);
			memcpy(writeblock, dInfo->entries, dInfo->count*sizeof(dirEntry));

			writeblock = start;

			WriteBlock(fd, -1, writeblock);
		}

		current = current->next;
	}

	free(writeblock);

	return 0;
}