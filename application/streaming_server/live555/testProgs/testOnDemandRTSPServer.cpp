/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2014, Live Networks, Inc.  All rights reserved
// A test program that demonstrates how to stream - via unicast RTP
// - various kinds of file on demand, using a built-in RTSP server.
// main program

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"

#include <iostream>

#include "auxdebug.hh"
#include <fstream>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#ifdef RTSP_SERVER_ENABLE_IVA
#include "avftr.h"
#include "avftr_conn.h"
#else
#endif

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_cmd.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>

#include "json.h"
#include "cc_data.h"

#include <cstdarg>
#include <alsa/error.h>
#include <alsa/asoundlib.h>
#include <pcm_interfaces.h>

using namespace std;
#ifdef RTSP_SERVER_ENABLE_IVA
int avftrUnxSktClientFD; //Unix Socket FD
int avftrResShmClientFD; //Shared memory for result FD
AVFTR_CTX_S *avftr_res_shm_client;
#else
#endif

/*for DISK dump function*/
int streamDump_init();
void diskUsageThread();
int DUMP_FLAG = 0;
FILE *xh264DumpFile = NULL;
char h264DumpFileName[48] = { 0 };
int SPLIT_FILE_SIZE = 0;
unsigned int DISK_MON_PERCENT = 98; // 2; ( this should be 100% - x )

/*for RTSP Server*/
#ifdef RTSP_SERVER_ENABLE_IVA
int g_avftr_conn = 1;
int g_avftr_dst_win = 0;
#else
#endif

MPI_WIN g_avftr_src_win = MPI_VIDEO_WIN(0,0,0);

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = true;
UsageEnvironment *gEnv;
TaskScheduler *gScheduler;
RTSPServer *gRtspServer = NULL;
UserAuthenticationDatabase *gAuthDB = NULL;

int gDftport[MPI_MAX_ENC_CHN_NUM] = { 9554, 7554, 6554, 5554, 4554, 3554, 2554, 1554 };
RtspServerConf gServerconf;

#define CC_JSON_KEY_CLIENT_NAME "name"
#define CC_JSON_KEY_MASTER_ID "master_id"
#define CC_JSON_KEY_CMD_ID "cmd_id"
#define CC_JSON_KEY_CMD_TYPE "cmd_type"
#define CC_JSON_KEY_RET_VAL "rval"
#define CC_JSON_STR_BUF_SIZE 30000

typedef struct {
	AGTX_INT32 master_id;
	AGTX_UINT32 cmd_id;
	AGTX_CMD_TYPE_E cmd_type;
	AGTX_INT32 rval;
} CC_COMMON_MSG_INFO_S;

static int g_master_id = 0;

static int send_all(int s, const char *buf, int len)
{
	int total = 0;
	int bytesleft = len;
	int n;

	while (total < len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == -1) {
			printf("Only %d bytes are sent\n", total);
			break;
		}

		total += n;
		bytesleft -= n;
	}

	return (n == -1) ? -1 : 0;
}

static void parse_common_msg_info(CC_COMMON_MSG_INFO_S *info, struct json_object *obj)
{
	struct json_object *tmp_obj = NULL;
	const char *str = NULL;

	if (json_object_object_get_ex(obj, CC_JSON_KEY_MASTER_ID, &tmp_obj)) {
		info->master_id = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(obj, CC_JSON_KEY_CMD_ID, &tmp_obj)) {
		info->cmd_id = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(obj, CC_JSON_KEY_CMD_TYPE, &tmp_obj)) {
		str = json_object_get_string(tmp_obj);

		if (str) {
			if (!strcmp(str, "notify")) {
				info->cmd_type = AGTX_CMD_TYPE_NOTIFY;
			} else if (!strcmp(str, "ctrl")) {
				info->cmd_type = AGTX_CMD_TYPE_CTRL;
			} else if (!strcmp(str, "set")) {
				info->cmd_type = AGTX_CMD_TYPE_SET;
			} else if (!strcmp(str, "get")) {
				info->cmd_type = AGTX_CMD_TYPE_GET;
			} else if (!strcmp(str, "reply")) {
				info->cmd_type = AGTX_CMD_TYPE_REPLY;
			} else {
				printf("Invalid cmd_type (%s)\n", str);
			}
		}
	}

	if (json_object_object_get_ex(obj, CC_JSON_KEY_RET_VAL, &tmp_obj)) {
		info->rval = json_object_get_int(tmp_obj);
	}

	return;
}

static struct json_object *validate_json_string(char *buf, int len)
{
	struct json_object *obj = NULL;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;

	/* Parse the buf */
	obj = json_tokener_parse_ex(tok, buf, len);

	/* Check parsing result */
	jerr = json_tokener_get_error(tok);
	if (jerr != json_tokener_success) {
		printf("JSON Tokener errNo: %d \n", jerr);
		printf("JSON Parsing errorer %s \n", json_tokener_error_desc(jerr));
	}

	json_tokener_free(tok);

	return obj;
}

int register_to_ccserver(int sockfd)
{
	int len;
	struct json_object *obj = NULL;
	char msg_buf[128] = { 0 };

	CC_COMMON_MSG_INFO_S msg_info;

	sprintf(msg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"CGI\" }",
	        AGTX_CMD_REG_CLIENT);

	len = strlen(msg_buf);

	/* Send message to ccserver for registration */
	if (send_all(sockfd, msg_buf, len)) {
		fprintf(stderr, "Write socket error\n");
	}

	/* Receive reply of registration from ccserver */
	len = recv(sockfd, msg_buf, 128, 0);
	msg_buf[len] = '\0';

	obj = validate_json_string(msg_buf, len);
	if (!obj) {
		fprintf(stderr, "Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);

	json_object_put(obj);

	if (msg_info.cmd_id != AGTX_CMD_REG_CLIENT || msg_info.rval != 0) {
		fprintf(stderr, "Failed to register to ccserver\n");
	}

	//printf("Succeed to register to ccserver\n");

	return 0;
}

int start_session_with_ccserver(int sockfd)
{
	int len;
	struct json_object *obj = NULL;
	char msg_buf[128] = { 0 };

	CC_COMMON_MSG_INFO_S msg_info;

	sprintf(msg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\" }", AGTX_CMD_SESS_START);

	len = strlen(msg_buf);

	/* Send message to ccserver to start session */
	if (send_all(sockfd, msg_buf, len)) {
		fprintf(stderr, "Write socket error\n");
	}

	/* Receive session (master) ID from ccserver */
	len = recv(sockfd, msg_buf, 128, 0);
	msg_buf[len] = '\0';

	obj = validate_json_string(msg_buf, len);
	if (!obj) {
		fprintf(stderr, "Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);

	json_object_put(obj);

	if (msg_info.cmd_id != AGTX_CMD_SESS_START || msg_info.rval != 0) {
		fprintf(stderr, "Failed to start session with ccserver\n");
	}

	g_master_id = msg_info.master_id;

	//printf("Start session with master_id: %d\n", g_master_id);

	return 0;
}

int read_audio_cmd_from_database(int sockfd, int cmd_id, const char *str, int *pcodec, int *psampleBits, int *pfrequency,
                           int *pgain)
{
	int len;
	struct json_object *obj = NULL;
	struct json_object *obj2 = NULL;
	char *msg_buf = (char *)malloc((size_t)CC_JSON_STR_BUF_SIZE);

	CC_COMMON_MSG_INFO_S msg_info;

	sprintf(msg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"get\" }", cmd_id);

	len = strlen(msg_buf);

	/* Send message to ccserver for registration */
	if (send_all(sockfd, msg_buf, len)) {
		fprintf(stderr, "Write socket error\n");
	}

	/* Receive reply of registration from ccserver */
	len = recv(sockfd, msg_buf, CC_JSON_STR_BUF_SIZE, 0);
	msg_buf[len] = '\0';

	obj = validate_json_string(msg_buf, len);
	if (!obj) {
		fprintf(stderr, "Not a valid json string\n");
	}

	parse_common_msg_info(&msg_info, obj);
	if (msg_info.rval != 0) {
		fprintf(stderr, "Failed to get setting from database\n");
	}

	/* delete following information */
	json_object_object_del(obj, CC_JSON_KEY_MASTER_ID);
	json_object_object_del(obj, CC_JSON_KEY_CMD_TYPE);
	json_object_object_del(obj, CC_JSON_KEY_RET_VAL);
	json_object_object_add(obj, CC_JSON_KEY_CMD_ID, json_object_new_string(str));

	/* Print JSON */
	//printf("%s", json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY));
	json_object_object_get_ex(obj, "enabled", &obj2);
	printf("enabled:%d\n", atoi(json_object_get_string(obj2)));

	if (atoi(json_object_get_string(obj2)) == 0) {
		fprintf(stderr, "ccserver disable audio\n");
		return -1;
	}

	int codec = SND_PCM_FORMAT_A_LAW;
	json_object_object_get_ex(obj, "codec", &obj2);

	if (0 == strcmp("ALAW", json_object_get_string(obj2))) {
		printf("codec:%s\n", json_object_get_string(obj2));
		memcpy(pcodec, &codec, sizeof(int));
	} else if (0 == strcmp("ULAW", json_object_get_string(obj2))) {
		printf("codec:%s\n", json_object_get_string(obj2));
		codec = SND_PCM_FORMAT_MU_LAW;
		memcpy(pcodec, &codec, sizeof(int));
	}

	json_object_object_get_ex(obj, "gain", &obj2);
	printf("gain:%d\n", atoi(json_object_get_string(obj2)));
	int gain = atoi(json_object_get_string(obj2));
	memcpy(pgain, &gain, sizeof(int));

	json_object_object_get_ex(obj, "sampling_bit", &obj2);
	printf("sampling_bit:%d\n", atoi(json_object_get_string(obj2)));
	int samplebit = atoi(json_object_get_string(obj2));
	memcpy(psampleBits, &samplebit, sizeof(int));

	json_object_object_get_ex(obj, "sampling_frequency", &obj2);
	printf("sampling_frequency:%d\n", atoi(json_object_get_string(obj2)));
	int freq = atoi(json_object_get_string(obj2));
	memcpy(pfrequency, &freq, sizeof(int));

	json_object_put(obj);
	free(msg_buf);

	return 0;
}


static int checkCcserverAvailable(int *pcodec, int *psampleBits, int *pfrequency, int *pgain, char* pchn)
{
	int sockfd;
	int servlen;
	struct sockaddr_un serv_addr;
	const char *socket_path = "/tmp/ccUnxSkt";

	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, socket_path);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "failed to create socket\n");
		*pcodec = (int)SND_PCM_FORMAT_A_LAW;
		*psampleBits = (int)2;
		*pfrequency = (int)8000;
		*pgain = (int)0;
		return 0;
	}

	/* Connect to socket file created by ccserver */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		fprintf(stderr, "failed to connect ccserver, use default conf\n");
		*pcodec = (int)SND_PCM_FORMAT_A_LAW;
		*psampleBits = (int)2;
		*pfrequency = (int)8000;
		*pgain = (int)0;
		return 0;
	} else {
		printf("ccserver exist\n");
	}
	//read out from ccserver db
	register_to_ccserver(sockfd);
	start_session_with_ccserver(sockfd);

	if (read_audio_cmd_from_database(sockfd, AGTX_CMD_AUDIO_CONF, "AGTX_CMD_AUDIO_CONF", pcodec, psampleBits, pfrequency, pgain) < 0) {
		return -1;
	}
	close(sockfd);
	return 0;
}

static int checkAlsaAudioDevice()
{
	char **hints;
	int err = snd_device_name_hint(-1, "pcm", (void ***)&hints);
	if (err != 0)
		return -1;

	char **n = hints;
	printf("Find Audio Device:");
	while (*n != NULL) {
		char *name = snd_device_name_get_hint(*n, "NAME");

		if (name != NULL && 0 != strcmp("null", name)) {
			//Copy name to another buffer and then free it
			printf("%s\n", name);
			free(name);
		}
		n++;
	}
	snd_device_name_free_hint((void **)hints);

	if (n > 0)
		return 0;
	else
		return -1;
}

static void handle_sig_int(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else {
		perror("Unexpected signal!\n");
		exit(1);
	}
#ifdef RTSP_SERVER_ENABLE_IVA
	if (g_avftr_conn) {
		if (AVFTR_exitClient(&avftrResShmClientFD, &avftrUnxSktClientFD, &avftr_res_shm_client)) {
			printf("Exit AVFTR client failed\n");
			//exit(1);
		}
	}
#endif
	/*RTSP server graceful close, incluse TEARDown & close source class*/
	Medium::close(gRtspServer);
	exit(0);
}

static int isDriveMounted(const char *driveMntPoint)
{
	int driveAvailable = 0;
	struct mntent *mountEntry;
	FILE *fp = setmntent("/proc/mounts", "r");
	do {
		mountEntry = getmntent(fp);
		if (mountEntry != NULL) {
			driveAvailable = !(strcmp(mountEntry->mnt_dir, driveMntPoint));
		}
	} while ((!driveAvailable) && (mountEntry != NULL));

	if (fp != NULL) {
		fclose(fp);
	}
	return driveAvailable;
}

void dumpH264Stream2File(int sig)
{ // sig : 1 = open file, 2 = stop dump , 3 =  close file
	time_t tt;
	struct tm *ti;
	printf("Dump Stream to file :\n");

	if (sig == 1) {
		if ((DUMP_FLAG == 0)) {
			if (isDriveMounted("/mnt/sdcard")) { // Set DUMP_FLAG only if  /mnt/sdcard is mounted
				time(&tt);
				ti = localtime(&tt);
				bzero(h264DumpFileName, 48);
				sprintf(h264DumpFileName, "/mnt/sdcard/%d-%d-%d_%d-%d-%d.h264", (ti->tm_year + 1900),
				        ti->tm_mon, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec);
				fprintf(stderr, "=>OpenFile = %s  \n", h264DumpFileName);
				if ((xh264DumpFile = fopen(h264DumpFileName, "wb")) == NULL) {
					fprintf(stderr, "Error Opening Stream Dump file : Check your SD-Card \n");
					DUMP_FLAG = 0; // Set the Flag to 1 on openfile stream
				} else {
					DUMP_FLAG = 1; // Set the Flag to 1 on openfile stream
				}
			} else {
				fprintf(stderr, "=> SD Card is not mounted \n");
			}
		} else if (DUMP_FLAG > 0) {
			fprintf(stderr, "Dump File may be aleady open: %d \n", DUMP_FLAG);
		}
	}
	if (sig == 2) {
		if (DUMP_FLAG == 1) { // DUMP_FLAG =2 => inform to close file till next iFrame
			DUMP_FLAG = 2;
		}
	}
	if (sig == 3) {
		if (DUMP_FLAG == 3) { // DUMP_FLAG =2 => inform to close file till next iFrame
			fprintf(stderr, "=>Close File = %s  \n", h264DumpFileName);
			fflush(xh264DumpFile);
			fsync(fileno(xh264DumpFile));
			fclose(xh264DumpFile);
			bzero(h264DumpFileName, 48); // = {0};
			DUMP_FLAG = 0;
		}
	}
}

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms, char const *streamName,
                           char const *inputFileName); // fwd

// static char newDemuxWatchVariable;
int split(char **arr, char *str, const char *del)
{
	char *s = strtok(str, del);
	int loop = 0;
	while (s != NULL) {
		*arr++ = s;
		s = strtok(NULL, del);
		loop++;
	}

	return loop;
}
void help()
{
	fprintf(stderr, "Usage: \r\n"
	                "\t<chn_idx> [options] \r\n"
	                "or\r\n"
	                "\t[-p <port>] -s \"<streamNames>\" [options]\n\r\n\r\n"
	                "default mode:\t<chn_idx> [options]\n\r\n"
	                "manual mode:\t[-p <port>] -s \"<streamNames>\" [options]\n"
	                "\t-p <port>, dft:554\n"
	                "\t-s \"<urls:live/<chn>,liveaudio/<chn>,...>\", split by , or ;\n"
	                "\t live/<number> for streaming without sound\n"
	                "\t liveaudio/<number> for streaming with sound\n"
	                "\t don't set duplicated streamname!\r\n"
	                "Options:\n"
	                "\t-n NO SEI\n"
	                "\t-S *Force* SEI\n"
	                "\t-o <IVA idx>\n"
					"\t-b force get video frame in blocking mode\n"
	                "\t(default h264 has SEI, h265 no SEI)\r\n");
}

void reUrlCb(void *arg)
{
	unsigned char chn = *((unsigned char *)arg);
	printf("trigger event, re-URL: %d\r\n", chn);

	int ret = 0;
	int audioFormat, sampleBit, frequency, gain;
	char const *descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";

	if (gRtspServer == NULL) {
		fprintf(stderr, "No rtsp server available\r\n");
		return;
	}


	for (int i = 0; i < gServerconf.streamCount; i++) {
		/*if DftMode (only i chn in server), in this RTSP Server process only that chn get signal
		 and if ManualMode (many chn streaming in 1 RTSP server), the first chn get ENC restart will trigger this */
		gRtspServer->deleteServerMediaSession(gServerconf.Urls[i]);
		((ServerMediaSession *)gServerconf.sms[i])->deleteAllSubsessions();

		gServerconf.sms[i] = ServerMediaSession::createNew(*gEnv, gServerconf.Urls[i],
															gServerconf.Urls[i], descriptionString);

		/*always get ccserver data*/
		ret = checkCcserverAvailable(&audioFormat, &sampleBit, &frequency, &gain, &gServerconf.channels[i]);
		((ServerMediaSession *)gServerconf.sms[i])
				->addSubsession(DeviceServerMediaSubsession::createNew(*gEnv, reuseFirstSource,
																		gServerconf.channels[i]));

		/*Audio subsession*/
		if (strncmp(gServerconf.Urls[i], "liveaudio", 9) == 0) {
			if ((ret == 0) && (checkAlsaAudioDevice() == 0)) {
				((ServerMediaSession *)gServerconf.sms[i])
						->addSubsession(AudioDeviceServerMediaSubsession::createNew(
								*gEnv, reuseFirstSource, gServerconf.channels[i], audioFormat,
								sampleBit, frequency, gain));
				printf("re-add %s, A+V\r\n", gServerconf.Urls[i]);
			}
		} else {
			printf("re-add %s, only V\r\n", gServerconf.Urls[i]);
		}

		gRtspServer->addServerMediaSession((ServerMediaSession *)gServerconf.sms[i]);
		announceStream(gRtspServer, (ServerMediaSession *)gServerconf.sms[i], gServerconf.Urls[i],
						"re-Add");
	}
#ifdef RTSP_SERVER_ENABLE_IVA
	if (g_avftr_conn > 0) {
		if (AVFTR_exitClient(&avftrResShmClientFD, &avftrUnxSktClientFD, &avftr_res_shm_client)) {
			fprintf(stderr, "Exit AVFTR client failed\n");
			return;
		}

		while (AVFTR_initClient(&avftrResShmClientFD, &avftrUnxSktClientFD, &avftr_res_shm_client)) {
			fprintf(stderr, "Wait for AVFTR server ready.\n");
			sleep(1);
		}
	}
#else
#endif

	sleep(3);
}

int main(int argc, char **argv)
{
	if (2 > argc) {
		help();
		return -1;
	}

	// Begin by setting up our usage environment:
	if (signal(SIGINT, handle_sig_int) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		return -1;
	}

	if (signal(SIGTERM, handle_sig_int) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		return -1;
	}

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("-> Cannot handle SIGPIPE!\n");
		return -1;
	}

	gServerconf.launchMode = ManualMode;
	gServerconf.streamCount = 0;
	gServerconf.portNum = 0;
	gServerconf.isBlocking = false;

	/*how to check arg[1] is number but > 3 or < 0?*/

	/*select RTSP server mode*/
	if ((strncmp("0", argv[1], 1) == 0) || (strncmp("1", argv[1], 1) == 0) || (strncmp("2", argv[1], 1) == 0) ||  (strncmp("3", argv[1], 1) == 0)
		|| (strncmp("4", argv[1], 1) == 0) || (strncmp("5", argv[1], 1) == 0) || (strncmp("6", argv[1], 1) == 0) || (strncmp("7", argv[1], 1) == 0)) {
		gServerconf.launchMode = DftMode;
		gServerconf.streamCount = 2;

		sprintf(&gServerconf.Urls[0][0], "live/%s", argv[1]);
		gServerconf.channels[0] = atoi(argv[1]);

		sprintf(&gServerconf.Urls[1][0], "liveaudio/%s", argv[1]);
		gServerconf.channels[1] = atoi(argv[1]);

		if (1 == atoi(argv[1])) {
			streamDump_init();
		}
	}

	char streamNames[255];
	char *streamList[MAX_STREAM_CNT];
	char tmpName[2][16];
	char threadName[24] = { 0 };
	int c = 0;
	const char *short_options = "p:s:Sno:d:i:bh";
	while ((c = getopt_long(argc, (char**)argv, short_options, NULL, NULL)) != -1) {
		switch (c) {
#ifdef RTSP_SERVER_ENABLE_IVA
			case 'S':
				g_avftr_conn = 2;
				break;
			case 'n':
				g_avftr_conn = 0;
				break;
			case 'o':
				g_avftr_dst_win = atoi(optarg);
				break;
			case 'i':
				g_avftr_src_win = MPI_VIDEO_WIN(0, (uint8_t)atoi(argv[optind - 1]), (uint8_t)atoi(argv[optind]));
				break;
#else
			case 'S':
			case 'n':
			case 'o':
			case 'i':
				fprintf(stderr, "build config disabled SEI!!\n");
				help();
				return 0;
#endif
			case 'p':
				gServerconf.portNum = atoi(optarg);
				break;
			case 's':
				{
					if (gServerconf.launchMode == DftMode) {
						fprintf(stderr, "Default mode can't select port number\n");
						help();
						return 0;
					}
					sprintf(&streamNames[0], optarg, strlen(optarg));
					printf("stream:%s\r\n", streamNames);

					/*split stream name*/
					int times;
					char *del = (char *)",";
					if (strstr(streamNames, ",") != NULL && strstr(streamNames, ";") != NULL) {
						fprintf(stderr, "invalid dupliacted spliters\r\n");
						return -1;
					}

					if (strstr(streamNames, ",") == NULL && strstr(streamNames, ";") == NULL) {
					} else if (strstr(streamNames, ",") != NULL) {
						del = (char *)",";
					} else {
						del = (char *)";";
					}

					times = split(streamList, &streamNames[0], del);
					printf("get steam number: %d\n", times);
					if (times == 0) {
						fprintf(stderr, "Invaild stream name items: %s\r\n", streamNames);
						return -1;
					}

					gServerconf.streamCount = times;

					for (int i = 0; i < gServerconf.streamCount; i++) {
						for (int j = 0; j < gServerconf.streamCount; j++) {
							/*check same stream name*/
							if ((strcmp(streamList[i], streamList[j]) == 0) && (i != j)) {
								fprintf(stderr, "Find duplicate stream name %s %s\r\n", streamList[i],
										streamList[j]);
								return -1;
							}
						}

						sprintf(&gServerconf.Urls[i][0], "%s", streamList[i]);
						sscanf(&gServerconf.Urls[i][0], "%[^/]/%[^/]", tmpName[0], tmpName[1]);
						gServerconf.channels[i] = atoi(tmpName[1]);

						/*check invalid stream name*/
						if ((strncmp(tmpName[0], "live", 4) != 0) && (strncmp(tmpName[0], "liveaudio", 9) != 0)) {
							fprintf(stderr, "Invaild streamName in list[%s]", gServerconf.Urls[i]);
							return -1;
						}
					}

					break;
				}
				
			case 'b':
				{
					gServerconf.isBlocking = true;
					if (gServerconf.launchMode == DftMode) {
						/*if dftmode isBlocking, only support live/<chn>*/
						gServerconf.streamCount = 1;
					}
					break;
				}
			case 'h':
				help();
				return 0;
			default:
				help();
				return 0;
		}
	}

	if (gServerconf.portNum == 0 ) {
		if (gServerconf.launchMode == DftMode) {
			gServerconf.portNum = gDftport[gServerconf.channels[0]];
		} else {
			gServerconf.portNum = 554;
		}
	}

	/*blocking mode has only 1 URL*/
	if (gServerconf.isBlocking == true && ((gServerconf.streamCount > 1))) {
		fprintf(stderr, "blocking mode only support live/<chn>,\n only 1 video chn & no audio\n");
		return -ENAVAIL;
	}
	/*blocking mode no support audio*/
	if (gServerconf.isBlocking == true  && (strncmp(&gServerconf.Urls[0][0], "liveaudio", 9) == 0)) {
		fprintf(stderr, "blocking mode only support live/<chn>,\n only 1 video chn & no audio\n");
		return -ENAVAIL;
	}

	sscanf(&gServerconf.Urls[0][0], "%[^/]/%[^/]", tmpName[0], tmpName[1]);
	/*set thread name as rtsp_<1st URL chn>*/
	snprintf(&threadName[0], 16, "rtsp_%s", tmpName[1]);

	if (pthread_setname_np(pthread_self(), threadName) == 0) {
		puts("Set ThreadName:[Done]");
	} else {
		puts("Set ThreadName:[Fail]");
	}

	gScheduler = EpollTaskScheduler::createNew();
	gEnv = BasicUsageEnvironment::createNew(*gScheduler);
#ifdef RTSP_SERVER_ENABLE_IVA
	printf("create ENV, %d, %d, src: %d-%d\r\n", g_avftr_conn, g_avftr_dst_win, g_avftr_src_win.chn, g_avftr_src_win.win);
#else
    printf("create ENV, (NO IVA)\n");
#endif
#ifdef ACCESS_CONTROL
	// To implement client access control to the RTSP server, do the following:
	gAuthDB = new UserAuthenticationDatabase;
	gAuthDB->addUserRecord("username1",
	                       "password1"); // replace these with real strings
// Repeat the above with each <username>, <password> that you wish to allow
// access to the server.
#endif

	gRtspServer = RTSPServer::createNew(*gEnv, gServerconf.portNum, gAuthDB);
	if (gRtspServer == NULL) {
		fprintf(stderr, "failed to open RTSP Server\r\n");
		return -1;
	}

	char const *descriptionString = "Session streamed by \"testOnDemandRTSPServer\"";
	int audioFormat, sampleBit, frequency, gain;
	if (gServerconf.streamCount == 1 ||
	    ((gServerconf.streamCount == 2) && (gServerconf.channels[0] == gServerconf.channels[1]))) {
		gServerconf.launchMode = DftMode;
		/* only one chn , so can use blocking code*/
		printf("Manual mode switch to DftMode, since only one video chn: %d\r\n", gServerconf.channels[0]);
	}
	for (int i = 0; i < gServerconf.streamCount; i++) {
		gServerconf.sms[i] = ServerMediaSession::createNew(*gEnv, gServerconf.Urls[i], gServerconf.Urls[i],
		                                                   descriptionString);
		/*always get ccserver data*/
		checkCcserverAvailable(&audioFormat, &sampleBit, &frequency, &gain, &gServerconf.channels[i]);

		((ServerMediaSession *)gServerconf.sms[i])
		        ->addSubsession(DeviceServerMediaSubsession::createNew(*gEnv, reuseFirstSource,
		                                                               gServerconf.channels[i]));
		if (strstr(gServerconf.Urls[i], "liveaudio") != NULL) {
			if (checkAlsaAudioDevice() == 0) {
				((ServerMediaSession *)gServerconf.sms[i])
				        ->addSubsession(AudioDeviceServerMediaSubsession::createNew(
				                *gEnv, reuseFirstSource, gServerconf.channels[i], audioFormat,
				                sampleBit, frequency, gain));
			} else {
				fprintf(stderr, "Failed to add audio subsession, [%d]%s\r\n", i, gServerconf.Urls[i]);
			}
		}
		gRtspServer->addServerMediaSession(((ServerMediaSession *)gServerconf.sms[i]));
		if (strstr(gServerconf.Urls[i], "liveaudio") != NULL) {
			announceStream(gRtspServer, ((ServerMediaSession *)gServerconf.sms[i]), gServerconf.Urls[i],
			               "A+V stream");
		} else {
			announceStream(gRtspServer, ((ServerMediaSession *)gServerconf.sms[i]), gServerconf.Urls[i],
			               "V stream");
		}
	}
#ifdef RTSP_SERVER_ENABLE_IVA
	if (g_avftr_conn) {
		while (AVFTR_initClient(&avftrResShmClientFD, &avftrUnxSktClientFD, &avftr_res_shm_client)) {
			printf("Wait for AVFTR server ready.\n");
			sleep(1);
		}
	}
#else
#endif

	gServerconf.chnRestartTrigger = gScheduler->createEventTrigger(reUrlCb);
	gEnv->taskScheduler().doEventLoop(); // does not return

	return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms, char const *streamName,
                           char const *inputFileName)
{
	char *url = rtspServer->rtspURL(sms);
	UsageEnvironment &env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream, from the file \"" << inputFileName << "\"\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

static int isRootDir(const char *name)
{
	if (name == NULL || name[0] == '\0')
		return 1;
	else if (name[0] == '.') {
		if (name[1] == '\0')
			return 1;
		else if (name[1] == '.' && name[2] == '\0')
			return 1;
	}
	return 0;
}
static void deleteOldestfile(const char *dirPath)
{
	struct dirent *entry;
	time_t oldtime = time(NULL); // use current time as start
	struct stat statbuf;
	char oldestFile[1024];
	DIR *dir = opendir(dirPath);
	char tmpStr[1024] = { 0 };

	while (NULL != (entry = readdir(dir))) {
		if (!isRootDir(entry->d_name)) {
			sprintf(tmpStr, "%s%s", dirPath, entry->d_name);
			stat(tmpStr, &statbuf);

			if (statbuf.st_mtime < oldtime) {
				oldtime = statbuf.st_mtime;
				bzero(oldestFile, 1024);
				sprintf(oldestFile, "%s%s", dirPath, entry->d_name);
			}
			bzero(tmpStr, 1024);
		}
	}
	closedir(dir);
	/* delete the oldestFileName */
	fprintf(stderr, ">> Del File:%s = %d\n", oldestFile, unlink(oldestFile));
	bzero(oldestFile, 1024);
	// fprintf(stderr, ">>>>file to del %s :: %d\n",oldestFile,
	// remove(oldestFile));
}
int diskUsage(void *data)
{
	// pthread_t tid;
	struct statfs vfs;

	//const char *th_name = (const char *)data;
	char th_name[16] = "diskUsage"; // max. length is 16
	printf("\tUpdate thread scheduling policy...\n");
	while (SPLIT_FILE_SIZE) {
		sleep(30);
		if (statfs("/mnt/sdcard", &vfs) != 0) {
			fprintf(stderr, "statfs: fail to get disk stats\n");
		}
		if ((vfs.f_bavail * 100) / (vfs.f_blocks) < DISK_MON_PERCENT) {
			fprintf(stderr, " Free Space: remove oldest files \n");
			// system("rm -f /mnt/sdcard/`ls -t /mnt/sdcard|tail -1`");
			deleteOldestfile("/mnt/sdcard/");
		}
	}
	return 0;
}

int streamDump_thread(void *data)
{
	// pthread_t tid;
	const char *socket_path = "/tmp/auxDump";
	struct sockaddr_un addr;
	char buf[128];
	int fd, cl, rc;
	int split_size = 0;

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (*socket_path == '\0') {
		*addr.sun_path = '\0';
		strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);
	} else {
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
		unlink(socket_path);
	}

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("bind error");
		exit(-1);
	}

	if (listen(fd, 5) == -1) {
		perror("listen error");
		exit(-1);
	}

	while (1) {
		if ((cl = accept(fd, NULL, NULL)) == -1) {
			perror("accept error");
			continue;
		}
		while ((rc = read(cl, buf, sizeof(buf))) > 0) {
			printf("read %u bytes: %s\n", rc, buf);
			if (strstr(buf, "start")) {
				dumpH264Stream2File(1); // Start Dump
			}
			if (strstr(buf, "stop")) {
				dumpH264Stream2File(2); // Stop Dump till next Iframe
			}
			if (strstr(buf, "close")) {
				dumpH264Stream2File(3); // Close the Dump file
			}
			if (strstr(buf, "size=")) {
				char *tmpStr = strtok(buf, "size=");
				split_size = atoi(tmpStr);
				if ((DUMP_FLAG == 0) ||
				    (DUMP_FLAG == 1)) { // split file should always start with DUMP_FLAG=0
					// Thread to monitor disk space
					if (SPLIT_FILE_SIZE == 0) {
						SPLIT_FILE_SIZE = split_size * 1024; // To bytes
						diskUsageThread();
					} else if (split_size ==
					           -1) { // change of dump file size in middle is not allowed
						fprintf(stderr, "stopping Split dump \n");
						SPLIT_FILE_SIZE = 0;
					}
				} // else { fprintf(stderr,"Stop current file dump before starting Split
				// dump \n"); }
			}
			if (strstr(buf, "quit")) {
				rc = 0;
				break;
			}
			sprintf(buf, ">> %d ", DUMP_FLAG);
			write(cl, buf, rc);
			bzero(buf, 128);
		}
		if (rc == -1) {
			perror("read");
			exit(-1);
		} else if (rc == 0) {
			printf("EOF\n");
			close(cl);
		}
	}
	return 0;
}

int streamDump_init()
{
	char name[16] = "streamDump";
	char tmpname[16] = { 0 };
	pthread_t pid;
	int rc = 0;

	if (pthread_create(&pid, NULL, (void *(*)(void *))streamDump_thread, name)) {
		printf("streamDump_thread,  fail\n");
		return -1;
	}
	rc = pthread_setname_np(pid, name);
	sleep(1);
	if (rc == 0) {
		if (pthread_getname_np(pid, tmpname, sizeof(tmpname)) == 0) {
			printf("%s thread create [Done]", tmpname);
		} else {
			printf("%s thread create [Fail]", name);
		}
	}
	return 0;
}

void diskUsageThread()
{
	char name[16] = "diskUsage"; // max. length is 16
	char tmpname[16] = { 0 };
	pthread_t pid;
	int rc = 0;

	if (pthread_create(&pid, NULL, (void *(*)(void *))diskUsage, name)) {
		printf("diskUsage_thread,  fail\n");
	}
	rc = pthread_setname_np(pid, name);
	sleep(1); //
	if (rc == 0) {
		if (pthread_getname_np(pid, tmpname, sizeof(tmpname)) == 0) {
			printf("%s thread create [Done]", tmpname);
		} else {
			printf("%s thread create [Fail]", name);
		}
	}
}
