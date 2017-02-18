#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "funcs.h"


int checkdir(char *dirname, int depth)
{
	//printf("dirname: %s\n", dirname);
	
    chdir(dirname);
	int count = 0;
	struct stat st;
	char * file_name;
    DIR *dir;
    struct dirent *dp;
	char cur_dir[1024];
    getcwd(cur_dir,1024);

    //printf("Current dir: %s\n\n",cur_dir );
    
    dir = opendir(cur_dir);
    
    while ((dp=readdir(dir)) != NULL) 
    {
        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") )
       		continue;
    	        
    	count++;
        file_name = dp->d_name;

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
        	count+= checkdir(file_name, depth+1);
        }
        else
        {
	        printf("\n");
        }
		printStat(st);
    }
    closedir(dir);
    chdir("..");
    return count;   
}