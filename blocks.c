#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "blocks.h"

int BLK_open_file(const char * path)
{
	return open(path, O_RDWR | O_CREAT, 0666);
}


int BLK_close_file(int fd)
{
	return close(fd);
}


int BLK_add_blocks(int fd, int blocks_num)
{
	void * new_block;
	int error,i;

	new_block = malloc(blocks_num*BLOCK_SIZE);

	lseek(fd, 0, SEEK_END);
	error = write(fd, new_block, blocks_num*BLOCK_SIZE);
	free(new_block);

	return error;
}


int BLK_read_block(int fd, int block_num, void ** block)
{
	int error;

	if ( lseek(fd, block_num*BLOCK_SIZE, SEEK_SET) == (off_t) -1)
	{
		fprintf(stderr, "BLK_read_block: Invalid block_num!");
		return -2;
	}

	*block = malloc(BLOCK_SIZE);
	error = read(fd, *block, BLOCK_SIZE);

	return error;
}


int BLK_write_block(int fd, int block_num, void * block)
{
	int error;

	if ( lseek(fd, block_num*BLOCK_SIZE, SEEK_SET) == (off_t) -1)
	{
		fprintf(stderr, "BLK_write_block: Invalid block_num!");
		return -2;
	}

	error = write(fd, block, BLOCK_SIZE);

	return error;

}


int BLK_delete_block(int fd, char * filename, int block_num)
{
	int error,new_fd,total_blocks,prev_blocks,next_blocks;
	void * remaining_blocks;

	total_blocks = BLK_block_counter(fd);
	prev_blocks = block_num - 1;
	next_blocks = total_blocks - block_num;
	remaining_blocks = malloc( (total_blocks-1)*BLOCK_SIZE);

	lseek(fd, 0, SEEK_SET);
	if (prev_blocks >= 0)
	{
		if (read(fd, remaining_blocks, prev_blocks*BLOCK_SIZE) == -1)
			return -1;
	}
	if (next_blocks >=0)
	{
		if (read(fd, remaining_blocks + prev_blocks*BLOCK_SIZE, next_blocks*BLOCK_SIZE) == -1)
			return -1;
	}

	new_fd = BLK_open_file("./temp");
	if ( write(new_fd, remaining_blocks, (total_blocks-1)*BLOCK_SIZE) == -1)
		return -1;

	remove(filename);
	rename("temp",filename);

	return new_fd;
}


int BLK_block_counter(int fd)
{
	int size,counter;

	size = lseek(fd, 0, SEEK_END);
	counter = size / BLOCK_SIZE;
	printf("BLK_block_counter: File size %d bytes\n",size);

	return counter;
}