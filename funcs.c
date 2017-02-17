#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "block.h"

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

int writeFile(char *fileName, char *outName, int blkout)
{

	printf("blkout === %d \n",blkout);
	void *block = malloc(BLOCK_SIZE*sizeof(char));
	FILE *file = fopen(fileName, "r+");
	FILE *out = fopen(outName, "rw+");
	int blocks = GetBlockCounter(fileName);
	printf("blocks ==== %d, blkout === %d \n",blocks, blkout);	
	for (int i = 0; i < blocks; ++i)
	{
		ReadBlock(fileName, i, block);
		printf("blocks of out file = %d\n", blkout);
		blkout = GetBlockCounter(outName);
		WriteBlock(outName, blkout, block);
	}
	return 0;
}

int printStat(struct stat sb)
{
   printf("File type:                ");
   switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:  printf("block device\n");            break;
    case S_IFCHR:  printf("character device\n");        break;
    case S_IFDIR:  printf("directory\n");               break;
    case S_IFIFO:  printf("FIFO/pipe\n");               break;
    case S_IFLNK:  printf("symlink\n");                 break;
    case S_IFREG:  printf("regular file\n");            break;
    case S_IFSOCK: printf("socket\n");                  break;
    default:       printf("unknown?\n");                break;
    }

   printf("I-node number:            %ld\n", (long) sb.st_ino);

   printf("Mode:                     %lo (octal)\n",
            (unsigned long) sb.st_mode);

   printf("Link count:               %ld\n", (long) sb.st_nlink);
    printf("Ownership:                UID=%ld   GID=%ld\n",
            (long) sb.st_uid, (long) sb.st_gid);

   printf("Preferred I/O block size: %ld bytes\n",
            (long) sb.st_blksize);
    printf("File size:                %lld bytes\n",
            (long long) sb.st_size);
    printf("Blocks allocated:         %lld\n",
            (long long) sb.st_blocks);

   printf("Last status change:       %s", ctime(&sb.st_ctime));
    printf("Last file access:         %s", ctime(&sb.st_atime));
    printf("Last file modification:   %s", ctime(&sb.st_mtime));
	int statchmod = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
	printf("chmod: %o\n", statchmod);
    return 0;
}