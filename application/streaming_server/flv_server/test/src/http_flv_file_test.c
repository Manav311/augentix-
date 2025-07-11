#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

int g_run_flag = 0;
int srv_soc = 0, acpt_soc = 0; /* socket 句柄 */

/* 定义常量 */
#define HTTP_DEF_PORT 8005 /* 连接的缺省端口 */
#define HTTP_BUF_SIZE 1024 /* 缓冲区的大小 */
#define HTTP_FILENAME_LEN 256 /* 文件名长度 */

/* 定义文件类型对应的 Content-Type */
struct doc_type {
	char *suffix; /* 文件后缀 */
	char *type; /* Content-Type */
};

struct doc_type file_type[] = { { "html", "text/html" },
	                        { "gif", "image/gif" },
	                        { "jpeg", "image/jpeg" },
	                        { "flv", "video/x-flv" },
	                        { NULL, NULL } };

#if (0)
char *http_res_hdr_tmpl = "HTTP/1.1 200 OK\r\nServer: Huiyong's Server <0.1>\r\n"
                          "Accept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: close\r\n"
                          "Content-Type: %s\r\n\r\n";
#else
char *http_res_hdr_tmpl = "HTTP/1.1 200 OK\r\n"
                          "Server: Huiyong's Server <0.1>\r\n"
                          "Content-Type:  video/x-flv\r\n"
                          "Connection: keep-alive\r\n"
                          "Expires: -1\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Access-Control-Allow-Credentials: true\r\n\r\n";
#endif

/**************************************************************************
 *
 * 函数功能: 根据文件后缀查找对应的 Content-Type.
 *
 * 参数说明: [IN] suffix, 文件名后缀;
 *
 * 返 回 值: 成功返回文件对应的 Content-Type, 失败返回 NULL.
 *
 **************************************************************************/
char *http_get_type_by_suffix(const char *suffix)
{
	struct doc_type *type;

	for (type = file_type; type->suffix; type++) {
		if (strcmp(type->suffix, suffix) == 0)
			return type->type;
	}

	return NULL;
}

/**************************************************************************
 *
 * 函数功能: 解析请求行, 得到文件名及其后缀. 请求行格式:
 *           [GET http://www.baidu.com:8080/index.html HTTP/1.1]
 *
 * 参数说明: [IN]  buf, 字符串指针数组;
 *           [IN]  buflen, buf 的长度;
 *           [OUT] file_name, 文件名;
 *           [OUT] suffix, 文件名后缀;
 *
 * 返 回 值: void.
 *
 **************************************************************************/
void http_parse_request_cmd(char *buf, int buflen, char *file_name, char *suffix)
{
	int length = 0;
	char *begin, *end, *bias;

	/* 查找 URL 的开始位置 */
	begin = strchr(buf, ' ');
	begin += 1;

	/* 查找 URL 的结束位置 */
	end = strchr(begin, ' ');
	*end = 0;

	bias = strrchr(begin, '/');
	length = end - bias;

	/* 找到文件名的开始位置 */
	if ((*bias == '/') || (*bias == '\\')) {
		bias++;
		length--;
	}

	/* 得到文件名 */
	if (length > 0) {
		memcpy(file_name, bias, length);
		file_name[length] = 0;

		begin = strchr(file_name, '.');
		if (begin)
			strcpy(suffix, begin + 1);
	}
}

/**************************************************************************
 *
 * 函数功能: 向客户端发送 HTTP 响应.
 *
 * 参数说明: [IN]  buf, 字符串指针数组;
 *           [IN]  buf_len, buf 的长度;
 *
 * 返 回 值: 成功返回非0, 失败返回0.
 *
 **************************************************************************/
int http_send_response(int soc, char *buf, int buf_len)
{
	int read_len, file_len, hdr_len, send_len;
	char *type;
	char read_buf[HTTP_BUF_SIZE];
	char http_header[HTTP_BUF_SIZE];
	//char file_name[HTTP_FILENAME_LEN] = "index.html" , suffix[16] = "html" ;
	char file_name[HTTP_FILENAME_LEN] = "test-src.flv", suffix[16] = "flv";

	FILE *res_file;

	/* 得到文件名和后缀 */
	//http_parse_request_cmd(buf, buf_len, file_name, suffix);

	res_file = fopen(file_name, "rb+"); /* 用二进制格式打开文件 */
	printf("try open %s\n", file_name);
	if (res_file == NULL) {
		printf("[Web] The file [%s] is not existed\n", file_name);
		return 0;
	}
	printf("send open %s\n", file_name);

	fseek(res_file, 0, SEEK_END);
	file_len = ftell(res_file);
	//fseek (res_file, 0, SEEK_SET);
	fseek(res_file, 0, SEEK_SET);
	type = http_get_type_by_suffix(suffix); /* 文件对应的 Content-Type */
	if (type == NULL) {
		fclose(res_file);
		printf("[Web] There is not the related content type\n");
		return 0;
	}

#if (1)
	/* 构造 HTTP 首部，并发送 */
	//hdr_len = sprintf (http_header, http_res_hdr_tmpl, file_len, type);
	hdr_len = sprintf(http_header, http_res_hdr_tmpl);
	//hdr_len = sprintf (http_header, http_res_hdr_tmpl, type);
	send_len = send(soc, http_header, hdr_len, 0);
	//send_len=1;
	if (send_len == -1) {
		fclose(res_file);
		printf("[Web] Fail to send, error = %d\n", send_len);
		return 0;
	}

	printf("%s", http_header);
#endif
	do /* 发送文件, HTTP 的消息体 */
	{
		read_len = fread(read_buf, sizeof(char), HTTP_BUF_SIZE, res_file);
		printf("read: %d, %0x %0x %0x %0x\n", read_len, read_buf[0], read_buf[1], read_buf[2], read_buf[3]);

		if (read_len > 0) {
			send_len = send(soc, read_buf, read_len, 0);
			if (send_len == -1) {
				printf("[Web] Fail to send, error = %d\n", send_len);
				break;
			}
			file_len -= read_len;
		}
#if (0)
		int recv_len;
		char recv_buf[64];
		recv_len = recv(soc, recv_buf, 54, 0);
		if (recv_len == -1) /* 接收失败 */
		{
			close(soc);
			printf("[Web] Fail to recv, error = %d\n", recv_len);
			break;
		}

		printf("%s", recv_buf);
#endif
	} while ((read_len > 0) && (file_len > 0));

	fclose(res_file);
	printf("[Web] send finish\n");

	return 1;
}

void handleSignal(int signo)
{
	if (signo == SIGINT) {
		printf("received SIGINT\n");
		g_run_flag = 0;
		if (srv_soc != 0)
			close(srv_soc);
		if (acpt_soc != 0)
			close(acpt_soc);
		printf("close port :%d , %d\n", srv_soc, acpt_soc);
		exit(0);
	}

	return;
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSignal) == SIG_ERR) {
		printf("\ncan't catch SIGINT\n");
	}
	//	WSADATA wsa_data;
	//int  srv_soc = 0, acpt_soc;  /* socket 句柄 */
	struct sockaddr_in serv_addr; /* 服务器地址  */
	struct sockaddr_in from_addr; /* 客户端地址  */
	char recv_buf[HTTP_BUF_SIZE];
	unsigned short port = HTTP_DEF_PORT;
	socklen_t from_len = sizeof(from_addr);
	int result = 0, recv_len;

	if (argc == 2) /* 端口号 */
		port = atoi(argv[1]);

	//WSAStartup(MAKEWORD(2,0), &wsa_data); /* 初始化 WinSock 资源 */

	srv_soc = socket(PF_INET, SOCK_STREAM, 0); /* 创建 socket */
	if (srv_soc == -1) {
		printf("[Web] socket() Fails, error = %d\n", srv_soc);
		return -1;
	}

	int reuseaddr = 1;
	int len = sizeof(reuseaddr);
	int ret = setsockopt(srv_soc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, len);
	if (ret == -1) {
		fprintf(stderr, "Failed to set re-use addr\r\n");
	}

	/* 服务器地址 */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	result = bind(srv_soc, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (result == -1) /* 绑定失败 */
	{
		close(srv_soc);
		printf("[Web] Fail to bind, error = %d\n", result);
		return -1;
	}

	result = listen(srv_soc, SOMAXCONN);
	printf("[Web] The server is running ... ...\n");

	g_run_flag = 1;
	while (g_run_flag) {
		acpt_soc = accept(srv_soc, (struct sockaddr *)&from_addr, &from_len);
		if (acpt_soc == -1) /* 接受失败 */
		{
			printf("[Web] Fail to accept, error = %d\n", acpt_soc);
			break;
		}

		printf("[Web] Accepted address:[%s], port:[%d]\n", inet_ntoa(from_addr.sin_addr),
		       ntohs(from_addr.sin_port));

		recv_len = recv(acpt_soc, recv_buf, HTTP_BUF_SIZE, 0);
		if (recv_len == -1) /* 接收失败 */
		{
			close(acpt_soc);
			printf("[Web] Fail to recv, error = %d\n", recv_len);
			break;
		}

		printf("%s", recv_buf);
		recv_buf[recv_len] = 0;

		/* 向客户端发送响应数据 */
		result = http_send_response(acpt_soc, recv_buf, recv_len);
		//close(acpt_soc);
	}

	close(srv_soc);
	//WSACleanup();
	printf("[Web] The server is stopped.\n");

	return 0;
}
