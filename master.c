#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "def.h"
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

void assign_token(node_addr *target)
{
	struct sockaddr_in target_addr;
	socklen_t target_addr_len;
	int target_socket;
	int cmd;
	int rc;

	target_socket = socket(AF_INET, SOCK_STREAM, 0);
	target_addr_len = sizeof(target_addr);

	bzero(&target_addr, target_addr_len);

	printf("ASSIGN: ");
	print_node(target);

	target_addr.sin_family = AF_INET;
	target_addr.sin_addr.s_addr = target->addr;
	target_addr.sin_port = target->port;

	rc = connect(target_socket, (struct sockaddr *)&target_addr, target_addr_len);

	if(rc < 0)
	{
		perror(NULL);
		fatal_error("Can't connect to the next node");
	}

	cmd = TOKEN_CMD;
	rc = write(target_socket, &cmd, sizeof(cmd));

	if(rc != sizeof(cmd))
	{
		perror(NULL);
		fatal_error("write() failed");
	}

	close(target_socket);
}

void handle_init_req(int fd, list *node_list, int node_count)
{
	node_addr *entry;
	socklen_t addr_len;
	struct sockaddr_in client_addr;
	unsigned short port;
	int cmd;
	int rc;

	if(node_list->size < node_count)
	{
		rc = read(fd, &port, sizeof(port));

		if(rc != sizeof(port))
		{
			fatal_error("read() failed");
		}

		addr_len = sizeof(struct sockaddr_in);

		getpeername(fd, (struct sockaddr *)&client_addr, &addr_len);

		entry = malloc(sizeof(node_addr));

		/* Zero the memory block so that valgrind wouldn't complain */
		bzero(entry, sizeof(node_addr));

		entry->addr = client_addr.sin_addr.s_addr;
		entry->port = port;

		list_append(node_list, entry);

		{
			char *buf = malloc(100);

			printf(	"Added node %s:%hu\n",
				inet_ntop(AF_INET, &(entry->addr), buf, 100),
				ntohs(entry->port) );

			free(buf);
		}

		cmd = OKAY_CMD;

		write(fd, &cmd, sizeof(cmd));

		/* Send back the requesting node's own address */
		write(fd, entry, sizeof(entry));

		if(node_list->size == node_count)
		{
			assign_token(node_list->head->data);
		}
	}
	else
	{
		cmd = FULL_CMD;

		write(fd, &cmd, sizeof(cmd));
	}
}

void handle_list_req(int fd, list *node_list, int node_count)
{
	int cmd;

	if(node_list->size == node_count)
	{
		list_node *node;
		node_addr *sentinel;

		cmd = OKAY_CMD;

		write(fd, &cmd, sizeof(cmd));

		node = node_list->head;

		while(node)
		{
			write(fd, node->data, sizeof(node_addr));
			print_node(node->data);

			node = node->next;
		}

		/* Send a NULL so that the other side knows that we're done */
		sentinel = calloc(1, sizeof(node_addr));

		write(fd, sentinel, sizeof(node_addr));

		free(sentinel);
	}
	else
	{
		printf("LIST WAIT\n");
		cmd = WAIT_CMD;

		write(fd, &cmd, sizeof(cmd));
	}
}

void handle_req(int fd, list *node_list, int node_count)
{
	int cmd;
	int rc;

	rc = read(fd, &cmd, sizeof(cmd));

	if(rc != sizeof(cmd))
	{
		fatal_error("read() failed");
	}

	if(cmd == INIT_CMD)
	{
		printf("INIT\n");
		handle_init_req(fd, node_list, node_count);
	}
	else if(cmd == LIST_CMD)
	{
		printf("LIST\n");
		handle_list_req(fd, node_list, node_count);
	}
	else
	{
		printf("Unrecognized command received");
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in comm_addr;
	socklen_t comm_addr_len;
	list *node_list;
	int comm_socket;
	int rc;

	if(argc != 4)
	{
		printf(	"Usage: %s <bind addr> <bind port> <node count>\n",
			argv[0] );

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

	comm_addr.sin_addr
		= *((struct in_addr *)gethostbyname(argv[1])->h_addr_list[0]);

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

		handle_req(new_comm_socket, node_list, atoi(argv[3]));
		close(new_comm_socket);
	}

	close(comm_socket);
	list_free(node_list);

	return 0;
}
