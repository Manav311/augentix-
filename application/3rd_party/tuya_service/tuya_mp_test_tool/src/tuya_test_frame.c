/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tuya_test_frame.c - Tuya MP test frame definition
 * Copyright (C) 2019-2020 im14, Augentix Inc. <MAIL@augentix.com>
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "tuya_test_frame.h"
#include "tuya_mptt_common.h"
#include "tuya_cmd.h"
#include "tuya_test_handler.h"

#define HEADER_LEN 2
static const char header[HEADER_LEN] = { 0x55, 0xaa };
#define CMD_VERSION 0x00

int g_start_flag;
int g_event_interrupt = 0;

/*
static void __hexdump(void *data, int len)
{
	int i;
	unsigned char *d = (unsigned char *)data;
	DBG("Data length: %d : \n", len);
	for (i = 0; i < len; i++)
		DBG(" 0x%02x\n", d[i]);
	DBG("\n");
}
*/

static TuyaTestFrame *tuya_get_valid_frame(unsigned char *buffer, int size)
{
	unsigned int datalen = 0, datasum = 0;
	unsigned char checksum = 0x0;
	int i = 0, offset = 0;
	TuyaTestFrame *frame = NULL;
	/* Find cmd frame header */
	while (1) {
		if ((size - i) < 7)
			return NULL;
		if (memcmp(buffer + i, header, HEADER_LEN) == 0)
			break;
		i++;
	}
	offset = i;

	/* Skip header, version and command index to get data length */
	i += 4;
	datalen = (buffer[i] << 8) + (buffer[i + 1] << 0);
	i += 2;

	/* Length sanity check */
	if (datalen > TUYA_FRAME_MAX_LEN || (offset + 7 + datalen) < size)
		return NULL;

	/* Data checksum */
	checksum = buffer[i + datalen];
	for (i = 0, datasum = 0; i < (datalen + 6); i++) { /* header + version + id + data_len */
		datasum += (unsigned char)buffer[i];
	}
	datasum &= 0xff;
	if (datasum != checksum) {
		printf("Error: Checksum error: calculated 0x%02x, expected 0x%02x\n", checksum, datasum);
		return NULL;
	}

	/* Generate TuyaTestFrame */
	frame = (TuyaTestFrame *)calloc(1, sizeof(TuyaTestFrame));
	if (frame == NULL) {
		printf("Error: Failed to allocate Tuya Test Command Frame!\n");
		return NULL;
	}

	memcpy(frame->header, buffer + offset, 2);
	frame->version[0] = *(buffer + offset + 2);
	frame->command[0] = *(buffer + offset + 3);
	frame->check_sum[0] = checksum;
	frame->data = (unsigned char *)calloc((datalen + 1), sizeof(unsigned char));
	memcpy(frame->data, buffer + offset + 6, datalen);
	frame->data_len = datalen;

	DBG("==== Received valid frame: cmd = 0x%02x, data = %s ====\n", frame->command[0], frame->data);
	return frame;
}

TuyaTestFrame *tuya_get_frame(int *client_fd)
{
	unsigned char buf[TUYA_FRAME_MAX_LEN];
	TuyaTestFrame *frame = NULL;
	int ret;
	int fd = *client_fd;

	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf));
	if (ret < 0) {
		perror("Failed to read from socket");
		return NULL;
	} else if (ret == 0) {
		perror("Closed by client");
		close(fd);
		*client_fd = -1;
		return NULL;
	} else {
		//__hexdump(buf, ret);
		frame = tuya_get_valid_frame(buf, ret);
		return frame;
	}
}

static int tuya_put_out(int fd, char *buf, unsigned int len)
{
	if (buf != NULL) {
		return write(fd, buf, len);
	}
	return -1;
}

int tuya_put_frame(int fd, int cmd, char *data, int size)
{
	unsigned int datalen;
	int data_offset, total;
	unsigned char i = 0;
	char buf[TUYA_FRAME_MAX_LEN];

	DBG("Send frame: cmd 0x%02x, data %s\n", (unsigned char)cmd, data);
	memset(buf, 0, sizeof(buf));

	/* header */
	memcpy(buf + i, header, HEADER_LEN);
	i += sizeof(header);
	/* version */
	buf[i++] = CMD_VERSION;
	/* command index */
	buf[i++] = cmd;
	/* data length */
	datalen = size;
	buf[i++] = (datalen >> 8) & 0xff;
	buf[i++] = (datalen >> 0) & 0xff;
	/* data */
	data_offset = i;
	memcpy(buf + i, data, datalen);
	/* checksum */
	total = data_offset + datalen + 1;
	unsigned int datasum = 0;
	for (i = 0; i < total - 1; i++)
		datasum += buf[i];
	buf[total - 1] = datasum & 0xff;

	return tuya_put_out(fd, buf, total);
}

void tuya_free_frame(TuyaTestFrame *frame)
{
	if (frame == NULL)
		return;

	if (frame->data != NULL) {
		free(frame->data);
		frame->data = NULL;
	}
	free(frame);
	frame = NULL;
	return;
}

void run_thread_test(int com_fd, TuyaTestFrame *frame, void *(*test_fn)(void *))
{
	pthread_t tid;
	pthread_attr_t tattr;

	struct thread_info *info = calloc(1, sizeof(struct thread_info));
	{
		info->fd = com_fd, info->cmd = frame->command[0], info->put_cb = tuya_put_frame,
		info->data_len = frame->data_len, strncpy(info->data, (char *)frame->data, TUYA_FRAME_MAX_LEN);
		memcpy(info->ip_addr, frame->ip_addr, sizeof(info->ip_addr));
	}

	DBG("Loop test: cmd = 0x%02x, data = %s\n", info->cmd, info->data);

	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &tattr, test_fn, info);
}

void tuya_dispatch_cmd(int fd, TuyaTestFrame *frame)
{
	char buf[TUYA_FRAME_MAX_LEN] = { 0 };
	unsigned char cmd = frame->command[0];
	int ret;

	g_start_flag = true;
	DBG("Dispatch frame: cmd = 0x%02x, start_flag = %d\n", cmd, g_start_flag);

	if (g_start_flag == true) {
		switch (cmd) {
		default:
			printf("Error: Unknown command 0x%02x\n", cmd);
			return;
			break;
		case TUYATEST_MODE:
			ret = tuya_enter_test_mode(frame, buf);
			break;
		case TUYATEST_R_MASTER_FIRMWARE:
			g_event_interrupt = cmd;
			ret = tuya_read_master_firmware(frame, buf);
			break;
		case TUYATEST_R_SLAVE_FIRMWARE:
			DBG("Not supported!\n");
			break;
		case TUYATEST_W_CFGINFO:
			g_event_interrupt = cmd;
			ret = tuya_write_cfg(frame, buf);
			break;
		case TUYATEST_R_CFGINFO:
			g_event_interrupt = cmd;
			ret = tuya_read_cfg(frame, buf);
			break;
		case TUYATEST_W_MASTER_MAC:
			g_event_interrupt = cmd;
			ret = tuya_write_mac(frame, buf);
			break;
		case TUYATEST_R_MASTER_MAC:
			ret = tuya_read_mac(frame, buf);
			break;
		case TUYATEST_W_SLAVE_MAC:
			DBG("Not supported!\n");
			break;
		case TUYATEST_R_SLAVE_MAC:
			DBG("Not supported!\n");
			break;
		case TUYATEST_SELF_TEST:
			DBG("Not supported!\n");
			break;
		case TUYATEST_BUTTON_TEST:
			g_event_interrupt = cmd;
			run_thread_test(fd, frame, tuya_button_test);
			break;
		case TUYATEST_LED_TEST:
			g_event_interrupt = cmd;
			run_thread_test(fd, frame, tuya_led_test);
			break;
		case TUYATEST_W_BSN:
			g_event_interrupt = cmd;
			ret = tuya_write_bsn(frame, buf);
			break;
		case TUYATEST_R_BSN:
			g_event_interrupt = cmd;
			ret = tuya_read_bsn(frame, buf);
			break;
		case TUYATEST_W_SN:
			g_event_interrupt = cmd;
			ret = tuya_write_sn(frame, buf);
			break;
		case TUYATEST_R_SN:
			g_event_interrupt = cmd;
			ret = tuya_read_sn(frame, buf);
			break;
		case TUYATEST_MASTER_CAP:
			g_event_interrupt = cmd;
			ret = tuya_wifi_rssi_test(frame, buf);
			break;
		case TUYATEST_SLAVE_CAP:
			DBG("Not supported!\n");
			break;
		case TUYATEST_IPERF_TEST:
			g_event_interrupt = cmd;
			ret = tuya_wifi_iperf_test(frame, buf);
			break;
		case TUYATEST_IRCUT_TEST:
			g_event_interrupt = cmd;
			run_thread_test(fd, frame, tuya_ircut_test);
			break;
		case TUYATEST_SPEAKER_TEST:
			g_event_interrupt = cmd;
			ret = tuya_speaker_test(frame, buf);
			break;
		case TUYATEST_MIC_TEST:
			g_event_interrupt = cmd;
			ret = tuya_mic_test(frame, buf);
			break;
		case TUYATEST_IRLED_TEST:
			g_event_interrupt = cmd;
			run_thread_test(fd, frame, tuya_irled_test);
			break;
		case TUYATEST_VIDEO_TEST:
			g_event_interrupt = cmd;
			run_thread_test(fd, frame, tuya_rtsp_test);
			break;
		case TUYATEST_BLACK_TEST:
			g_event_interrupt = cmd;
			//run_thread_test(fd, frame, tuya_rtsp_test);
			break;
		case TUYATEST_W_COUNTRY_CODE:
			g_event_interrupt = cmd;
			ret = tuya_write_cn(frame, buf);
			break;
		case TUYATEST_R_COUNTRY_CODE:
			g_event_interrupt = cmd;
			ret = tuya_read_cn(frame, buf);
			break;
		case TUYATEST_MOTOR_TEST:
			DBG("Not supported!\n");
			break;
		case TUYATEST_PIR_TEST:
			DBG("Not supported!\n");
			break;
		}
	}

	if (buf[0] != 0) {
		ret = tuya_put_frame(fd, cmd, buf, strlen(buf));
		DBG("Put frame size = %d\n", ret);
	}
}
