#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "blocks.h"

int OpenFile(const char * path)
{
	return open(path, O_RDWR | O_CREAT, 0666);
}


int CloseFile(int fd)
{
	return close(fd);
}


int ReadBlock(int fd, int block_num, void * block)
{
	int error,blocks;

	blocks = BlockCounter(fd);

	if (block_num < 0 || block_num >= blocks)
	{
		fprintf(stderr, "ReadBlock:Invalid block num!\n");
		return -2;
	}

	if ( lseek(fd, block_num*BLOCK_SIZE, SEEK_SET) == (off_t) -1)
	{
		fprintf(stderr, "ReadBlock:ReadBlock Invalid block num!\n");
		return -2;
	}

	error = read(fd, block, BLOCK_SIZE);

	if (error == -1)
	{
		fprintf(stderr, "ReadBlock: Read failed!\n");
		free(block);
		return -1;
	}

	return error;/*return bytes read*/
}


int WriteBlock(int fd, int block_num, void * block)
{
	int error,blocks;

	blocks = BlockCounter(fd);

	if (block_num < -1 || block_num >= blocks)
	{
		fprintf(stderr, "WriteBlock:Invalid block num!\n");
		return -2;
	}

	if (block_num == -1)
		lseek(fd, 0, SEEK_END);
	else
		lseek(fd, block_num*BLOCK_SIZE, SEEK_SET);		

	error = write(fd, block, BLOCK_SIZE);

	if (error == -1)
	{
		fprintf(stderr, "WriteBlock: Write failed!\n");
		return -1;
	}

	return error;/*return bytes written*/

}


int BlockCounter(int fd)
{
	int size,counter;

	size = lseek(fd, 0, SEEK_END);
	counter = size / BLOCK_SIZE;

	if (size % BLOCK_SIZE != 0)
		counter++;

	//printf("BlockCounter: File size %d bytes\n",size);

	return counter;
}


int WriteFile(int fd, int block_num, const char * source, int compress, off_t * compression_size)
{
	int fds, source_blocks, dest_blocks, move_blocks;
	int i,j;
	void * block, * end_blocks, * start;
	char * command;
	char * compressed_name;
	struct stat st;

	if (compress == 1)
	{
		mkdir("temp_compress_dir", 0777);

		command = malloc( (22+strlen(source))*sizeof(char) );
		sprintf(command, "cp %s temp_compress_dir", source);
		system(command);
		free(command);

		compressed_name = malloc(strlen(source+4)*sizeof(char));
		sprintf(compressed_name, "%s.gz", source);

		chdir("temp_compress_dir");
		if (fork() == 0)
		{
			execlp("gzip", "gzip", source, (char *) NULL);
		}

		waitpid(-1,NULL,0);/*wait for zip to finish*/
		stat(compressed_name, &st);
		*compression_size = st.st_size;

		fds = OpenFile(compressed_name);/*open compression file for write to di file*/
	}
	else
	{
		*compression_size = -1;
		fds = OpenFile(source);/*open normal file for compression*/
	}

	block = malloc(BLOCK_SIZE);

	source_blocks = BlockCounter(fds);
	dest_blocks = BlockCounter(fd);

	/*If we want to write the source file,at a specific block*/
	/*Make sure to "push" blocks so as to not lose information of dest*/
	if (block_num > -1 && block_num < dest_blocks)
	{
		move_blocks = dest_blocks - block_num;

		end_blocks = malloc(move_blocks*BLOCK_SIZE);
		start = end_blocks;
		
		/*Save dest's blocks that will have to be moved forward*/
		for (i=block_num;i<dest_blocks;i++)
		{
			ReadBlock(fd, i, end_blocks);
			end_blocks += BLOCK_SIZE;
		}

		end_blocks = start;/*reset pointer*/

		/*Start writing source file at dest,starting from block_num block*/
		for (i=0;i<source_blocks;i++)
		{
			ReadBlock(fds, i, block);
			
			WriteBlock(fd, block_num, block);			
			block_num++;

			if (block_num == dest_blocks)
			{/*If dest file doesnt have more blocks to write*/
			/*Write the rest of the source's blocks at the end of dest (append)*/
				for (j=i+1;j<source_blocks;j++)
				{
					ReadBlock(fds, j, block);
					WriteBlock(fd, -1, block);
				}
				break;
			}
		}

		/*Finally move previously saved blocks of dest after the (now saved) source file*/
		for (i=0;i<move_blocks;i++)
		{
			if (block_num == dest_blocks)
			{/*If dest file doesnt have more blocks to write*/
			/*Write the rest of "moved" blocks at the end of dest (append)*/
				for (j=i;j<move_blocks;j++)	
				{
					WriteBlock(fd, -1, end_blocks);
					end_blocks += BLOCK_SIZE;
				}
				break;
			}
			else
			{
				WriteBlock(fd, block_num, end_blocks);
				end_blocks += BLOCK_SIZE;
				block_num++;
			}
		}

		end_blocks = start;/*reset pointer*/

		free(end_blocks);
	}
	else if (block_num == -1)
	{/*simply write at the end of dest file,block by block*/
		block_num = BlockCounter(fd);

		for (i=0;i<source_blocks;i++)
		{
			ReadBlock(fds, i, block);
			WriteBlock(fd, -1, block);
		}
	}
	else
	{
		printf("WriteFile: Invalid block num!\n");
		free(block);
		return -1;
	}

	CloseFile(fds);
	free(block);

	if (compress == 1)
	{
		remove(compressed_name);
		free(compressed_name);

		chdir("..");
		rmdir("temp_compress_dir");
	}

	return block_num;
}



int ExtractFile(int fd, char * filename, int start_block, off_t file_size, off_t compression_size)
{
	int extract_fd;
	char * compressed_name;
	char * create_name;

	if (compression_size != -1)
	{
		compressed_name = malloc( (strlen(filename)+4)*sizeof(char) );
		sprintf(compressed_name, "%s.gz", filename);
		create_name = compressed_name;
		extract_fd = OpenFile(compressed_name);/*Open compressed file (we will write the compressed file)*/
		file_size = compression_size;
	}
	else
	{
		extract_fd = OpenFile(filename);
		create_name = filename;
	}

	if (BlockCounter(extract_fd) != 0)
	{/*file already exists,do nothing*/
		printf("ExtractFile: File '%s' already exists\n", create_name);
		CloseFile(extract_fd);
		return -1;
	}
	
	int full_blocks = file_size / BLOCK_SIZE;
	int remaining_bytes = file_size % BLOCK_SIZE;
//	printf("%d %d\n",full_blocks,remaining_bytes);
	int i;
	int curr_block = start_block;
	void * block = malloc(BLOCK_SIZE);
	void * lastblock;

	for (i=0;i<full_blocks;i++)
	{
		ReadBlock(fd, curr_block, block);
		WriteBlock(extract_fd, -1, block);
		curr_block++;
	}

	if (remaining_bytes > 0)
	{
		lastblock = malloc(remaining_bytes);

		lseek(fd, curr_block*BLOCK_SIZE, SEEK_SET);
		read(fd, lastblock, remaining_bytes);
		lseek(extract_fd, 0, SEEK_END);
		write(extract_fd, lastblock, remaining_bytes);

		free(lastblock);
	}

	free(block);
	CloseFile(extract_fd);

	if (compression_size != -1)
	{
		if (fork() == 0)
		{
			execlp("gzip", "gzip", "-d", compressed_name, (char *) NULL);
		}

		free(compressed_name);
		waitpid(-1,NULL,0);/*wait for unzip to finish*/
	}

	return 0;
}



int WriteSoftLink(int fd, int block_num, const char * path)
{
	int dest_blocks, move_blocks;
	int i,j,path_len;
	void * block, * end_blocks, * start;
	void * block_start;

	dest_blocks = BlockCounter(fd);

	block = malloc(BLOCK_SIZE);
	block_start = block;

	/*If we want to write the source file,at a specific block*/
	/*Make sure to "push" blocks so as to not lose information of dest*/
	if (block_num > -1 && block_num < dest_blocks)
	{
		move_blocks = dest_blocks - block_num;

		end_blocks = malloc(move_blocks*BLOCK_SIZE);
		start = end_blocks;
		
		/*Save dest's blocks that will have to be moved forward*/
		for (i=block_num;i<dest_blocks;i++)
		{
			ReadBlock(fd, i, end_blocks);
			end_blocks += BLOCK_SIZE;
		}

		end_blocks = start;/*reset pointer*/

		/*Write link path at block_num block*/
		path_len = strlen(path);
		memcpy(block, &path_len, sizeof(int));
		block += sizeof(int);
		memcpy(block, path, path_len*sizeof(char));
		block = block_start;

		WriteBlock(fd, block_num, block);
		block_num++;

		/*Finally move previously saved blocks of dest after the (now saved) source file*/
		for (i=0;i<move_blocks;i++)
		{
			if (block_num == dest_blocks)
			{/*If dest file doesnt have more blocks to write*/
			/*Write the rest of "moved" blocks at the end of dest (append)*/
				for (j=i;j<move_blocks;j++)	
				{
					WriteBlock(fd, -1, end_blocks);
					end_blocks += BLOCK_SIZE;
				}
				break;
			}
			else
			{
				WriteBlock(fd, block_num, end_blocks);
				end_blocks += BLOCK_SIZE;
				block_num++;
			}
		}

		end_blocks = start;/*reset pointer*/

		free(end_blocks);
	}
	else if (block_num == -1)
	{/*simply write at the end of dest file,block by block*/
		block_num = BlockCounter(fd);

		path_len = strlen(path);
		memcpy(block, &path_len, sizeof(int));
		block += sizeof(int);
		memcpy(block, path, path_len*sizeof(char));
		block = block_start;

		WriteBlock(fd, -1, block);

	}
	else
	{
		printf("WriteFile: Invalid block num!\n");
		free(block);
		return -1;
	}

	free(block);

	return block_num;

}



char * ReadSoftLink(int fd, int block_num)
{
	void * block = malloc(BLOCK_SIZE);
	void * start_block = block;
	
	int path_len;
	char * path;

	ReadBlock(fd, block_num, block);

	memcpy(&path_len, block, sizeof(int));
	block += sizeof(int);

	path = malloc( (path_len + 1)*sizeof(char) );
	memcpy(path, block, path_len*sizeof(char));
	path[path_len] = '\0';

	free(start_block);

	return path;
}