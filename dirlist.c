#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dirlist.h"


int dirlist_create(listofdirs ** dirlist)
{
	*dirlist = malloc(sizeof(listofdirs));

	if (*dirlist == NULL) return 10;

	(*dirlist)->first = NULL;
	(*dirlist)->last = NULL;

	return 0;
}



int dirlist_add_dir(listofdirs * dirlist, char dir[30])
{
	dirNode * new_dir = malloc(sizeof(dirNode));
	strcpy(new_dir->dir, dir);
	new_dir->next = NULL;

	if (dirlist->first == NULL)
	{
		dirlist->first = new_dir;
		dirlist->last = new_dir;
	}
	else
	{
		dirlist->last->next = new_dir;
		dirlist->last = new_dir;	
	}

	return 0;
}



int dirlist_free(listofdirs ** dirlist)
{
	if (*dirlist == NULL)/*no list to free*/
	{
		return 1;
	}

	dirNode * current;
	dirNode * next;

	current = (*dirlist)->first;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}

	free(*dirlist);
	*dirlist = NULL;

	return 0;
}



listofdirs * path_to_list(char * path)
{
	listofdirs * path_list;
	dirlist_create(&path_list);

	char copy[100];
	strcpy(copy, path);

	char * token;

	token = strtok(copy, "/");
	while( token != NULL ) 
   	{
		dirlist_add_dir(path_list, token);

    	token = strtok(NULL, "/");
 
   	}

	dirNode * current = path_list->first;

	while (current != NULL)
	{
		printf("print: %s\n",current->dir);

		current = current->next;
	}

	return path_list;
}