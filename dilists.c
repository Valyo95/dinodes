#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dilists.h"

int dirInfo_create(dirInfoHead ** head, int current, int parent)
{
	*head = malloc(sizeof(dirInfoHead));

	if (*head == NULL) return 10;

	(*head)->count = 0;
	(*head)->current = current;
	(*head)->parent = parent;
	(*head)->first = NULL;
	(*head)->last = NULL;

	return 0;
}


int dirInfo_add(dirInfoHead * head, char name[30], int dinode_num)
{
	dirInfoNode * newNode = malloc(sizeof(dirInfoNode));

	if (newNode == NULL) return 10;

	strcpy(newNode->entry.name, name);
	newNode->entry.dinode_num = dinode_num;
	newNode->next = NULL;

	if (head->first == NULL)
	{
		head->first = newNode;
		head->last = newNode;
	}
	else
	{
		head->last->next = newNode;
		head->last = newNode;
	}

	return 0;
}


int dirInfo_delete(dirInfoHead * head, char name[30])
{
	int found = 0;

	dirInfoNode * prev = NULL;
	dirInfoNode * current = head->first;

	while (current != NULL)
	{
		if (!strcmp(name,current->entry.name))
		{
			found = 1;
			break;
		}

		prev = current;
		current = current->next;
	}

	if (found)
	{
		if (prev == NULL)/*its the first int the list*/
			head->first = current->next;
		else
			prev->next = current->next;

		free(current);

		return 0;
	}
	else
	{
		return 1;
	}
}


void dirInfo_printall(dirInfoHead * head)
{
	printf("\n\n---Printing all dirInfo---\n\n");
	printf(". : %d\n", head->current);
	printf(".. : %d\n", head->parent);

	dirInfoNode * current = head->first;

	while (current != NULL)
	{
		printf("%s :%d\n", current->entry.name, current->entry.dinode_num);

		current = current->next;
	}
}


int dirInfo_free(dirInfoHead ** head)
{
	if(*head == NULL)
	{/*No list to free*/
		return 0;
	}

	dirInfoNode * current = (*head)->first;
	dirInfoNode * next = NULL;

	if (current != NULL)
	{/*if there is a first node (list is not empty)*/

		while (current->next != NULL)
		{/*free all nodes that have a next*/
			next = current->next;
			free(current);
			current = next;
		}
	
		free(current);/*free the last (has no next)*/
	}

	free(*head);/*free the list itself (struct)*/
	*head = NULL;

	return 0;
}