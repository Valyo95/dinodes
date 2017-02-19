#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "difile.h"
#include "dirlist.h"
#include "metadata.h"
#include "blocks.h"


int di_createfile(char * filename, listofdirs * dirlist)
{
	int fd;
	fd = OpenFile(filename);

	/*WRITE HEADER (FOR NOW)*/
	void * block = malloc(BLOCK_SIZE);

	Header head;
	head.file_size = 0;
	head.dinodes = 0;
	head.metadata_block = 0;

	memcpy(block, &head, sizeof(Header));

	WriteBlock(fd, -1, block);

	/*START CREATING FILE*/
	metadata * md;
	md_create(&md, BLOCK_SIZE);

	dirInfo * dInfo;
	struct stat st;
	int count = 1;

    md_add_dinode(md,st,'d',0);

    md_create_dirInfo(md,&dInfo);
    md_add_dirEntry(md,&dInfo,".", 1);
    md_add_dirEntry(md,&dInfo,"..", 1);

	dirNode * current = dirlist->first;
	char * file_name;

	while (current != NULL)
	{
		count++;
	    int file_block;

		file_name = current->dir;
		stat(file_name, &st);

		md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);

		if(S_ISDIR(st.st_mode))
        {
        	count += di_add_dir(fd, file_name, 1, md);
        }
        else
        {
        	file_block = WriteFile(fd, -1, file_name);
            md_add_dinode(md, st,'f', file_block);
        }

        current = current->next;
	}

	/*WRITE CORRECT HEADER*/
	head.file_size = (BlockCounter(fd) + md->block_count)*BLOCK_SIZE;
	head.dinodes = md->dinode_count;
	head.metadata_block = BlockCounter(fd);

	memcpy(block, &head, sizeof(head));
	WriteBlock(fd, 0, block);

	md_writetofile(md, fd);
	md_printall(md);

	printf("DI FILE '%s' CREATED!Printing head:\n", filename);
	printf("FILE_SIZE: %d\n",head.file_size);
	printf("DINODES: %d\n",head.dinodes);
	printf("METADATA_BLOCK: %d\n",head.metadata_block);

	CloseFile(fd);
	md_free(&md);
	free(block);

	return 0;
}



int di_add_dir(int fd, char *dirname, int parent_num, metadata * md)
{
    dirInfo * dInfo;

    int count = 1;
    int file_block;
    struct stat st;
    int dinode_num;

    stat(dirname, &st);
    md_add_dinode(md,st,'d',0);

    chdir(dirname);
    dinode_num = md->dinode_count;

    md_create_dirInfo(md,&dInfo);
    md_add_dirEntry(md,&dInfo,".", dinode_num);
    md_add_dirEntry(md,&dInfo,"..", parent_num);

    char * file_name;
    DIR *dir;
    struct dirent *dp;
    char cur_dir[1024];
    getcwd(cur_dir,1024);
    
    dir = opendir(cur_dir);
    while ((dp=readdir(dir)) != NULL) 
    {
        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") )
            continue;
                
        count++;
        file_name = dp->d_name;

        stat(file_name, &st);
        
        md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);    


        if(S_ISDIR(st.st_mode))
        {
            /*this dirInfo will be created at next available block*/
            count += di_add_dir(fd, file_name, dinode_num, md);
        }
        else
        {
            /*SAVE FILE AND GET THE BLOCK WHERE IT WAS SAVED TO PASS IN DINODE*/
            file_block = WriteFile(fd, -1, file_name);
            md_add_dinode(md, st,'f', file_block);
        }
    }
    closedir(dir);
    chdir("..");

    return count;   
}