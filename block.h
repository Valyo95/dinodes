#ifndef BLOCK_H
#define BLOCK_H

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1024
#endif

int ReadBlock(char *fileName, int blockNum, void *block);

int WriteBlock(char *fileName, int blockNum, void *block);

int GetBlockCounter(char *fileName);



#endif