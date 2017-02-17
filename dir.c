#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "funcs.h"

int check_dir(char *dir, int depth);


int main(int argc, char *argv[])
{
	struct stat st;

    DIR *dir;
    struct dirent *dp;
    char cur_dir[1024];
    getcwd(cur_dir,1024);
    //printf("Checking dir: %s\n\n",argv[1] );
    int inodes = check_dir(argv[1], 0);
    printf("i_nodes needed = %d\n", inodes);
    return 0;
}

int check_dir(char *dirname, int depth)
{
//	printf("dirname: %s\n", dirname);
	chdir(dirname);
	int count = 0;
	struct stat st;
	char * file_name;
    DIR *dir;
    struct dirent *dp;
	  char cur_dir[1024];
    getcwd(cur_dir,1024);
//    printf("Current dir: %s\n\n",cur_dir );
    
    dir = opendir(cur_dir);
    while ((dp=readdir(dir)) != NULL) {
//    printf("dasda\n");
        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") )
       		continue;
    	        
    	count++;
        file_name = dp->d_name; // use it
        for (int i = 0; i < depth; ++i)
        {
        	printf("|");
        	if(i == depth-1)
	        	printf("----");
    		else
    			printf("\t");
        }
        printf("%s",file_name);
		stat(file_name, &st);
        if(S_ISDIR(st.st_mode))
        {
        	printf("\n");
        	count+= check_dir(file_name, depth+1);
        }
        else
        {
            chmod(st.)
	        printf("\n");
        }
		printStat(st);
    }
    closedir(dir);
    chdir("..");
    return count;   
}