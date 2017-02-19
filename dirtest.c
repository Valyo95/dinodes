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
int printMetadata(int metadataStart, int allInodes, int fd);
void printArrayNode(node *inode);

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
//    md_printall(md);

    printf("i_nodes needed = %d\n", inodes);

    int fd;
    fd = OpenFile("metadata.di");
    md_writetofile(md, fd);
    

        printMetadata(0, inodes+1, fd);


    CloseFile(fd);

    md_free(&md);
    remove("metadata.di");
    return 0;
}

int check_dir(char *dirname, int parent_num, int depth, metadata * md)
{
//	printf("dirname: %s\n", dirname);

    dirInfo * dInfo;

    int count = 0;
    struct stat st;
    int dinode_num;

    stat(dirname, &st);
    md_add_dinode(md,st,'d',0);
    //printStat(st);

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
	//	printStat(st);

    }
    //printf("\n");
    closedir(dir);
    chdir("..");
    return count;   
}


int printMetadata(int metadataStart, int allInodes, int fd)
{
    //printf("metadataStart: %d, allInodes: %d, fd: %d\n", metadataStart, allInodes, fd);
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
        int end = nodesPerBlock;
    //      printf("Blocknum = %d, nextInodeBlock = %d, nodesPerBlock = %d, end = %d  \n", currBlock, nextInodeBlock, nodesPerBlock, end);
        for(int j=0; j<end;j++)
        {
    //        printf("j = %d, i = %d, currBlock = %d\n", j, i, currBlock);
    //        printf("pointer : %d, \n", temp[j].pointer );
            arr[i]->block = currBlock;
            arr[i]->offset = temp[j].pointer;
            arr[i]->node_info = temp[j].node_info;
            //printArrayNode(arr[i]);
        //printf("\n");
            i++;
        }
        currBlock+= nextInodeBlock;
        block = start;
    } while (nextInodeBlock != -1);
/*    for (int i = 0; i < allInodes; ++i)
    {
        printArrayNode(arr[i]);
        printf("\n");
    }
*/      dirTraverse(arr[0]->block + arr[0]->offset, fd, arr, 0);

    for (int i = 0; i < allInodes; ++i)
    {
        free(arr[i]);
    }
    free(temp);
    free(arr);
    free(block);    
    return 0;

}

void printArrayNode(node *inode)
{
    printf("inode: %d\tblock: %d\toffset: %d\n", inode->node_info.st_ino, inode->block, inode->offset);
    printStat(inode->node_info);
    return;
}


int dirTraverse(int blockNum, int fd, node **arr, int depth)
{
    //printf("blockNum = %d, fd = %d, depth = %d\n", blockNum, fd, depth);

    int i;
    void *start = malloc(BLOCK_SIZE*sizeof(char));
    void *block = start;
    dirInfo dir;
    dir.entries = malloc(MAX_DIR_ENTRIES*sizeof(dirEntry));
    do
    {
        ReadBlock(fd, blockNum, start);
        memcpy(&(dir.count), block, sizeof(int));
        block += sizeof(int);
        memcpy(&(dir.next), block, sizeof(int));
        block += sizeof(int);
        memcpy(dir.entries, block, dir.count*sizeof(dirEntry));
        
        for (i = 0; i < dir.count; ++i)
        {
            if ( !strcmp(dir.entries[i].name, ".") || !strcmp(dir.entries[i].name, "..") )
            {
                continue;
            }
            for (int i = 0; i < depth; ++i)
            {
                printf("|");
                if(i == depth-1)
                    printf("----");
                else
                    printf("\t");
            }
            int inodeNum = dir.entries[i].dinode_num-1;
            printf("%s \n", dir.entries[i].name, depth);
        //            printf("inodenum = %d\n\n\n", inodeNum);
        //            printStat(arr[inodeNum]->node_info);
            if(S_ISDIR(arr[inodeNum]->node_info.st_mode))
            {
                //printf("%s is a DIR\n", dir.entries[i].name);
                //printf("block = %d, offset = %d, inode = %d\n",arr[inodeNum]->block, arr[inodeNum]->offset, arr[inodeNum]->node_info.st_ino );            
                //printf("\n");
                dirTraverse(arr[inodeNum]->block + arr[inodeNum]->offset, fd, arr, depth+1);
            }
            else
            {

            }    
        }
    } while (dir.next != -1);

    free(dir.entries);
    free(start);
    return 0;
}
