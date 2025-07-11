#ifndef _AGTX_CMD_HANDLE_H_
#define _AGTX_CMD_HANDLE_H_

void agtx_cmd_handler(unsigned char *buf, int length);
void agtx_req_handler(unsigned char *buf, int length, int sock);

void agtx_lpw_init(int argc, char **argv);
int agtx_lpw_sync(void);

#endif /* _AGTX_CMD_HANDLE_H_ */
