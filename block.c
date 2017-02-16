#include "stdio.h"
#include "block.h"

int ReadBlock(char *fileName, int blockNum, void *block)
{
	FILE *file = fopen(fileName, "r+");
	fseek(file, blockNum*BLOCK_SIZE, SEEK_SET);
	int b = ftell(file);
//	printf("fp =  %d\n", b);

	if (!fread(block, 1, BLOCK_SIZE, file))
	{
		fprintf(stderr, "Fread failed. No data readed\n");
		return 1;
	}
	fclose(file);
	return 0;
}

int WriteBlock(char *fileName, int blockNum, void *block)
{
	FILE *file = fopen(fileName, "rw+");
	fseek(file, blockNum*BLOCK_SIZE, SEEK_SET);
	int b = ftell(file);
//	printf("fp =  %d\n", b);

	if (!fwrite(block, 1, BLOCK_SIZE, file))
	{
		fprintf(stderr, "Fwrite failed. No data writed\n");
		return 1;
	}
	fclose(file);
	return 0;
	
}


int GetBlockCounter(char *fileName)
{
	FILE *file = fopen(fileName, "r");
	fseek(file, 0, SEEK_END);
	int b = ftell(file);
	printf("file has %d blocks\n", b/BLOCK_SIZE);
	fclose(file);
	return b;
}