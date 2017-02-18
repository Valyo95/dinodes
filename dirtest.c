#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "funcs.h"
#include "metadata.h"
#include "blocks.h"
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


int printMetadata(int metadataStart, int allInodes, int fd)
{
    void *start = malloc(BLOCK_SIZE*sizeof(char));
    void *block = start;

    int i = 0;
    int currBlock = metadataStart;
    int nodesPerBlock = 0;
    int nextInodeBlock = 0;
    dinodelist inode;
    node **arr = malloc(allInodes*sizeof(struct node*));
    struct dinode *temp = malloc(MAX_I_NODES*sizeof(struct dinode));
    for (int i = 0; i < allInodes; ++i)
    {
        arr[i] = malloc(sizeof(struct node));
    }
    
    do
    {
        ReadBlock(fd, currBlock, block);
        memcpy(&nodesPerBlock, block, sizeof(int));
        block += sizeof(int);
        memcpy(&nextInodeBlock, block, sizeof(int));
        block += sizeof(int);
        
        memcpy(temp, block, nodesPerBlock*sizeof(struct dinode));
        int end = i+nodesPerBlock;
        for(int j=i; j<end;j++)
        {
            arr[j+i]->block = currBlock;
            arr[j+i]->offset = temp[j].pointer;
            arr[j+i]->node_info = temp[j].node_info;
            i++;
        }

        currBlock = nextInodeBlock;
    } while (currBlock != -1);

}
/*
int dirTraverse(int currInode, int allInodes, int blockNum, int fd, node **arr)
{
    void *start = malloc(BLOCK_SIZE*sizeof(char));
    void *block = start;
    dirInfo dir;
    dir.entries = malloc(dir.count*sizeof(dirEntry));
    ReadBlock(fd, blockNum, start);
    memcpy(&(dir.count), block, sizeof(int));
    block += sizeof(int);
    memcpy(&(dir.next), block, sizeof(int));
    block += sizeof(int);
    
    memcpy(dir.entries, block, dir.count*sizeof(dirEntry));
    while(dir.next != -1)
    {
        for (int i = 0; i < dir.count; ++i)
        {
            
        }
    }   
}
*/