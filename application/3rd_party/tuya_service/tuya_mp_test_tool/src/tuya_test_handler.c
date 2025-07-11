/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tuya_test_handler.c - Tuya test command handler
 * Copyright (C) 2019 ShihChieh Lin, Augentix Inc. <shihchieh.lin@augentix.com>
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
#include <assert.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include "json.h"

#include "gpio/gpio.h"
#include "tuya_test_handler.h"
#include "tuya_test_frame.h"

int tuya_sd_write(void)
{
	char filename[128];
	const char test_buffer[] = "Hello world!";
	snprintf(filename, sizeof(filename), "/mnt/sdcard/sd_test.txt");
	int len = strlen(test_buffer) + 1; // null-terminated

	FILE *test_fp = fopen(filename, "w+");
	DBG("Card write, size = %d\n", len);
	if (test_fp) {
		int ret_w = fwrite(test_buffer, 1, sizeof(test_buffer), test_fp);
		DBG("Card write, size = %d\n", ret_w);

		fclose(test_fp);
		test_fp = NULL;

		return (ret_w == len) ? 0 : -1;
	} else {
		return -1;
	}
}

int tuya_enter_test_mode(TuyaTestFrame *frame, char *buf)
{
	DBG("Enter Tuya test mode\n");
	/* Run SD write test */
	if (tuya_sd_write() == 0)
		snprintf(buf, TUYA_FRAME_MAX_LEN,
		         "{\"comProtocol\":\"1.0.0\",\"deviceType\":\"IPcamera\",\"writeMac\":\"true\"}");
	else
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");

	return 0;
}

static int tuya_get_version(char *buf, size_t size)
{
	FILE *fp = NULL;
	int i;
	if (buf == NULL || size < 5) /* version starts from 0.0.0 */
		return -1;

	fp = fopen(VERSION_FILE, "r");
	if (fp) {
		fread(buf, 1, size, fp);
		fclose(fp);
		fp = NULL;
		for (i = 0; i < size; i++) {
			if (buf[i] == '\n') {
				buf[i] = '\0';
				break;
			}
		}
		return 0;
	} else {
		DBG("Failed to read file %s\n", VERSION_FILE);
	}
	return -1;
}

int tuya_read_master_firmware(TuyaTestFrame *frame, char *buf)
{
	char data[TUYA_FRAME_MAX_LEN] = { 0 };
	if (tuya_get_version(data, sizeof(data)) != 0) {
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	} else {
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true,\"firmName\":\"%s\",\"firmVer\":\"%s\"}", FW_NAME,
		         data);
		DBG("FW version info: %s\n", buf);
	}
	return 0;
}

static int string_to_json(char *string, size_t len, json_object **json)
{
	json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;
	do {
		*json = json_tokener_parse_ex(tok, string, len);
	} while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue);
	if (jerr != json_tokener_success || tok->char_offset < (int)len) {
		json_tokener_free(tok);
		fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
		return -1;
	}
	if (json_object_get_type(*json) == json_type_object || json_object_get_type(*json) == json_type_array) {
		json_tokener_free(tok);
		return 0;
	}
	json_tokener_free(tok);
	return -1;
}

static int get_json_value_string(json_object *json, const char *key, char value[])
{
	if (json_object_get_type(json) == json_type_object) {
		json_object_object_foreach(json, json_key, json_value)
		{
			if (strcmp(json_key, key) == 0) {
				strcpy(value, json_object_get_string(json_value));
				return 0;
			}
			get_json_value_string(json_value, key, value);
		}
	}
	return -1;
}

static int get_json_value_int(json_object *json, const char *key, int *value)
{
	if (json_object_get_type(json) == json_type_object) {
		json_object_object_foreach(json, json_key, json_value)
		{
			if (strcmp(json_key, key) == 0) {
				*value = json_object_get_int(json_value);
				return 0;
			}
			get_json_value_int(json_value, key, value);
		}
	}
	return -1;
}

#define CFG_CMD_MAX_LEN 64
enum tuya_cfg_type { CFG_PID = 0, CFG_UUID = 1, CFG_AUTHKEY = 2, CFG_NUM };
static const char tuya_cfg_token[CFG_NUM][16] = { "pid", "uuid", "auzkey" };
static const char agtx_cfg_token[CFG_NUM][16] = { "IPC_APP_PID", "IPC_APP_UUID", "IPC_APP_AUTHKEY" };

#define for_each_cfg_type(t) for ((t) = 0; (t) < CFG_NUM; ++(t))

int tuya_write_cfg(TuyaTestFrame *frame, char *buf)
{
	char value[32], sys_cmd[CFG_CMD_MAX_LEN];
	enum tuya_cfg_type type;
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		for_each_cfg_type(type)
		{
			if (get_json_value_string(json_obj, tuya_cfg_token[type], value) == 0) {
				snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_setenv %s %s", agtx_cfg_token[type], value);
				DBG("Write %s with command: %s\n", agtx_cfg_token[type], sys_cmd);
				system(sys_cmd);
			} else {
				goto fail;
			}
		}
	}
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

int tuya_read_cfg(TuyaTestFrame *frame, char *buf)
{
	FILE *fp = NULL;
	char value[3][32] = { "Invalid", "Invalid", "Invalid" };
	char sys_cmd[CFG_CMD_MAX_LEN];
	enum tuya_cfg_type type;

	for_each_cfg_type(type)
	{
		snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_printenv -n %s", agtx_cfg_token[type]);
		DBG("Read %s with command: %s\n", agtx_cfg_token[type], sys_cmd);

		fp = popen(sys_cmd, "r");
		if (fp == NULL) {
			pclose(fp);
			goto fail;
		}

		if (fgets(value[type], 32, fp) == NULL) {
			pclose(fp);
			goto fail;
		}
		if (value[type][strlen(value[type]) - 1] == '\n')
			value[type][strlen(value[type]) - 1] = '\0';
		DBG("%s: %s\n", agtx_cfg_token[type], value[type]);

		pclose(fp);
	}
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"auzkey\":\"%s\",\"uuid\":\"%s\",\"pid\":\"%s\"}", value[CFG_AUTHKEY],
	         value[CFG_UUID], value[CFG_PID]);
	DBG("Read cfg: %s\n", buf);
	return 0;
}

#define TUYA_MAC_TOKEN "mac"
#define AGTX_MAC_TOKEN "MAC"
int tuya_write_mac(TuyaTestFrame *frame, char *buf)
{
	char value[32], sys_cmd[CFG_CMD_MAX_LEN];
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, TUYA_MAC_TOKEN, value) == 0) {
			snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_setenv %s %s", AGTX_MAC_TOKEN, value);
			DBG("Write %s with command: %s\n", AGTX_MAC_TOKEN, sys_cmd);
			system(sys_cmd);
		} else {
			goto fail;
		}
	}
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

int tuya_read_mac(TuyaTestFrame *frame, char *buf)
{
	FILE *fp = NULL;
	char value[32] = "Invalid";
	char sys_cmd[CFG_CMD_MAX_LEN];
	snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_printenv -n %s", AGTX_MAC_TOKEN);
	DBG("Read %s with command: %s\n", AGTX_MAC_TOKEN, sys_cmd);

	fp = popen(sys_cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		goto fail;
	}

	if (fgets(value, 32, fp) == NULL) {
		pclose(fp);
		goto fail;
	}
	if (value[strlen(value) - 1] == '\n')
		value[strlen(value) - 1] = '\0';
	DBG("%s: %s\n", AGTX_MAC_TOKEN, value);

	pclose(fp);
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"mac\":\"%s\"}", value);
	DBG("Read MAC: %s\n", buf);
	return 0;
}

#define TUYA_BSN_TOKEN "bsn"
#define AGTX_BSN_TOKEN "BSN"
int tuya_write_bsn(TuyaTestFrame *frame, char *buf)
{
	char value[32], sys_cmd[CFG_CMD_MAX_LEN];
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, TUYA_BSN_TOKEN, value) == 0) {
			snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_setenv %s %s", AGTX_BSN_TOKEN, value);
			DBG("Write %s with command: %s\n", AGTX_BSN_TOKEN, sys_cmd);
			system(sys_cmd);
		} else {
			goto fail;
		}
	}
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

int tuya_read_bsn(TuyaTestFrame *frame, char *buf)
{
	FILE *fp = NULL;
	char value[32] = "Invalid";
	char sys_cmd[CFG_CMD_MAX_LEN];
	snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_printenv -n %s", AGTX_BSN_TOKEN);
	DBG("Read %s with command: %s\n", AGTX_BSN_TOKEN, sys_cmd);

	fp = popen(sys_cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		goto fail;
	}

	if (fgets(value, 32, fp) == NULL) {
		pclose(fp);
		goto fail;
	}
	if (value[strlen(value) - 1] == '\n')
		value[strlen(value) - 1] = '\0';
	DBG("%s: %s\n", AGTX_BSN_TOKEN, value);

	pclose(fp);
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"bsn\":\"%s\"}", value);
	DBG("Read BSN: %s\n", buf);
	return 0;
}

#define TUYA_SN_TOKEN "sn"
#define AGTX_SN_TOKEN "SN"
int tuya_write_sn(TuyaTestFrame *frame, char *buf)
{
	char value[32], sys_cmd[CFG_CMD_MAX_LEN];
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, TUYA_SN_TOKEN, value) == 0) {
			snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_setenv %s %s", AGTX_SN_TOKEN, value);
			DBG("Write %s with command: %s\n", AGTX_SN_TOKEN, sys_cmd);
			system(sys_cmd);
		} else {
			goto fail;
		}
	}
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

int tuya_read_sn(TuyaTestFrame *frame, char *buf)
{
	FILE *fp = NULL;
	char value[32] = "Invalid";
	char sys_cmd[CFG_CMD_MAX_LEN];
	snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_printenv -n %s", AGTX_SN_TOKEN);
	DBG("Read %s with command: %s\n", AGTX_SN_TOKEN, sys_cmd);

	fp = popen(sys_cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		goto fail;
	}

	if (fgets(value, 32, fp) == NULL) {
		pclose(fp);
		goto fail;
	}
	if (value[strlen(value) - 1] == '\n')
		value[strlen(value) - 1] = '\0';
	DBG("%s: %s\n", AGTX_SN_TOKEN, value);

	pclose(fp);
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"sn\":\"%s\"}", value);
	DBG("Read SN: %s\n", buf);
	return 0;
}

#define TUYA_CN_TOKEN "cn"
#define AGTX_CN_TOKEN "CN"
int tuya_write_cn(TuyaTestFrame *frame, char *buf)
{
	char value[32], sys_cmd[CFG_CMD_MAX_LEN];
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, TUYA_CN_TOKEN, value) == 0) {
			snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_setenv %s %s", AGTX_CN_TOKEN, value);
			DBG("Write %s with command: %s\n", AGTX_CN_TOKEN, sys_cmd);
			system(sys_cmd);
		} else {
			goto fail;
		}
	}
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

int tuya_read_cn(TuyaTestFrame *frame, char *buf)
{
	FILE *fp = NULL;
	char value[32] = "Invalid";
	char sys_cmd[CFG_CMD_MAX_LEN];
	snprintf(sys_cmd, CFG_CMD_MAX_LEN, "fw_printenv -n %s", AGTX_CN_TOKEN);
	DBG("Read %s with command: %s\n", AGTX_CN_TOKEN, sys_cmd);

	fp = popen(sys_cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		goto fail;
	}

	if (fgets(value, 32, fp) == NULL) {
		pclose(fp);
		goto fail;
	}
	if (value[strlen(value) - 1] == '\n')
		value[strlen(value) - 1] = '\0';
	DBG("%s: %s\n", AGTX_CN_TOKEN, value);

	pclose(fp);
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"cn\":\"%s\"}", value);
	DBG("Read CN: %s\n", buf);
	return 0;
}

/* Button test */
#define BUTTON_GPIO 12 /* I2S_TX_CLK */
#define BUTTON_POLARITY_INVERT 1 /* Invert: 0 = pressed, 1 = released */

enum button_state { BUTTON_IDLE = 0, BUTTON_PRESSED = 1, BUTTON_RELEASED = 2 };

inline int is_button_pressed(Gpio *gpio)
{
#if (BUTTON_POLARITY_INVERT == 1)
	return (getGpioValue(gpio) == 0);
#else
	return (getGpioValue(gpio) == 1);
#endif /* BUTTON_POLARITY_INVERT */
}

void *tuya_button_test(void *arg)
{
	Gpio *button = NULL;
	int pressed;
	enum button_state bstate = BUTTON_IDLE;
	char buf[TUYA_FRAME_MAX_LEN];
	struct thread_info *info = (struct thread_info *)arg;
	DBG("Start button test thread: cmd 0x%02x, data %s\n", info->cmd, info->data);

	button = initGpio(BUTTON_GPIO);
	if (button == NULL) {
		fprintf(stderr, "Failed to init GPIO %d\n", BUTTON_GPIO);
		goto fail;
	}

	setGpioDirection(button, GPIO_DIR_IN);
	bstate = (is_button_pressed(button)) ? BUTTON_PRESSED : BUTTON_IDLE;

	while (g_event_interrupt == info->cmd) {
		pressed = is_button_pressed(button);
		switch (bstate) {
		case BUTTON_IDLE:
			if (pressed)
				bstate = BUTTON_PRESSED;
			break;
		case BUTTON_PRESSED:
			if (!pressed)
				bstate = BUTTON_RELEASED;
			break;
		case BUTTON_RELEASED:
			snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"keyEvent\":\"key%d\"}", BUTTON_GPIO);
			info->put_cb(info->fd, info->cmd, buf, strlen(buf));
			bstate = BUTTON_IDLE;
			break;
		default:
			fprintf(stderr, "Invalid button state %d\n", bstate);
			goto fail;
			break;
		}
		usleep(200000);
	}

	releaseGpio(button);
fail:
	free(info);
	return NULL;
}

/* Status LED test */

#define LED_POLARITY_INVERT 0 /* Invert: 0 = on, 1 = off */
#define LED_NUM 2
static const int led_gpio_id[LED_NUM] = { 70, 73 };

enum led_pattern { LED_PTRN_ON = 0, LED_PTRN_OFF = 1, LED_PTRN_TOGGLE = 2, LED_PTRN_NUM };

inline int is_led_lit(Gpio *gpio)
{
#if (LED_POLARITY_INVERT == 1)
	return (getGpioValue(gpio) == 0);
#else
	return (getGpioValue(gpio) == 1);
#endif /* LED_POLARITY_INVERT */
}

int set_led_lit(Gpio *gpio, int enable)
{
#if (LED_POLARITY_INVERT == 1)
	int value = enable ? 0 : 1;
#else
	int value = enable;
#endif /* LED_POLARITY_INVERT */
	setGpioValue(gpio, value);
	return 0;
}

static int get_led_action(char *data, unsigned int data_len)
{
	int value;
	json_object *json_obj;
	int ret = string_to_json(data, data_len, &json_obj);
	if (ret) {
		goto fail;
	} else {
		if (get_json_value_int(json_obj, "lightAction", &value) == 0) {
			return value;
		} else {
			DBG("Error: Failed to find lightAction\n");
			goto fail;
		}
	}
fail:
	return -1;
}

void *tuya_led_test(void *arg)
{
#define LED_PERIOD_US 500000

	Gpio *led[LED_NUM];
	int led_action, i, j;
	char buf[TUYA_FRAME_MAX_LEN];
	struct thread_info *info = (struct thread_info *)arg;

	led_action = get_led_action(info->data, info->data_len);
	if (led_action < 0 || led_action >= LED_PTRN_NUM) {
		fprintf(stderr, "Error: Failed to parse LED action!");
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
		info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));
		goto fail;
	}
	DBG("LED action value %d\n", led_action);
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));

	DBG("Start LED test thread\n");
	for (i = 0; i < LED_NUM; i++) {
		led[i] = initGpio(led_gpio_id[i]);
		if (led[i] == NULL) {
			fprintf(stderr, "Failed to init GPIO %d\n", led_gpio_id[i]);
			/* Release previous GPIOs */
			for (j = (i - 1); j >= 0; j++)
				releaseGpio(led[j]);
			goto fail;
		}
		setGpioDirection(led[i], GPIO_DIR_OUT);
		setGpioValue(led[i], GPIO_HIGH);
	}

	j = 0;
	while (g_event_interrupt == info->cmd) {
		/* Set status LED patterns */
		switch (led_action) {
		case LED_PTRN_OFF:
			for (i = 0; i < LED_NUM; ++i) {
				if (is_led_lit(led[i]))
					set_led_lit(led[i], 0);
			}
			break;
		case LED_PTRN_ON:
			for (i = 0; i < LED_NUM; ++i) {
				if (!is_led_lit(led[i]))
					set_led_lit(led[i], 1);
			}
			break;
		case LED_PTRN_TOGGLE:
			for (i = 0; i < LED_NUM; ++i) {
				set_led_lit(led[i], ((i == j) ? 1 : 0));
			}
			break;
		default:
			fprintf(stderr, "Error: Invalid LED patterin specified!\n");
			goto ptrn_error;
			break;
		}
		DBG("GPIO: (%d, %d)\n", getGpioValue(led[0]), getGpioValue(led[1]));
		j = (j + 1) % LED_NUM;
		usleep(LED_PERIOD_US);
	}

ptrn_error:
	for (i = 0; i < LED_NUM; ++i)
		releaseGpio(led[i]);
fail:
	free(info);
	return NULL;
}

#define AUDIO_SAMPLE_FILE "/calib/test.wav"
int tuya_speaker_test(TuyaTestFrame *frame, char *buf)
{
	int ret;
	char cmd[TUYA_FRAME_MAX_LEN];
	snprintf(cmd, TUYA_FRAME_MAX_LEN, "aplay %s", AUDIO_SAMPLE_FILE);
	ret = system(cmd);
	if (ret != 0) {
		fprintf(stderr, "Error: Failed to play audio file!\n");
		goto fail;
	}

	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

#define TCP_BUFFER_MAX_LEN
static int send_audio_file(char *file, char *server_addr, int port)
{
	/* Send file through TCP socket to server port 8095 */
	int sock_fd = -1, bytes_sent = 0;
	struct sockaddr_in server; /* server's address information */
	FILE *fp = NULL;
	struct stat filestat;
	char buf[100 * 1024];

	DBG("Create socket\n");
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error: socket() error. Failed to initiate a socket");
		return -1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(server_addr);
	bzero(&(server.sin_zero), 8);
	DBG("Connect to server\n");
	if (connect(sock_fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
		perror("Error: connect() error. Failed to connect to server");
		close(sock_fd);
		return -1;
	}

	/* Get file stat */
	if (lstat(file, &filestat) < 0) {
		perror("Error: Failed to get record file stat");
		close(sock_fd);
		return -1;
	}

	DBG("Sending file %s\n", file);

	fp = fopen(file, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Failed to read file %s\n", file);
		close(sock_fd);
		return -1;
	}

	bytes_sent = 0;
	while (!feof(fp)) {
		bytes_sent = fread(buf, sizeof(char), sizeof(buf), fp);
		DBG("Read %d bytes of data\n", bytes_sent);
		bytes_sent = write(sock_fd, buf, bytes_sent);
		DBG("Send %d bytes of data\n", bytes_sent);
	}
	fclose(fp);
	fp = NULL;

	close(sock_fd);
	sock_fd = -1;
	return 0;
}

#define TUYA_MQTT_AUDIO_SOCKET_PORT 8095
#define AUDIO_RECORD_FILE "/tmp/record.wav"
int tuya_mic_test(TuyaTestFrame *frame, char *buf)
{
	/* Record a 3-sec, 8k sampling rate audio file */
	int ret;
	char cmd[TUYA_FRAME_MAX_LEN];

	snprintf(cmd, TUYA_FRAME_MAX_LEN, "arecord -f s16_le -c 1 -r 8000 -d 3 %s", AUDIO_RECORD_FILE);
	ret = system(cmd);
	if (ret != 0) {
		fprintf(stderr, "Error: Failed to record audio file!\n");
		goto fail;
	}

	DBG("Send audio file...\n");
	ret = send_audio_file(AUDIO_RECORD_FILE, g_client_addr, TUYA_MQTT_AUDIO_SOCKET_PORT);
	if (ret != 0) {
		fprintf(stderr, "Error: Failed to send audio file!\n");
		goto fail;
	}
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

#define IRLED_POLARITY_INVERT 0 /* Invert: 0 = pressed, 1 = released */
static int set_irled_lit(Gpio *irled, int enable)
{
#if (IRLED_POLARITY_INVERT == 1)
	int value = enable ? 0 : 1;
#else
	int value = enable;
#endif /* LED_POLARITY_INVERT */
	setGpioValue(irled, value);
	return 0;
}

static int get_irled_action(char *data, int data_len)
{
	char value[8]; /* true / false */
	json_object *json_obj;
	int ret = string_to_json(data, data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, "irLED", value) == 0) {
			return (strncmp(value, "true", 4) == 0) ? 1 : 0;
		} else {
			DBG("Error: Failed to find irLED\n");
			goto fail;
		}
	}
fail:
	return 0;
}

/* IRLED test */
#define IRLED_GPIO 56

void *tuya_irled_test(void *arg)
{
	Gpio *irled = NULL;
	int irled_action = 0;
	struct thread_info *info = (struct thread_info *)arg;
	char buf[TUYA_FRAME_MAX_LEN];

	irled = initGpio(IRLED_GPIO);
	if (irled == NULL) {
		fprintf(stderr, "Failed to init GPIO %d\n", IRLED_GPIO);
		goto fail;
	}

	setGpioDirection(irled, GPIO_DIR_OUT);

	irled_action = get_irled_action((char *)info->data, info->data_len);
	set_irled_lit(irled, irled_action);

	while (g_event_interrupt == info->cmd) {
		usleep(500000);
	}

	set_irled_lit(irled, 0);
	releaseGpio(irled);

	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));
	return 0;
}

/* IR cut switch test */

static const int ircut_gpio_id[2] = { 11, 10 };

static int get_ircut_times(char *data, unsigned int data_len)
{
	int value;
	json_object *json_obj;
	int ret = string_to_json(data, data_len, &json_obj);
	if (ret) {
		goto fail;
	} else {
		if (get_json_value_int(json_obj, "ircutSwitch", &value) == 0) {
			return value;
		} else {
			DBG("Error: Failed to find lightAction\n");
			goto fail;
		}
	}
fail:
	return -1;
}

void *tuya_ircut_test(void *arg)
{
#define IRCUT_PERIOD_US 300000

	int ircut_times, i, toggle;
	char buf[TUYA_FRAME_MAX_LEN];
	struct thread_info *info = (struct thread_info *)arg;

	ircut_times = get_ircut_times(info->data, info->data_len);
	if (ircut_times < 0) {
		fprintf(stderr, "Error: Failed to get IR cut switch times!");
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
		info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));
		goto fail;
	}
	DBG("IR cut switch times: %d\n", ircut_times);

	snprintf(buf, TUYA_FRAME_MAX_LEN, "/system/mpp/script/ir_cut.sh %d %d remove", ircut_gpio_id[0],
	         ircut_gpio_id[1]);
	system(buf);

	for (i = 0, toggle = 0; i < ircut_times; ++i) {
		if (toggle == 0) {
			snprintf(buf, TUYA_FRAME_MAX_LEN, "/system/mpp/script/ir_cut.sh %d %d active", ircut_gpio_id[0],
			         ircut_gpio_id[1]);
			toggle = 1;
		} else {
			snprintf(buf, TUYA_FRAME_MAX_LEN, "/system/mpp/script/ir_cut.sh %d %d remove", ircut_gpio_id[0],
			         ircut_gpio_id[1]);
			toggle = 0;
		}
		DBG("Toggle %d\n", i);
		system(buf);
		DBG("Toggle %d done \n", i);
		usleep(IRCUT_PERIOD_US);
	}

	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true}");
	info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));

fail:
	free(info);
	return NULL;
}

int tuya_pir_test(TuyaTestFrame *frame, char *buf)
{
	return 0;
}

void *tuya_rtsp_test(void *arg)
{
	//char rtsp_cmd[256];
	char buf[TUYA_FRAME_MAX_LEN];
	struct thread_info *info = (struct thread_info *)arg;

	//snprintf(rtsp_cmd, sizeof(rtsp_cmd), "/system/bin/testOnDemandRTSPServer 0 &");
	//system(rtsp_cmd);

	if (strcmp((char *)info->ip_addr, "0.0.0.0") == 0) {
		DBG("Error: Failed to get ip address\n");
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	} else {
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true, \"rtspUrl\":\"rtsp://%s:8554/live/0\"\n}", info->ip_addr);
	}
	info->put_cb(info->fd, (int)info->cmd, buf, strlen(buf));

	while (g_event_interrupt == info->cmd) {
		usleep(200000);
	}
	//snprintf(rtsp_cmd, sizeof(rtsp_cmd), "killall -2 testOnDemandRTSPServer");
	//system(rtsp_cmd);
	return NULL;
}

/* Return positive number on failure */
static int get_wifi_rssi(char * wlan, char * ssid)
{
	FILE *fp;
	char buf[256];
	int rssi = 1;
	snprintf(buf, sizeof(buf), "iwlist %s scan | grep \"SSID:\\\"%s\\\"\" -B2 | awk -F'[ =]' '{if (NR == 1) print $(NF-3)}'", wlan, ssid);

	fp = popen(buf, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run RSSI test command\n");
		goto fail;
	}

	if (fgets(buf, sizeof(rssi), fp) == NULL) {
		fprintf(stderr, "Failed to acquire RSSI from %s, SSID %s\n", wlan, ssid);
		goto fail;
	}

	sscanf(buf, "%d", &rssi);
	DBG("RSSI: %d\n", rssi);

fail:
	return rssi;
}

int tuya_wifi_rssi_test(TuyaTestFrame *frame, char *buf)
{
	char ssid[32];
	int rssi = 0;
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, "ssid", ssid) == 0) {
			DBG("SSID: %s\n", ssid);
		} else {
			goto fail;
		}
	}

	rssi = get_wifi_rssi(TUYA_MQTT_NETWORK_IF, ssid);

	if (rssi > 0) {
		fprintf(stderr, "Failed to get RSSI of SSID %s\n", ssid);
		goto fail;
	}

	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true, \"rssi\":%d\n}", rssi);
	return 0;
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}

int tuya_wifi_iperf_test(TuyaTestFrame *frame, char *buf)
{
	FILE *fp;
	char iperf_cmd[256];
	char thresh[32];
	int out_thresh;
	float bandwidth;
	json_object *json_obj;
	int ret = string_to_json((char *)frame->data, frame->data_len, &json_obj);
	if (ret) {
		fprintf(stderr, "Error: Failed to parse config string!");
		goto fail;
	} else {
		if (get_json_value_string(json_obj, "thresh", thresh) == 0) {
			DBG("thresh: %s\n", thresh);
		} else {
			goto fail;
		}
	}
	/*Wait for iperf server ready.*/
	usleep(2000000);
	snprintf(iperf_cmd, sizeof(iperf_cmd), "iperf3 -c %s -b %sM -t 15 -i 3 | awk '/sender/{print $5}'", g_client_addr, thresh);

	fp = popen(iperf_cmd, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run iperf test command\n");
		goto fail;
	}

	if (fgets(iperf_cmd, sizeof(iperf_cmd), fp) != NULL) {
		sscanf(iperf_cmd, "%f", &bandwidth);
	} else {
		fprintf(stderr, "Failed to acquire bandwidth from iperf\n");
		goto fail;
	}

	sscanf(thresh, "%d", &out_thresh);
	DBG("bandwidth: %f, thresh: %d\n", bandwidth, out_thresh);
	if (bandwidth > out_thresh) {
		snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":true, \"bandwidth\":%2.2f\n}", bandwidth);
		return 0;
	} else {
		fprintf(stderr, "Bandwidth(%2.2fMB/sec) is less than thresh(%sMB/sec)\n", bandwidth, thresh);
		goto fail;
	}
	
fail:
	snprintf(buf, TUYA_FRAME_MAX_LEN, "{\"ret\":false}");
	return 0;
}
