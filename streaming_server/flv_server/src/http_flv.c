#include "http_flv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "log_define.h"
#include "http_flv_parser.h"
#include "video.h"

bool HTTP_checkCodecInvalid(char chn_num)
{
	MPI_VENC_ATTR_S venc_attr;
	MPI_ENC_getVencAttr(MPI_ENC_CHN(chn_num), &venc_attr);
	if (venc_attr.type == MPI_VENC_TYPE_H264 || venc_attr.type == MPI_VENC_TYPE_H265) {
		return true;
	}

	return false;
}

int HTTP_checkCodecId(char chn_num, MPI_VENC_TYPE_E *type)
{
	int ret = 0;
	MPI_VENC_ATTR_S venc_attr;
	ret = MPI_ENC_getVencAttr(MPI_ENC_CHN(chn_num), &venc_attr);
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to get Venc attr: %d", ret);
		return -EINVAL;
	}
	if (venc_attr.type != MPI_VENC_TYPE_H264 && venc_attr.type != MPI_VENC_TYPE_H265) {
		return -EINVAL;
	}

	*type = venc_attr.type;

	return 0;
}

void HTTP_checkResponseSize(const char *src, uint32_t strlen, char *size)
{
	/*remove space*/
	char size_cnt = 0;
	for (int i = 0; (unsigned)i < strlen; i++) {
		if ((src[i] != 0x00) & (src[i] != '\0')) {
			size_cnt++;
		}
	}
	*size = size_cnt;
}

int HTTP_executeDeviceCmd(Message *m)
{
	if (m->method == HTTP_GET) {
		flv_server_log_info("url: %s.", m->request_url);
		m->chn_num = -1;
		m->is_audio = false;
		/*format TBD*/
		char audio[16] = { 0 };
		sscanf(m->request_url, "/%15[^/]/%c\r\n", &audio[0], &(m->chn_num));
		m->chn_num = m->chn_num - 0x30;

		flv_server_log_info("get in chn %d", m->chn_num);

		if (NULL != strstr(&audio[0], "liveaudio")) {
			m->is_audio = true;
		} else if (NULL != strstr(&audio[0], "live")) {
			m->is_audio = false;
		}

	} else {
		flv_server_log_err("Req not support.");
		return -EPERM;
	}

	return 0;
}

int HTTP_setlistenPort(int port, int timeout_s, char *ip_address)
{
	AGTX_UNUSED(timeout_s);

	int sockfd = 0;
	struct sockaddr_in info;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		flv_server_log_err("failed to create server sock");
		return -EIO;
	}

	int reuseaddr = 1;
	int len = sizeof(reuseaddr);
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, len);
	if (ret == -1) {
		flv_server_log_err("Failed to set re-use addr");
	}

	memset(&info, 0, sizeof(info));

	info.sin_family = AF_INET;
	info.sin_port = htons(port);
	if (ip_address == NULL) {
		info.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		info.sin_addr.s_addr = inet_addr(ip_address);
		if (info.sin_addr.s_addr == INADDR_NONE) {
			flv_server_log_err("Invalid IP address: %s", ip_address);
		}
	}

	if (bind(sockfd, (struct sockaddr *)&info, sizeof(info)) < 0) {
		flv_server_log_err("failed bind port :%d, %s", port, strerror(errno));
	}

	flv_server_log_debug("Start to listen at port %d", port);

	if (listen(sockfd, SOMAXCONN) < 0) {
		flv_server_log_err("failed to listen: %d, %s", port, strerror(errno));
	}

	return (sockfd);
}
