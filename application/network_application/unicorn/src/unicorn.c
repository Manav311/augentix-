#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <net/if.h>
#include <libgen.h>

#include "chip_id.h"
#include "action.h"
#include "ccclient.h"
#include "utils.h"
#include "frame.h"
#include "stream.h"
#include "agtx_types.h"

#define MAX_NUM_CLIENTS 64
#define BUFLEN 16384 * 2 // 2048
#define PORT "6666" // double quotes for getaddrinfo()
#define CODE "AgtxCrossPlatCommn"
#define CODE_LEN sizeof(CODE)

/* general fixed point number division with rounding
 * precision of the result is:
 * dividend + p - divisor
 * if the dividend and the divisor share the same precision,
 * then the precision of the result is p
 */
#define UNSIGN_FIXED_DIVID(dividend, divisor, precision) ((((dividend) << (precision)) + ((divisor) >> 1)) / (divisor))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum status { disconnected, connected, sendF, receiveFile };

struct clientSession {
	int clientfd;
	bool is_streaming;
	pid_t pid_stream;
	enum status state;
};

typedef enum action {
	ACT_stream = 0,
	ACT_stop = 1,
	ACT_cmdsender = 2,
	ACT_snapshot = 3,
	ACT_sendFile = 4,
	ACT_recvFile = 5,
	ACT_getCC = 6,
	ACT_setCC = 7,
	ACT_burnin = 8,
	ACT_connectcc = 9,
	ACT_MpiSys = 10,
	ACT_getFilePath = 11,
	ACT_monitor = 12,
	ACT_cat = 13,
	ACT_cmd = 14,
	ACT_header = 15,
	ACT_folderCtrl = 16,
	ACT_executable_mode = 17,
	ACT_collectData = 18,
} action;

typedef enum {
	CMD_INVALID_rm = -2,
	CMD_NO_MATCH = -1,
	CMD_VALID_audioctrl = 1,
	CMD_VALID_light_test = 2,
	CMD_VALID_ir_cut = 3,
	CMD_VALID_led = 4,
	CMD_VALID_sh = 5,
	CMD_VALID_touch = 6,
	CMD_VALID_echo = 7,
	CMD_VALID_find = 8,
	CMD_VALID_iwconfig = 9,
	CMD_VALID_ifconfig = 10,
	CMD_VALID_killall = 11,
	CMD_VALID_reboot = 12,
	CMD_VALID_mode = 13,
	CMD_VALID_gpio_utils = 14,
	CMD_VALID_bp_utils = 15,
	CMD_VALID_sync = 16,
	CMD_VALID_chmod = 17,
	CMD_VALID_dos2unix = 18,
	CMD_VALID_mkdir = 19,
} command;

typedef enum response {
	res_Error = 0,
	res_Success = 1,
} response;

struct sockaddr_in server, client;
struct timeval timeout;
struct sockaddr_storage remoteaddr;

int sockfd = -1;
int mpi_sys_status = 0; //0: none, 1:mpi ready
socklen_t addrlen;

char g_socket_type[16] = "NONE";
char g_connect_port[5] = PORT;

pid_t forkStream(char *case_config);
int dumpImage(char *type, int *dump_size, int clientfd);
int mpiSysConnect(char *type, int clientfd);
int sendCalibFilePath(char *type, int clientfd);
int sendBurnInReg(int sockfd, char *reg);
int sendCatReg(int sockfd, char *reg);

static int isSystemCommandValid(const char *src)
{
	int ret = -1;

	/* Invaild command */
	if (strstr(src, "rm ") != NULL)
		return CMD_INVALID_rm;
	else
		ret = CMD_NO_MATCH;

	/* Valid command */
	if (strstr(src, "audioctrl") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_audioctrl;
	if (strstr(src, "light_test") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_light_test;
	if (strstr(src, "ir_cut.sh") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_ir_cut;
	if (strstr(src, "led.sh") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_led;
	if (strstr(src, "sh") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_sh;
	if (strstr(src, "touch") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_touch;
	if (strstr(src, "echo") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_echo;
	if (strstr(src, "find") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_find;
	if (strstr(src, "iwconfig") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_iwconfig;
	if (strstr(src, "ifconfig") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_ifconfig;
	if (strstr(src, "killall") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_killall;
	if (strstr(src, "reboot") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_reboot;
	if (strstr(src, "mode") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_mode;
	if (strstr(src, "gpio_utils") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_gpio_utils;
	if (strstr(src, "bp_utils") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_bp_utils;
	if (strstr(src, "sync") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_sync;
	if (strstr(src, "chmod") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_chmod;
	if (strstr(src, "dos2unix") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_dos2unix;
	if (strstr(src, "mkdir") == NULL)
		ret = CMD_NO_MATCH;
	else
		return CMD_VALID_mkdir;

	return ret;
}

static void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

static int getClientIdx(int clientfd, struct clientSession clients[], int num_clients)
{
	for (int i = 0; i < num_clients; ++i) {
		if (clients[i].clientfd == clientfd)
			return i;
	}
	return ACT_FAILURE;
}

static char *interpretData(const char *buf, int clientFd, fd_set *fdset)
{
	AGTX_UNUSED(fdset);

	char *ret = strstr(buf, CODE);
	DBG_MED("%s(): '%s'\n", __func__, buf);
	if (!ret) {
		DBG_MED("Invalid command '%s' received! Closing client fd %d\n", ret, clientFd);
		//close(clientFd);
		//FD_CLR(clientFd, fdset);
		return NULL;
	}
	return ret + CODE_LEN - 1;
}

static void showClientInfo(struct clientSession *client)
{
	DBG_MED("----- client info -----\n");
	DBG_MED("clientfd = %d\n", client->clientfd);
	DBG_MED("streaming = %d\n", client->is_streaming);
	DBG_MED("streaming pid = %d\n", client->pid_stream);
	DBG_MED("client state = %d\n", client->state);
	DBG_MED("-----------------------\n\n");
}
/* zero warning
static int parseAction(char *message)
{
	const char *action = message;
	return atoi(action);
}
*/
static void sendResponse(int sockfd, int resp)
{
	char respOut[4];
	snprintf(respOut, sizeof(respOut), "%d", resp);
	int ret = send(sockfd, respOut, sizeof(respOut), 0);
	if (!ret) {
		ERR("send response");
	}
	DBG_MED("Sent response '%s' to client %d\n", respOut, sockfd);
}

static int handleAction(char *sig, struct clientSession *client, int nbytes)
{
	if (sig == NULL) {
		DBG_MED("Invalid action: sig is NULL \n");
		return ACT_NULL;
	}

	int action = parseFirstNum(sig);
	char *params = extractSingleMessage(sig, nbytes);
	DBG_MED("%s(): action %d, nbytes %d, params %s\n", __func__, action, nbytes, params);

	int ret;
	int cmd_code;
	char jstr[JSON_STR_LEN];
	unsigned long int fSize = 0;
	char *fPath;
	char *module_name;
	char ret_value[MAX_DATA_SIZE_BYTES] = { 0 };
	char *table = NULL;
	char *dip_extend = NULL;
	char *result = NULL;
	switch (action) {
	default:
		DBG_MED("default: %d\n", *sig);
		break;
	case ACT_stream:
		DBG_MED("Start streaming\n");
		if (!client->is_streaming) {
			truncateMessage(params, DELIMITER, nbytes);
			client->pid_stream = forkStream(params);
			client->is_streaming = true;
		} else
			DBG_MED("Client on sockfd %d is already streaming!\n", client->clientfd);
		break;
	case ACT_stop:
		DBG_MED("Stop streaming\n");
		if (client->is_streaming) {
			client->is_streaming = false;
			kill(client->pid_stream + 1, SIGTERM);
		} else
			DBG_MED("Client on sockfd %d is not streaming!\n", client->clientfd);
		break;
	case ACT_cmdsender:
		truncateMessage(params, DELIMITER, nbytes);
		char cmdsender[BUFLEN] = "/system/bin/cmdsender ";
		strcat(cmdsender, params);
		ret = executeSystemCommand(cmdsender);
		break;
	case ACT_snapshot:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("Dumping image\n");
		int dump_size = 0;
		ret = dumpImage(params, &dump_size, client->clientfd);
		if (ret != ACT_SUCCESS)
			DBG_MED("ERROR dump imgae failed !!!!!\n");
		break;
	case ACT_sendFile:
		truncateMessage(params, DELIMITER, nbytes);
		ret = sendFile(client->clientfd, params);
		if (ret != ACT_SUCCESS)
			DBG_MED("Send file failed!\n");
		break;
	case ACT_recvFile:
		truncateMessage(params, DELIMITER, nbytes);

		fSize = parseFileSize(params);
		fPath = extractSingleMessage(params, strlen(params));
		DBG_MED("fSize = %lu\n", fSize);
		DBG_MED("fPath = '%s'\n", fPath);

		// send file size
		sprintf(ret_value, "%lu", fSize);
		DBG_MED("ret_value: %s\n", ret_value);
		sendData(client->clientfd, ret_value, strlen(ret_value));

		ret = recvFile(client->clientfd, fPath, fSize, true);
		sendResponse(client->clientfd, ret);

		break;
	case ACT_connectcc:
		DBG_MED("Some action detected!\n");
		ret = ccClientSet(jstr);
		break;
	case ACT_getCC:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_getCC params %s\n", params);
#if CONFIG_CCSERVER_SUPPORT
		ret = ccClientGet(params, CC_GET_REG);
		if (ret == 0) {
			DBG_MED("Return string is %s\n", params);
			ret = sendData(client->clientfd, params, strlen(params));
			if (ret < 0) {
				DBG_HIGH("writeCcSocket failed with ret = %d\n", ret);
				ret = -1;
			} else {
				DBG_HIGH("success sending cc return to PC!\n");
				ret = 0;
			}
		}

		DBG_MED("ccClientGet returns %lu\n", ret);
#else
		module_name = unicorn_json_get_string(params, "module", strlen(params));
		if (module_name == NULL) {
			ERR("Error: Unable to get module name, %s\n", params);
			ret = -1;
			break;
		}

		if (strcmp(module_name, "dip_extend") == 0) {
			fSize = getDipExtend(params, &result, &dip_extend);
		} else {
			cmd_code = get_command_id(module_name);
			DBG_MED("Get Command %d\n", cmd_code);
			if (cmd_code < 0) {
				ERR("Invalid command\n");
				ret = -1;
				if (module_name) {
					free(module_name);
				}
				break;
			}
			fSize = getMpiSetting(params, &result, cmd_code, &table);
		}
		if (module_name) {
			free(module_name);
		}
		if (result == NULL) {
			ERR("Return string is NULL.\n");
			break;
		}
		fSize = strlen(result);
		if (fSize > 0) {
			DBG_MED("Return string is %s\n", result);
			ret = sendData(client->clientfd, result, fSize);

			if (ret < 0) {
				DBG_HIGH("writeCcSocket failed with ret = %d\n", ret);
				free(result);
				ret = -1;
				break;
			}
			if (dip_extend) {
				int dip_size = unicorn_json_get_int(result, "Content", strlen(result));
				ret = sendData(client->clientfd, dip_extend, dip_size);
				free(dip_extend);
				if (ret < 0) {
					DBG_HIGH("writeCcSocket failed with ret = %d\n", ret);
					ret = -1;
					free(result);
					break;
				}
			}
			if (table) {
				int tableSize = unicorn_json_get_int(result, "current_table", strlen(result));
				ret = sendData(client->clientfd, table, tableSize);
				free(table);
				if (ret < 0) {
					DBG_HIGH("writeCcSocket failed with ret = %d\n", ret);
					ret = -1;
					free(result);
					break;
				}
			}

			DBG_HIGH("success sending cc return to PC!\n");
			ret = 0;
		}
		free(result);
#endif
		break;
	case ACT_setCC:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_setCC params %s\n", params);

#if CONFIG_CCSERVER_SUPPORT
		char *setting = unicorn_json_get_string(params, "module", strlen(params));
		if (setting == NULL) {
			ERR("Error: Unable to get module name, %s\n", params);
			ret = -1;
			break;
		}
		DBG_MED("Received JSON string |%s| from PC!\n", params);
		DBG_MED("setting = |%s|\n", setting);
		DBG_MED("Before adding (key, val) : |%s|\n", params);
		strcpy(jstr, unicorn_json_add_key_int(params, "sample", 999, strlen(params)));
		DBG_MED("After adding (key, val) : |%s|\n", jstr);
		DBG_MED("-------------");
		ret = ccClientSet(jstr);
		if (ret == 0) {
			DBG_MED("success setting to cc\n");
		} else {
			DBG_MED("fail setting to cc\n");
		}
		if (setting) {
			free(setting);
		}
#else
		module_name = unicorn_json_get_string(params, "module", strlen(params));
		if (module_name == NULL) {
			ERR("Error: Unable to get module name, %s\n", params);
			ret = -1;
			break;
		}
		if (strcmp(module_name, "dip_extend") == 0) {
			ret = setDipExtend(params, client->clientfd);
		} else {
			cmd_code = get_command_id(module_name);
			if (cmd_code < 0) {
				ERR("Invalid command\n");
				ret = -1;
				if (module_name) {
					free(module_name);
				}
				break;
			}
			DBG_MED("Received JSON string |%s| from PC!\n", params);
			DBG_MED("Set Command %d\n", cmd_code);

			ret = setMpiSetting(params, strlen(params), cmd_code, client->clientfd);
		}
		if (module_name) {
			free(module_name);
		}
		if (ret == 0) {
			DBG_MED("success setting to cc\n");
		} else {
			DBG_MED("fail setting to cc\n");
		}
#endif
		DBG_MED("-------------");
		sendResponse(client->clientfd, ret);
		break;
	case ACT_burnin:
		truncateMessage(params, DELIMITER, nbytes);
		//DBG_MED( "ACT_burnin JSON string |%s| from PC!\n", params);
		if (strcmp(params, "") == 0) {
			ret = executeSystemCommand(
			        "fw_setenv --script /system/partition"); // system return 0 as success
			sendResponse(client->clientfd, ret);
		} else if (findSpase(params) == 1) {
			sprintf(ret_value, "fw_setenv %s", params);
			ret = executeSystemCommand(ret_value); // system return 0 as success
			sendResponse(client->clientfd, ret);
		} else {
			ret = sendBurnInReg(client->clientfd, params);
			if (ret == -1) {
				sprintf(ret_value, "%d", ret);
				sendData(client->clientfd, ret_value, strlen(ret_value));
			}
		}
		break;
	case ACT_MpiSys:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_MpiSys JSON string |%s| from PC!\n", params);
		ret = mpiSysConnect(params, client->clientfd);
		sendResponse(client->clientfd, ret);
		if (ret != ACT_SUCCESS)
			DBG_MED("ERROR Mpi sys failed !!!!!\n");
		break;
	case ACT_getFilePath:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_getFilePath: |%s| \n", params);
		ret = sendCalibFilePath(params, client->clientfd);
		if (ret != ACT_SUCCESS) {
			sendResponse(client->clientfd, ret);
			DBG_MED("ERROR get dut file path failed !!!!!\n");
		}
		break;
	case ACT_monitor:
		DBG_MED("ACT_monitor !!!!!!!!!!!!!!\n");
		break;
	case ACT_cat:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_cat: |%s| \n", params);

		ret = sendCatReg(client->clientfd, params);
		if (ret == -1) {
			sprintf(ret_value, "%d", ret);
			sendData(client->clientfd, ret_value, strlen(ret_value));
		}
		break;
	case ACT_cmd:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_cmd: |%s| \n", params);
		cmd_code = isSystemCommandValid(params);

		if (cmd_code < 0) {
			ERR("Invaild command code: %d\n", cmd_code);
			ret = ACT_FAILURE;
		} else {
			DBG_MED("Command code: %d\n", cmd_code);
			ret = executeSystemCommand(params);
		}

		sendResponse(client->clientfd, ret);
		break;
	case ACT_header:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_header params %s\n", params);
		UnicornFrame *frame = NULL;
		frame = unicorn_get_valid_frame((unsigned char *)params, JSON_STR_LEN);
		if (frame) {
			unicorn_dispatch_cmd(client->clientfd, frame);
		}
		break;
	case ACT_folderCtrl:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_folderCtrl: |%s| \n", params);

		ret = folderCtrl(params);

		sendResponse(client->clientfd, ret);
		break;
	case ACT_executable_mode:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_executable_mode: |%s| \n", params);
		ret = isExecutableMode(params);
		sendResponse(client->clientfd, ret);
		break;
	case ACT_collectData:
		truncateMessage(params, DELIMITER, nbytes);
		DBG_MED("ACT_collectData: |%s| \n", params);
		char *unicornPath = realpath("/proc/self/exe", NULL);
		char *unicornPathCopy = strndup(unicornPath, strlen(unicornPath));
		char *curDir = dirname(unicornPathCopy);

		ret = collectData(curDir, params);
		if (ret == ACT_SUCCESS) {
			char filePath[256] = { 0 };
			snprintf(filePath, 256, "%s/%s.tar", curDir, params);
			ret = sendFile(client->clientfd, filePath);
			if (ret != ACT_SUCCESS) {
				DBG_MED("Send file failed\n");
				sendResponse(client->clientfd, ret);
			}
			remove(filePath);
		} else {
			DBG_MED("Collect data failed\n");
			sendResponse(client->clientfd, ret);
		}

		if (unicornPath) {
			free(unicornPath);
		}
		if (unicornPathCopy) {
			free(unicornPathCopy);
		}
		break;
	}

	return ACT_SUCCESS;
}

static void startService()
{
	fd_set master; // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int fdmax; // maximum file descriptor number

	int listener; // listening socket descriptor
	int newfd; // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	char buf[BUFLEN]; // buffer for client data
	int nbytes; // number of bytes involved during I/O
	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1; // for setsockopt() SO_REUSEADDR
	int i, j, rv, client_idx, num_clients = 0;
	struct addrinfo hints, *listener_address;
	struct clientSession clients[MAX_NUM_CLIENTS]; // list of clients (and their info)

	int err; // for handle action fail

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	// get a socket and bind it
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, g_connect_port, &hints, &listener_address)) != 0) {
		DBG_MED("before fprintf\n");
		ERR("unicorn_server: %s\n", gai_strerror(rv));
		exit(1);
	}

	struct addrinfo *p;
	for (p = listener_address; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
			continue;
		// get rid of "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		// bind socket to specific device
		if (strstr(g_socket_type, "NONE") == NULL) {
			struct ifreq interface;
			strncpy(interface.ifr_ifrn.ifrn_name, g_socket_type, sizeof(g_socket_type));
			if (setsockopt(listener, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) <
			    0) {
				perror("SO_BINDTODEVICE failed");
				exit(2);
			}
		}

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			DBG_MED("setsock close!\n");
			continue;
		} else {
			DBG_MED("setsock listening!\n");
		}
		break;
	}

	if (p == NULL) {
		DBG_MED("WTF happened!\n");
		ERR("unicorn_server: failed to bind\n");
		exit(2);
	}
	freeaddrinfo(listener_address);

	if (listen(listener, MAX_NUM_CLIENTS) == -1) {
		ERR("listen");
		exit(3);
	}
	DBG_MED("unicorn waiting for connections...\n");
	FD_SET(listener, &master);
	fdmax = listener;

	for (int k = 0; k < fdmax; k++) {
		DBG_MED("###--- [%2d] ---###\n", k);
		showClientInfo(&clients[k]);
	}

	DBG_MED("Waiting for client connection...\n");
	for (;;) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			ERR("select");
			exit(4);
		}

		DBG_MED("#--- Selected, max fd: %d ---#\n", fdmax);

		// run through existing connections looking for data to read
		for (i = 0; i <= fdmax; ++i) {
			bool ret_isset;
			ret_isset = FD_ISSET(i, &read_fds);
			if (ret_isset) { // got a match!
				if (i == listener) { // handle new connection
					addrlen = sizeof(remoteaddr);
					newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
					if (newfd == -1) {
						ERR("accept");
					} else {
						FD_SET(newfd, &master);
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						DBG_MED("New connection from socket %s %d, fdmax = %d\n",
						        inet_ntop(remoteaddr.ss_family,
						                  get_in_addr((struct sockaddr *)&remoteaddr), remoteIP,
						                  INET6_ADDRSTRLEN),
						        newfd, fdmax);

						if (newfd - 1 >= MAX_NUM_CLIENTS) {
							ERR("newfd: %2d (> %d)\n", newfd, MAX_NUM_CLIENTS);
							exit(5);
						}

						clients[newfd - 1].clientfd = newfd;
						clients[newfd - 1].is_streaming = false;
						clients[newfd - 1].pid_stream = 0;
						clients[newfd - 1].state = connected;

						if (num_clients >= MAX_NUM_CLIENTS - 4) {
							ERR("Number of clients: %2d (> %d)\n", num_clients,
							    MAX_NUM_CLIENTS - 4);

							for (int k = 0; k < fdmax; k++) {
								DBG_MED("###--- [%2d] ---###\n", k);
								showClientInfo(&clients[k]);
							}
							assert(0);
						}

						showClientInfo(&clients[newfd - 1]);
						++num_clients;
						DBG_MED("Number of clients: %2d\n", num_clients);
					}
				} else { // handle data from existing client
					memset(&buf, '\0', sizeof(buf));
					int offs = 0, n = 0;
					n = recv(i, buf, sizeof(buf), 0);
					if (n > 0) {
						if (buf[n - 1] != DELIMITER) {
							offs += n;
							// until recv DELIMITER
							while (1) {
								n = recv(i, buf + offs, sizeof(buf) - (size_t)offs, 0);
								if (n <= 0) {
									nbytes = offs;
									break;
								}

								offs += n;
								if (buf[offs - 1] == DELIMITER) {
									nbytes = offs;
									break;
								}
							}
						} else {
							nbytes = n;
						}
					} else {
						nbytes = 0;
					}

					DBG_MED("\t Recive %d bytes of data\n", nbytes);
					DBG_MED("\t in buf (%s)\n", buf);
					DBG_MED("\t from client %2d\n", i);
					if (nbytes <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							DBG_MED("Client %d hung up\n", i);
						} else {
							ERR("recv");
						}
						close(i);
						FD_CLR(i, &master);
						--num_clients;
					} else { // we got some data from a client
						for (j = 0; j <= fdmax; ++j) {
							if (FD_ISSET(j, &master) && i == j) {
								DBG_MED("Handle action for client %2d, fdmax = %d ...\n",
								        j, fdmax);
								client_idx = getClientIdx(j, clients, fdmax);

								if (client_idx < 0) {
									ERR("client_idx = %2d\n", client_idx);

									for (int k = 0; k < fdmax; k++) {
										DBG_MED("###--- [%2d] ---###\n", k);
										showClientInfo(&clients[k]);
									}

									assert(0 && "Failed to getClientIdx");
									exit(6);
								}

								err = handleAction(interpretData(&buf[0], j, &master),
								                   &clients[client_idx], nbytes);

								if (err != ACT_SUCCESS) {
									sendResponse(clients[client_idx].clientfd, err);
								}

								//if (send(j, buf, nbytes, 0) == -1) ERR("send");
							}
						}
					}
				} // END handle data from client
			} // END new incoming connection
		} // END looping through file descriptors
	} // END for(;;)
}

/* zero warning
static void handleSigIng(int signo)
{
	if (signo == SIGINT) {
		DBG_HIGH("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		DBG_HIGH("Caught SIGTERM!\n");
	} else {
		ERR("Unexpected signal!\n");
		exit(1);
	}
}
*/

pid_t forkStream(char *case_config)
{
	pid_t pid;
	pid = fork();
	if (pid == -1)
		ERR("fork");
	// child process
	DBG_MED("pid = %ld\n", (long)pid);
	if (!pid) {
		DBG_MED("child process begin!\n");
		int ret;
		int retd = daemon(0, 0); //zero warning
		DBG_MED("retd = %d\n", retd);
		char caseConfig[BUFLEN] = "/system/mpp/case_config/";
		strcat(caseConfig, case_config);

		ret = execl("/system/bin/mpi_stream", "mpi_stream", "-d", caseConfig, ">", "/dev/null", "&", NULL);
		if (ret == -1) {
			ERR("execl");
			exit(EXIT_FAILURE);
		}
	}
	return pid;
}

int compareChannel(int chn_ratio, int real_resolution, int input_ratio, int chn_idx,
				   int temp_chn_ratio, int temp_real_resolution, int temp_input_ratio, int temp_chn_idx)
{
	if (chn_ratio > temp_chn_ratio) {
		return -1;
	} else if (chn_ratio < temp_chn_ratio) {
		return 1;
	}
	if (real_resolution > temp_real_resolution) {
		return -1;
	} else if (real_resolution < temp_real_resolution) {
		return 1;
	}
	if (input_ratio > temp_input_ratio) {
		return -1;
	} else if (input_ratio < temp_input_ratio) {
		return 1;
	}
	if (chn_idx < temp_chn_idx) {
		return -1;
	} else if (chn_idx > temp_chn_idx) {
		return 1;
	}
	return 0;
}

int chooseNrwChannel(MPI_PATH path_idx, MPI_CHN *chn_chosen)
{
	int chn_null_cnt = 0;
	MPI_WIN win_chosen_idx = MPI_VIDEO_WIN(path_idx.dev, path_idx.path, 0);
	int chn_ratio = 0; // Ratio occupying the output streaming
	int real_resolution = 0; // Real resolution, in pixels
	int input_ratio = 0; // ROI size of input streaming
	int chn_idx = -1; // Index of the channel

	for (int chn = 0; chn < MPI_MAX_VIDEO_CHN_NUM; chn++) {
		MPI_CHN chn_id = MPI_VIDEO_CHN(path_idx.dev, chn);
		MPI_CHN_LAYOUT_S p_chn_layout;
		memset(&p_chn_layout, 0, sizeof(MPI_CHN_LAYOUT_S));
		int ret = MPI_DEV_getChnLayout(chn_id, &p_chn_layout);

		if (ret < 0) {
			chn_null_cnt++;
			continue;
		}

		for (int w = 0; w < p_chn_layout.window_num; ++w) {
			int w_idx = p_chn_layout.win_id[w].win;
			int p_idx = 0;
			MPI_WIN_ATTR_S p_window_attr;
			memset(&p_window_attr, 0, sizeof(MPI_WIN_ATTR_S));
			MPI_DEV_getWindowAttr(p_chn_layout.win_id[w], &p_window_attr);

			/* obtain p_idx for this window */
			for (unsigned bit = 0; bit < 32; bit++) {
				if (p_window_attr.path.bmp & (1u << bit)) {
					p_idx = bit;
					break;
				}
			}

			if (p_window_attr.path.bmp == 0 || p_idx >= MPI_MAX_INPUT_PATH_NUM) {
				DBG_MED("Invaild path_bmp (%d) on d, c, w(= %d, %d, %d)", p_window_attr.path.bmp,
				        path_idx.dev, chn, w_idx);
				continue;
			}

			if (p_idx == path_idx.path) {
				const MPI_RECT_S *win_res = &p_chn_layout.window[w];

				MPI_CHN_ATTR_S p_chn_attr;
				memset(&p_chn_attr, 0, sizeof(MPI_CHN_ATTR_S));
				MPI_DEV_getChnAttr(chn_id, &p_chn_attr);

				const MPI_SIZE_S *chn_res = &p_chn_attr.res;
				const MPI_RECT_S *win_roi = &p_window_attr.roi;

				int temp_chn_ratio = UNSIGN_FIXED_DIVID(win_res->height, MAX(chn_res->height, 1), 10) *
									 UNSIGN_FIXED_DIVID(win_res->width, MAX(chn_res->width, 1), 10);
				int temp_real_resolution = win_res->height * win_res->width;
				int temp_input_ratio = win_roi->height * win_roi->width;
				int temp_chn_idx = chn;

				if (chn_idx == -1 || compareChannel(chn_ratio, real_resolution, input_ratio, chn_idx,
													temp_chn_ratio, temp_real_resolution, temp_input_ratio, temp_chn_idx) == 1)
				{
					real_resolution = temp_real_resolution;
					chn_ratio = temp_chn_ratio;
					input_ratio = temp_input_ratio;
					chn_idx = temp_chn_idx;
					win_chosen_idx = MPI_VIDEO_WIN(path_idx.dev, chn, w_idx);
				}
			}
		}
	}
	if (chn_null_cnt == MPI_MAX_VIDEO_CHN_NUM || chn_idx == -1) {
		goto chn_not_ready;
	}

	chn_chosen->chn = win_chosen_idx.chn;
	return 0;

chn_not_ready:
	DBG_MED("Video channels are not ready yet, skipping the query.\n");
	return ACT_FAILURE;
}

static void getTfwSnapshotInfo(int path_index, int stitch_en, char *port_type, FileInfo *file_info)
{
	MPI_PATH path = MPI_INPUT_PATH(0, path_index);
	MPI_PATH_ATTR_S path_attr = { 0 };
	MPI_DEV_getPathAttr(path, &path_attr);

	sprintf(port_type, "ISW-WP%d", path_index);
	DBG_MED("port_type = %s", port_type);

#if CONFIG_CCSERVER_SUPPORT
	strcpy(jstr, "{\"module\": \"video_dev_conf\"}\0");
	input_path_cnt = ccClientGet(jstr, CC_GET_INPUT_PATH_CNT);
	strcpy(jstr, "{\"module\": \"video_dev_conf\"}\0");
	file_info->col = ccClientGet(jstr, CC_GET_TFW_W);
	strcpy(jstr, "{\"module\": \"video_dev_conf\"}\0");
	file_info->row = ccClientGet(jstr, CC_GET_TFW_H);
	// TODO: correctly distinguish stitching and dual ISP scenarios
	if (input_path_cnt == 2) {
		file_info->col = file_info->col * 2;
	}
#else
	if (stitch_en) {
		MPI_PATH path1 = MPI_INPUT_PATH(0, 1);
		MPI_PATH_ATTR_S path1_attr = { 0 };
		MPI_DEV_getPathAttr(path1, &path1_attr);
		file_info->col = path_attr.res.width + path1_attr.res.width;
	} else {
		file_info->col = path_attr.res.width;
	}
	file_info->row = path_attr.res.height;
#endif
	DBG_MED(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	DBG_MED("TFW : col = %d, row = %d\n", file_info->col, file_info->row);
	DBG_MED(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
}

static int getNrwSnapshotInfo(int path_index, char *port_type, FileInfo *file_info)
{
	MPI_PATH path = MPI_INPUT_PATH(0, path_index);
	MPI_CHN chn = MPI_VIDEO_CHN(0, 0);
	if (chooseNrwChannel(path, &chn) < 0) {
		DBG_MED("Cannot find appropriate yuv snapshot channel");
		return ACT_FAILURE;
	}
	MPI_CHN_ATTR_S chn_attr;
	memset(&chn_attr, 0, sizeof(chn_attr));
	MPI_DEV_getChnAttr(chn, &chn_attr);
	sprintf(port_type, "isp_NRW_%d", chn.chn);
#if CONFIG_CCSERVER_SUPPORT
	strcpy(jstr, "{\"module\": \"video_strm_conf\"}\0");
	file_info->col = ccClientGet(jstr, CC_GET_NRW_W);
	strcpy(jstr, "{\"module\": \"video_strm_conf\"}\0");
	file_info->row = ccClientGet(jstr, CC_GET_NRW_H);
#else
	file_info->col = chn_attr.res.width;
	file_info->row = chn_attr.res.height;
#endif
	DBG_MED(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	DBG_MED("NRW : col = %d, row = %d\n", file_info->col, file_info->row);
	DBG_MED(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	return ACT_SUCCESS;
}

int dumpImage(char *type, int *dump_size, int clientfd)
{
	FileInfo file_info;
	int ret;
	char port_type[20] = { 0 };
	char snapshot_type[10] = { 0 };
	int path_index = 0;
	sscanf(type, "%s %d", snapshot_type, &path_index);

#if CONFIG_CCSERVER_SUPPORT
	// use ccclient if using ccserver
	int input_path_cnt = 1;
	char jstr[JSON_STR_LEN] = { 0 };
#else
	// get info from MPI if not using ccserver
	MPI_DEV dev0 = { .dev = 0 };
	MPI_DEV_ATTR_S dev_attr = { 0 };
	MPI_DEV_getDevAttr(dev0, &dev_attr);
#endif

	DBG_MED("ENTER DUMP IMAGE!\n");

	if (strcmp(snapshot_type, "TFW") == 0) {
		getTfwSnapshotInfo(path_index, dev_attr.stitch_en, port_type, &file_info);
	} else if (strcmp(snapshot_type, "NRW") == 0) {
		if (getNrwSnapshotInfo(path_index, port_type, &file_info) == ACT_FAILURE) {
			return ACT_FAILURE;
		}
	} else {
		DBG_MED("There are no this port type, %s\n", snapshot_type);
		return ACT_FAILURE;
	}

#if CONFIG_CCSERVER_SUPPORT
	bzero(jstr, JSON_STR_LEN);
	strcpy(jstr, "{\"module\": \"video_dev_conf\"}\0");
	file_info.phase = ccClientGet(&jstr[0], CC_GET_BAYER);
#else
	file_info.phase = dev_attr.bayer;
#endif
	DBG_MED(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	DBG_MED("DUMP : bayer = %d\n", file_info.phase);
	DBG_MED(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	file_info.bit = 8; //10;
	file_info.port_type = port_type;

	closeCC();

	ret = Snapshot_run(&file_info, dump_size, clientfd);

	return ret;
}

int mpiSysConnect(char *type, int clientfd)
{
	int ret = ACT_SUCCESS;

	if (strcmp(type, "START") == 0) {
		if (mpi_sys_status == 0) {
			ret = initMpiSys();
			if (ret == ACT_SUCCESS) {
				mpi_sys_status = 1;
				DBG_MED("Success to MPI_SYS_init!");
			}
		}
	} else if (strcmp(type, "END") == 0) {
		if (mpi_sys_status == 1) {
			ret = exitMpiSys();
			if (ret == ACT_SUCCESS) {
				mpi_sys_status = 0;
				DBG_MED("Success to MPI_SYS_exit!");
			}
		}
	} else {
		if (mpi_sys_status == 1) {
			ret = runMpiSys(type, clientfd);
		}
	}

	return ret;
}

int sendCalibFilePath(char *type, int clientfd)
{
	int ret = 0;
	char file_path[MAX_DATA_SIZE_BYTES] = { 0 };
	char send_data[MAX_DATA_SIZE_BYTES] = { 0 };

	ret = getCalibFilePath(type, file_path);

	if (ret != 0)
		return -1;
	sprintf(send_data, "%s%c", file_path, (DELIMITER));
	if (sendData(clientfd, send_data, strlen(send_data)) < 0) {
		DBG_MED("Fail to send STITCH file path to PC!");
		return -1;
	}
	return 0;
}

int sendBurnInReg(int sockfd, char *reg)
{
	FILE *fp;
	int status = -1;
	char str[MAX_DATA_SIZE_BYTES] = { 0 };
	char ret_value[MAX_DATA_SIZE_BYTES] = { 0 };
	char cmd[128] = { 0 };

	sprintf(cmd, "fw_printenv %s", reg);
	fp = popen(cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		return -1;
	}

	if (fgets(str, MAX_DATA_SIZE_BYTES, fp) != NULL) {
		sprintf(ret_value, "%s", str);
	} else {
		sprintf(ret_value, "%d", status);
	}

	status = pclose(fp);
	if (status == -1) {
		return -1;
	}
	sendData(sockfd, ret_value, strlen(ret_value));

	return 0;
}

int sendCatReg(int sockfd, char *reg)
{
	FILE *fp;
	int status = -1;
	char str[MAX_DATA_SIZE_BYTES] = { 0 };
	char ret_value[MAX_DATA_SIZE_BYTES] = { 0 };
	char cmd[128] = { 0 };

	sprintf(cmd, "cat %s", reg);
	fp = popen(cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		return -1;
	}

	if (fgets(str, MAX_DATA_SIZE_BYTES, fp) != NULL) {
		sprintf(ret_value, "%s", str);
	} else {
		sprintf(ret_value, "%d", status);
	}

	status = pclose(fp);
	if (status == -1) {
		return -1;
	}
	sendData(sockfd, ret_value, strlen(ret_value));

	return 0;
}

void help(void)
{
	DBG_MED("Usage:\n");
	DBG_MED("\t To run unicorn as daemon, do not use any args.\n");
	DBG_MED("\t'-g get '\t get product setting. D:img_pref\n");
	DBG_MED("\t'-s set '\t set product setting. D:img_pref\n");
	DBG_MED("\t'-c socket_type '\t set accept sockey type. D:all\n");
	DBG_MED("\t'-p port '\t set product setting. D:img_pref\n");
	DBG_MED("\n");
	DBG_MED("\tSample usage\n");
	DBG_MED("\t$ unicorn -g img_pref\n");
	DBG_MED("\t$ unicorn -c eth0\n");
	DBG_MED("\t$ unicorn -p 6668\n");
}

int main(int argc, char **argv)
{
	char *gModule = NULL;
	char jstr[JSON_STR_LEN] = { 0 };
	int c;
	int ret;
	int index;

	while ((c = getopt(argc, argv, "hs:g:c:p:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'g':
			gModule = optarg;
			strcpy(jstr, "{\"module\": \"");
			strcat(jstr, gModule);
			strcat(jstr, "\"}");
			ret = ccClientGet(&jstr[0], CC_GET_CMD_MODE);
			if (ret == -1) {
				DBG_MED("ccClientGet failed!\n");
			}
			break;
		case 'c':
			sprintf(g_socket_type, optarg);
			break;
		case 'p':
			sprintf(g_connect_port, optarg);
			break;
		case 's':
			//#TODO
			break;
		case '?':
			if ((optopt == 'g') || (optopt == 's') || (optopt == 'c') || (optopt == 'p'))
				ERR("Option '-%c' requires an argument.\n", optopt);
			else if (isprint(optopt))
				ERR("Unknown option '-%c'.\n", optopt);
			else
				ERR("Unknown option character '\\x%x.\n", optopt);
			return 1;
		default:
			DBG_MED("aborting...\n");
			abort();
		}
	}

	DBG_HIGH("unicorn daemon ready to serve with port %s!\n", g_connect_port);
	ret = initMpiSys();
	if (ret) {
		ERR("Failed to initialize MPI\n");
		exit(1);
	}

	startService();
	exitMpiSys();

	for (index = optind; index < argc; index++)
		DBG_MED("Non-option argument %s\n", argv[index]);
	return 0;
}
