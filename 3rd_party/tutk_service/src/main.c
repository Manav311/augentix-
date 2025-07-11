#include <asm-generic/errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "linux/limits.h"

// Video src
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_osd.h"
#include "mpi_enc.h"
#include "mpi_dev.h"
#include "mpi_index.h"

#include "tutk_define.h"
#include "sample_constants.h"

#include "tutk_init.h"
#include "tutk_video.h"
#include "tutk_sdmonit.h"
#include "tutk_timesync.h"
#include "tutk_pair.h"
#include "log_define.h"
#include <json.h>

#include "agtx_lpw.h"

/** Augentix unique license key for TUTK Server*/
static const char license_key[] =
        "AQAAALK+NOrfekPlgtMxz85xgRjrsujgbGBscbUqEOfmWkJbKXyv4Po9vwN0n5o5E22E30kALk0onBfAVLSfOUYdNuR+8c4lCJSLjDCqAL1f/vMW1oIVcBXZYBvUm55+sVqg3nhqibgGmRNhSpAk0VY0JCI5x9uHubrB+8M/pPMhQ9puar9MqAbSqf/fhrPfeiFVJ6NBs8VMYNj9Adk8trRGMDIb";

//########################################################
//# Nebula configurations data
//########################################################
char gUdid[MAX_UDID_LENGTH + 1] = { 0 };
char gPinCode[MAX_PIN_CODE_LENGTH + 1] = { 0 };
char gSecretId[MAX_NEBULA_SECRETID_LENGTH + 1] = { 0 };

unsigned int gBindAbort = 0;
struct timeval gNoSessionTime = { 0 };
unsigned int gBindType = LOCAL_BIND;
char gProfilePath[PATH_MAX] = { 0 };
char gConfigPath[PATH_MAX] = { 0 };
TutkConfigs gConfigs;
NebulaJsonObject *gProfileJsonObj = NULL;
VSaaSContractInfo gVsaasContractInfo = { 0 };
bool gEnableVsaas = false;
bool gVsaasConfigExist = false;
bool gEnablePushNotification = false;
bool gSettingsExist = false;
bool gEnableWakeUp = true;
bool gProgressRun = false;
int gTime_ = 0;
lpw_handle gWifihd;

//########################################################
//# AV file path
//########################################################

char gVsaasInfoFilePath[] = "./vsaasinfo";
char gRecordFile[] = "frames";

AV_Client gClientInfo[MAX_CLIENT_NUMBER];

int gOnlineNum = 0;
int gP2Pstate = 0;

//########################################################
//# setting file path
//########################################################
char gDefaultSettingsFilePath[PATH_MAX] = { 0 };

//########################################################
//# identity file path
//########################################################
char gUserIdentitiesFilePath[PATH_MAX] = { 0 };

/** inlcude in TUTK_run*/
int StartAvServer(int sid, unsigned char chid, unsigned int timeout_sec, unsigned int resend_buf_size_kbyte,
                  int *stream_mode)
{
	struct st_SInfoEx session_info;

	AVServStartInConfig av_start_in_config;
	AVServStartOutConfig av_start_out_config;

	memset(&av_start_in_config, 0, sizeof(AVServStartInConfig));
	av_start_in_config.cb = sizeof(AVServStartInConfig);
	av_start_in_config.iotc_session_id = sid;
	av_start_in_config.iotc_channel_id = chid;
	av_start_in_config.timeout_sec = timeout_sec;
	av_start_in_config.password_auth = NULL;
	av_start_in_config.token_auth = NULL;
	av_start_in_config.server_type = SERVTYPE_STREAM_SERVER;
	av_start_in_config.resend = ENABLE_RESEND;
	av_start_in_config.token_delete = ExTokenDeleteCallBackFn;
	av_start_in_config.token_request = ExTokenRequestCallBackFn;
	av_start_in_config.identity_array_request = ExGetIdentityArrayCallBackFn;
	av_start_in_config.change_password_request = ExChangePasswordCallBackFn;
	av_start_in_config.ability_request = ExAbilityRequestFn;
	av_start_in_config.json_request = ExJsonRequestFn;

	if (ENABLE_DTLS)
		av_start_in_config.security_mode = AV_SECURITY_DTLS; // Enable DTLS, otherwise use AV_SECURITY_SIMPLE
	else
		av_start_in_config.security_mode = AV_SECURITY_SIMPLE;

	av_start_out_config.cb = sizeof(av_start_out_config);

	int av_index = avServStartEx(&av_start_in_config, &av_start_out_config);

	if (av_index < 0) {
		tutkservice_log_err("avServStartEx failed!! SID[%d] code[%d]!!!\n", sid, av_index);
		return -1;
	}
	session_info.size = sizeof(session_info);
	if (IOTC_Session_Check_Ex(sid, &session_info) == IOTC_ER_NoERROR) {
		char *mode[3] = { "P2P", "RLY", "LAN" };

		if (isdigit(session_info.RemoteIP[0]))
			tutkservice_log_err(
			        "Client is from[IP:%s, Port:%d] Mode[%s] VPG[%d:%d:%d] VER[%X] NAT[%d] AES[%d]\n",
			        session_info.RemoteIP, session_info.RemotePort, mode[(int)session_info.Mode],
			        session_info.VID, session_info.PID, session_info.GID, session_info.IOTCVersion,
			        session_info.LocalNatType, session_info.isSecure);
	}
	tutkservice_log_info("avServStartEx OK, SID[%d] avIndex[%d], resend[%d] two_way_streaming[%d] auth_type[%d]\n",
	                     sid, av_index, av_start_out_config.resend, av_start_out_config.two_way_streaming,
	                     av_start_out_config.auth_type);
	if (stream_mode) {
		*stream_mode = av_start_out_config.two_way_streaming;
	}
	avServSetResendSize(av_index, resend_buf_size_kbyte);

	return av_index;
}

static int parseUDID()
{
	FILE *fp = fopen(DEFAULT_UDID, "r");
	if (fp == NULL) {
		tutkservice_log_err("Can't find : %s UDID\n", DEFAULT_UDID);
		return -ENAVAIL;
	}

	fseek(fp, 0, SEEK_SET);
	fread(gUdid, MAX_UDID_LENGTH, 1, fp);
	printf("UDID: %s", gUdid);
	fclose(fp);

	return 0;
}

static int ParseInputOptions(int argc, char *argv[])
{
	int option = 0;
	while ((option = getopt(argc, argv, "u:f:b:hvwpi:")) > 0) {
		switch (option) {
		case 'u':
			strncpy(gUdid, optarg, sizeof(gUdid));
			gUdid[MAX_UDID_LENGTH] = '\0';
			printf("UDID %s\n", gUdid);
			break;
		case 'f':
			strncpy(gProfilePath, optarg, sizeof(gProfilePath));
			printf("Profile %s\n", gProfilePath);
			break;
		case 'b':
			if ((strlen(optarg) == strlen("l") && strncmp(optarg, "l", 1) == 0) ||
			    (strlen(optarg) == strlen("local") && strncmp(optarg, "local", strlen("local")) == 0)) {
				gBindType = LOCAL_BIND;
			} else if ((strlen(optarg) == strlen("s") && strncmp(optarg, "s", 1) == 0) ||
			           (strlen(optarg) == strlen("server") &&
			            strncmp(optarg, "server", strlen("server")) == 0)) {
				gBindType = SERVER_BIND;
			} else if ((strlen(optarg) == strlen("n") && strncmp(optarg, "n", 1) == 0) ||
			           (strlen(optarg) == strlen("none") && strncmp(optarg, "none", strlen("none")) == 0)) {
				gBindType = DISABLE_BIND;
			} else {
				tutkservice_log_err("Unknown optarg %s\n", optarg);
				return -1;
			}
			break;
		case 'v':
			gEnableVsaas = true;
			break;
		case 'p':
			gEnablePushNotification = true;
			break;
		case 'w':
			gEnableWakeUp = true;
			break;
		case 'i':
			strncpy(gConfigPath, optarg, sizeof(gConfigPath));
			printf("JSON config %s\n", gConfigPath);
			break;
		case 'h':
			return -1;
		default:
			tutkservice_log_err("Unknown option %c\n", option);
			return -1;
		}
	}
	if (strlen(gUdid) == 0 || strlen(gProfilePath) == 0 || strlen(gConfigPath) == 0) {
		tutkservice_log_err("Must specify UDID, profile, configs\n");
		return -1;
	}
	return 0;
}

static int parseConfigs()
{
	json_object *root = NULL;
	json_object *pwr_mgmt_child = NULL;
	json_object *detect_event_child = NULL;

	json_object *video_child = NULL;
	json_object *child = NULL;

	json_object *battery_level_child = NULL;
	json_object *low_battery_warning_child = NULL;

	json_object *adc_val_child = NULL;
	json_object *level_percentage_child = NULL;

	// Error is logged by json-c.
	root = (gConfigPath[0] != '\0') ? json_object_from_file(gConfigPath) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	/* power management configs */
	if (!json_object_object_get_ex(root, "pwr_mgmt", &pwr_mgmt_child)) {
		perror("pwr mgmt parameter is not found!\n");
		return -EINVAL;
	}

	if (json_object_object_get_ex(pwr_mgmt_child, "gpio", &child)) {
		gConfigs.pwr_mgmt.gpio = json_object_get_int(child);
	}

	if (json_object_object_get_ex(pwr_mgmt_child, "active_status", &child)) {
		gConfigs.pwr_mgmt.active_status = json_object_get_int(child);
	}

	if (json_object_object_get_ex(pwr_mgmt_child, "battery_adc_ch", &child)) {
		gConfigs.pwr_mgmt.battery_adc_ch = json_object_get_int(child);
	}

	if (json_object_object_get_ex(pwr_mgmt_child, "off_thre", &child)) {
		gConfigs.pwr_mgmt.off_thre = json_object_get_int(child);
	}

	if (json_object_object_get_ex(pwr_mgmt_child, "on_thre", &child)) {
		gConfigs.pwr_mgmt.on_thre = json_object_get_int(child);
	}

	/* video configs */
	if (!json_object_object_get_ex(root, "video", &video_child)) {
		perror("video parameter is not found!\n");
		goto parse_end;
	}

	if (json_object_object_get_ex(video_child, "chn", &child)) {
		gConfigs.video_chn = json_object_get_int(child);
	}

	/* sleep configs */
	if (json_object_object_get_ex(root, "no_session_timeout", &child)) {
		gConfigs.no_session_timeout = json_object_get_int(child);
	}

	if (json_object_object_get_ex(root, "sleep_timeout", &child)) {
		gConfigs.sleep_timeout = json_object_get_int(child);
	}

	if (json_object_object_get_ex(root, "sleep_response", &child)) {
		gConfigs.sleep_response = json_object_get_int(child);
	}

	/* PIR configs*/
	if (!json_object_object_get_ex(root, "detect_event", &detect_event_child)) {
		perror("detect_event_child parameter is not found!\n");
		goto parse_end;
	}

	if (json_object_object_get_ex(detect_event_child, "adc", &child)) {
		gConfigs.pir_adc.adc_chn = json_object_get_int(child);
	}

	if (json_object_object_get_ex(detect_event_child, "threshold", &child)) {
		gConfigs.pir_adc.thres = json_object_get_int(child);
	}

	if (json_object_object_get_ex(detect_event_child, "detect_event_gpio", &child)) {
		gConfigs.pir_adc.detect_event_gpio = json_object_get_int(child);
	}

	/* ADC battery level configs */
	if (!json_object_object_get_ex(root, "battery_level", &battery_level_child)) {
		perror("battery_level parameter is not found!\n");
		goto parse_end;
	}

	if (json_object_object_get_ex(battery_level_child, "low_battery_warning", &low_battery_warning_child)) {
		gConfigs.low_battery_warning = json_object_get_int(low_battery_warning_child);
	} else {
		gConfigs.low_battery_warning = DEFAULT_LOW_BATTERY_WARNING;
		perror("low_battery_warning is not found\n");
	}

	tutkservice_log_info("parse: %d low_battery_warning\n", gConfigs.low_battery_warning);

	if (json_object_object_get_ex(battery_level_child, "adc_val", &adc_val_child) &&
	    json_object_object_get_ex(battery_level_child, "level", &level_percentage_child)) {
		memset(&gConfigs.battery_level[0], 0x00, sizeof(gConfigs.battery_level));

		int val_cnt = json_object_array_length(adc_val_child);
		int level_cnt = json_object_array_length(level_percentage_child);
		if (level_cnt != val_cnt) {
			fprintf(stderr, "Warning: val_cnt:%d != level_cnt: %d\n", val_cnt, level_cnt);
		}

		int array_cnt = level_cnt < val_cnt ? level_cnt : val_cnt;

		if (array_cnt > MAX_LEVEL_CNT) {
			tutkservice_log_warn(" config array cnt:%d > %d\n", array_cnt, MAX_LEVEL_CNT);
			array_cnt = MAX_LEVEL_CNT;
		}

		for (int i = 0; i < array_cnt; i++) {
			gConfigs.battery_level[i].adc_val =
			        json_object_get_int(json_object_array_get_idx(adc_val_child, i));
			gConfigs.battery_level[i].level =
			        json_object_get_int(json_object_array_get_idx(level_percentage_child, i));
		}

	} else {
		perror("not found adc val or level_percentage array");
	}

parse_end:
	json_object_put(root);

	/*TODO*/
	printf("sleep: %d %d %d, pwr: %d %d %d %d %d, video: %d\n", gConfigs.no_session_timeout, gConfigs.sleep_timeout,
	       gConfigs.sleep_response, gConfigs.pwr_mgmt.gpio, gConfigs.pwr_mgmt.active_status,
	       gConfigs.pwr_mgmt.battery_adc_ch, gConfigs.pwr_mgmt.off_thre, gConfigs.pwr_mgmt.on_thre,
	       gConfigs.video_chn);

	return 0;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}

	gProgressRun = false;
}

//########################################################
//# Main function
//########################################################
int main(int argc, char *argv[])
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	int ret = TUTK_SDK_Set_License_Key(license_key);
	tutkservice_log_info("TUTK_SDK_Set_License_Key: %d\n", ret);
	if (ret < 0) {
		return -1;
	}

	/* Default active settings */
	snprintf(&gProfilePath[0], sizeof(gProfilePath), "%s", DEFAULT_DEVICE_PROFILE);
	snprintf(&gDefaultSettingsFilePath[0], sizeof(gDefaultSettingsFilePath), "%s", DEFAULT_DEVICE_SETTING);
	snprintf(&gUserIdentitiesFilePath[0], sizeof(gUserIdentitiesFilePath), "%s", DEFAULT_IDENTITY_FILE);

	/*Parse UDID form file*/
	parseUDID();

	/* Check command line options */
	ret = ParseInputOptions(argc, argv);
	if (ret < 0) {
		tutkservice_log_err("ParseInputOptions ret[%d]\n", ret);
		PrintUsage();
		return -1;
	}

	/*Parse JSON format*/
	parseConfigs();

	//Set log file path
	LogAttr log_attr;
	memset(&log_attr, 0, sizeof(log_attr));
	char log_path[] = "./device_log.txt";
	log_attr.log_level = LEVEL_VERBOSE;
	log_attr.path = log_path;

	ret = IOTC_Set_Log_Attr(log_attr);
	printf("IOTC_Set_Log_Attr ret[%d], path: %s\n", ret, log_attr.path);
	if (TUTK_pairingWiFi(gUdid) < 0) {
		TUTK_exeSystemCmd("reboot -f");
	}

	TUTK_setTimeZone("UTC-8");
	TUTK_timeSync();

	/* Initialization */
	TUTK_sd_monit_init();
	TUTK_initVideoSystem(gConfigs.video_chn);
	TUTK_run(gUdid, gProfilePath);

	/* De-initialization */
	TUTK_deInitVideoSystem();
	tutkservice_log_info("MPI system de-initialized.\n");

	return 0;
}
