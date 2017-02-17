#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "funcs.h"
#include "metadata.h"

int check_dir(char *dir, int parent_num, int depth, metadata * md);


int main(int argc, char const *argv[])
{
	struct stat st;
    metadata * md;
    md_create(&md, 512);

    DIR *dir;
    struct dirent *dp;
    char cur_dir[1024];
    getcwd(cur_dir,1024);
    //printf("Checking dir: %s\n\n",argv[1] );
//    md_add_dinode(md,st,'d',0);
    int inodes = check_dir(argv[1], 2, 0, md);
    md_printall(md);
    md_free(&md);

    printf("i_nodes needed = %d\n", inodes);
    return 0;
}

int check_dir(char *dirname, int parent_num, int depth, metadata * md)
{
//	printf("dirname: %s\n", dirname);

    dirInfo * dInfo;

	chdir(dirname);
	int count = 0;
	struct stat st;
    int dinode_num;

    md_add_dinode(md,st,'d',0);
    dinode_num = md->dinode_count;

    md_create_dirInfo(md,&dInfo);
    md_add_dirEntry(md,&dInfo,".", dinode_num);
    md_add_dirEntry(md,&dInfo,"..", parent_num);

	char * file_name;
    DIR *dir;
    struct dirent *dp;
	char cur_dir[1024];
    getcwd(cur_dir,1024);
//    printf("Current dir: %s\n\n",cur_dir );
    
    dir = opendir(cur_dir);
    while ((dp=readdir(dir)) != NULL) 
    {
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
        
        md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);    

        if(S_ISDIR(st.st_mode))
        {
        	printf("\n");
            /*this dirInfo will be created at next available block*/
        	count+= check_dir(file_name, dinode_num, depth+1, md);
        }
        else
        {
	        printf("\n");
            /*SAVE FILE AND GET THE BLOCK WHERE IT WAS SAVED TO PASS IN DINODE*/
            md_add_dinode(md, st,'f',0);
        }
	//	printf("%d\n\n",S_ISDIR(st.st_mode));
//		printStat(st);

    }
    //printf("\n");
    closedir(dir);
    chdir("..");
    return count;   
}