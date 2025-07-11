#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <fcgi_stdio.h>

#define JSON_BUF_STR_SIZE 16384
#define TZ_FILE_PATH "/etc/TZ"
#define Machine_Mode "/usrdata/mode"

void error(const char *);

int isHex(int x);

int decode(char *s, char *dec);

int readPostData(int len, char *dec);

char *strrpc(char *str, char *oldstr, char *newstr);

#endif /* _UTILS_H_ */
