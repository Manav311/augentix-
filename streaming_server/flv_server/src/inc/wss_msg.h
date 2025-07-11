#ifndef WSS_MSG_H_
#define WSS_MSG_H_

#include <stdlib.h>
#include <string.h>
#include "audio.h"

typedef enum { S16LE, ALAW, MULAW } MimeType;

typedef struct {
	unsigned long id;
	char msg_type[32];
	MimeType type;
	char *data;
	int length;
} PcmMessage;

int WSS_parseWssPcmMessage(char *fd, PcmMessage *msg);

#endif