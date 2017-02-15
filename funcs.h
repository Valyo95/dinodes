#ifndef FUNCS_H
#define FUNCS_H

#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>


//           struct stat {
//               dev_t     st_dev;         /* ID of device containing file */
//               ino_t     st_ino;         /* inode number */
//               mode_t    st_mode;        /* protection */
//               nlink_t   st_nlink;       /* number of hard links */
//              uid_t     st_uid;         /* user ID of owner */
//               gid_t     st_gid;         /* group ID of owner */
//               dev_t     st_rdev;        /* device ID (if special file) */
//               off_t     st_size;        /* total size, in bytes */
//               blksize_t st_blksize;     /* blocksize for filesystem I/O */
//               blkcnt_t  st_blocks;      /* number of 512B blocks allocated */
//
//               struct timespec st_atim;  /* time of last access */
//               struct timespec st_mtim;  /* time of last modification */
//              struct timespec st_ctim;  /* time of last status change */
//
//           #define st_atime st_atim.tv_sec      /* Backward compatibility */
//           #define st_mtime st_mtim.tv_sec
//           #define st_ctime st_ctim.tv_sec
//          };


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

#endif