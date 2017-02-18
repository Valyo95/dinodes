#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tests.h"

int main(int argc, char const *argv[])
{
	test_blocks("testfile","writetest");
	//test_metadata();
	//test_checkdir(char *dir);

	return 0;
}