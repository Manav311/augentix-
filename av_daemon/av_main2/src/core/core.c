#include "core.h"
#include "handlers.h"
#include "nodes.h"
#include "log_define.h"

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/un.h>

#include "avftr_conn.h"

extern int g_run_flag;
GlobalConf g_conf;
extern Node g_nodes[NODE_NUM];
pthread_t tid = 0;

static int readCcCmd(int sockfd, AGTX_MSG_HEADER_S *cmd)
{
	int ret = 0;
	void *cmd_data = malloc(cmd->len);
	avmain2_log_info("read fd %d len %d", sockfd, cmd->len);

	if (cmd_data == NULL) {
		avmain2_log_err("invaild cmd data == NULL");
		return -EINVAL;
	}

	if (cmd->len == 0) {
		avmain2_log_err("invaild cmd len: %d", cmd->len);
		return -EINVAL;
	}

	if (cmd->len != (size_t)read(sockfd, cmd_data, cmd->len)) {
		avmain2_log_err("failed to read ccserver, err:%d(%m)", errno);
		free(cmd_data);
		return -ENOLINK;
	}

	/* low level DIP attr agtx cmd idx from AGTX_CMD_DIP_CAL to AGTX_CMD_DIP_WB_INFO */
	int dip_not_support_cmd_id_list[23] = {
		AGTX_CMD_DIP_CAL,  AGTX_CMD_DIP_DBC,      AGTX_CMD_DIP_DCC,    AGTX_CMD_DIP_LSC, AGTX_CMD_DIP_CTRL,
		AGTX_CMD_DIP_AE,   AGTX_CMD_DIP_ISO,      AGTX_CMD_DIP_AWB,    AGTX_CMD_DIP_PTA, AGTX_CMD_DIP_CSM,
		AGTX_CMD_DIP_SHP,  AGTX_CMD_DIP_NR,       AGTX_CMD_DIP_ROI,    AGTX_CMD_DIP_TE,  AGTX_CMD_DIP_GAMMA,
		AGTX_CMD_DIP_ENH,  AGTX_CMD_DIP_CORING,   AGTX_CMD_DIP_FCS,    AGTX_CMD_DIP_DMS, AGTX_CMD_DIP_HDR_SYNTH,
		AGTX_CMD_DIP_STAT, AGTX_CMD_DIP_EXP_INFO, AGTX_CMD_DIP_WB_INFO
	};

	/* av main2 not support low level DIP attr set to ini.db */
	for (int i = 0; (unsigned)i < (sizeof(dip_not_support_cmd_id_list) / sizeof(int)); i++) {
		if ((int)cmd->cid == dip_not_support_cmd_id_list[i]) {
			avmain2_log_err("av main2 not support DIP attr");
			free(cmd_data);
			return -EINVAL;
		}
	}

	ret = HANDLERS_apply(cmd->cid, cmd->len, cmd_data /*AGTX struct*/, &g_nodes[VB]);

	free(cmd_data);
	return ret;
}

static int sendCCReply(INT32 sockfd, AGTX_MSG_HEADER_S *cmd, int ret)
{
	avmain2_log_info("id: %d, len: %d, ret : %d", cmd->cid, sizeof(ret), ret);

	if (write(sockfd, cmd, sizeof(*cmd)) < 0) {
		avmain2_log_err("write socket error %d(%m)", errno);
		return -ENETDOWN;
	}
	if (write(sockfd, &ret, sizeof(int)) < 0) {
		avmain2_log_err("write socket error %d(%m)", errno);
		return -ENETDOWN;
	}
	return 0;
}

static int registerCC(INT32 sockfd)
{
	int ret;
	char buf[128] = { 0 };
	char reg_buf[128] = { 0 };
	char ret_cmd[128] = { 0 };

	sprintf(reg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"AV_MAIN\"}",
	        AGTX_CMD_REG_CLIENT);
	sprintf(ret_cmd, "{ \"master_id\": 0, \"cmd_id\": %d, \"cmd_type\": \"reply\", \"rval\": 0 }",
	        AGTX_CMD_REG_CLIENT);

	/*Send register information*/
	if (write(sockfd, &reg_buf, strlen(reg_buf)) < 0) {
		avmain2_log_info("write socket error %d(%m)", errno);
		return -ENETDOWN;
	}

	do {
		ret = read(sockfd, buf, strlen(ret_cmd));
		if (ret != (int)strlen(ret_cmd)) {
			avmain2_log_info("read socket error %d(%m)", errno);
			continue;
		}

		if (strncmp(buf, ret_cmd, strlen(ret_cmd))) {
			usleep(100000);
			avmain2_log_info("Wating CC replay register cmd");
			continue;
		} else {
			avmain2_log_info("Registered to CC from %s", "AV_MAIN");
			break;
		}
	} while (1); /*only once*/

	return 0;
}

static void *connectCC(void *arg)
{
	AGTX_UNUSED(arg);

	INT32 ret = 0;
	INT32 err_cnt = 0;

	INT32 sockfd = -1;
	INT32 servlen = 0;
	struct timeval tv = { 0 };

	struct sockaddr_un serv_addr;
	fd_set read_fds;
	AGTX_MSG_HEADER_S cmd_header = { 0 };
	AGTX_MSG_HEADER_S cmd_reply = { 0 };

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, CC_SOCKET_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		avmain2_log_err("Create sockfd failed %d(%m)", errno);
		return NULL;
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		avmain2_log_err("Connecting to server failed %d(%m)", errno);
		close(sockfd);
		return NULL;
	}

	if (registerCC(sockfd) < 0) {
		avmain2_log_err("failed to register to cc");
		close(sockfd);
		return NULL;
	}

	/*continue read cmd request*/
	while (g_run_flag) {
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);

		ret = select(sockfd + 1, &read_fds, NULL, NULL, &tv);
		if (ret < 0) {
			avmain2_log_err("select error, %d(%m)", errno);
			continue;
		} else if (ret == 0) {
			continue;
		} else {
			if (err_cnt > 5) {
				avmain2_log_err("Too many error close socket and leave.");
				break;
			}

			ret = read(sockfd, &cmd_header, sizeof(cmd_header));

			if (ret < 0) {
				avmain2_log_err("Read failed %d(%m),leave thread.", errno);
				break;
			} else if (ret == 0) {
				avmain2_log_err("Read socket return value 0");
				break;
			} else if (ret != sizeof(cmd_header)) {
				avmain2_log_err("Read too short %d(%m)", errno);
				err_cnt++;
				continue;
			}

			/* reply ccserver/http server failed and not write db */
			/* reply ccserver/http server success and  write db */
			ret = readCcCmd(sockfd, &cmd_header);
			cmd_reply.cid = cmd_header.cid;
			cmd_reply.sid = 0;
			cmd_reply.len = sizeof(ret);

			/*tell ccserver exec this cmd failed or not*/
			sendCCReply(sockfd, &cmd_reply, ret);
		}
		sleep(1);
	}

	close(sockfd);

	return NULL;
}

int CORE_init(void)
{
	INT32 ret = 0;

	/*JSON cmd handler read from cc/db start*/
	HANDLERS_allReadDb(TMP_ACTIVE_DB);
	NODES_initNodes();

	Node *current = &g_nodes[VB];
	ret = NODES_enterNodespreOrderTraversal(current);
	if (0 != ret) {
		/*NODE graph.stop in err case*/
		NODES_leaveNodespreOrderTraversal(&g_nodes[VB]);
		return ret;
	}

	/* do all start after every init funcs */
	current = &g_nodes[VB];
	ret = NODES_startNodespreOrderTraversal(current);
	if (0 != ret) {
		/*NODE graph.stop in err case*/
		NODES_leaveNodespreOrderTraversal(&g_nodes[VB]);
		return ret;
	}

	/*continuous connect cc/db*/
	char tmpname[16] = { 0 };

#ifdef USE_CCSERVER
	char name[16] = "av_main_cc";
	if (pthread_create(&tid, NULL, connectCC, NULL) != 0) {
		avmain2_log_info("Create thread to avmain_cc failed.");
		return -1;
	}
#else
	char name[16] = "av_main_http";
#endif
	ret = pthread_setname_np(tid, name);
	sleep(1);
	if (ret == 0) {
		if (pthread_getname_np(tid, tmpname, sizeof(tmpname)) == 0) {
			avmain2_log_info("%s: Get thread name [Done]", tmpname);
		} else {
			avmain2_log_info("%s: Get thread name [Fail]", name);
		}
	}
	return 0;
}

int CORE_exit(void)
{
	int ret = 0;

	ret = NODES_leaveNodespreOrderTraversal(&g_nodes[VB]);
	if (0 != ret) {
		avmain2_log_err("failed to leave nodes");
	}

	/*exit all module*/
	pthread_join(tid, NULL);
	memset(&g_conf, 0, sizeof(g_conf));
	avmain2_log_info("core exit");

	return 0;
}