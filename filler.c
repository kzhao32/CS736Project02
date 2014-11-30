#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include "def.h"
#include "common.h"
#include "list.h"

#define TOKEN_PATH "token"
#define COMM_ADDR INADDR_ANY
#define COMM_PORT 2345
#define BUF_PATH "/tmp/bonsai.dat"
#define BUF_RSIZE 256

struct filler_ctx
{
	struct sockaddr_in master_addr;
	struct node_addr *self_addr;
	struct node_addr *next_addr;
	struct list *node_list;
};

typedef struct filler_ctx filler_ctx;

void print_node(node_addr *node)
{
	if(node)
	{
		char *buf = malloc(100);

		printf(	"Node %s:%d\n",
			inet_ntop(AF_INET, &(node->addr), buf, 100),
			ntohs(node->port) );

		free(buf);
	}
	else
	{
		printf("NULL\n");
	}
}

int read_all(int fd, void *buf, size_t count)
{
	int rc = 0;

	while(rc != -1 && count > 0)
	{
		count -= rc;

		rc = read(fd, buf, count);
	}

	return (rc == -1)?-1:0;
}

node_addr *get_next_node(filler_ctx *ctx)
{
	list_node *node = ctx->node_list->head;
	node_addr *addr = node?node->data:NULL;
	node_addr *result = NULL;

	/* Iterate the list until own addr and port is found */
	while(node
	&& ctx->self_addr
	&& (addr->addr != ctx->self_addr->addr
	||  addr->port != ctx->self_addr->port))
	{
		node = node->next;
		addr = node?node->data:NULL;
	}

	/* Select the next node in the list as the next node, if own entry is at
	 * the end of the list, choose the first entry
	 */
	if(node && ctx->self_addr)
	{
		result = node->next?node->next->data:ctx->node_list->head->data;
	}

	return result;
}

int req_init(filler_ctx *ctx, unsigned short self_port)
{
	int fd;
	int rc;
	int cmd;

	printf(	"MASTER: %s:%d\n",
		inet_ntoa(ctx->master_addr.sin_addr),
		ntohs(ctx->master_addr.sin_port) );

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd < 0)
	{
		fatal_error("Failed to create socket");
	}

	rc = connect(	fd,
			(struct sockaddr *)&ctx->master_addr,
			sizeof(struct sockaddr_in) );

	if(rc < 0)
	{
		fatal_error("connect() failed");
	}

	cmd = INIT_CMD;

	write(fd, &cmd, sizeof(cmd));
	write(fd, &self_port, sizeof(unsigned short));
	read(fd, &cmd, sizeof(cmd));

	if(cmd == OKAY_CMD)
	{
		printf("INIT OKAY\n");

		ctx->self_addr = malloc(sizeof(node_addr));

		/* Read own address from server */
		read(fd, ctx->self_addr, sizeof(node_addr));
	}
	else
	{
		printf("INIT FULL\n");
	}

	return (cmd == OKAY_CMD)-1;
}

int req_list(filler_ctx *ctx)
{
	int fd;
	int rc;
	int cmd;
	int done;
	node_addr *node;

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd < 0)
	{
		fatal_error("Failed to create socket");
	}

	rc = connect(	fd,
			(struct sockaddr *)&ctx->master_addr,
			sizeof(struct sockaddr_in) );

	if(rc < 0)
	{
		fatal_error("connect() failed");
	}

	cmd = LIST_CMD;

	write(fd, &cmd, sizeof(cmd));
	read(fd, &cmd, sizeof(cmd));

	if(cmd == OKAY_CMD)
	{
		printf("LIST OKAY\n");

		done = 0;

		do
		{
			node = malloc(sizeof(node_addr));

			read_all(fd, node, sizeof(node_addr));

			if(node->addr != 0 || node->port != 0)
			{
				print_node(node);

				list_append(ctx->node_list, node);
			}
			else
			{
				done = 1;
			}
		}
		while(!done);

		ctx->next_addr = get_next_node(ctx);

		printf("Next node: ");
		print_node(ctx->next_addr);

		/* Last malloc() wil be unused because the last node sent by the
		 * server will be the sentinel.
		 */
		free(node);
	}
	else
	{
		printf("LIST WAIT\n");
	}

	return (cmd == OKAY_CMD)-1;
}

void wait_for_token(	int comm_socket,
			struct sockaddr_in *comm_addr,
			socklen_t *comm_addr_len )
{
	int new_comm_socket;
	int buf;
	int rc;

	/* Wait for token */
	new_comm_socket = accept(	comm_socket,
					(struct sockaddr *)
					comm_addr,
					comm_addr_len );

	printf("Connection accepted\n");

	if(new_comm_socket < 0)
	{
		fatal_error("socket() failed");
	}

	printf("READ\n");
	rc = read(new_comm_socket, &buf, sizeof(buf));

	close(new_comm_socket);

	if(rc != sizeof(buf))
	{
		fatal_error("read() failed");
	}

	/* TODO: Re-enable TOKEN_CMD check */
	if(buf != TOKEN_CMD && 0)
	{
		fatal_error("Magic number mismatch");
	}

	rc = open(TOKEN_PATH, O_RDONLY|O_CREAT, 0666);

	printf("Create token file\n");

	if(rc < 0)
	{
		perror(NULL);
		fatal_error("Failed to create token file");
	}

	close(rc);
}


void pass_token(filler_ctx *ctx)
{
	struct sockaddr_in next_addr;
	socklen_t next_addr_len;
	int next_socket;
	int buf;
	int rc;

	next_socket = socket(AF_INET, SOCK_STREAM, 0);
	next_addr_len = sizeof(next_addr);

	bzero(&next_addr, next_addr_len);

	printf("PASS: ");
	print_node(ctx->next_addr);

	next_addr.sin_family = AF_INET;
	next_addr.sin_addr.s_addr = ctx->next_addr->addr;
	next_addr.sin_port = ctx->next_addr->port;

	rc = connect(next_socket, (struct sockaddr *)&next_addr, next_addr_len);

	if(rc < 0)
	{
		perror(NULL);
		fatal_error("Can't connect to the next node");
	}

	buf = TOKEN_CMD;
	rc = write(next_socket, &buf, sizeof(buf));

	if(rc != sizeof(buf))
	{
		perror(NULL);
		fatal_error("write() failed");
	}

	close(next_socket);
}

int main(int argc, char **argv)
{
	int buf_fd;
	filler_ctx ctx;
	struct sockaddr_in comm_addr;
	socklen_t comm_addr_len;
	int comm_socket;
	int exit_flag;
	int rc;

	if(argc != 3)
	{
		printf("Usage: %s <master addr> <master port>\n", argv[0]);

		fatal_error("Invalid argument");
	}

	ctx.self_addr = NULL;
	ctx.next_addr = NULL;
	ctx.node_list = list_new();

	bzero(&ctx.master_addr, sizeof(struct sockaddr_in));

	ctx.master_addr.sin_family = AF_INET;
	ctx.master_addr.sin_port = htons(atoi(argv[2]));

	ctx.master_addr.sin_addr
		= *((struct in_addr *)gethostbyname(argv[1])->h_addr_list[0]);

	/* TODO: Call fallocate() if file doesn't already exist */
	buf_fd = open(BUF_PATH, O_RDWR|O_CREAT, 0644);

	if(buf_fd < 0)
	{
		fatal_error("Failed to open buffer file");
	}

	comm_socket = socket(AF_INET, SOCK_STREAM, 0);
	comm_addr_len = sizeof(comm_addr);

	if(comm_socket < 0)
	{
		fatal_error("socket() failed");
	}

	bzero(&comm_addr, comm_addr_len);

	comm_addr.sin_family = AF_INET;
	comm_addr.sin_addr.s_addr = COMM_ADDR;

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

	req_init(&ctx, comm_addr.sin_port);

	while(req_list(&ctx) != 0)
	{
		sleep(1);
		printf("LIST POLL\n");
	}

	printf("LIST DONE\n");

	exit_flag = 0;

	while(!exit_flag)
	{
		int pid;
		int rc;

		wait_for_token(comm_socket, &comm_addr, &comm_addr_len);

		printf("Collecting data...\n");

		pid = fork();

		printf("PID: %d\n", pid);

		if(pid == -1)
		{
			fatal_error("fork() failed");
		}
		else if(pid == 0)
		{
			rc = execlp(	"java",
					"java",
					"-jar",
					"collector.jar",
					BUF_PATH,
					"5000",
					NULL );

			if(rc == -1)
			{
				fatal_error("exec() failed");
			}
			printf("FOO\n");

			exit(1);
		}

		wait(&rc);

		if(rc != 0)
		{
			fatal_error("Collector process exited abnormally");
		}

		pass_token(&ctx);
		printf("Remove token file\n");
		unlink(TOKEN_PATH);
	}

	close(comm_socket);
	close(buf_fd);

	return 0;
}
