#ifndef TUYA_TEST_FRAME_H
#define TUYA_TEST_FRAME_H

#include <stdbool.h>
/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tuya_test_frame.h - Tuya MP test frame definition
 * Copyright (C) 2019-2020 Shihchieh Lin, Augentix Inc. <shihchieh.lin@augentix.com>
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 */

#define TUYA_FRAME_MAX_LEN 256

extern int g_start_flag;
extern int g_event_interrupt;

typedef struct tuya_test_frame {
	unsigned char header[2];
	unsigned char version[1];
	unsigned char command[1];
	unsigned char length[2]; /* data length in cmd frame */
	unsigned char *data;
	unsigned char check_sum[1];
	unsigned int data_len; /* actual data length in unsigned int */
	unsigned char ip_addr[32];
	//unsigned char *frame_buffer;
} TuyaTestFrame;

struct thread_info {
	int fd;
	unsigned char cmd;
	char data[TUYA_FRAME_MAX_LEN];
	unsigned int data_len;
	unsigned char ip_addr[32];
	int (*put_cb)(int fd, int cmd, char *data, int size);
};

/* Tuya command frame handlers */

/*
 * tuya_get_frame: Get a parsed TuyaTestFrame pointer
 *
 * @client_fd input socket descriptor
 * @return valid TuyaTestFrame pointer on success; NULL on failure
 */
TuyaTestFrame *tuya_get_frame(int *client_fd);

/*
 * tuya_put_frame: Assemble a TuyaTestFrame and send it to socket
 *
 * @fd output socket descriptor
 * @cmd cmd type
 * @data output data
 * @size length of data to be sent
 * @return 0 on success; -1 on failure
 */
int tuya_put_frame(int fd, int cmd, char *data, int size);

/*
 * tuya_free_frame: Release given TuyaTestFrame
 *
 * @frame Tuya test frame
 */
void tuya_free_frame(TuyaTestFrame *frame);

/*
 * tuya_dispatch_cmd: Execute corresponding test command
 *
 * @frame Tuya test frame
 */
void tuya_dispatch_cmd(int fd, TuyaTestFrame *frame);

#endif /* TUYA_TEST_FRAME_H */
