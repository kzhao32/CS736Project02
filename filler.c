#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "datasource.h"

#define MAGICNO 0xca11ab1e

#define TOKEN_PATH ".token"

#define COMM_ADDR INADDR_ANY
#define COMM_PORT 2345

#define SQL_PATH "db.sqlite"
#define SQL_MAXLEN 256
#define SQL_CREATE_TABLE "create table tweets (timestamp integer, content text)"

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

	if(new_comm_socket < 0)
	{
		fatal_error("socket() failed");
	}

	rc = read(new_comm_socket, &buf, sizeof(buf));

	close(new_comm_socket);

	if(rc != sizeof(buf))
	{
		fatal_error("read() failed");
	}

	if(buf != MAGICNO)
	{
		fatal_error("Magic number mismatch");
	}

	rc = open(TOKEN_PATH, O_RDONLY|O_CREAT);

	if(rc < 0)
	{
		fatal_error("Failed to create token file");
	}

	close(rc);
}

void pass_token()
{
	/* TODO: Pass token to next node */
}

int main()
{
	sqlite3* db;
	struct sockaddr_in comm_addr;
	socklen_t comm_addr_len;
	int comm_socket;
	int exit_flag;
	int rc;

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
	comm_addr.sin_port = htons(COMM_PORT);
	comm_addr.sin_addr.s_addr = COMM_ADDR;

	rc = bind(comm_socket, (struct sockaddr *)&comm_addr, comm_addr_len);

	if(rc != 0)
	{
		fatal_error("bind() failed");
	}

	listen(comm_socket, 1);

	req_init();

	exit_flag = 0;

	while(!exit_flag)
	{
		wait_for_token(comm_socket, &comm_addr, &comm_addr_len);

		/* TODO: Collect tweets and insert into database */

		pass_token();
	}

	close(comm_socket);
	sqlite3_close(db);

	return 0;
}
