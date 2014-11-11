#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "common.h"
#include "list.h"

void print_node(node_addr *node)
{
	char *buf = malloc(100);

	printf(	"Node %s:%d\n",
		inet_ntop(AF_INET, &(node->addr), buf, 100),
		ntohs(node->port) );

	free(buf);
}

void handle_req(int fd, list *node_list)
{
	/* buf[0] -> IPv4 address, buf[1] -> port number */
	list_node* node;
	node_addr *entry;
	struct sockaddr_in client_addr;
	socklen_t addr_len;
	unsigned short buf;
	int rc;

	getsockname(fd, (struct sockaddr *)&client_addr, &addr_len);

	{
		char *buf = malloc(100);

		printf(	"Peer %s:%d\n",
			inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), buf, 100),
			client_addr.sin_port );

		free(buf);
	}

	rc = read(fd, &buf, sizeof(buf));

	if(rc != sizeof(buf))
	{
		fatal_error("read() failed");
	}

	entry = malloc(sizeof(node_addr));

	entry->addr = client_addr.sin_addr.s_addr;
	entry->port = buf;

	printf("BUF: %d %hu\n", buf, ntohs(buf));

	list_append(node_list, entry);

	{
		char *buf = malloc(100);

		printf(	"Added node %s:%hu\n",
			inet_ntop(AF_INET, &(entry->addr), buf, 100),
			ntohs(entry->port) );

		free(buf);
	}

	node = node_list->head;

	while(node)
	{
		write(fd, node->data, sizeof(node_addr));

		print_node(node->data);

		node = node->next;
	}

	/* Send a NULL so that the other side knows that we're done */
	entry = calloc(1, sizeof(node_addr));

	write(fd, entry, sizeof(node_addr));

	free(entry);
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
