#ifndef LIST_H
#define LIST_H

struct list_node
{
	struct list_node *next;
	void *data;
};

struct list
{
	struct list_node *head;
	struct list_node *tail;
	size_t size;
};

typedef struct list_node list_node;
typedef struct list list;

list *list_new();
void list_append(list* list, void *data);
void list_free(list *list);

#endif
