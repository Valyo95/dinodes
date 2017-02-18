#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests.h"

int main(int argc, char const *argv[])
{
	printf("---TESTING BLOCKS---\n");
	test_blocks("testfile","writetest");
	
	printf("---TESTING METADATA---\n");
	test_metadata();

	//printf("---TESTING CHECKDIR---\n");
	//test_checkdir(char *dir);

	return 0;
}