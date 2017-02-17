#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blocks.h"

int main(int argc, char const *argv[])
{
	int fd,i,error;
	char * filename = "testfile";

	fd = OpenFile(filename);

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

	printf("Wrote to testfile.It now has %d blocks\n",BlockCounter(fd));

	WriteFile(fd,1,"writetest");
	printf("Wrote file writetest to file testfile!\n");

	printf("testfile now has %d blocks!\n",BlockCounter(fd));

	CloseFile(fd);
	free(block);
	free(array);

	return 0;
}