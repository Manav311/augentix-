#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <fcgi_stdio.h>
#include "inc/utils.h"

void error(const char *);

int isHex(int x)
{
	return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
}

int decode(char *s, char *dec)
{
	char *o;
	char *end;
	unsigned int c;

	int len;
	len = strlen(s);

	if (len > 2047) {
		len = 2047;
	}

	end = s + len;

	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '%' && (!isHex(*s++) || !isHex(*s++) || !sscanf(s - 2, "%2x", &c))) {
			return -1;
		}
		if (dec) {
			*o = c;
		}
	}

	return o - dec;
}

int readPostData(int len, char *dec)
{
	int ch = 0;
	for (int i = 0; i < len; i++) {
		if ((ch = getchar()) < 0) {
			return i;
		}
		dec[i] = (char)ch;
	}
	dec[len] = '\0';
	return len;
}

char *strrpc(char *str, char *oldstr, char *newstr)
{
	char bstr[strlen(str)];
	memset(bstr, 0, sizeof(bstr));
	int len;
	len = strlen(str);
	for (int i = 0; i < len; i++) {
		if (!strncmp(str + i, oldstr, strlen(oldstr))) {
			strcat(bstr, newstr);
			i += strlen(oldstr) - 1;
		} else {
			strncat(bstr, str + i, 1);
		}
	}

	strcpy(str, bstr);
	return str;
}
