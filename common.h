#ifndef COMMON_H
#define COMMON_H

struct node_addr
{
	int addr;
	unsigned short port;
};

typedef struct node_addr node_addr;

void fatal_error(const char *msg);

#endif
