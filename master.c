#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

struct list_node
{
	struct list_node *next;
	void *data;
};

struct list
{
	struct list_node *head;
	struct list_node *tail;
};

struct node_list_entry
{
	int addr;
	int port; /* TODO: Consider using short */
};

typedef struct list_node list_node;
typedef struct list list;
typedef struct node_list_entry node_list_entry;

void fatal_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);

	exit(1);
}

list *list_new()
{
	list *new_list = malloc(sizeof(list));

	new_list->head = NULL;
	new_list->tail = NULL;

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

void handle_req(int fd, list *node_list)
{
	/* buf[0] -> IPv4 address, buf[1] -> port number */
	node_list_entry *entry;
	int buf[2];
	int rc;

	rc = read(fd, buf, sizeof(buf));

	if(rc != sizeof(buf))
	{
		printf("%d %d\n", rc, sizeof(buf));
		fatal_error("read() failed");
	}

	entry = malloc(sizeof(node_list_entry));

	entry->addr = ntohl(buf[0]);
	entry->port = ntohl(buf[1]);

	list_append(node_list, entry);

	{
		char *buf = malloc(100);

		printf(	"Added node %s:%d\n",
			inet_ntop(AF_INET, &(entry->addr), buf, 100),
			entry->port );

		free(buf);
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in comm_addr;
	socklen_t comm_addr_len;
	list *node_list;
	int comm_socket;
	int rc;

	if(argc != 3)
	{
		fatal_error("Invalid argument");
	}

	node_list = list_new();
	comm_socket = socket(AF_INET, SOCK_STREAM, 0);
	comm_addr_len = sizeof(comm_addr);

	if(comm_socket < 0)
	{
		fatal_error("socket() failed");
	}

	bzero(&comm_addr, comm_addr_len);

	comm_addr.sin_family = AF_INET;
	comm_addr.sin_port = htons(atoi(argv[2]));

	inet_pton(AF_INET, argv[1], &comm_addr.sin_addr.s_addr);

	rc = bind(comm_socket, (struct sockaddr *)&comm_addr, comm_addr_len);

	if(rc != 0)
	{
		fatal_error("bind() failed");
	}

	listen(comm_socket, 1);

	{
		char *buf = malloc(comm_addr_len);

		getsockname(comm_socket, (struct sockaddr *)&comm_addr, &comm_addr_len);
		printf(	"Listening on %s:%d\n",
			inet_ntop(AF_INET, &comm_addr.sin_addr, buf, comm_addr_len),
			ntohs(comm_addr.sin_port) );

		free(buf);
	}

	while(1)
	{
		int new_comm_socket;

		new_comm_socket = accept(	comm_socket,
						(struct sockaddr *)
						&comm_addr,
						&comm_addr_len );

		handle_req(new_comm_socket, node_list);

		close(new_comm_socket);
	}

	close(comm_socket);

	list_free(node_list);

	return 0;
}
