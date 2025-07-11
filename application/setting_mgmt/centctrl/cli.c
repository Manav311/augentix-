/* a client in the unix domain */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <ctype.h>

#include "json.h"

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_cmd.h"

#include "cc_common.h"
#include "cc_data.h"

#define JSON_BLK_BUF_SIZE    128


typedef struct {
	AGTX_INT32      master_id;
	AGTX_UINT32     cmd_id;
	AGTX_CMD_TYPE_E cmd_type;
	AGTX_INT32      rval;
} CC_COMMON_MSG_INFO_S;


static int g_master_id = 0;


static void error(const char *msg)
{
	fprintf(stderr, msg);
	exit(0);
}

static void help()
{
	printf("Usage:\n");
	printf("\tccclient [OPTION]\n");
	printf("\tccclient -c CMD [OPTION]\n");
	printf("Options:\n");
	printf("\t-l\t\tlist available commands.\n");
	printf("\t-c CMD\tif CMD is not specified, enter interactive mode.\n");
	printf("\t-f SKTFD\tspecify socket file path (default: /tmp/ccUnxSkt).\n");
	printf("\t-g\t\tget command settings\n");
	printf("\t-s STR or @FILENAME\tset command settings. if '@' presented, \
	       followed by file name.\n");
	printf("\t-h\t\thelp message\n");
	printf("\n\n");

	return;
}

static char* read_json_from_file(const char *file_path)
{
	int ret = 0;
	char *ptr = NULL;
	long fsize = 0;
	FILE *fp = NULL;

	fp = fopen(file_path, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file %s\n", file_path);
		goto end;
	}

	ret = fseek(fp, 0L, SEEK_END);
	if (ret) {
		fprintf(stderr, "Failed to seek file %s\n", file_path);
		goto free_fp;
	}

	fsize = ftell(fp);
	if (fsize < 0) {
		fprintf(stderr, "Failed to get file size %s\n", file_path);
		goto free_fp;
	}

	ret = fseek(fp, 0L, SEEK_SET);
	if (ret) {
		fprintf(stderr, "Failed to rewind file pointer for %s\n", file_path);
		goto free_fp;
	}

	ptr = (char *)malloc(fsize + 1);
	if (!ptr) {
		fprintf(stderr, "Failed to alloc memory for reading file\n");
		goto free_fp;
	}

	fread(ptr, fsize, 1, fp);
	ptr[fsize] = '\0';

	if (ferror(fp)) {
		fprintf(stderr, "Failed to read file %s\n", file_path);
		free(ptr);
		ptr = NULL;
	}

free_fp:
	fclose(fp);

end:
	return ptr;
}

static int read_input_message(char **buf, FILE* fp)
{
	char *str;
	int ch, len = 0;
	size_t size = JSON_BLK_BUF_SIZE;

	str = malloc(size);
	if (!str) {
		*buf = NULL;
		return 0;
	}

	while ((EOF != (ch = fgetc(fp))) && (ch != '\n')) {
		str[len++]=ch;

		if ((unsigned)len == size) {
			size += JSON_BLK_BUF_SIZE;

			str = realloc(str, size);
			if (!str) {
				*buf = NULL;
				return 0;
			}
		}
	}

	str[len++]='\0';
	str = realloc(str, (size_t)len);
	*buf = str;

	return len;
}

static int send_all(int s, const  char *buf, int len)
{
	int total = 0;
	int bytesleft = len;
	int n;

	while (total < len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			printf("Only %d bytes are sent\n", total);
			break;
		}

		total += n;
		bytesleft -= n;
	}

	return (n == -1) ? -1 : 0;
}

#if 0
static int recv_all(int s, char *buf, int len)
{
	int total = 0;
	int byteleft = len;
	int n;

	while (total < len) {
		n = recv(s, buf + total, bytesleft, 0);
		if (n == 0) {
			fprintf(stderr, "Only %d bytes are read since socket has been closed\n", total);
			break;
		}

		total += n;
		byteleft -= n;
	}

	return (n == 0) ? -1 : total;
}
#endif

static struct json_object* validate_json_string(char *buf, int len)
{
	struct json_object  *obj = NULL;
	struct json_tokener *tok = json_tokener_new();
	enum   json_tokener_error jerr;

	/* Parse the buf */
	obj = json_tokener_parse_ex(tok, buf, len);

	/* Check parsing result */
	jerr = json_tokener_get_error(tok);
	if (jerr != json_tokener_success) {
		printf("JSON Tokener errNo: %d \n", jerr);
		printf("JSON Parsing errorer %s \n", json_tokener_error_desc(jerr));
	}

	json_tokener_free(tok);

	return obj;
}

static void parse_common_msg_info(CC_COMMON_MSG_INFO_S *info, struct json_object *obj)
{
	struct json_object *tmp_obj = NULL;
	const char  *str = NULL;

	if (json_object_object_get_ex(obj, CC_JSON_KEY_MASTER_ID, &tmp_obj)) {
		info->master_id = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(obj, CC_JSON_KEY_CMD_ID, &tmp_obj)) {
		info->cmd_id = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(obj, CC_JSON_KEY_CMD_TYPE, &tmp_obj)) {
		str = json_object_get_string(tmp_obj);

		if (str) {
			if (!strcmp(str, "notify")) {
				info->cmd_type = AGTX_CMD_TYPE_NOTIFY;
			} else if (!strcmp(str, "ctrl")) {
				info->cmd_type = AGTX_CMD_TYPE_CTRL;
			} else if (!strcmp(str, "set")) {
				info->cmd_type = AGTX_CMD_TYPE_SET;
			} else if (!strcmp(str, "get")) {
				info->cmd_type = AGTX_CMD_TYPE_GET;
			} else if (!strcmp(str, "reply")) {
				info->cmd_type = AGTX_CMD_TYPE_REPLY;
			} else {
				printf("Invalid cmd_type (%s)\n", str);
			}
		}
	}

	if (json_object_object_get_ex(obj, CC_JSON_KEY_RET_VAL, &tmp_obj)) {
		info->rval = json_object_get_int(tmp_obj);
	}

	return;
}

int register_to_ccserver(int sockfd)
{
	int len;
	struct json_object *obj = NULL;
	char msg_buf[128] = { 0 };

	CC_COMMON_MSG_INFO_S msg_info;

	sprintf(msg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"CGI\" }",
		AGTX_CMD_REG_CLIENT);

	len = strlen(msg_buf);

	/* Send message to ccserver for registration */
	if (send_all(sockfd, msg_buf, len)) {
		error("Write socket error\n");
	}

	/* Receive reply of registration from ccserver */
	len = recv(sockfd, msg_buf, 128, 0);
	msg_buf[len] = '\0';

	obj = validate_json_string(msg_buf, len);
	if (!obj) {
		error("Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);

	json_object_put(obj);

	if (msg_info.cmd_id != AGTX_CMD_REG_CLIENT || msg_info.rval != 0) {
		error("Failed to register to ccserver\n");
	}

	//printf("Succeed to register to ccserver\n");

	return 0;
}

int read_cmd_from_database(int sockfd, int cmd_id, const char *str)
{
	int len;
	struct json_object *obj = NULL;
	char *msg_buf = malloc((size_t)CC_JSON_STR_BUF_SIZE);
	char error_buf[128] = { 0 };

	CC_COMMON_MSG_INFO_S msg_info;

	sprintf(msg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"get\" }", cmd_id);

	len = strlen(msg_buf);

	/* Send message to ccserver for registration */
	if (send_all(sockfd, msg_buf, len)) {
		error("Write socket error\n");
	}

	/* Receive reply of registration from ccserver */
	len = recv(sockfd, msg_buf, CC_JSON_STR_BUF_SIZE, 0);
	msg_buf[len] = '\0';

	obj = validate_json_string(msg_buf, len);
	if (!obj) {
		error("Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);
	if (msg_info.rval == CC_DEPRECATED) {
		sprintf(error_buf, "%s is not supported\n", str);
		error(error_buf);
	} else if (msg_info.rval != 0) {
		error("Failed to get setting from database\n");
	}

	/* delete following information */
	json_object_object_del(obj, CC_JSON_KEY_MASTER_ID);
	json_object_object_del(obj, CC_JSON_KEY_CMD_TYPE);
	json_object_object_del(obj, CC_JSON_KEY_RET_VAL);
	json_object_object_add(obj, CC_JSON_KEY_CMD_ID, json_object_new_string(str));

	/* Print JSON */
	printf("%s", json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));

	//printf("Succeed to register to ccserver\n");
	json_object_put(obj);
	free(msg_buf);

	return 0;
}

int write_cmd_to_database(int sockfd, int cmd_id, char *buf)
{
	char *str = NULL;
	struct json_object *obj = NULL;
	int len = 0;

	if (!buf) {
		error("Invalid JSON argument\n");
	}

	if (buf[0] == '@') {
		str = read_json_from_file(&buf[1]);
		if (!str) {
			error("Invalid json file path\n");
		}

		len = strlen(str);
		obj = validate_json_string(str, len);
		if (!obj) {
			free(str);
			error("Not a valid json string\n");
		}

		free(str);
	} else {
		len = strlen(buf);
		obj = validate_json_string(buf, len);
		if (!obj) {
			error("Not a valid json string\n");
		}
	}

	json_object_object_add(obj, CC_JSON_KEY_MASTER_ID, json_object_new_int(g_master_id));
	json_object_object_add(obj, CC_JSON_KEY_CMD_ID, json_object_new_int(cmd_id));
	json_object_object_add(obj, CC_JSON_KEY_CMD_TYPE, json_object_new_string("set"));

	const char *cmd_str = json_object_to_json_string(obj);
	len = strlen(cmd_str);

	/* Send message to ccserver to start session */
	if (send_all(sockfd, cmd_str, len)) {
		error("Write socket error\n");
	}

	json_object_put(obj);

	/* Receive session (master) ID from ccserver */
	char *msg_buf = malloc((size_t)CC_JSON_STR_BUF_SIZE);
	CC_COMMON_MSG_INFO_S msg_info;

	len = recv(sockfd, msg_buf, CC_JSON_STR_BUF_SIZE, 0);
	msg_buf[len] = '\0';
	obj = validate_json_string(msg_buf, len);

	if (!obj) {
		error("Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);

	json_object_put(obj);

	if (msg_info.rval != 0) {
		error("Failed to start session with ccserver\n");
	} else {
		printf("Set success !\n");
	}

	free(msg_buf);
	return 0;
}

int start_session_with_ccserver(int sockfd)
{
	int len;
	struct json_object *obj = NULL;
	char msg_buf[128] = { 0 };

	CC_COMMON_MSG_INFO_S msg_info;

	sprintf(msg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\" }",
		AGTX_CMD_SESS_START);

	len = strlen(msg_buf);

	/* Send message to ccserver to start session */
	if (send_all(sockfd, msg_buf, len)) {
		error("Write socket error\n");
	}

	/* Receive session (master) ID from ccserver */
	len = recv(sockfd, msg_buf, 128, 0);
	msg_buf[len] = '\0';

	obj = validate_json_string(msg_buf, len);
	if (!obj) {
		error("Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);

	json_object_put(obj);

	if (msg_info.cmd_id != AGTX_CMD_SESS_START || msg_info.rval != 0) {
		error("Failed to start session with ccserver\n");
	}

	g_master_id = msg_info.master_id;

	//printf("Start session with master_id: %d\n", g_master_id);

	return 0;
}

int main(int argc, char *argv[])
{
	int sockfd, len;
	int servlen;
	struct sockaddr_un  serv_addr;
	char *send_buf = NULL;
	const char *socket_path = "/tmp/ccUnxSkt";
	char *cmd_str = NULL;
	char *json_str = NULL;
	int cmd_id = 0;
	int mode = 0; /* 0: read, 1: write, 2: list 3: old */
	int c;

	while ((c = getopt(argc, argv, "f:c:gs:hl")) != -1) {
		switch (c) {
			case 'c':
				cmd_str = optarg;
				break;
			case 'f':
				socket_path = optarg;
				break;
			case 'g':
				mode = 0;
				break;
			case 's':
				mode = 1;
				json_str = optarg;
				break;
			case 'h':
				help();
				return 0;
			case '?':
					if (optopt == 'c')
						fprintf(stderr, "Option -%c requires an argument.\n", optopt);
					else if (isprint(optopt))
						fprintf(stderr, "Unkown option '-%c'.\n", optopt);
					else 
						fprintf(stderr, "Unkown option character '\\x%x'.\n", optopt);
				return 1;
			case 'l':
				list_cmd_table();
				return 0;
			default:
				abort();
		}
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, socket_path);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		error("Creating socket");
	}

	/* Connect to socket file created by ccserver */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		error("Connecting");
	}

	register_to_ccserver(sockfd);
	start_session_with_ccserver(sockfd);

	if (!cmd_str) {
		char *recv_buf = malloc((size_t)CC_JSON_STR_BUF_SIZE);
		printf("Enter interactive mode.\n");

		while (1) {
			printf("Please input a JSON string: ");

			len = read_input_message(&send_buf, stdin);
			if (len <= 0) {
				error("No memory for reading data from stdin\n");
			}

			if (strstr(send_buf, "quit")) {
				break;
			}

			/* send message to ccserver */
			send_all(sockfd, send_buf, len);
			free(send_buf);

			len = recv(sockfd, recv_buf, CC_JSON_STR_BUF_SIZE, 0);
			recv_buf[len] = '\0';

			/* print to stdout */
			printf("The return message was %d: \n", len);
			write(1, recv_buf, len);
			printf("\n");
		}

		free(recv_buf);
	} else if (mode == 1) {
		cmd_id = get_cmd_id(cmd_str);

		if (!cmd_id) {
			error("Invalid command id\n");
		}

		write_cmd_to_database(sockfd, cmd_id, json_str);
	} else {
		cmd_id = get_cmd_id(cmd_str);

		if (!cmd_id) {
			error("Invalid command id\n");
		}

		read_cmd_from_database(sockfd, cmd_id, cmd_str);
	}


	close(sockfd);

	return 0;
}

