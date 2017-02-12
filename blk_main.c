#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blocks.h"

int main(int argc, char const *argv[])
{
	int fd,i,error;

	fd = BLK_open_file("./testfile");

	error = BLK_add_blocks(fd, 2);

	void * block1;
	void * block2;

	BLK_read_block(fd,0,&block1);
	BLK_read_block(fd,1,&block2);

	char test1[20],test2[20];

	strcpy(test1,"MANOS");
	strcpy(test2,"TORAKIS");	

	memcpy(block1, test1, strlen(test1)*sizeof(char));
	memcpy(block2, test2, strlen(test2)*sizeof(char));

	BLK_write_block(fd,0,block1);
	BLK_write_block(fd,1,block2);

	if (error == -1)
	{
		printf("BLK_add_blocks error %d\n",error);
		perror("BLK");
	}

	//BLK_delete_block(fd, "testfile", 1);
	i = BLK_block_counter(fd);
	printf("Block #: %d\n",i);

	BLK_close_file(fd);

	return 0;
}