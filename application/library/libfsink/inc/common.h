#ifndef COMMON_H
#define COMMON_H

/* Stream server daemon print function*/
#ifdef DEBUG
#define SSD_PRINTF(...)              \
	do {                         \
		printf(__VA_ARGS__); \
	} while (0);
#else
#define SSD_PRINTF(...)
#endif

#endif
