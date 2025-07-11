#include "foo.h"

#include <stdio.h>
#include <stdlib.h>

#include "sum.h"

int foo(void)
{
	int a = 5;
	int b = 3;
	int c = 0;

	puts("Hello from libfoo.");

	c = sum(a, b);
	printf("The result of sum(%d, %d) is %d.\n", a, b, c);

	return EXIT_SUCCESS;
}
