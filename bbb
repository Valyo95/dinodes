#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCK_SIZE 512

int OpenFile(const char * path);
int CloseFile(int fd);

int ReadBlock(int fd, int block_num, void * block);
int WriteBlock(int fd, int block_num, void * block);
int BlockCounter(int fd);

int WriteFile(int fd, int block_num, const char * source);

#endif