#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <json.h>
#include <errno.h>

#include "log_define.h"
#include "audio.h"

typedef enum { S16LE, ALAW, MULAW } MimeType;

typedef struct {
	int id;
	char msg_type[32];
	MimeType type;
	uint8_t *data;
	int length;
} PCM_MESSAGE;

static const char *g_device[] = { "default", "hw:0,0" }; /* sound device */
int g_run_flag = 0;
static int fd;
static char *buffer;

int freeParseHandle(json_object *config_obj)
{
	json_object_put(config_obj);
	return 0;
}

int parsePCMMessage(char *file_name, PCM_MESSAGE *msg)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	char dataBuf[64];

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		flv_server_log_err("Cannot open %s", file_name);
		goto end;
	}

	json_object_object_get_ex(test_obj, "id", &tmp_obj);
	msg->id = json_object_get_int(tmp_obj);
	flv_server_log_info("Date: %d", msg->id);

	json_object_object_get_ex(test_obj, "type", &tmp_obj);
	sprintf(&(msg->msg_type[0]), "%s", json_object_get_string(tmp_obj));
	flv_server_log_info("type: %s", msg->msg_type);

	json_object_object_get_ex(test_obj, "mimeType", &tmp_obj);
	sprintf(&dataBuf[0], "%s", json_object_get_string(tmp_obj));
	flv_server_log_info("mime type: %s", dataBuf);

	json_object_object_get_ex(test_obj, "length", &tmp_obj);
	msg->length = json_object_get_int(tmp_obj);
	flv_server_log_info("len: %d\r\n", msg->length);

	msg->data = malloc(msg->length);
	json_object_object_get_ex(test_obj, "data", &tmp_obj);
	if (!tmp_obj) {
		flv_server_log_err("failed to get pcm data");
		free(msg->data);
		goto end;
	}
	memcpy(msg->data, json_object_get_string(tmp_obj), msg->length);

	freeParseHandle(test_obj);
	return 0;
end:
	freeParseHandle(test_obj);
	return -EINVAL;
}

void help()
{
	printf("Usage: playback audio in JSON from ws client\r\n");
	printf("-i json path\r\n");
	printf("-j json analysis, else pcm playback\r\n");
	printf("-h help()\r\n");
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		printf("pls re-start flv-recorder\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}

	g_run_flag = 0;
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		return -1;
	}

	char *file = NULL;
	snd_pcm_uframes_t frame = 256;
	snd_pcm_sframes_t rframes;
	snd_pcm_sframes_t wframes;
	snd_pcm_t *p_capture;
	int dsize = 0;
	int gain = 25;
	int err;
	int c;
	unsigned int buf_sz = 256;
	unsigned int format, channel, rate, stream;
	int bytes_per_sample = 2;

	format = SND_PCM_FORMAT_S16_LE;
	channel = 1;
	rate = 8000;
	stream = SND_PCM_STREAM_PLAYBACK;
	//int size = channel * bytes_per_sample * buf_sz;
	int size = 0;

	bool isJson = false;

	while ((c = getopt(argc, argv, "i:hj")) != -1) {
		switch (c) {
		case 'i':
			file = optarg;
			stream = SND_PCM_STREAM_PLAYBACK;
			break;
		case 'h':
			help();
			return 0;
		case 'j':
			isJson = true;
			break;
		default:
			help();
			exit(1);
		}
	}

	if (!file) {
		flv_server_log_err("Please specify a input and output file.");
		return -EPERM;
	}

	if (isJson == true) {
		char *addr = strstr(file, ".json");
		if (addr == NULL) {
			flv_server_log_err("failed to find %s.", file);
			return -EPERM;
		}
	}

	flv_server_log_debug("try open file %s.", file);
	fd = open(file, O_RDONLY);
	if (fd <= 0) {
		flv_server_log_err("failed to open src. %s", stderror(errno));
		agtxPcmUninit(p_capture);
		return -ENODEV;
	}

	printf("%s %s %s\r\n", strstr(file, ".raw"), strstr(file, "g711a"), strstr(file, "g711u"));
	if (isJson) {
		PCM_MESSAGE msg = { 0 };
		parsePCMMessage(file, &msg);

		err = agtxPcmInitNoTs(&p_capture, g_device[0], stream, format, frame, rate, channel);
		if (err < 0) {
			flv_server_log_err("pcm init failed");
			g_run_flag = 0;
			return -ENODEV;
		}

		err = agtxSetGain(gain);
		if (err < 0) {
			flv_server_log_err("set_gain failed: %s", snd_strerror(err));
			g_run_flag = 0;
		}

		size = channel * bytes_per_sample * buf_sz;
		wframes = buf_sz;
		int remain_size = msg.length;
		int idx = 0;

		while (remain_size > 0) {
			rframes = snd_pcm_writei(p_capture, &msg.data[idx], wframes);
			if (rframes == -EPIPE) {
				/* EPIPE means xrun */
				flv_server_log_err("Underrun occrred: %s", snd_strerror(err));
				snd_pcm_prepare(p_capture);
			} else if (rframes < 0) {
				flv_server_log_err("error from write: %s", snd_strerror(err));
			} else if (rframes != wframes) {
				flv_server_log_err("Short write (expected %d, wrote %d)", (int)wframes, (int)rframes);
			}

			remain_size -= size;
			idx += size;
		}

		free(msg.data);
		return 0;
	} else {
		g_run_flag = 1;
		if (NULL != strstr(file, ".raw")) {
			bytes_per_sample = 2;
			format = SND_PCM_FORMAT_S16_LE;
		} else if (!strstr(file, "g7lla")) {
			bytes_per_sample = 1;
			format = SND_PCM_FORMAT_A_LAW;
			flv_server_log_debug("format alaw");
		} else if (!strstr(file, "g7llu")) {
			bytes_per_sample = 1;
			format = SND_PCM_FORMAT_MU_LAW;
			flv_server_log_debug("format mulaw");
		} else {
			flv_server_log_err("failed to get pcm format");
			return -ENODEV;
		}
	}

	err = agtxPcmInitNoTs(&p_capture, g_device[0], stream, format, frame, rate, channel);
	if (err < 0) {
		flv_server_log_err("pcm init failed");
		g_run_flag = 0;
		return -ENODEV;
	}

	err = agtxSetGain(gain);
	if (err < 0) {
		flv_server_log_err("set_gain failed: %s", snd_strerror(err));
		g_run_flag = 0;
	}

	size = channel * bytes_per_sample * buf_sz;
	buffer = (char *)malloc(size);
	wframes = buf_sz;

	while (((dsize = read(fd, buffer, size)) > 0) && (g_run_flag == 1)) {
	retry_writei:
		rframes = snd_pcm_writei(p_capture, buffer, wframes);
		if (rframes == -EPIPE) {
			/* EPIPE means xrun */
			flv_server_log_err("Underrun occrred: %s", snd_strerror(err));
			snd_pcm_prepare(p_capture);
			goto retry_writei;
		} else if (rframes < 0) {
			flv_server_log_err("error from write: %s", snd_strerror(err));
		} else if (rframes != wframes) {
			flv_server_log_err("Short write (expected %d, wrote %d)", (int)wframes, (int)rframes);
		}
	}

	flv_server_log_info("try close file");
	free(buffer);
	close(fd);

	err = agtxPcmUninit(p_capture);

	return 0;
}