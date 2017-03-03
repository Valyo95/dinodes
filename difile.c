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


int di_createfile(char * filename, listofdirs * dirlist, int compress)
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
    off_t compression_size;

    md_add_dinode(md,st,'d', 0, 0);

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
            count += di_add_dir(fd, file_name, 1, md, compress);
        }
        else if(S_ISREG(st.st_mode))
        {
            dinode_num = md_find_dinode(md, st.st_ino);

            if (dinode_num == -1)
            {
                md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
                printf("New file '%s' with dinodenum %d!Saving\n",file_name, md->dinode_count+1);
                file_block = WriteFile(fd, -1, file_name, compress, &compression_size);
                md_add_dinode(md, st,'f', file_block, compression_size);
            }
            else
            {
                printf("New hard link '%s' pointing to dinode %d\n", file_name, dinode_num);
                md_add_dirEntry(md,&dInfo,file_name, dinode_num);
            }
        }
        else if(S_ISLNK(st.st_mode))
        {
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
            char *linkName = getSymLinkPath(file_name);
            printf("Symbolic link :%s shows to name: %s\n", file_name, linkName);
            file_block = WriteSoftLink(fd, -1, linkName);
            md_add_dinode(md, st,'f', file_block, 0);
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
    // md_printall(md);

    printf("\nDI FILE '%s' CREATED!Printing head:\n", filename);
    printf("FILE SIZE: %d bytes\n",head.file_size);
    printf("DINODES: %d\n",head.dinodes);
    printf("METADATA BLOCK: %d\n",head.metadata_block);

	CloseFile(fd);
	md_free(&md);
	free(block);

	return 0;
}



int di_add_dir(int fd, char *dirname, int parent_num, metadata * md, int compress)
{
    dirInfo * dInfo;

    int count = 1;
    int file_block;
    struct stat st;
    int dinode_num;

    lstat(dirname, &st);
    md_add_dinode(md,st,'d',0, 0);

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
    off_t compression_size;
    
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
            count += di_add_dir(fd, file_name, dinode_num, md, compress);
        }
        else if(S_ISREG(st.st_mode))
        {
            dinode_num = md_find_dinode(md, st.st_ino);

            if (dinode_num == -1)
            {
                md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
                printf("New file '%s' with dinodenum %d!Saving\n",file_name, md->dinode_count+1);
                file_block = WriteFile(fd, -1, file_name, compress, &compression_size);
                md_add_dinode(md, st,'f', file_block, compression_size);
            }
            else
            {
                printf("New hard link '%s' pointing to dinode %d\n", file_name, dinode_num);
                md_add_dirEntry(md,&dInfo,file_name, dinode_num);
            }
        }
        else if(S_ISLNK(st.st_mode))
        {
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
            char *linkName = getSymLinkPath(file_name);
            printf("Symbolic link :%s shows to name: %s\n", file_name, linkName);

            file_block = WriteSoftLink(fd, -1, linkName);
            //printf("Symlink saved at %d\n",file_block);
            md_add_dinode(md, st,'f', file_block, 0);
            
            free(linkName);
        }
    }
    closedir(dir);
    chdir("..");

    return count;   
}



int di_append(char * filename, listofdirs * dirlist, int compress)
{
    int fd;
    fd = OpenFile(filename);

    /*WRITE HEADER (FOR NOW)*/
    if (BlockCounter(fd) == 0)
    {/*no file exists*/
        printf("'%s' does not exist.Using create instead of append!\n", filename);
        return di_createfile(filename, dirlist, compress);
    }

    Header head = di_getHeader(fd);
    int metadata_block = head.metadata_block;
    void * block = malloc(BLOCK_SIZE);
    void * start = block;
    node * old_nodes = getInodesArray(fd);

    /*START CREATING FILE*/
    metadata * md;
    md_create(&md, BLOCK_SIZE);
    int count = head.dinodes;
    md->dinode_count = head.dinodes;

    dirInfo * dInfo;
    struct stat st;
    st.st_ino = -1;
    int dinode_num;
    off_t compression_size;

    dirNode * current = dirlist->first;
    int list_size = 0;
    while (current != NULL)
    {
        list_size++;
        current = current->next;
    }
    current = dirlist->first;

    int * entry_nums = malloc(list_size*sizeof(int));
    entry_nums[0] = count + 1;
    int index = 1;

    char * file_name;
    int dinodes_added;

    while (current != NULL)
    {
        dinodes_added = 0;
        int file_block;

        file_name = current->dir;
        lstat(file_name, &st);


        if(S_ISDIR(st.st_mode))
        {
            dinodes_added = di_append_dir(fd, file_name, 1, md, compress, &metadata_block, old_nodes, head.dinodes);
        }
        else if(S_ISREG(st.st_mode))
        {
            /*try finding it in old dinodes (before append)*/
            dinode_num = find_dinode(old_nodes, st.st_ino, head.dinodes);
            if (dinode_num == -1)/*try finding it in new dinodes (during append)*/
                dinode_num = md_find_dinode(md, st.st_ino);

            if (dinode_num == -1)
            {
                dinodes_added = 1;
                md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
                printf("New file '%s'  with dinodenum %d!Saving\n", file_name, md->dinode_count +1);
                WriteFile(fd, metadata_block, file_name, compress, &compression_size);
                file_block = metadata_block;

                if (compression_size != -1)
                {
                    metadata_block += compression_size / BLOCK_SIZE;
                    if (compression_size % BLOCK_SIZE != 0)
                        metadata_block++; 
                }
                else
                {
                    metadata_block += st.st_size / BLOCK_SIZE;
                    if (st.st_size % BLOCK_SIZE != 0)
                        metadata_block++; 
                }
                md_add_dinode(md, st,'f', file_block, compression_size);
            }
            else
            {
                printf("New hard link '%s' pointing to dinode %d\n", file_name, dinode_num);
                md_add_dirEntry(md,&dInfo,file_name, dinode_num);
            }
        }
        else if(S_ISLNK(st.st_mode))
        {
            dinodes_added = 1;
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
            char *linkName = getSymLinkPath(file_name);
            printf("Symbolic link :%s shows to name: %s\n", file_name, linkName);
            file_block = WriteSoftLink(fd, metadata_block, linkName);
            file_block = metadata_block;
            metadata_block++;
            md_add_dinode(md, st,'f', file_block, 0);
            free(linkName);

        }

        current = current->next;
        count += dinodes_added;
        if (index != list_size)
            entry_nums[index++] = count + 1; 
    }


    /*WRITE CORRECT HEADER*/
    freeNodeArray(&old_nodes, fd);
    head.file_size = (BlockCounter(fd) + md->block_count)*BLOCK_SIZE;
    head.dinodes = md->dinode_count;
    int new_meta_start = BlockCounter(fd);
    head.metadata_block = metadata_block;

    memcpy(block, &head, sizeof(head));
    WriteBlock(fd, 0, block);

    md_writetofile(md, fd);
    // md_printall(md);


    int currBlock = metadata_block + 1;
    int next = 0;
    do
    {
        currBlock += next;        

        ReadBlock(fd, currBlock, block);
        block = start;
        block += sizeof(int);
        memcpy(&next, block, sizeof(int));
    }
    while (next != -1);

    block = start;
    memcpy(&count, block, sizeof(int));

    block += 2*sizeof(int) + count*sizeof(dirEntry);

    dirEntry new_entry;
    current = dirlist->first;
    listofdirs * path;

    int i;
    for (i=0;i<list_size;i++)
    {
        path = path_to_list(current->dir);
        strcpy(new_entry.name, path->last->dir);
        new_entry.dinode_num = entry_nums[i];

        memcpy(block, &new_entry, sizeof(dirEntry));
        block += sizeof(dirEntry);
        count++;
    }

    block = start;
    memcpy(block, &count, sizeof(int));
    WriteBlock(fd, currBlock, block);

    currBlock = metadata_block;
    next = 0;
    do
    {
        currBlock += next;        
        ReadBlock(fd, currBlock, start);
        block = start;
        block += sizeof(int);
        memcpy(&next, block, sizeof(int));
    }
    while (next != -1);

    int new_offset = new_meta_start - currBlock;
    memcpy(block, &new_offset, sizeof(int));
    block = start;
    WriteBlock(fd, currBlock, block);


    printf("\nAPPEND AT DI FILE '%s' FINISHED!Printing head:\n", filename);
    printf("FILE SIZE: %d bytes\n",head.file_size);
    printf("DINODES: %d\n",head.dinodes);
    printf("METADATA BLOCK: %d\n",head.metadata_block);

    CloseFile(fd);
    md_free(&md);
    free(block);

    return 0;
}



int di_append_dir(int fd, char *dirname, int parent_num, metadata * md,
                    int compress, int * metadata_block, node * old_nodes, int old_count)
{
    dirInfo * dInfo;

    int count = 1;
    int file_block;
    struct stat st;
    int dinode_num;

    lstat(dirname, &st);
    md_add_dinode(md,st,'d',0, 0);

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
    off_t compression_size;
    
    dir = opendir(cur_dir);
    while ((dp=readdir(dir)) != NULL) 
    {
        if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") )
            continue;
                
        file_name = dp->d_name;

        lstat(file_name, &st);

        if(S_ISDIR(st.st_mode))
        {
            /*this dirInfo will be created at next available block*/
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);    
            count += di_append_dir(fd, file_name, dinode_num, md, compress, metadata_block, old_nodes, old_count);
        }
        else if(S_ISREG(st.st_mode))
        {
            /*try finding it in old dinodes (before append)*/
            dinode_num = find_dinode(old_nodes, st.st_ino, old_count);
            if (dinode_num == -1)/*try finding it in new dinodes (during append)*/
                dinode_num = md_find_dinode(md, st.st_ino);

            if (dinode_num == -1)
            {
                md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
                printf("New file '%s'  with dinodenum %d!Saving\n", file_name, md->dinode_count +1);
                WriteFile(fd, *metadata_block, file_name, compress, &compression_size);
                file_block = *metadata_block;

                if (compression_size != -1)
                {
                    *metadata_block += compression_size / BLOCK_SIZE;
                    if (compression_size % BLOCK_SIZE != 0)
                        (*metadata_block)++; 
                }
                else
                {
                    *metadata_block += st.st_size / BLOCK_SIZE;
                    if (st.st_size % BLOCK_SIZE != 0)
                        (*metadata_block)++; 
                }
                md_add_dinode(md, st,'f', file_block, compression_size);
                count++;
            }
            else
            {
                printf("New hard link '%s' pointing to dinode %d\n", file_name, dinode_num);
                md_add_dirEntry(md,&dInfo,file_name, dinode_num);
            }
        }
        else if(S_ISLNK(st.st_mode))
        {
            md_add_dirEntry(md,&dInfo,file_name, md->dinode_count + 1);
            char *linkName = getSymLinkPath(file_name);
            printf("Symbolic link :%s shows to name: %s saving at block %d\n", file_name, linkName, *metadata_block);

            file_block = WriteSoftLink(fd, *metadata_block, linkName);
            file_block = *metadata_block;
            (*metadata_block)++;
            md_add_dinode(md, st,'f', file_block, 0);
            count++;
            
            free(linkName);
        }
    }
    closedir(dir);
    chdir("..");


    return count;   
}



int di_find_dirlist(int fd, listofdirs * dirlist)
{
	int count = 0;
    listofdirs * pathlist; 
	dirNode * current = dirlist->first;
	char * file_name;

	node * dinodes_array = getInodesArray(fd);

	while (current != NULL)
	{
		file_name = current->dir;
        pathlist = path_to_list(current->dir);
        
		di_find_dir(fd,pathlist->first , dinodes_array[0].block + dinodes_array[0].offset, dinodes_array, file_name);
		
        
        current = current->next;
        dirlist_free(&pathlist);

	}

    freeNodeArray(&dinodes_array, fd);

    return count;	
}



int di_find_dir(int fd, dirNode *list,int blockNum, node *arr, char *path)
{
    int i;
    int found = 0;
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
            
            if(S_ISDIR(arr[inodeNum].node_info.st_mode))
            {
                if(strcmp(list->dir, dir.entries[i].name)== 0)
                {
                    free(dir.entries);
                    free(start);
                    if(list->next != NULL)
                        return di_find_dir(fd, list->next, arr[inodeNum].block + arr[inodeNum].offset, arr, path);

                    found = 1;
                    printf("%s found!\n", path);
                    return 0;    
                }
            }
            else if(S_ISREG(arr[inodeNum].node_info.st_mode) && list->next == NULL)
            {
                if (strcmp(list->dir, dir.entries[i].name) == 0)
                {
                    free(dir.entries);
                    free(start);
                
                    found = 1;
                    printf("%s found!\n", path);
                    return 0;                    
                }
            }
        }

    } while (dir.next != -1);

    free(dir.entries);
    free(start);
    if (found == 0)
    {
        fprintf(stderr, "No file/dir with name %s\n", path);
        return 1;
    }
    return 0;
}





Header di_getHeader(int fd)
{
    Header head;
    void *block = malloc(BLOCK_SIZE);

    ReadBlock(fd, 0, block);
    memcpy(&head, block, sizeof(Header));
/*  printf("FILE SIZE: %d bytes\n",head.file_size);
    printf("DINODES: %d\n",head.dinodes);
    printf("METADATA BLOCK: %d\n",head.metadata_block);
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
            arr[i].compression_size = temp[j].compression_size;
           
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



int find_dinode(node * arr, ino_t inode_num , int max)
{
    int i;

    for (i = 0; i < max; ++i)
    {
        if ( arr[i].node_info.st_ino == inode_num)
            return (i+1);
    }

    return -1;
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






int extractDiFile(int fd, char *fileName,listofdirs *list)
{
    int found = 0;
    node *arr;
    arr = getInodesArray(fd);

    dirNode * current = list->first;
    listofdirs * pathlist; 
    char *name = malloc((strlen(fileName)+1)*sizeof(char));
    strcpy(name,fileName);
    name[strlen(name)-3] = '\0';

    mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO);
    chdir(name);

    if(list->first == NULL)
    {    
        
        extractDir(arr[0].block + arr[0].offset, fd, arr, 0);
        chdir("..");

    }
    else
    {
        while ( current != NULL)
        {
            pathlist = path_to_list(current->dir);

            if(SearchNode(arr[0].block + arr[0].offset, fd, arr, 0, pathlist->first) == 0 )
            {
                found = 1;
            }

            current = current->next;

            dirlist_free(&pathlist);
        }

        if (found == 0)
            remove(name);
    }

    free(name);
    freeNodeArray(&arr, fd);
    return 0;
}



int SearchNode(int blockNum, int fd, node *arr, int depth, dirNode *list)
{
    int i;
    int found = 0;
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
            printf("Found: %s\n",  dir.entries[i].name);
            printf("Searching for: %s\n",  list->dir);

            if(S_ISDIR(arr[inodeNum].node_info.st_mode))
            {
                if(strcmp(list->dir, dir.entries[i].name)== 0)
                {
                    found = 1;
                    if(list->next != NULL)
                    {

                        free(dir.entries);
                        free(start);
                        return SearchNode(arr[inodeNum].block + arr[inodeNum].offset, fd, arr, depth+1, list->next);
                    }
                    else
                    {
                        mkdir(dir.entries[i].name, arr[inodeNum].node_info.st_mode);
                        chdir(dir.entries[i].name);

                        extractDir(arr[inodeNum].block + arr[inodeNum].offset, fd, arr, depth+1);

                        chdir("..");
                        chown(dir.entries[i].name, arr[inodeNum].node_info.st_uid, arr[inodeNum].node_info.st_gid);
                        if (utime(dir.entries[i].name, &times) != 0)
                            perror("ERROR:");
                    }    
                }
            }
            else if(S_ISREG(arr[inodeNum].node_info.st_mode) && list->next == NULL)
            {
                if (strcmp(list->dir, dir.entries[i].name)== 0)
                {
                    found = 1;
                    if (arr[inodeNum].extracted == 0)
                    {
                        {
                            arr[inodeNum].extracted = 1;
                            ExtractFile(fd, dir.entries[i].name, arr[inodeNum].offset, arr[inodeNum].node_info.st_size, arr[inodeNum].compression_size);

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
            }
        }

    } while (dir.next != -1);

    free(dir.entries);
    free(start);
    if (found == 0)
    {
        fprintf(stderr, "No such file/dir!\n");
        return 1;
    }
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
                    arr[inodeNum].extracted = 1;
                    ExtractFile(fd, dir.entries[i].name, arr[inodeNum].offset, arr[inodeNum].node_info.st_size, arr[inodeNum].compression_size);


                    int ffd = open(dir.entries[i].name, O_RDWR | O_CREAT, 0666);
                    chmod(dir.entries[i].name, arr[inodeNum].node_info.st_mode);
                    chown(dir.entries[i].name, arr[inodeNum].node_info.st_uid, arr[inodeNum].node_info.st_gid);
                    if (utime(dir.entries[i].name, &times) != 0)
                        perror("utime() error");

                        close(ffd);

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
                char * softPath = ReadSoftLink(fd, arr[inodeNum].offset);
                symlink(softPath, dir.entries[i].name);

                chmod(dir.entries[i].name, arr[inodeNum].node_info.st_mode);
                chown(dir.entries[i].name, arr[inodeNum].node_info.st_uid, arr[inodeNum].node_info.st_gid);
                //BELOW IS COMMENTED BECAUSE UTIME WONT WORK FOR SOFT LINKS
                // if (utime(dir.entries[i].name, &times) != 0)
                //     perror("utime() error");
                
                free(softPath);
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


int printStat(struct stat sb)
{
   printf("File type:                ");
    switch (sb.st_mode & S_IFMT) 
    {
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
    return 0;
}