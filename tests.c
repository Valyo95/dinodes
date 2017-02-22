#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "funcs.h"
#include "dir.h"
#include "blocks.h"
#include "metadata.h"

int test_blocks(char * dest, char * source)
{
	int fd,i;
	off_t compression_size;

	fd = OpenFile(dest);

	void * block;
	block = malloc(BLOCK_SIZE);

	char * array = malloc(1024*sizeof(char));
	for (i=0;i<1024;i++)
		array[i] = 'A';

	memcpy(block,array,BLOCK_SIZE);
	WriteBlock(fd, -1, block);

	for (i=0;i<1024;i++)
		array[i] = 'B';

	memcpy(block,array,BLOCK_SIZE);
	//ReadBlock(fd,0,block);
	WriteBlock(fd, -1, block);

	printf("Wrote to '%s'.It now has %d blocks\n",dest,BlockCounter(fd));

	WriteFile(fd,1,source, 0, &compression_size);
	printf("Wrote file '%s' to file '%s'!\n",source,dest);

	printf("'%s' now has %d blocks!\n",dest,BlockCounter(fd));

	CloseFile(fd);
	free(block);
	free(array);

	return 0;
}


int test_metadata()
{
	metadata * md;
	struct stat sample;

	dirInfo * dInfo;

	md_create(&md,200);

	md_add_dinode(md, sample, 'd', 1, 0);
	md_add_dinode(md, sample, 'd', 2, 1);
	md_create_dirInfo(md, &dInfo);
	md_add_dirEntry(md,&dInfo,".",1);
	md_add_dirEntry(md,&dInfo,"..",2);
	md_add_dirEntry(md,&dInfo,"fileA",3);
	md_create_dirInfo(md,&dInfo);
	md_add_dirEntry(md,&dInfo,".",2);
	md_add_dirEntry(md,&dInfo,"..",3);

	md_printall(md);
	md_free(&md);

	return 0;
}


int test_checkdir(char *directory)
{
	char cur_dir[1024];
    getcwd(cur_dir,1024);
    
    printf("Checking dir: %s\n\n",directory );
    int inodes = checkdir(directory, 0);
    
    printf("i_nodes needed = %d\n", inodes);
    
    return 0;
}