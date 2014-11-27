#include <stdlib.h>

#include "list.h"

list *list_new()
{
	list *new_list = malloc(sizeof(list));

	new_list->head = NULL;
	new_list->tail = NULL;
	new_list->size = 0;

	return new_list;
}

void list_append(list* list, void *data)
{
	list_node *new_node = malloc(sizeof(list_node));

	new_node->data = data;
	new_node->next = NULL;

	if(list->tail)
	{
		list->tail->next = new_node;
	}

	if(!list->head)
	{
		list->head = new_node;
	}

	list->tail = new_node;
	list->size++;
}

void list_free(list *list)
{
	list_node *current = list->head;

	while(current)
	{
		list_node *next = current->next;

		free(current->data);
		free(current);

		current = next;
	}
}
