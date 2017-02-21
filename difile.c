#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <utime.h>
#include <time.h>
#include <unistd.h>

#include "difile.h"
#include "dirlist.h"
#include "metadata.h"
#include "blocks.h"
#include "funcs.h"


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
    st.st_ino = -1;
	int count = 1;
    int dinode_num;

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
        lstat(file_name, &st);


        if(S_ISDIR(st.st_mode))
        {
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
            count += di_add_dir(fd, file_name, 1, md);
        }
        else if(S_ISREG(st.st_mode))
        {
            dinode_num = md_find_dinode(md, st.st_ino);

            if (dinode_num == -1)
            {
                md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
                printf("New file '%s' with dinodenum %d!Extracting\n",file_name, md->dinode_count+1);
                file_block = WriteFile(fd, -1, file_name);
                md_add_dinode(md, st,'f', file_block);
            }
            else
            {
                printf("New hard link '%s' pointing to dinode %d\n", file_name, dinode_num);
                md_add_dirEntry(md,&dInfo,file_name, dinode_num);
            }
        }
        else if(S_ISLNK(st.st_mode))
        {
            char *linkName = getSymLinkPath(file_name);
            printf("Symbolic link :%s shows to name: %s\n", file_name, linkName);
            free(linkName);

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

    lstat(dirname, &st);
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

        lstat(file_name, &st);

        if(S_ISDIR(st.st_mode))
        {
            /*this dirInfo will be created at next available block*/
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);    
            count += di_add_dir(fd, file_name, dinode_num, md);
        }
        else if(S_ISREG(st.st_mode))
        {
            dinode_num = md_find_dinode(md, st.st_ino);

            if (dinode_num == -1)
            {
                md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
                printf("New file '%s' with dinodenum %d!Extracting\n",file_name, md->dinode_count+1);
                file_block = WriteFile(fd, -1, file_name);
                md_add_dinode(md, st,'f', file_block);
            }
            else
            {
                printf("New hard link '%s' pointing to dinode %d\n", file_name, dinode_num);
                md_add_dirEntry(md,&dInfo,file_name, dinode_num);
            }
        }
        else if(S_ISLNK(st.st_mode))
        {
            char *linkName = getSymLinkPath(file_name);
            printf("Symbolic link :%s shows to name: %s\n", file_name, linkName);
            free(linkName);
        }
    }
    closedir(dir);
    chdir("..");

    return count;   
}



int di_find_dirlist(int fd, listofdirs * dirlist)
{
	int found = 1;
	int count = 0;

	dirNode * current = dirlist->first;
	char * file_name;

	node * dinodes_array = getInodesArray(fd);

	while (current != NULL)
	{
		file_name = current->dir;

		if (!di_find_dir(fd, file_name, dinodes_array[0].block + dinodes_array[0].offset, dinodes_array))
		{
			printf("'%s' was NOT found!\n", file_name);
			found = 0;
		}
		else
		{
			printf("'%s' was found!\n", file_name);
			count++;
		}

        current = current->next;
	}

    freeNodeArray(&dinodes_array, fd);

	if (found)
	{
		printf("List of dirs found!\n");
		return 1;
	}
	else
	{
		printf("List of dirs not in di file!(%d found)\n",count);
		return 0;
	}
	
}



int di_find_dir(int fd, char * dirname,int blockNum, node *arr)
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

            if ( !strcmp(dirname, dir.entries[i].name))
            {
            	free(dir.entries);
    			free(start);
      			return 1;
            }

            int inodeNum = dir.entries[i].dinode_num - 1;

            if(S_ISDIR(arr[inodeNum].node_info.st_mode))
            {
                if ( di_find_dir(fd, dirname, arr[inodeNum].block + arr[inodeNum].offset, arr) == 1)
                {
                	free(dir.entries);
    				free(start);
                	return 1;
                }
            }

        }

    } while (dir.next != -1);

    free(dir.entries);
    free(start);
    return 0;
}



Header di_getHeader(int fd)
{
    Header head;
    void *block = malloc(BLOCK_SIZE);

    ReadBlock(fd, 0, block);
    memcpy(&head, block, sizeof(Header));
/*  printf("FILE_SIZE: %d\n",head.file_size);
    printf("DINODES: %d\n",head.dinodes);
    printf("METADATA_BLOCK: %d\n",head.metadata_block);
*/
    free(block);
    
    return head;
}


node * getInodesArray(int fd)
{
    int i = 0;
    int nodesPerBlock = 0;
    int nextInodeBlock = 0;
    void *start = malloc(BLOCK_SIZE*sizeof(char));
    void *block = start;

    Header head;
    head = di_getHeader(fd);

    int currBlock = head.metadata_block;
    int allInodes = head.dinodes;

    node *arr = malloc(allInodes*sizeof(struct node));
    struct dinode *temp = malloc(MAX_I_NODES*sizeof(struct dinode));

    do
    {
        ReadBlock(fd, currBlock, block);
        memcpy(&nodesPerBlock, block, sizeof(int));
        block += sizeof(int);
        memcpy(&nextInodeBlock, block, sizeof(int));
        block += sizeof(int);
        
        memcpy(temp, block, nodesPerBlock*sizeof(struct dinode));
        int end = nodesPerBlock;
        for(int j=0; j<end;j++)
        {
            arr[i].block = currBlock;
            arr[i].offset = temp[j].pointer;
            arr[i].node_info = temp[j].node_info;
           
            arr[i].pathname = NULL;
            arr[i].extracted = 0;
                
            i++;
        }
        currBlock+= nextInodeBlock;
        block = start;
    } while (nextInodeBlock != -1);

    free(start);
    free(temp);

    return arr;
}

int freeNodeArray(node **arr, int fd)
{
    Header head = di_getHeader(fd);
    for (int i = 0; i < head.dinodes; ++i)
    {
        if ((*arr)[i].pathname != NULL)
            free((*arr)[i].pathname);
    }    
        
    free(*arr);
    arr = NULL;
    return 0;
}


int printHierarchy(int fd)
{
//    printf("-------Starting printMetadata()-------\n");    
    node * arr = getInodesArray(fd);
    dirTraverse(arr[0].block + arr[0].offset, fd, arr, 0);
    freeNodeArray(&arr, fd);
    return 0;
}


int printMetadata(int fd)
{
    printf("-------Starting printMetadata()-------\n");
	
    node * arr = getInodesArray(fd);

	dirTraverseMetaData(arr[0].block + arr[0].offset, fd, arr, 0);

    freeNodeArray(&arr, fd);

    return 0;
}


void printArrayNode(node inode)
{
    printf("inode: %ld\tblock: %d\toffset: %d\n", inode.node_info.st_ino, inode.block, inode.offset);
    printf("Extracted: %d\tPathname: %s\n",inode.extracted, inode.pathname);
    printStat(inode.node_info);
    return;
}


int dirTraverse(int blockNum, int fd, node *arr, int depth)
{
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
                continue;
            for (int i = 0; i < depth; ++i)
            {
                printf("|");
                if(i == depth-1)
                    printf("--");
                else
                    printf("\t");
            }
            printf("%s\n", dir.entries[i].name);

            int inodeNum = dir.entries[i].dinode_num-1;
            if(S_ISDIR(arr[inodeNum].node_info.st_mode))
                dirTraverse(arr[inodeNum].block + arr[inodeNum].offset, fd, arr, depth+1);
        }
    } while (dir.next != -1);

    free(dir.entries);
    free(start);
    return 0;
}

int dirTraverseMetaData(int blockNum, int fd, node *arr, int depth)
{
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
                continue;

            int inodeNum = dir.entries[i].dinode_num-1;

            printf("Name: %s\n", dir.entries[i].name);
            printStat(arr[inodeNum].node_info);
            printf("\n");
            if(S_ISDIR(arr[inodeNum].node_info.st_mode))
                dirTraverseMetaData(arr[inodeNum].block + arr[inodeNum].offset, fd, arr, depth+1);    
        }

    }while (dir.next != -1);

    free(dir.entries);
    free(start);
    return 0;
}

int extractDiFile(int fd)
{
    node * arr = getInodesArray(fd);

    mkdir("extractDir", S_IRWXU | S_IRWXG | S_IRWXO);
    chdir("extractDir");

    extractDir(arr[0].block + arr[0].offset, fd, arr, 0);
    chdir("..");

    freeNodeArray(&arr, fd);
    return 0;
}

int extractDir(int blockNum, int fd, node *arr, int depth)
{
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
                continue;
            int inodeNum = dir.entries[i].dinode_num-1;
            struct utimbuf times;
            times.actime = arr[inodeNum].node_info.st_atime;
            times.modtime = arr[inodeNum].node_info.st_mtime;
            
            if(S_ISDIR(arr[inodeNum].node_info.st_mode))
            {
                mkdir(dir.entries[i].name, arr[inodeNum].node_info.st_mode);
                chdir(dir.entries[i].name);

                extractDir(arr[inodeNum].block + arr[inodeNum].offset, fd, arr, depth+1);

                chdir("..");
                chown(dir.entries[i].name, arr[inodeNum].node_info.st_uid, arr[inodeNum].node_info.st_gid);
                if (utime(dir.entries[i].name, &times) != 0)
                    perror("ERROR:");
            }
            else if(S_ISREG(arr[inodeNum].node_info.st_mode))
            {

                if (arr[inodeNum].extracted == 0)
                {
                    int ffd = open(dir.entries[i].name, O_RDWR | O_CREAT, 0666);
                    arr[inodeNum].extracted = 1;
                    ExtractFile(fd, dir.entries[i].name, arr[inodeNum].block, arr[inodeNum].node_info.st_size);
                    
                    close(ffd);
                    
                    chmod(dir.entries[i].name, arr[inodeNum].node_info.st_mode);
                    chown(dir.entries[i].name, arr[inodeNum].node_info.st_uid, arr[inodeNum].node_info.st_gid);
                    if (utime(dir.entries[i].name, &times) != 0)
                        perror("utime() error");
                       
                    char *path;
                    path=get_current_dir_name();
                    char *name = malloc(strlen(path)+1+strlen(dir.entries[i].name)+1);
                    name[0] = '\0';
                    strcat(name,path);
                    strcat(name,"/");
                    strcat(name,dir.entries[i].name);
                    
                    arr[inodeNum].pathname = name;
//                    printArrayNode(arr[inodeNum]);


                    free(path);
                }
                else    //File has been extracted. So I'm dealing with hard link.
                {
                    char *relLink;
                    char *path;
                    path=get_current_dir_name();
                    relLink = relative_string(arr[inodeNum].pathname, path, 30*depth);
                    if(link(relLink, dir.entries[i].name) == -1)
                        perror("ERROR:");
                    free(path);
                    free(relLink);
                }
            }
            else if(S_ISLNK(arr[inodeNum].node_info.st_mode))
            {

            }    
        }
    } while (dir.next != -1);

    free(dir.entries);
    free(start);
    return 0;
}

char * getSymLinkPath(char *argv)
{
    struct stat sb;
    char  *linkname;
    int r;
    if (lstat(argv, &sb) == -1) {
       perror("lstat");
       exit(EXIT_FAILURE);
       }
    linkname = malloc(sb.st_size + 1);
    if (linkname == NULL) 
    {
       fprintf(stderr, "insufficient memory\n");
       exit(EXIT_FAILURE);
    }

    r = readlink(argv, linkname, sb.st_size + 1);

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
    return (linkname);
}

char * relative_string(char * path1, char * path2, int max_len)
{
    char * relative = malloc(max_len*sizeof(char));

    if (relative == NULL) return NULL;

    int max1 = strlen(path1);
    int max2 = strlen(path2);
    int last_slash = 0;
    int diff_start;
    int i = 0;
    int index = 0;

    while (path1[i] == path2[i] && i < max1 && i < max2)
    {
        if (path1[i] == '/')
        {
            last_slash = i;
        }

        i++;
    }

    if (i == max1 || i == max2)
        diff_start = i;
    else
        diff_start = last_slash;
    
    if (i != 0)
    {
        for (i=diff_start;i<max2;i++)
        {
            if (path2[i] == '/')
            {
                relative[index++] = '.';
                relative[index++] = '.';
                relative[index++] = '/';
            }
        }

        for (i=diff_start+1;i<max1;i++)
            relative[index++] = path1[i];

        relative[index] = '\0';

        return relative;

    }
    else
    {
        printf("Error.No relative path exists!\n");
        return NULL;
    } 


}