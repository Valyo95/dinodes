#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests.h"
#include "difile.h"
#include "dirlist.h"
#include "blocks.h"
#include "metadata.h"

int main(int argc, char const *argv[])
{
	// printf("---TESTING BLOCKS---\n");
	// test_blocks("testfile","writetest");
	
	// printf("---TESTING METADATA---\n");
	// test_metadata();

	//printf("---TESTING CHECKDIR---\n");
	//test_checkdir(char *dir);

	printf("Testing di_createfile!\n");
	listofdirs * dirlist;
	dirlist_create(&dirlist);
	dirlist_add_dir(dirlist, "ekf");
	dirlist_add_dir(dirlist, "blocks.h");
	

	di_createfile("testcreate.di", dirlist);
	int fd = OpenFile("testcreate.di");
	int blks = BlockCounter(fd);
	printf("file has %d blocks\n", blks);
	dirlist_free(&dirlist);

	int metaDataStartBlock = getMetaDataBlock("testcreate.di");
	int inodesNum = getInodesNum("testcreate.di");

	node **arr = getInodesArray(fd, metaDataStartBlock, inodesNum);
   	for (int i = 0; i < inodesNum; ++i)
    {
        printArrayNode(arr[i]);
    }

	printMetadata(metaDataStartBlock, inodesNum, fd);

	for (int i = 0; i < inodesNum; ++i)
    {
     	free(arr[i]);
    }
    free(arr);
	return 0;
}