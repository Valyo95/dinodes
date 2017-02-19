#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests.h"
#include "difile.h"
#include "dirlist.h"

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

	di_createfile("testcreate.di", dirlist);

	dirlist_free(&dirlist);

	return 0;
}