#ifndef BLOCKS_H
#define BLOCKS_H

#define BLOCK_SIZE 512

int BLK_open_file(const char * path);
int BLK_close_file(int fd);

int BLK_add_blocks(int fd, int blocks_num);
int BLK_read_block(int fd, int block_num, void ** block);
int BLK_write_block(int fd, int block_num, void * block);
int BLK_delete_block(int fd, char * filename, int block_num);
int BLK_block_counter(int fd);

#endif