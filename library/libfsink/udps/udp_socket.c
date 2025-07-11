#include "common.h"
#include "udp_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_NAME_LENG 20
#define SET_SOCKET_TIMEOUT 1

/** @brief Open an UDP Sink.
 *      Return 0 upon completion. Otherwise, return -1 when the socket is
 *      not opened, and errno is set appropriately.
 *
 *  The function use the info previously stored in UdpSockAttr,
 *  such as IP address and port number, to create a UDP socket, and pass
 *  the file descriptor of the socket back to the UpdSockAttr.
 *  @param[in, out] p        Pointer of the input UdpSockAttr.
 *  @param[in]      flags    Unused.
 */
static int openUdpSock(void *p, int flags __attribute__((unused)))
{
	UdpSockAttr *sock_attr = (UdpSockAttr *)p;
	SSD_PRINTF("%s\n", __func__);

#ifdef SET_SOCKET_TIMEOUT
	struct timeval recv_tmo = {
		.tv_sec = 0,
		.tv_usec = 10,
	};
#endif

	/* Create a socket */
	sock_attr->client_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_attr->client_sockfd == -1) {
		perror("Failed to create client socket!");
		return -1;
	}
#ifdef SET_SOCKET_TIMEOUT
	SSD_PRINTF("Set socket timeout.\n");
	setsockopt(sock_attr->client_sockfd, SOL_SOCKET, SO_RCVTIMEO, &recv_tmo, sizeof(recv_tmo));
#endif

	return 0;
}

/** @brief Write data to an UDP Sink.
 *      Return size of data written to the socket.
 *      Otherwise, return -1 on error, and errno is set appropriately.
 *
 *  @param[in] p     Pointer of the input UdpSockAttr.
 *  @param[in] src   Pointer of the starting address of the source to be copied.
 *  @param[in] nbyte Number of bytes to be copied from src.
 */
ssize_t writeUdpSock(void *p, void *src, size_t nbyte)
{
	UdpSockAttr *sock_attr = (UdpSockAttr *)p;
	return sendto(sock_attr->client_sockfd, src, nbyte, 0, (struct sockaddr *)&sock_attr->server_addr,
	              sizeof(sock_attr->server_addr));
}

/** @brief Close an UDP Sink.
 *      Return 0 upon completion.
 *      Otherwise, return -1 on error, and errno is set appropriately.
 *
 *  @param[in] p     Pointer of the input UdpSockAttr.
 */
static int closeUdpSock(void *p)
{
	int ret;
	UdpSockAttr *sock_attr = (UdpSockAttr *)p;
	SSD_PRINTF("%s\n", __func__);
	ret = close(sock_attr->client_sockfd);
	if (ret != 0) {
		perror("Failed to close UDP socket!");
		return ret;
	}
	sock_attr->client_sockfd = -1;
	return ret;
}

/** @brief Create an UDP Sink assigned to the UDP server,
 *      i.e. receiver end of the stream.
 *      Return the pointer to the created UDP Sink upon completion.
 *      Otherwise, return NULL on error, and errno is set appropriately.
 *
 *  The function creates UDP SinkUdp, SinkUdpOps and UdpSockAttr,
 *  apply the input IP address & port to the UdpSockAttr,
 *  and binds them together in the created UDP Sink.
 *  @param[in] sink_name     Name of the create UDP Sink.
 *  @param[in] server_ip     Server IP address of the UDP Sink.
 *  @param[in] server_port   Server port assigned of the UDP Sink.
 */
SinkUdp *createUdpSink(const char *const sink_name, const char *const server_ip, const int server_port)
{
	SinkUdp *udp_sock;
	SinkUdpOps *udp_sock_ops;
	UdpSockAttr *udp_sock_attr;
	struct sockaddr_in *saddr;

	udp_sock = malloc(sizeof(*udp_sock));
	if (udp_sock == NULL) {
		perror("Failed to allocate udp_sock!");
		goto exit_err;
	}

	udp_sock->name = malloc(MAX_NAME_LENG * sizeof(*udp_sock->name));
	if (udp_sock->name == NULL) {
		perror("Failed to allocate udp_sock name!");
		goto exit_name;
	}
	strncpy(udp_sock->name, sink_name, MAX_NAME_LENG);

	udp_sock_ops = malloc(sizeof(*udp_sock_ops));
	if (udp_sock_ops == NULL) {
		perror("Failed to allocate udp_sock_ops!");
		goto exit_ops;
	}
	/* Binding actual ops functions */
	{
		udp_sock_ops->open = openUdpSock;
		udp_sock_ops->write = writeUdpSock;
		udp_sock_ops->close = closeUdpSock;
	}

	udp_sock_attr = malloc(sizeof(*udp_sock_attr));
	if (udp_sock_attr == NULL) {
		perror("Failed to allocate udp_sock_attr!");
		goto exit_attr;
	}

	/* set UDP socket attributes */
	saddr = &udp_sock_attr->server_addr; /* pointer alias */
	memset(saddr, 0, sizeof(*saddr));
	saddr->sin_family = AF_INET;
	if (inet_aton(server_ip, (struct in_addr *)&saddr->sin_addr.s_addr) == 0) {
		perror(server_ip);
		goto exit_ip;
	}
	saddr->sin_port = htons(server_port);
	/* Leave sin_zero alone for now. */

	udp_sock->info = (void *)udp_sock_attr;
	udp_sock->ops = udp_sock_ops;

	SSD_PRINTF("UDP socket sink created.\n");

	return udp_sock;

exit_ip:
	free(udp_sock_attr);
exit_attr:
	free(udp_sock_ops);
exit_ops:
	free(udp_sock->name);
exit_name:
	free(udp_sock);
exit_err:
	return NULL;
}

/** @brief Destroy an UDP Sink by freeing the allocated memory spaces.
 *
 *  @param[in] udp_sock     Pointer to the UDP Sink to be destroyed.
 */
void releaseUdpSink(SinkUdp *udp_sock)
{
	free(udp_sock->info);
	free(udp_sock->ops);
	free(udp_sock->name);
	free(udp_sock);
	SSD_PRINTF("UDP socket released.\n");
}
