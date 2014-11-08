#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sqlite3.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAGICNO 0xca11ab1e

#define TOKEN_PATH "token"

#define COMM_ADDR INADDR_ANY
#define COMM_PORT 2345

#define SQL_PATH "db.sqlite"
#define SQL_MAXLEN 256

#define SQL_CREATE_TABLE	"create table if not exists tweets" \
				"(timestamp integer, content text)"

/* TODO: Use req_init() to obtain list of nodes */
const char* node_list[] = {	"localhost", "12345",
				"localhost", "12344",
				"localhost", "12343",
				NULL };

void fatal_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);

	exit(1);
}

void req_init()
{
	/* TODO: Ask master for configuration info */
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
	printf("pREAD\n");

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

	/* TODO: Pass token to next node */
}

int main(int argc, char **argv)
{
	sqlite3* db;
	struct sockaddr_in comm_addr;
	socklen_t comm_addr_len;
	int comm_socket;
	int exit_flag;
	int rc;

	if(argc != 2)
	{
		fatal_error("Invalid argument");
	}

	rc = sqlite3_open(SQL_PATH, &db);

	if(rc != SQLITE_OK)
	{
		fatal_error("Failed to open database");
	}

	rc = sqlite3_exec(db, SQL_CREATE_TABLE, NULL, NULL, NULL);

	if(rc != SQLITE_OK)
	{
		fatal_error("Failed to create table");
	}

	comm_socket = socket(AF_INET, SOCK_STREAM, 0);
	comm_addr_len = sizeof(comm_addr);

	if(comm_socket < 0)
	{
		fatal_error("socket() failed");
	}

	bzero(&comm_addr, comm_addr_len);

	comm_addr.sin_family = AF_INET;
	comm_addr.sin_port = htons(atoi(node_list[2*atoi(argv[1])+1]));
	comm_addr.sin_addr.s_addr = COMM_ADDR;

	rc = bind(comm_socket, (struct sockaddr *)&comm_addr, comm_addr_len);

	if(rc != 0)
	{
		fatal_error("bind() failed");
	}

	listen(comm_socket, 1);

	req_init();

	printf("Listening on port: %s\n", node_list[2*atoi(argv[1])+1]);

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
	sqlite3_close(db);

	return 0;
}
