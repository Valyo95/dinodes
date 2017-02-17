#include "stdio.h"
#include "block.h"

int ReadBlock(char *fileName, int blockNum, void *block)
{
	FILE *file = fopen(fileName, "r+");
	fseek(file, blockNum*BLOCK_SIZE, SEEK_SET);
	int b = ftell(file);
/*	printf("filename :%s, blockNum: %d\n",fileName, blockNum);
	printf("fp =  %d\n", b);
*/
	int ret = fread(block, 1,BLOCK_SIZE, file);
//	printf("%d\n", ret);
	
	if (!ret)
	{
		fprintf(stderr, "Fread failed. No data readed\n");
		return 1;
	}
	fclose(file);
	return ret;
}

int WriteBlock(char *fileName, int blockNum, void *block)
{
	FILE *file = fopen(fileName, "aw+");
	fseek(file, blockNum*BLOCK_SIZE, SEEK_SET);
	int b = ftell(file);
	
/*
	printf("filename :%s, blockNum: %d\n",fileName, blockNum);
	printf("fp =  %d\n", b);
*/
	int ret =fwrite(block, 1,BLOCK_SIZE, file);
//	printf("%d\n", ret);
	if (!ret)
	{
		fprintf(stderr, "Fwrite failed. No data writed\n");
		return 1;
	}
	fclose(file);
	return ret;
	
}


int GetBlockCounter(char *fileName)
{
	FILE *file = fopen(fileName, "r");
	fseek(file, 0, SEEK_END);
	int b = ftell(file);
//	printf("b = %d\n", b);
	int dive = b/BLOCK_SIZE;
	int mod = b%BLOCK_SIZE;
	if (mod > 0)
		b = dive + 1;
/*	printf("dive : %d, mod : %d\n", dive, mod);
	printf("file has %d blocks\n", b);
*/
	fclose(file);
	return b;
}