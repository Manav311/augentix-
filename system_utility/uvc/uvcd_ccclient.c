#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "agtx_cmd.h"
#include "uvcd_common.h"
#include "uvcd_ccclient.h"

#define CC_SOCKET_PATH "/tmp/ccUnxSkt"
#define UVCD_REGISTER_CC_NAME "CGI"

int master_id = 0;
int sfd = 0;
/*
 * Effects: read till JSON_EOF_CHAR
 */
static int readCcSocket(int sockfd, char *buf, int buf_size)
{
	int ret, ret_len;

	ret_len = 0;

	while(1) {
		ret = read(sockfd, buf+ret_len, buf_size-ret_len);
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		} else if (0 == ret) {
			break;
		}

		ret_len += ret;
		break;
		/**************TBD****************/
		/*  EOF of JSON are not defined  */
		/**************TBD****************/
#if 0
		if (buf[ret_len-1] == JSON_EOF_CHAR) {
			break;
		} else {
			printf(">===EOF==%d==< = %c\n", ret_len, buf[ret_len-1]);
			error("Not yet done");
			continue;
		}
#endif
	}
	debug("%s:\n %s\n", __func__, buf);
	return ret_len;
}

/*
 * Effects: write len bytes from buf into sockfd for sure
 */
static int writeCcSocket(int sockfd, char *buf, int len)
{
	int ret = 0;

	debug("%s:\n %s\n", __func__, buf);
	while(len != 0) {
		ret = write(sockfd, buf, len);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			error("writeCcSocket");
			break;
		}
		len -= ret;
		buf += ret;
	}
	return ret;
}

/*
 * Requires: sockfd is a Unix socket
 *
 * Effects:  make connection to cc and returns 0 if successful, -1 otherwise
 */
static int connectCC(int sockfd)
{
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, CC_SOCKET_PATH, sizeof(addr.sun_path) - 1);

	return connect(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un));
}

/*
 * Requires: connectCC has been invoked successfully beforehand
 *
 * Effects:  register connection with cc, returns 0 if successful and -1 if failure
 */
static int registerCC(int sockfd)
{
	int ret;
	char *buf;
	char jstr2cc[JSON_STR_LEN] = {0};
	char ret_cmd[JSON_STR_LEN] = {0};

	ret = 0;

	sprintf(jstr2cc, "{\"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"%s\"}", AGTX_CMD_REG_CLIENT, UVCD_REGISTER_CC_NAME);

	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret == -1) {
		error("Failed to send register information to CC.\n");
		return ret;
	}

	ret = readCcSocket(sockfd, ret_cmd, sizeof(ret_cmd));
	if (ret == -1) {
		error("Failed to read socket.\n");
		return ret;
	}

	buf = strstr(ret_cmd, "\"rval\"");
	sscanf(buf, "\"rval\": %d", &ret);
	return ret;
}

/*
 * Requires: connectCC and registerCC have been invoked successfully beforehand
 *
 * Effects:  returns master_id (>= 11) from cc or -1 upon failure
 *
 */
static int startSessionWithCC(int sockfd)
{
	int ret;
	char *buf;
	char jstr2cc[JSON_STR_LEN] = {0};
	char ret_cmd[JSON_STR_LEN] = {0};

	sprintf(jstr2cc, "{\"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\"}", AGTX_CMD_SESS_START);
	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret == -1) {
		error("Failed to write CC socket.\n");
		return ret;
	}

	ret = readCcSocket(sockfd, ret_cmd, sizeof(ret_cmd));
	if (ret == -1) {
		error("Failed to read socket.\n");
		return ret;
	}

	buf = strstr(ret_cmd, "\"rval\"");
	sscanf(buf, "\"rval\": %d", &ret);
	if (ret == 0) {
		buf = strstr(ret_cmd, "\"master_id\"");
		sscanf(buf, "\"master_id\": %d", &ret);
	}

	return ret;
}

/*
 * Requires: connectCC, registerCC, and startSessionWithCC have been invoked
 *           successfully beforehand
 *
 * Effects:  send JSON string jstr to sockfd, returns rval from cc
 *
 */
static int setCC(int sockfd, int master_id, char *jstr)
{
	int ret;
	char *buf;
	char jstr2cc[JSON_STR_LEN] = {0};
	char ret_cmd[JSON_STR_LEN] = {0};

	sprintf(jstr2cc, "{%s, \"master_id\":%d, \"cmd_type\":\"set\"}", jstr, master_id);
	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret == -1) {
		error("Failed to write CC socket.");
		return ret;
	}

	ret = readCcSocket(sockfd, ret_cmd, sizeof(ret_cmd));
	if (ret == -1) {
		error("Failed to read socket.");
		return ret;
	}

	buf = strstr(ret_cmd, "\"rval\"");
	sscanf(buf, "\"rval\": %d", &ret);
	return ret;
}

static int getCC(int sockfd, int master_id, char *jstr)
{
	int ret;
	char *buf;
	char jstr2cc[JSON_STR_LEN] = {0};

	sprintf(jstr2cc, "{%s, \"master_id\":%d, \"cmd_type\":\"get\"}", jstr, master_id);
	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret == -1) {
		error("Failed to write CC socket.");
		return ret;
	}

	ret = readCcSocket(sockfd, jstr, sizeof(char) * JSON_STR_LEN);
	if (ret == -1) {
		error("Failed to read socket.");
		return ret;
	}

	buf = strstr(jstr, "\"rval\"");
	sscanf(buf, "\"rval\": %d", &ret);
	return ret;
}

int openCC(void)
{
	int ret;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sfd < 0) {
		error("unix socket");
	}

	ret = connectCC(sfd);
	if (ret < 0) {
		error("connectCC");
		return ret;
	}

	ret = registerCC(sfd);
	if (ret != 0) {
		error("registerCC");
		close(sfd);
		return ret;
	}

	master_id = startSessionWithCC(sfd);
	if (master_id <= 0) {
		error("startSessionWithCC");
		return master_id;
	}

	return 0;
}

void closeCC(void)
{
	close(sfd);
}

int ccSet(char *jstr)
{
	int ret = 0;

	if (master_id > 0) {
		ret = setCC(sfd, master_id, jstr);
		if (ret == 0) {
			error("cc return successfully after set value");
		} else {
			error("set cc failed!");
		}
	} else {
		error("Session not start!");
	}
	return ret;
}

int ccGet(char *jstr)
{
	int ret = 0;

	if (master_id > 0) {
		ret = getCC(sfd, master_id, jstr);
		if (ret == 0) {
			error("cc return successfully after get value");
		} else {
			error("get cc failed!");
		}
	} else {
		error("Session not start!");
	}
	return ret;
}
