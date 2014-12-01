#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_SIZE (4096)
#define READ_COUNT (100)

int main(int argc, char** argv)
{
	char *buf;
	int infd;
	int outfd;
	int i;

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <output file>\n", argv[0]);

		exit(EXIT_FAILURE);
	}

	buf = malloc(sizeof(char)*BUF_SIZE*READ_COUNT);
	infd = open("/dev/urandom", O_RDONLY);
	outfd = open(argv[1], O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

	for(i = 0; i < READ_COUNT; i++)
	{
		if(read(infd, buf, BUF_SIZE) != BUF_SIZE)
		{
			fprintf(stderr, "read() failed\n");

			exit(EXIT_FAILURE);
		}

		if(write(outfd, buf, BUF_SIZE) != BUF_SIZE)
		{
			fprintf(stderr, "write() failed\n");

			exit(EXIT_FAILURE);
		}
	}

	close(infd);
	close(outfd);

	return EXIT_SUCCESS;
}
