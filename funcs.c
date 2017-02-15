#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int printStat(struct stat stat)
{
	printf("dev: %d\ninode: %x\nmode: %x\nnlink: %d\nuid: %d\ngid: %d\ndev_ID: %d\nsize: %d\nblocksize: %d\nblocks: %d\n\n",stat.st_dev, stat.st_ino, stat.st_mode, stat.st_nlink, stat.st_uid, stat.st_gid, stat.st_rdev, stat.st_size, stat.st_blocks, stat.st_blocks);
}

char *my_itoa(int num, char **str)
{
	if(*str == NULL)
	{
		return NULL;
	}
	sprintf(*str, "%d", num);
	return *str;
}

int copy_string(char ** dest, char *s)
{
	*dest = malloc((strlen(s)+1)*sizeof(char));
	if (*dest== NULL)
	{
		fprintf(stderr, "Error. Couldn't alloce memory.\n");
		return 1;
	}
	strcpy(*dest, s);
	return 0;
}