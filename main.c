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
//	dirlist_add_dir(dirlist, "tempdir");

	di_createfile("testcreate.di", dirlist, 1);

	int fd = OpenFile("testcreate.di");
	int blks = BlockCounter(fd);
	printf("file has %d blocks\n", blks);
	dirlist_free(&dirlist);

	node *arr = getInodesArray(fd);
/*   	for (int i = 0; i < inodesNum; ++i)
    {
        printArrayNode(arr[i]);
    }
*/

    /*Extract file test*/
/*    ExtractFile(fd, "file3", 4, 1486);
    printf("done extractFile\n");
*/
	printHierarchy(fd);
	printMetadata(fd);

    extractDiFile(fd);
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

/*
    //RELATIVE PATH TEST
    char *path1 = malloc(100*sizeof(char));
    char *path2 = malloc(100*sizeof(char));
    char *relative;
    printf("Starting relative path tests.Give path1 A and path2 B to end test\n");
    strcpy(path1,"test");
    strcpy(path2,"test");
    while ( strcmp(path1,"A") != 0 || strcmp(path2,"B") != 0)
    {
        printf("Path1: ");
        scanf("%s", path1);
        printf("Path2: ");
        scanf("%s", path2);
        relative = relative_string(path1,path2,100);
        printf("\nRelative is %s\n\n",relative);
    }

    free(path1);
    free(path2);
    free(relative);
*/


	return 0;
}