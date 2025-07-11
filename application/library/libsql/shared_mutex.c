#include "shared_mutex.h"

shared_mutex_t shared_mutex_init(char *name __attribute__((unused)))
{
	shared_mutex_t mutex = { NULL, 0, NULL, 0 };

	return mutex;
}

int shared_mutex_close(shared_mutex_t mutex __attribute__((unused)))
{
	return 0;
}

int shared_mutex_destroy(shared_mutex_t mutex __attribute__((unused)))
{
	return 0;
}
