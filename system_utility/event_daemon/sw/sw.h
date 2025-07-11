#ifndef SW_H
#define SW_H

#define SW_SUCCESS (0)
#define SW_FAILURE (-1)
#define SW_CC_SOCKET_PATH "/tmp/ccUnxSkt"

int createServerListen(const char *socket_path);
int waitServerAccept(int listen_fd);
int startClientConnect(const char *socket_path);

#endif /* SW_H */