#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"

#define MAGICNO 0xca11ab1e
#define TOKEN_PATH "token"
#define COMM_ADDR INADDR_ANY
#define COMM_PORT 2345
#define BUF_PATH "cbuf"
#define BUF_RSIZE 256

/* TODO: Use req_init() to obtain list of nodes */
const char* node_list[] = {	"localhost", "12345",
				"localhost", "12344",
				"localhost", "12343",
				NULL };

void req_init(struct sockaddr_in *addr, short self_port)
{
	int fd;
	int rc;

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd < 0)
	{
		fatal_error("Failed to create socket");
	}

	rc = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

	if(rc < 0)
	{
		fatal_error("connect() failed");
	}

	write(fd, &self_port, sizeof(short));

	/* TODO: Read master's reply */
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

	/* TODO: Re-enable MAGICNO check */
	if(buf != MAGICNO && 0)
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

void pass_token(int port)
{
	struct sockaddr_in next_addr;
	socklen_t next_addr_len;
	int next_socket;
	int buf;
	int rc;

	next_socket = socket(AF_INET, SOCK_STREAM, 0);
	next_addr_len = sizeof(next_addr);

	bzero(&next_addr, next_addr_len);

	printf("PASS: %d\n", port);
	inet_pton(AF_INET, "127.0.0.1", &next_addr.sin_addr);

	next_addr.sin_port = htons(port);
	next_addr.sin_family = AF_INET;

	rc = connect(next_socket, (struct sockaddr *)&next_addr, next_addr_len);

	if(rc < 0)
	{
		perror(NULL);
		fatal_error("Can't connect to the next node");
	}

	buf = MAGICNO;
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
	struct sockaddr_in comm_addr;
	struct sockaddr_in master_addr;
	socklen_t comm_addr_len;
	int comm_socket;
	int exit_flag;
	int rc;

	if(argc != 3)
	{
		fatal_error("Invalid argument");
	}

	bzero(&master_addr, sizeof(struct sockaddr_in));
	inet_pton(AF_INET, argv[1], &(master_addr.sin_addr));

	master_addr.sin_port = atoi(argv[2]);

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
	/* comm_addr.sin_port = htons(atoi(node_list[2*atoi(argv[1])+1])); */
	comm_addr.sin_addr.s_addr = COMM_ADDR;

	rc = bind(comm_socket, (struct sockaddr *)&comm_addr, comm_addr_len);

	if(rc != 0)
	{
		fatal_error("bind() failed");
	}

	listen(comm_socket, 1);

	req_init(&master_addr, comm_addr.sin_port);

	{
		char *buf = malloc(comm_addr_len);

		getsockname(comm_socket, (struct sockaddr *)&comm_addr, &comm_addr_len);

		printf(	"Listening on %s:%d\n",
			inet_ntop(AF_INET, &comm_addr.sin_addr, buf, comm_addr_len),
			ntohs(comm_addr.sin_port) );

		free(buf);
	}

	exit_flag = 0;

	while(!exit_flag)
	{
		wait_for_token(comm_socket, &comm_addr, &comm_addr_len);

		/* TODO: Collect tweets and insert into database */
		sleep(10);

		pass_token(atoi(node_list[2*(atoi(argv[1])+1)+1]));
		printf("Remove token file\n");
		unlink(TOKEN_PATH);
	}

	close(comm_socket);
	close(buf_fd);

	return 0;
}
