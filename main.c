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

	Header head;
	head = di_getHeader(fd);

	int metaDataStartBlock = head.metadata_block;
	int inodesNum = head.dinodes;

	node *arr = getInodesArray(fd);
   	for (int i = 0; i < inodesNum; ++i)
    {
        printArrayNode(arr[i]);
    }

	printMetadata(fd);

    free(arr);


    /*find listofdirst test*/
    dirlist_create(&dirlist);
    dirlist_add_dir(dirlist, "ekf");
    dirlist_add_dir(dirlist, "dirA");
    dirlist_add_dir(dirlist, "dirB");
    dirlist_add_dir(dirlist, "file1");
    dirlist_add_dir(dirlist, "file2");
    dirlist_add_dir(dirlist, "file3");
    dirlist_add_dir(dirlist, "file5");
    dirlist_add_dir(dirlist, "file6");
    //something not inside
    dirlist_add_dir(dirlist, "blablabla");

    di_find_dirlist(fd, dirlist);
    dirlist_free(&dirlist);


    CloseFile(fd);
    remove("testcreate.di");

	return 0;
}