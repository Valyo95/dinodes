#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "funcs.h"

int main(int argc, char *argv[])
{
	char *bla = "block\n\n\n\n";
	int blocks;
	char *b= malloc(BLOCK_SIZE*sizeof(char));
	int ret;
	int i;

	printf("%s\n", argv[2]);

	WriteBlock(argv[2], 0, (void*) bla);	
	WriteBlock(argv[2], 1, (void*) bla);
	WriteBlock(argv[2], 2, (void*) bla);
	WriteBlock(argv[2], 3, (void*) bla);

	blocks = GetBlockCounter(argv[1]);

/*	for (i = 0; i < blocks-1; ++i)
	{
		printf("i = %d\n", i);
		ret = ReadBlock(argv[1], i, (void*) b);
		printf("%s\n\n\n", b);
	}	
	
	printf("i = %d\n", i);
	ret = ReadBlock(argv[1], i, (void*) b);
	b[ret] = '\0';
	printf("%s\n\n\n", b);
*/

	ret = GetBlockCounter(argv[2]);
	printf("ret = %d\n", ret);
//	writeFile(argv[1], argv[2], );

	free(b);
	return 0;
}