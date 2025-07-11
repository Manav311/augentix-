#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stddef.h>
#include <netinet/in.h>

/** @brief Struct that contains Sink operation functions.
 *
 * The struct contains function pointers to be assigned.
 * The actual behavior of the functions should be defined by programmers.
 */
typedef struct SinkUdpOpsType {
	int (*open)(void *p, int flags);
	ssize_t (*write)(void *p, void *src, size_t nbyte);
	int (*close)(void *p);
} SinkUdpOps;

/** @brief Struct of Sink, the abstract bitstream output.
 */
typedef struct SinkUdpType {
	char *name; /**< Name of the Sink. */
	void *info; /**< Pointer of programmer-defined Sink information. */
	SinkUdpOps *ops; /**< Pointer of SinkOps, a struct of programmer-defined Sink operations. */
} SinkUdp;

/** @brief Struct that contains UDP-specific Sink information.
 */
typedef struct UdpSockAttrType {
	int client_sockfd; /**< File descriptor of the UDP socket. */
	struct sockaddr_in server_addr; /**< Socket information. */
} UdpSockAttr;

SinkUdp *createUdpSink(const char *const sink_name, const char *const server_ip, const int server_port);

void releaseUdpSink(SinkUdp *udp_sock);

#endif /* UDP_SOCKET_H */
