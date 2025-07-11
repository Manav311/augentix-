#ifdef CONFIG_CCSERVER_SUPPORT
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "agtx_cmd.h"
#include "unicorn_debug.h"
#include "utils.h"
#include "ccclient.h"
#include "gpio.h"

#define CC_PATH "/tmp/ccUnxSkt"

int g_master_id = -1;
int g_sfd = -1;

int initCC(void);
int openCC(void);
static int connectCC(int sockfd);
static int registerCC(int sockfd);
static int startSessionWithCC(int sockfd);
int getCCRegister(char *jstr, int cmd_mode);

/*
 * Effects: write len bytes from buf into sockfd for sure
 */
int writeCcSocket(int sockfd, char *buf, int len)
{
	int ret = 0;
	while (len != 0) {
		ret = write(sockfd, buf, len);
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			ERR("writeCcSocket");
			break;
		}
		len -= ret;
		buf += ret;
	}
	return ret;
}

/*
 * Effects: read len bytes from sockfd into buf for sure
 */
int readCcSocket(int sockfd, char *buf, int buf_size)
{
	int ret, ret_len;

	ret_len = 0;

	while (1) {
		ret = read(sockfd, buf + ret_len, buf_size - ret_len);
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
	}

	return ret_len;
}

int checkCCStatus(void)
{
	int ret = 0;

	if (g_master_id < 0) {
		ret = openCC();
		if (ret < 0) {
			ERR("ccClient is disconnect !!\n");
			ret = -1;
		}
	}

	return ret;
}

int initCC(void)
{
	int sockfd = -1;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		ERR("Create sockfd failed %d(%m)\n", errno);
		return errno;
	}

	return sockfd;
}

/*
 * Requires: sockfd is a Unix socket
 *
 * Effects:  make connection to cc and returns 0 if successful, -1 otherwise
 */
static int connectCC(int sockfd)
{
	int servlen = 0;
	struct sockaddr_un serv_addr;

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, CC_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		ERR("Connecting to server failed %d(%m)\n", errno);
		return -1;
	}

	return 0;
}

/*
 * Requires: connectCC has been invoked successfully beforehand
 *
 * Effects:  register connection with cc, returns 0 if successful and -1 if failure
 */
static int registerCC(int sockfd)
{
	int ret;
	char buf[128] = { 0 };
	char reg_buf[128] = { 0 };
	char ret_cmd[128] = { 0 };

	sprintf(reg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"UNICORN\"}",
	        AGTX_CMD_REG_CLIENT);
	sprintf(ret_cmd, "{ \"master_id\": 0, \"cmd_id\": %d, \"cmd_type\": \"reply\", \"rval\": 0 }",
	        AGTX_CMD_REG_CLIENT);

	/*Send register information*/
	if (write(sockfd, &reg_buf, strlen(reg_buf)) < 0) {
		ERR("write socket error %d(%m)\n", errno);
		return -1;
	}

	while (1) {
		ret = read(sockfd, buf, strlen(ret_cmd));
		if (ret != strlen(ret_cmd)) {
			ERR("read socket error %d(%m)\n", errno);
			DBG_MED("Failed to read from CC socket! Retry...\n");
			continue;
		}

		if (strncmp(buf, ret_cmd, strlen(ret_cmd))) {
			usleep(100000);
			DBG_MED("Waiting CC replay register cmd\n");
			DBG_MED("Waiting CC to replay register cmd! Retry...\n");
			continue;
		} else {
			DBG_MED("Registered to CC from APP\n");
			break;
		}
	}

	return 0;
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
	char jstr2cc[JSON_STR_LEN] = { 0 };
	char ret_cmd[JSON_STR_LEN] = { 0 };

	sprintf(jstr2cc, "{\"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\"}", AGTX_CMD_SESS_START);
	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret == -1) {
		ERR("Failed to write CC socket.\n");
		return ret;
	}

	ret = readCcSocket(sockfd, ret_cmd, JSON_STR_LEN);
	if (ret == -1) {
		ERR("Failed to read socket.\n");
		return ret;
	}

	ret = unicorn_json_validation(ret_cmd, strlen(ret_cmd));
	if (ret != 0) {
		ERR("Read start session from cc incomplete!\n");
		ERR("The return message was %s\n", ret_cmd);
		return ret;
	}

	ret = unicorn_json_get_int(ret_cmd, "rval", strlen(ret_cmd));
	if (ret < 0) {
		ERR("Unable to extract rval: %d\n", ret);
		return ret;
	}

	ret = unicorn_json_get_int(jstr2cc, "master_id", strlen(jstr2cc));
	if (ret < 0) {
		ERR("Unable to extract master_id: %d\n", ret);
		return ret;
	}

	return ret;
}

void replaceCmdId(char *jstr, char *jstr_out)
{
	int cmd_id = -1;
	char *module_name = unicorn_json_get_string(jstr, "module", strlen(jstr));
	if (module_name == NULL) {
		ERR("Unable to get module name\n");
		return;
	}
	cmd_id = get_command_id(module_name);
	strcpy(jstr_out, jstr);
	unicorn_json_delete_key(jstr_out, "module", strlen(jstr_out)); //delete "module"
	unicorn_json_add_key_int(jstr_out, "cmd_id", cmd_id, strlen(jstr_out)); // add "cmd_id"
	if (module_name) {
		free(module_name);
	}
}

void removeCmdId(char *jstr, char *jstr_out)
{
	unicorn_json_delete_key(jstr, "master_id", strlen(jstr));
	unicorn_json_delete_key(jstr, "cmd_id", strlen(jstr));
	unicorn_json_delete_key(jstr, "cmd_type", strlen(jstr));
	unicorn_json_delete_key(jstr, "rval", strlen(jstr));
	strncpy(jstr_out, jstr, strlen(jstr));
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
	char jstr2cc[JSON_STR_LEN] = { 0 };
	char ret_cmd[JSON_STR_LEN] = { 0 };

	sprintf(jstr2cc, "{\"master_id\":%d, \"cmd_type\":\"set\", ", master_id);
	strncat(jstr2cc, jstr + 1, strlen(jstr));

	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret < 0) {
		ERR("Failed to write CC socket.");
		return ret;
	}

	ret = readCcSocket(sockfd, ret_cmd, JSON_STR_LEN);
	if (ret < 0) {
		ERR("readCcSocket failed = %d", ret);
		return -1;
	}

	ret = unicorn_json_get_int(ret_cmd, "rval", strlen(ret_cmd));
	if (ret < 0) {
		ERR("Failed to extract return value from cc = %d", ret);
		return ret;
	}
	return ret;
}

/*
 * Requires: connectCC, registerCC, and startSessionWithCC have been invoked
 *           successfully beforehand
 *
 * Modifies: jstr
 *
 * Effects:  send module query from jstr to sockfd, store the returned JSON string
 *           into jstr and returns rval from cc
 *
 */
static int getCC(int sockfd, int master_id, char *jstr)
{
	int ret;
	char jstr2cc[JSON_STR_LEN] = { 0 };
	char ret_cmd[JSON_STR_LEN] = { 0 };

	sprintf(jstr2cc, "{\"master_id\":%d, \"cmd_type\":\"get\", ", master_id);
	strncat(jstr2cc, jstr + 1, strlen(jstr));
	DBG_MED("jstr2cc %s\n", jstr2cc);

	ret = writeCcSocket(sockfd, jstr2cc, strlen(jstr2cc));
	if (ret < 0) {
		ERR("Failed to write CC socket.");
		return ret;
	}

	ret = readCcSocket(sockfd, ret_cmd, JSON_STR_LEN);
	if (ret < 0) {
		ERR("readCcSocket failed = %d", ret);
		return -1;
	}

	strncpy(jstr, ret_cmd, strlen(ret_cmd));
	ret = unicorn_json_get_int(ret_cmd, "rval", strlen(ret_cmd));
	if (ret < 0) {
		ERR("Failed to extract return value from cc = %d", ret);
		return ret;
	}

	return ret;
}

/*
 * Requires: jstr be a valid JSON string containing data fields
 *           of a specific product setting and key 'module'
 *
 * Modifies: jstr
 *
 * Effects:  Connects to CC and parses the module from jstr (PC)
 *           Set the product setting from PC to CC and returns 0
 *           upon success from cc and -1 if failure.
 *
 */
int ccClientSet(char *jstr)
{
	int ret = 0;
	char ccjstr[JSON_STR_LEN] = { 0 };

	ret = checkCCStatus();
	if (ret < 0) {
		return ret;
	}

	replaceCmdId(jstr, ccjstr);
	ret = setCC(g_sfd, g_master_id, ccjstr);
	if (ret != 0) {
		DBG_HIGH("set cc failed!\n");
	}

	return ret;
}

/*
 * Requires: jstr be a JSON string that contains the key 'module',
 *           clientfd is a socketfd connected to PC
 *
 * Modifies: jstr
 *
 * Effects:  Parses input jstr and connects to CC to get corresponding
 *           product setting (from jstr), the returned JSON string from
 *           CC is reduced to only the data fields and the module. The
 *           final JSON string is then sent to clientfd (PC). This
 *           function returns 0 upon success, -1 for failure.
 *
 * Special usage:
 *           Certain values of clientfd have been reserved for the purpose
 *           of getting resolution from the CC and the attained values will
 *           not be sent to PC. Instead the attained value will be returned
 *           to the caller, the reserved values for clientfd include:
 *
 *           -11 : width in video_strm_conf with index 0 (NRW)
 *           -12 : height in video_strm_conf with index 0 (NRW)
 *           -13 : width in video_dev_conf with index 0 (TFW)
 *           -14 : height in video_dev_conf with index 0 (TFW)
 *           -15 : bayer in video_dev_conf
 */
int ccClientGet(char *jstr, int cmd_mode)
{
	int ret = 0;
	char ret_cmd[JSON_STR_LEN] = { 0 };

	ret = checkCCStatus();
	if (ret < 0) {
		snprintf(jstr, sizeof(ret), "%d", ret);
		return ret;
	}

	replaceCmdId(jstr, ret_cmd);
	ret = getCC(g_sfd, g_master_id, ret_cmd);
	if (ret == 0) {
		ret = unicorn_json_get_int(ret_cmd, "rval", strlen(ret_cmd));
		if (ret == 0) {
			removeCmdId(ret_cmd, jstr);
			//DBG_MED("cc return successfully after get value %s\n", ret_cmd);
			ret = getCCRegister(ret_cmd, cmd_mode);
		} else {
			DBG_MED("Bad rval in cc return: %s\n", ret_cmd);
			ret = -1;
		}
	} else {
		DBG_MED("set cc failed!\n");
		ret = -1;
	}
	return ret;
}

int getCCRegister(char *jstr, int cmd_mode)
{
	int ret = 0;
	switch (cmd_mode) {
	case CC_GET_CMD_MODE:
		DBG_MED("\n==============================\n");
		ret = write(1, jstr, strlen(jstr));
		DBG_MED("\n==============================\n");
		if (ret != strlen(jstr)) {
			ret = -1;
		}
		break;
	case CC_GET_TFW_W:
		ret = unicorn_json_get_int_from_array(jstr, "input_path_list", "width", 0, strlen(jstr));
		break;
	case CC_GET_TFW_H:
		ret = unicorn_json_get_int_from_array(jstr, "input_path_list", "height", 0, strlen(jstr));
		break;
	case CC_GET_NRW_W:
		ret = unicorn_json_get_int_from_array(jstr, "video_strm_list", "width", 0, strlen(jstr));
		break;
	case CC_GET_NRW_H:
		ret = unicorn_json_get_int_from_array(jstr, "video_strm_list", "height", 0, strlen(jstr));
		break;
	case CC_GET_BAYER:
		ret = unicorn_json_get_int(jstr, "bayer", strlen(jstr));
		break;
	case CC_GET_INPUT_PATH_CNT:
		ret = unicorn_json_get_int(jstr, "input_path_cnt", strlen(jstr));
		break;
	case CC_GET_REG:
		break;
	case CC_GET_GPIO:
		ret = parseGpio(jstr);
		break;
	case CC_GET_ADC:
		ret = parseAdc(jstr);
		break;
	case CC_GET_WIN_NUM:
		ret = unicorn_json_get_int_from_array(jstr, "video_layout", "window_num", 0, strlen(jstr));
		break;
	default:
		DBG_MED("Error: Unknown command 0x%02x\n", cmd_mode);
		ret = -1;
		break;
	}

	return ret;
}

void closeCC(void)
{
	close(g_sfd);
	g_master_id = -1;
	g_sfd = -1;
}

int openCC(void)
{
	int ret;

	g_sfd = initCC();
	if (g_sfd < 0) {
		ERR("unix socket");
		return g_sfd;
	}

	ret = connectCC(g_sfd);
	if (ret < 0) {
		ERR("connectCC");
		goto close_cc;
	}

	ret = registerCC(g_sfd);
	if (ret != 0) {
		ERR("registerCC");
		goto close_cc;
	}

	g_master_id = startSessionWithCC(g_sfd);
	if (g_master_id < 0) {
		ERR("startSessionWithCC");
		ret = g_master_id;
		goto close_cc;
	}

	return 0;

close_cc:
	closeCC();

	return ret;
}

#else
#include <errno.h>

#include "agtx_types.h"

int ccClientSet(char *jstr)
{
	AGTX_UNUSED(jstr);

	return -ENODEV;
}

int ccClientGet(char *jstr, int clientfd)
{
	AGTX_UNUSED(jstr);
	AGTX_UNUSED(clientfd);

	return -ENODEV;
}

void closeCC(void)
{
	/* Do Nothing */
}

#endif
