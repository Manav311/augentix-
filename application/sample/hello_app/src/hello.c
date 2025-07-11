#include <stdio.h>
#include <stdlib.h>

#include <foo.h>

#include "help.h"

int main(int argc, char **argv)
{
	const char *basename = argv[0];
	int ret = EXIT_FAILURE;

	if (argc < 2) {
		help(basename);
		return EXIT_FAILURE;
	}

	ret = foo();
	if (ret != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to invoke the `foo()` function.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
