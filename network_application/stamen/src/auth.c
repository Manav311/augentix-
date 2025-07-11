#include "auth.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define MAX_LENGTH 32

int set_auth(const char *uname, const char *passwd)
{
	int n1 = strlen(uname), n2 = strlen(passwd);
	if (n1 < 1 || n1 > MAX_LENGTH || n2 < 1 || n2 > MAX_LENGTH) {
		// Too long
		return -1;
	}
	for (int i = 0; i < n1; i++) {
		// Check username only allow alphanumeric characters and underline
		if (!isalnum(uname[i]) && uname[i] != 0x5F) {
			return -1;
		}
	}
	for (int i = 0; i < n2; i++) {
		// Check password allow any printable characters except single-quote mark 0x27
		if (passwd[i] < 0x20 || passwd[i] > 0x7E || passwd[i] == 0x27) {
			return -1;
		}
	}

	char cmd[128] = { 0 };
	sprintf(cmd, "echo \"%s:$(openssl passwd -apr1 '%s')\" > /etc/nginx/.htpasswd", uname, passwd);
	int err = system(cmd);
	if (err != 0) {
		syslog(LOG_ERR, "Cannot set WebUI authentication.");
	}

	return err;
}
