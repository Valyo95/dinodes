

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
	struct stat sb;
	char  *linkname;
	int r;
		if (lstat(argv[1], &sb) == -1) {
               perror("lstat");
               exit(EXIT_FAILURE);
           }

           printStat(sb);

           linkname = malloc(sb.st_size + 1);
           if (linkname == NULL) {
               fprintf(stderr, "insufficient memory\n");
               exit(EXIT_FAILURE);
           }

           r = readlink(argv[1], linkname, sb.st_size + 1);

           if (r == -1) {
               perror("readlink");
               exit(EXIT_FAILURE);
           }

           if (r > sb.st_size) {
               fprintf(stderr, "symlink increased in size "
                               "between lstat() and readlink()\n");
               exit(EXIT_FAILURE);
           }

           linkname[r] = '\0';

           printf("'%s' points to '%s'\n", argv[1], linkname);

           free(linkname);

           exit(EXIT_SUCCESS);
       

	
	return 0;
}