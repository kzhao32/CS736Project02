#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "common.h"
#include "list.h"

void handle_req(int fd, list *node_list)
{
	/* buf[0] -> IPv4 address, buf[1] -> port number */
	list_node* node;
	node_list_entry *entry;
	int buf[2];
	int rc;

	rc = read(fd, buf, sizeof(buf));

	if(rc != sizeof(buf))
	{
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

	node = node_list->head;

	while(node)
	{
		write(fd, &(node->data), sizeof(node_list_entry));

		node = node->next;
	}

	node = malloc(sizeof(node_list_entry));

	/* Send a NULL so that the other side knows that we're done */
	bzero(node, sizeof(node_list_entry));
	write(fd, &(entry), sizeof(node_list_entry));

	free(node);
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
