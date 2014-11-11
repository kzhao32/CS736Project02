#ifndef COMMON_H
#define COMMON_H

struct node_list_entry
{
	int addr;
	int port; /* TODO: Consider using short */
};

typedef struct node_list_entry node_list_entry;

void fatal_error(const char *msg);

#endif
