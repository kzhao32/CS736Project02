#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void fatal_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);

	exit(1);
}
