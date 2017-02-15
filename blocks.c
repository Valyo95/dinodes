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
	void * new_blocks;
	int error,block_start,total_bytes;

	block_start = lseek(fd, 0, SEEK_END);
	total_bytes = blocks_num*BLOCK_SIZE;
	new_blocks = malloc(total_bytes);
	
	if ( write(fd, new_blocks, total_bytes) != total_bytes)
	{
		fprintf(stderr, "BLK_add_blocks: Write failed!\n");
		free(new_blocks);
		return -1;
	}
	free(new_blocks);

	return block_start;

}


int BLK_read_block(int fd, int bytePtr, void ** block)
{
	int error;

	if (bytePtr < 0)
	{
		fprintf(stderr, "BLK_read_block:Invalid byte pointer!\n");
		return -2;
	}

	if ( lseek(fd, bytePtr, SEEK_SET) == (off_t) -1)
	{
		fprintf(stderr, "BLK_read_block: Invalid byte pointer!\n");
		return -2;
	}

	*block = malloc(BLOCK_SIZE);
	error = read(fd, *block, BLOCK_SIZE);

	if (error == -1)
	{
		fprintf(stderr, "BLK_read_block: Read failed!\n");
		free(block);
		return -1;
	}

	return error;/*return bytes read*/
}


int BLK_write_block(int fd, int bytePtr, void * block)
{
	int error;


	if (bytePtr < 0)
	{
		fprintf(stderr, "BLK_write_block:Invalid byte pointer!\n");
		return -2;
	}

	if ( lseek(fd, bytePtr, SEEK_SET) == (off_t) -1)
	{
		fprintf(stderr, "BLK_write_block: Invalid byte pointer!\n");
		return -2;
	}

	error = write(fd, block, BLOCK_SIZE);

	if (error == -1)
	{
		fprintf(stderr, "BLK_write_block: Write failed!\n");
		return -1;
	}

	return error;/*return bytes written*/

}


int BLK_delete_block(int fd, char * filename, int bytePtr)
{
	int error,new_fd,total_blocks,next_byte,last_byte;
	void * remaining_blocks;

	if (bytePtr < 0)
	{
		fprintf(stderr, "BLK_delete_block:Invalid byte pointer!\n");
		return -2;
	}

	total_blocks = BLK_block_counter(fd);
	last_byte = total_blocks*BLOCK_SIZE;
	next_byte = bytePtr + BLOCK_SIZE;
	remaining_blocks = malloc( (total_blocks-1)*BLOCK_SIZE);

	if (bytePtr != 0)
	{
		lseek(fd, 0, SEEK_SET);
		if (read(fd, remaining_blocks, bytePtr) == -1)
			return -1;
	}
	if (next_byte <= last_byte)
	{
		lseek(fd, next_byte, SEEK_SET);
		if (read(fd, remaining_blocks + bytePtr, last_byte - next_byte) == -1)
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