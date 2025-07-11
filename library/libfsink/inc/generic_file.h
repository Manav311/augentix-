#ifndef GENERIC_FILE_H
#define GENERIC_FILE_H

#include <stdio.h>
#include "fsink.h"

/** @brief Struct that contains File Sink information.
 */
typedef struct FileAttrType {
	char *fname;
	int flags;
	FILE *fptr;
} FileAttr;

Sink *createFileSink(const char *const sink_name, int flags);

void releaseFileSink(Sink *file_sink);

#endif /* UDP_SOCKET_H */
