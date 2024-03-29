#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCK_SIZE 1024

int OpenFile(const char * path);
int CloseFile(int fd);

int ReadBlock(int fd, int block_num, void * block);
int WriteBlock(int fd, int block_num, void * block);
int BlockCounter(int fd);

int WriteFile(int fd, int block_num, const char * source, int compress, off_t * compression_size);
int ExtractFile(int fd, char * filename, int start_block, off_t file_size, off_t compression_size);
int WriteSoftLink(int fd, int block_num, const char * path);
char * ReadSoftLink(int fd, int block_num);

#endif