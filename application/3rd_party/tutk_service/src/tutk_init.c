#include "inc/tutk_define.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/statfs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>

#include "tutk_define.h"
#include "sample_constants.h"

#include "tutk_init.h"
#include "tutk_video.h"
#include "tutk_audio.h"
#include "tutk_adc.h"
#include "tutk_pir.h"
#include "tutk_querylist.h"
#include "agtx_lpw.h"
#include "log_define.h"
#include "tutk_record.h"

//########################################################
//# Nebula configurations data
//########################################################
extern char gUdid[MAX_UDID_LENGTH + 1];
extern char gPinCode[MAX_PIN_CODE_LENGTH + 1];
extern char gSecretId[MAX_NEBULA_SECRETID_LENGTH + 1];
extern lpw_handle gWifihd;
extern TutkConfigs gConfigs;

extern unsigned int gBindAbort;
extern struct timeval gNoSessionTime;
extern unsigned int gBindType;
extern char gProfilePath[128];
extern NebulaJsonObject *gProfileJsonObj;
extern VSaaSContractInfo gVsaasContractInfo;
extern bool gEnableVsaas;
extern bool gVsaasConfigExist;
extern bool gEnablePushNotification;
extern bool gSettingsExist;
extern bool gEnableWakeUp;
extern bool gProgressRun;
unsigned int gPushNotificationAbort = 0;
NebulaDeviceCtx *gDeviceCtx = NULL;
extern unsigned int ginterpolation_level;

//########################################################
//# AV file path
//########################################################

extern char gVsaasInfoFilePath[];
extern char gRecordFile[];
extern int gOnlineNum;
extern int gP2Pstate;

extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];

//########################################################
//# setting file path
//########################################################
extern char gDefaultSettingsFilePath[];

//########################################################
//# identity file path
//########################################################
extern char gUserIdentitiesFilePath[];

//########################################################
//# Callback functions for avServStartEx()
//########################################################
int ExTokenDeleteCallBackFn(int av_index, const char *identity)
{
	//not implement in this sample
	(void)(av_index);
	(void)(identity);
	return 0;
}

int ExTokenRequestCallBackFn(int av_index, const char *identity, const char *identity_description, char *token,
                             unsigned int token_length)
{
	//not implement in this sample
	(void)(av_index);
	(void)(identity);
	(void)(identity_description);
	(void)(token);
	(void)(token_length);
	return 0;

	return 0;
}

void ExGetIdentityArrayCallBackFn(int av_index, avServSendIdentityArray send_identity_array)
{
	//not implement in this sample
	(void)(av_index);
	(void)(send_identity_array);
}

int ExChangePasswordCallBackFn(int av_index, const char *account, const char *old_password, const char *new_password,
                               const char *new_iotc_authkey)
{
	//not implement in this sample
	(void)(av_index);
	(void)(account);
	(void)(old_password);
	(void)(new_password);
	(void)(new_iotc_authkey);
	return 0;
}

void ExAbilityRequestFn(int av_index, avServSendAbility send_ability)
{
	//not implement in this sample
	(void)(av_index);
	(void)(send_ability);
}

//########################################################
//# settings_change_handler callback function
//########################################################
int SettingsChangeHandle(NebulaDeviceCtx *device, const char *settings)
{
	(void)(device);
	FILE *fp = NULL;
	fp = fopen(gDefaultSettingsFilePath, "w+");
	if (fp) {
		fwrite(settings, 1, strlen(settings), fp);
		fclose(fp);
		fp = NULL;
		printf("[%s] get new settings, save to %s success\n", __func__, gDefaultSettingsFilePath);
	} else {
		tutkservice_log_err("[%s] get new settings, save to %s fail\n", __func__, gDefaultSettingsFilePath);
	}
	gSettingsExist = true;
	return 0;
}

int DeviceLoginStateHandle(NebulaDeviceCtx *device, NebulaDeviceLoginState state)
{
	(void)(device);
	printf("in %s\n", __func__);
	printf("P2P login state[%d]\n", state);

	gP2Pstate = state;
	return 0;
}

int ExJsonRequestFn(int av_index, const char *func, const NebulaJsonObject *json_args, NebulaJsonObject **response)
{
	int ret = 0, value = 0, status_code = 0;
	const NebulaJsonObject *json_value = NULL;
	printf("ExJsonRequestFn %s\n", func);
	int sid = GetSidFromAvIndex(av_index);

	printf("av_index = %d.....\n", av_index);
	printf("SID = %d.....\n", sid);

	if (sid < 0) {
		tutkservice_log_err("No coresponding SID for index:%d !!", av_index);
		return 400;
	}

	if (strcmp(func, "startVideo") == 0) {
		ret = Nebula_Json_Obj_Get_Sub_Obj(json_args, "value", &json_value);
		if (ret != NEBULA_ER_NoERROR || json_value == NULL) {
			tutkservice_log_err("Unable to get value object\n");
			return 400;
		}

		ret = Nebula_Json_Obj_Get_Bool(json_value, &value);
		if (ret == NEBULA_ER_NoERROR) {
			printf("func is [%s] value is [%d]\n", func, value);
			if (value == 1) {
				RegEditClientToVideo(sid, av_index);
			} else {
				UnRegEditClientFromVideo(sid);
			}
			status_code = 200;
		} else {
			tutkservice_log_err("Unable to get correct value\n");
			status_code = 400;
		}
	} else if (strcmp(func, "startAudio") == 0) {
		ret = Nebula_Json_Obj_Get_Sub_Obj(json_args, "value", &json_value);
		if (ret != NEBULA_ER_NoERROR || json_value == NULL) {
			tutkservice_log_err("Unable to get value object\n");
			return 400;
		}

		ret = Nebula_Json_Obj_Get_Bool(json_value, &value);
		if (ret == NEBULA_ER_NoERROR) {
			printf("func is [%s] value is [%d]\n", func, value);
			if (value == 1) {
				RegEditClientToAudio(sid, av_index);
				AV_Client *client_info = search_session_av_index(av_index);

				pthread_t ThreadAudioFrameData_ID = 0;
				if ((ret = pthread_create(&ThreadAudioFrameData_ID, NULL, thread_AudioFrameData,
				                          (void *)client_info))) {
					tutkservice_log_err("pthread_create ret=%d\n", ret);
					return -1;
				}
				pthread_detach(ThreadAudioFrameData_ID);
			} else {
				UnRegEditClientFromAudio(sid);
			}
			status_code = 200;
		} else {
			tutkservice_log_err("Unable to get correct value\n");
			status_code = 400;
		}
	} else if (strcmp(func, "playbackControl") == 0) {
		const NebulaJsonObject *json_ctrl = NULL;
		const NebulaJsonObject *json_filename = NULL;
		Nebula_Json_Obj_Get_Sub_Obj(json_args, "ctrl", &json_ctrl);
		Nebula_Json_Obj_Get_Sub_Obj(json_args, "fileName", &json_filename);

		if (json_ctrl == NULL || json_filename == NULL) {
			tutkservice_log_err("Unable to get playbackControl object!!\n");
			return 400;
		}

		int ctrl_value;
		Nebula_Json_Obj_Get_Int(json_ctrl, &ctrl_value);
		const char *filename = NULL;
		Nebula_Json_Obj_Get_String(json_filename, &filename);
		printf("func is [%s] value[%d] file[%s]\n", func, ctrl_value, filename);
		/** ThreadRecordFileData */
		ret = HandlePlaybackControl(sid, ctrl_value, filename);
		if (ret != NEBULA_ER_NoERROR) {
			status_code = 400;
		} else {
			status_code = 200;
		}
	} else if (strcmp(func, "getCameraCapability") == 0) {
		const char json[] =
		        "{\"channels\":[{\"protocols\":[\"iotc-av\"],\"channelId\":0,\"lens\":{\"type\":\"normal\"},\"video\":{\"codecs\":[\"h264\"],\"averageBitrates\":[100000],\"presets\":[{\"name\":\"1080p\",\"codec\":\"h264\",\"averageBitrate\":100000,\"resolution\":\"1920×1080\"}]},\"audio\":{\"presets\":[{\"name\":\"pcm_8000_16_1\",\"codec\":\"pcm\",\"sampleRate\":8000,\"bitsPerSample\":16,\"channelCount\":1}]},\"speaker\":{\"presets\":[{\"name\":\"speaker_1\",\"codec\":\"pcm\",\"sampleRate\":8000,\"bitsPerSample\":16,\"channelCount\":1}]}},{\"protocols\":[\"iotc-av\"],\"channelId\":1,\"lens\":{\"type\":\"normal\"},\"video\":{\"codecs\":[\"h264\"],\"averageBitrates\":[100000],\"presets\":[{\"name\":\"720p\",\"codec\":\"h264\",\"averageBitrate\":100000,\"resolution\":\"1280×720\"},{\"name\":\"1080p\",\"codec\":\"h264\",\"averageBitrate\":500000,\"resolution\":\"1920x1080\"}]}}]}";

		ret = Nebula_Json_Obj_Create_From_String(json, response);
		printf("create getCameraCapability response[%d]\n", ret);
		if (ret != 0) {
			status_code = 400;
		} else {
			status_code = 200;
		}
	} else if (strcmp(func, "startSpeaker") == 0) {
		int enable_speaker = 0;
		Nebula_Json_Obj_Get_Sub_Obj_Bool(json_args, "value", &enable_speaker);

		ret = HandleSpeakerControl(sid, enable_speaker);

		if (ret != 0) {
			status_code = 400;
			printf("Error, can't get data\n");
		} else {
			printf("Speaker value is %d\n", enable_speaker);
			status_code = 200;
		}

	} else {
		tutkservice_log_err("av_index[%d], recv unknown function request[%s]\n", av_index, func);
		status_code = 400;
	}

	return status_code;
}

//########################################################
//# identity_handler callback function
//########################################################
void IdentityHandle(NebulaDeviceCtx *device, const char *identity, char *psk, unsigned int psk_size)
{
	(void)(device);
	int ret = GetPskFromFile(identity, gUserIdentitiesFilePath, psk, psk_size);
	if (ret != 200) {
		tutkservice_log_err("[%s] get psk fail\n", __func__);
	}
}

//########################################################
//#  Initialize / Deinitialize client list of AV server
//########################################################
void InitAVInfo()
{
	int i;
	for (i = 0; i < MAX_CLIENT_NUMBER; i++) {
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].av_index = -1;
		gClientInfo[i].enable_record_video = PLAYBACK_STOP;
		pthread_rwlock_init(&(gClientInfo[i].lock), NULL);
	}
}

void DeInitAVInfo()
{
	int i;
	for (i = 0; i < MAX_CLIENT_NUMBER; i++) {
		pthread_rwlock_destroy(&gClientInfo[i].lock);
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].av_index = -1;
	}
}

//########################################################
//# command_handler callback function
//########################################################
int CommandHandle(NebulaDeviceCtx *device, const char *identity, const char *func, const NebulaJsonObject *json_args,
                  NebulaJsonObject **json_response)
{
	(void)(device);

	int ret = 0;
	char resp[1024] = { 0 };
	NebulaJsonObject *response = NULL;

	printf("arg %s\n", Nebula_Json_Obj_To_String(json_args));
	if (strcmp("getNightMode", func) == 0) {
		// The profile or document might describe getNightMode as
		// {
		//   "func":"getNightMode",
		//   "return": {
		//     "value":"Int"
		//   }
		// }
		// When the value of night mode is 10, please make a JSON response as { "value": 10 }
		// There is no need to add key "content" here
		int night_mode = 0;
		sprintf(resp, "{\"value\":%d}", night_mode);
		printf("%s: send response[%s]\n", __func__, resp);

		ret = Nebula_Json_Obj_Create_From_String(resp, &response);

		if (response == NULL) {
			tutkservice_log_err("%s: Invalid response error ret [%d]!!\n", __func__, ret);
			return 400;
		}

		*json_response = response;
		return 200;
	} else if (strcmp(func, "queryEventList") == 0) {
		const NebulaJsonObject *json_ctrl;
		tutkservice_log_err("%s \n", Nebula_Json_Obj_To_String(json_args));
		Nebula_Json_Obj_Get_Sub_Obj(json_args, "startTime", &json_ctrl);
		int starttime = 0;
		ret = Nebula_Json_Obj_Get_Int(json_ctrl, &starttime);

		Nebula_Json_Obj_Get_Sub_Obj(json_args, "endTime", &json_ctrl);
		int endtime = 0;
		Nebula_Json_Obj_Get_Int(json_ctrl, &endtime);

		Nebula_Json_Obj_Get_Sub_Obj(json_args, "eventType", &json_ctrl);
		const char *eventType = NULL;
		Nebula_Json_Obj_Get_String(json_ctrl, &eventType);

		printf("startTime[%d], endtime[%d], eventType[%s]\n", starttime, endtime, eventType);
		//List the file name that match the search criteria.
		NebulaJsonObject *response = NULL;
#if 0

		Nebula_Json_Obj_Get_Sub_Obj(json_args, "listNumber", &json_ctrl);
		int listnum = 0;
		Nebula_Json_Obj_Get_Int(json_ctrl, &listnum);

		/** don't know if have order or not*/
		ret = Nebula_Json_Obj_Create_From_String(TUTK_querylist_build_response(starttime, listnum, 1),
		                                         &response);
#else
		GetRecordFileList(starttime, endtime, eventType, &response);
		printf("Resp[%s]\n", Nebula_Json_Obj_To_String(response));
#endif
		if (response == NULL) {
			tutkservice_log_err("%s: Empty response error!!\n", __func__);
			return 400;
		}
		*json_response = response;
		return 200;
	} else if (strcmp("X_Demo_getBatteryPercentage", func) == 0) {
		if (gWifihd == (lpw_handle)NULL) {
			gWifihd = lpw_open();
		}

		int adc_val = 0;
		int level_idx = 0;
		int interpolation_level = 0;
		int report_level[5] = { 100, 80, 60, 40, 20 };
		int report = report_level[0];

		adc_val = lpw_adc_get(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch);
		level_idx = sortAdcLevelIdx(adc_val);
		interpolation_level = getAdcLevelInterpolation(adc_val, level_idx);

		for (int i = 0; i < sizeof(report_level) / sizeof(report_level[0]); i++) {
			if (interpolation_level > report_level[0]) {
				report = report_level[0];
				break;
			}

			if (interpolation_level < report_level[4]) {
				report = report_level[4];
				break;
			}

			if (interpolation_level < report_level[i] && interpolation_level > report_level[i + 1]) {
				report = report_level[i + 1];
				break;
			}
		}

		sprintf(resp, "{\"value\":%d}", report);
		printf("%s: send response[%s]\n", __func__, resp);

		tutkservice_log_info("Response ADC[%d] value: %d= %d%%.\n", gConfigs.pwr_mgmt.battery_adc_ch, adc_val,
		                     interpolation_level);

		ret = Nebula_Json_Obj_Create_From_String(resp, &response);

		if (response == NULL) {
			printf("%s: Invalid response error ret [%d]!!\n", __func__, ret);
			return 400;
		}

		*json_response = response;
		return 200;
	} else if (strcmp(func, "getStorageInfo") == 0) {
		const NebulaJsonObject *json_value = NULL;
		const char *value = NULL;
		ret = Nebula_Json_Obj_Get_Sub_Obj(json_args, "value", &json_value);
		if (ret != NEBULA_ER_NoERROR || json_value == NULL) {
			printf("Unable to get value object\n");
			return 400;
		}

		Nebula_Json_Obj_Get_String(json_value, &value);
		printf("func is [%s] value[%s]\n", func, value);

		int totalSize = 0;
		int freeSize = 0;
		struct statfs sd_fs;

		if (access("/sys/block/mmcblk0/size", F_OK) != 0) {
			totalSize = 0;
			freeSize = 0;
		} else {
			if (statfs(SD_MOUNT_PATH, &sd_fs) != 0) {
				printf("statfs failed!/n");
				totalSize = 0;
				freeSize = 0;
				return 400;
			}
			totalSize =
			        (int)(((unsigned long long)sd_fs.f_blocks * (unsigned long long)sd_fs.f_bsize) >> 20);
			freeSize = (int)(((unsigned long long)sd_fs.f_bfree * (unsigned long long)sd_fs.f_bsize) >> 20);
		}

		sprintf(resp, "{\"totalSize\":%d,\"freeSize\":%d}", totalSize, freeSize);
		printf("command_handle: send response[%s]\n", resp);

		ret = Nebula_Json_Obj_Create_From_String(resp, &response);
		if (response == NULL) {
			printf("command_handle: Invalid response error ret [%d]!!\n", ret);
			return 400;
		}

		*json_response = response;
		return 200;

	} else if (strcmp(func, "createCredential") == 0) {
		const char *user_identity = NULL;
		const char *mode = NULL;
		char *credential = NULL;

		if (strcmp(identity, "admin") != 0) {
			tutkservice_log_err("[%s] %s is not from admin ,return 403(Forbidden)\n", __func__, func);
			return 403;
		}

		ret = Nebula_Json_Obj_Get_Sub_Obj_String(json_args, "identity", &user_identity);
		if (ret != NEBULA_ER_NoERROR) {
			tutkservice_log_err("[%s] identity is not in %s ,return 400(Bad Request)\n", __func__, func);
			return 400;
		}

		ret = Nebula_Json_Obj_Get_Sub_Obj_String(json_args, "createMode", &mode);
		if (ret != NEBULA_ER_NoERROR) {
			tutkservice_log_err("[%s] createMode is not in %s ,return 400(Bad Request)\n", __func__, func);
			return 400;
		}
		ret = GetCredential(gUdid, gSecretId, user_identity, mode, gUserIdentitiesFilePath, &credential);
		if (ret == 200) {
			sprintf(resp, "{\"credential\":\"%s\"}", credential);
			printf("Resp[%s]\n", resp);
			free(credential);
			Nebula_Json_Obj_Create_From_String(resp, &response);
			*json_response = response;
		}
		return ret;
	} else if (strcmp(func, "deleteCredential") == 0) {
		const char *user_identity = NULL;

		if (strcmp(identity, "admin") != 0) {
			tutkservice_log_err("[%s] %s is not from admin ,return 403(Forbidden)\n", __func__, func);
			return 403;
		}

		ret = Nebula_Json_Obj_Get_Sub_Obj_String(json_args, "identity", &user_identity);
		if (ret != NEBULA_ER_NoERROR) {
			tutkservice_log_err("[%s] identity is not in %s ,return 400(Bad Request)\n", __func__, func);
			return 400;
		}

		return DeleteCredential(user_identity, gUserIdentitiesFilePath);
	} else if (strcmp(func, "getAllIdentities") == 0) {
		char *identities_json_str = NULL;

		if (strcmp(identity, "admin") != 0) {
			tutkservice_log_err("[%s] %s is not from admin ,return 403(Forbidden)\n", __func__, func);
			return 403;
		}

		ret = GetAllIdentitiesFromFile(gUserIdentitiesFilePath, &identities_json_str);
		if (ret == 200) {
			tutkservice_log_err("Resp[%s]\n", identities_json_str);
			Nebula_Json_Obj_Create_From_String(identities_json_str, &response);
			free(identities_json_str);
			*json_response = response;
		}
		return ret;
	} else if (strcmp(func, "setUnbindDevice") == 0) {
		if (access(BIND_FLAG, F_OK) == 0) {
			tutkservice_log_err("del %s\n", BIND_FLAG);
			remove(BIND_FLAG);
		} else {
			tutkservice_log_err("can't found %s\n", BIND_FLAG);
		}

		gBindType = LOCAL_BIND;

		lpw_tcp_disconnect(gWifihd);
		system("reboot -f");

	} else {
		strcpy(resp, "{\"error\":\"unknow func\"}");
		ret = Nebula_Json_Obj_Create_From_String(resp, json_response);
		tutkservice_log_err("[%s] unknown func : %s[%d]\n", __func__, func, ret);
	}
	return 400;
}

int getNoSessionTime()
{
	struct timeval now;
	gettimeofday(&now, NULL);

	return now.tv_sec - gNoSessionTime.tv_sec;
}

int ShouldDeviceGoToSleep()
{
	struct timeval now;
	gettimeofday(&now, NULL);

	if ((access(KEEP_ALIVE_FILE, F_OK) != 0) &&
	    (now.tv_sec - gNoSessionTime.tv_sec > gConfigs.no_session_timeout)) {
		return 1;
	}

	return 0;
}

void ResetNoSessionTime()
{
	gettimeofday(&gNoSessionTime, NULL);
}

void UpdateNoSessionTime()
{
	if (gOnlineNum > 0) {
		ResetNoSessionTime();
	}
}

//########################################################
//# Get Timestamp
//########################################################
unsigned int GetTimeStampMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

unsigned int GetTimeStampSec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec);
}

void PrepareWakeupDataBeforeSleep(NebulaDeviceCtx *device_ctx, NebulaSocketProtocol nebula_protocol,
                                  char *wakeup_pattern, NebulaWakeUpData **data, unsigned int *data_count)
{
	int ret = 0;
	NebulaSleepConfig config;
	memset(&config, 0, sizeof(config));
	struct timeval now = { 0 };
	gettimeofday(&now, NULL);

	sprintf(wakeup_pattern, "WakeMeUp%u", (unsigned int)now.tv_usec);

	config.cb = sizeof(NebulaSleepConfig);
	config.wake_up_pattern = (unsigned char *)wakeup_pattern;
	config.pattern_size = strlen(wakeup_pattern);
	config.protocol = nebula_protocol;
	config.alive_interval_sec = 0; // Set 0 for default values. UDP: 25 secs, TCP: 90 secs
	config.disable_tcp_keep_alive = 0; // Enable tcp keep alive
	config.tcp_keep_alive_sec = 0; // set 0 for default values
	config.enable_tcp_reconnect = 0; // Diaable tcp reconnect

	do {
		ret = Nebula_Device_Get_Sleep_PacketEx(device_ctx, &config, data, data_count, 10000);
	} while (ret != NEBULA_ER_NoERROR);
}

#define EVENT_LEN 4 // count of enum: gpio_event_t
#define EVENT_TRUE_CHAR '1'
static gpio_event_t strToGpioEvent(char *event)
{
	gpio_event_t gpio_event = 0;
	int shift_cnt = 0;

	for (int i = 0; i < EVENT_LEN; i++) {
		shift_cnt = (EVENT_LEN - i - 1);
		if (EVENT_TRUE_CHAR == event[i]) {
			gpio_event = (gpio_event | (0x01 << shift_cnt));
		}
	}
	return gpio_event;
}

void WaitForBatteryChargeSleeping(void)
{
	int ret = lpw_pm_init(gWifihd, gConfigs.pwr_mgmt.gpio, gConfigs.pwr_mgmt.active_status,
	                      gConfigs.pwr_mgmt.battery_adc_ch, gConfigs.pwr_mgmt.off_thre, gConfigs.pwr_mgmt.on_thre);

	ret = lpw_gpio_set_dir(gWifihd, gConfigs.pwr_mgmt.gpio, GPIO_OUTPUT, gConfigs.pwr_mgmt.active_status);
	if (ret < 0) {
		tutkservice_log_err(" Power control GPIO[%d] set direction and value fail.\n", gConfigs.pwr_mgmt.gpio);
	}

	ret = lpw_sleep_detect_adc_en(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch, getAdcValueInterpolation(45),
	                              1 /*greater equal*/);

	tutkservice_log_info("Battery level reach 1 percent, Now safe sleeping");
	ret = lpw_sleep_start(gWifihd, 0, gConfigs.sleep_response);
	if (ret < 0) {
		tutkservice_log_err("Sleep start fail.\n");
	}
}

void WaitForWakeupPatternWhenSleeping(NebulaSocketProtocol nebula_protocol, char *wakeup_pattern,
                                      NebulaWakeUpData *data, unsigned int data_count, unsigned int interpolation_level)
{
	printf("Device going to sleep, data count: %d\n", data_count);

	int ret = lpw_pm_init(gWifihd, gConfigs.pwr_mgmt.gpio, gConfigs.pwr_mgmt.active_status,
	                      gConfigs.pwr_mgmt.battery_adc_ch, gConfigs.pwr_mgmt.off_thre, gConfigs.pwr_mgmt.on_thre);
	tutkservice_log_info("Sleep init success. Power control GPIO[%d]\n", gConfigs.pwr_mgmt.gpio);

	ret = lpw_gpio_set_dir(gWifihd, gConfigs.pwr_mgmt.gpio, GPIO_OUTPUT, gConfigs.pwr_mgmt.active_status);
	if (ret < 0) {
		tutkservice_log_err(" Power control GPIO[%d] set direction and value fail.\n", gConfigs.pwr_mgmt.gpio);
	}

	/* connect to the server via TCP */
	in_addr_t ser_ip;
	inet_pton(AF_INET, data[0].ip, &ser_ip);
	while (1) {
		ret = lpw_tcp_connect_to(gWifihd, ser_ip, data[0].port);
		if (ret != 0) {
			tutkservice_log_err("Connect server: %d:%d fail.\n", ser_ip, data[0].port);
			sleep(1);
			continue;
		}
		break;
	}

	tutkservice_log_info("Connect server success.\n");

	/*low battery enable*/
	int warning_level = 0;
	warning_level = parseWarningAdcLevel(interpolation_level);
	//warnig_adc_val = getAdcValueInterpolation(warning_level);

	if (warning_level <= 5) {
		if (interpolation_level > 5) {
			ret = lpw_sleep_detect_adc_en(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch,
			                              getAdcValueInterpolation(warning_level), 0 /*under*/);
		} else {
			ret = lpw_sleep_detect_adc_en(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch,
			                              getAdcValueInterpolation(2), 0 /*under*/);
		}
	} else {
		ret = lpw_sleep_detect_adc_en(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch,
		                              getAdcValueInterpolation(warning_level), 0 /*under*/);
	}

	if (ret < 0) {
		tutkservice_log_err("Enable ADC detection fail. %d\n", ret);
	}

	tutkservice_log_info("enable ADC < %d wakeup\n", warning_level);

	/* PIR wakeup */
	ret = lpw_sleep_detect_adc_en(gWifihd, gConfigs.pir_adc.adc_chn, gConfigs.pir_adc.thres, 1 /*above*/);
	if (ret < 0) {
		tutkservice_log_err("Enable PIR detection fail. %d\n", ret);
	}

	tutkservice_log_info("detect PIR ADC[%d] > %d wakeup\n", gConfigs.pir_adc.adc_chn, gConfigs.pir_adc.thres);

	tcp_wake_t tcp_wake_pattern;
	memset(&tcp_wake_pattern, 0x00, sizeof(tcp_wake_t));

	/* tcp awake*/
	{
		tcp_wake_pattern.keep_alive_pack_len = data[0].packet_size;
		tcp_wake_pattern.keep_alive_period = 1;
		tcp_wake_pattern.keep_alive_pack = data[0].sleep_alive_packet;
		tcp_wake_pattern.wake_pack = (unsigned char *)wakeup_pattern;
		tcp_wake_pattern.wake_pack_len = strlen(wakeup_pattern);
		/* if tcp disconnect while sleeping, DUT wakeup and re-connect*/
		tcp_wake_pattern.bad_conn_handle = 1;

		while (1) {
			ret = lpw_sleep_tcp_wake_en(gWifihd, &tcp_wake_pattern);
			if (ret < 0) {
				tutkservice_log_err("Enable TCP detection fail. %d\n", ret);
				continue;
			}

			break;
		}

		tutkservice_log_info("Enable TCP detection success. wakeup patern:%s\n",
		                     (unsigned char *)wakeup_pattern);
	}

	ret = lpw_sleep_start(gWifihd, gConfigs.sleep_timeout, gConfigs.sleep_response);
	if (ret < 0) {
		tutkservice_log_err("Sleep start fail.\n");
	}
}

void PrintUsage()
{
	printf("#########################################################################\n");
	printf("./Nebula_Device_AV [options]\n");
	printf("[options]\n");
	printf("[M]\t-i [config]\t\t\te.g agt_ma300027_v1.json\n");
	printf("[O]\t-u [UDID]\t\t\tConfig UDID.\n");
	printf("[O]\t-f [profile]\t\t\tConfig profile path.\n");
	printf("[O]\t-b [l(ocal)|s(erver)|n(one)]\tBinding mode. Bind in local, bind through TUTK bind server or do not bind. Default is server.\n");
	printf("[O]\t-v \t\t\t\tEnable VSaaS server pull stream from device. Disabled by default.\n");
	printf("[O]\t-w \t\t\t\tDemo wakeup. Device will go to sleep if the process is idled 10 secs. Disabled by default.\n");
	printf("[O]\t-p \t\t\t\tEnable push notification to APP. Disabled by default.\n");
	printf("[O]\t-h \t\t\t\tShow usage\n");
}

//########################################################
//# Print error message
//########################################################
void PrintErrHandling(int error)
{
	switch (error) {
	case IOTC_ER_MASTER_NOT_RESPONSE:
		//-60 IOTC_ER_MASTER_NOT_RESPONSE
		printf("[Error code : %d]\n", IOTC_ER_MASTER_NOT_RESPONSE);
		printf("Master server doesn't respond.\n");
		printf("Please check the network wheather it could connect to the Internet.\n");
		break;
	case IOTC_ER_SERVER_NOT_RESPONSE:
		//-1 IOTC_ER_SERVER_NOT_RESPONSE
		printf("[Error code : %d]\n", IOTC_ER_SERVER_NOT_RESPONSE);
		printf("P2P Server doesn't respond.\n");
		printf("Please check the network wheather it could connect to the Internet.\n");
		break;
	case IOTC_ER_FAIL_RESOLVE_HOSTNAME:
		//-2 IOTC_ER_FAIL_RESOLVE_HOSTNAME
		printf("[Error code : %d]\n", IOTC_ER_FAIL_RESOLVE_HOSTNAME);
		printf("Can't resolve hostname.\n");
		break;
	case IOTC_ER_ALREADY_INITIALIZED:
		//-3 IOTC_ER_ALREADY_INITIALIZED
		printf("[Error code : %d]\n", IOTC_ER_ALREADY_INITIALIZED);
		printf("Already initialized.\n");
		break;
	case IOTC_ER_FAIL_CREATE_MUTEX:
		//-4 IOTC_ER_FAIL_CREATE_MUTEX
		printf("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_MUTEX);
		printf("Can't create mutex.\n");
		break;
	case IOTC_ER_FAIL_CREATE_THREAD:
		//-5 IOTC_ER_FAIL_CREATE_THREAD
		printf("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_THREAD);
		printf("Can't create thread.\n");
		break;
	case IOTC_ER_UNLICENSE:
		//-10 IOTC_ER_UNLICENSE
		printf("[Error code : %d]\n", IOTC_ER_UNLICENSE);
		printf("This UID is unlicense.\n");
		printf("Check your UID.\n");
		break;
	case IOTC_ER_NOT_INITIALIZED:
		//-12 IOTC_ER_NOT_INITIALIZED
		printf("[Error code : %d]\n", IOTC_ER_NOT_INITIALIZED);
		printf("Please initialize the IOTCAPI first.\n");
		break;
	case IOTC_ER_TIMEOUT:
		//-13 IOTC_ER_TIMEOUT
		break;
	case IOTC_ER_INVALID_SID:
		//-14 IOTC_ER_INVALID_SID
		printf("[Error code : %d]\n", IOTC_ER_INVALID_SID);
		printf("This SID is invalid.\n");
		printf("Please check it again.\n");
		break;
	case IOTC_ER_EXCEED_MAX_SESSION:
		//-18 IOTC_ER_EXCEED_MAX_SESSION
		printf("[Error code : %d]\n", IOTC_ER_EXCEED_MAX_SESSION);
		printf("[Warning]\n");
		printf("The amount of session reach to the maximum.\n");
		printf("It cannot be connected unless the session is released.\n");
		break;
	case IOTC_ER_CAN_NOT_FIND_DEVICE:
		//-19 IOTC_ER_CAN_NOT_FIND_DEVICE
		printf("[Error code : %d]\n", IOTC_ER_CAN_NOT_FIND_DEVICE);
		printf("Device didn't register on server, so we can't find device.\n");
		printf("Please check the device again.\n");
		printf("Retry...\n");
		break;
	case IOTC_ER_SESSION_CLOSE_BY_REMOTE:
		//-22 IOTC_ER_SESSION_CLOSE_BY_REMOTE
		printf("[Error code : %d]\n", IOTC_ER_SESSION_CLOSE_BY_REMOTE);
		printf("Session is closed by remote so we can't access.\n");
		printf("Please close it or establish session again.\n");
		break;
	case IOTC_ER_REMOTE_TIMEOUT_DISCONNECT:
		//-23 IOTC_ER_REMOTE_TIMEOUT_DISCONNECT
		printf("[Error code : %d]\n", IOTC_ER_REMOTE_TIMEOUT_DISCONNECT);
		printf("We can't receive an acknowledgement character within a TIMEOUT.\n");
		printf("It might that the session is disconnected by remote.\n");
		printf("Please check the network wheather it is busy or not.\n");
		printf("And check the device and user equipment work well.\n");
		break;
	case IOTC_ER_DEVICE_NOT_LISTENING:
		//-24 IOTC_ER_DEVICE_NOT_LISTENING
		printf("[Error code : %d]\n", IOTC_ER_DEVICE_NOT_LISTENING);
		printf("Device doesn't listen or the sessions of device reach to maximum.\n");
		printf("Please release the session and check the device wheather it listen or not.\n");
		break;
	case IOTC_ER_CH_NOT_ON:
		//-26 IOTC_ER_CH_NOT_ON
		printf("[Error code : %d]\n", IOTC_ER_CH_NOT_ON);
		printf("Channel isn't on.\n");
		printf("Please open it by IOTC_Session_Channel_ON() or IOTC_Session_Get_Free_Channel()\n");
		printf("Retry...\n");
		break;
	case IOTC_ER_SESSION_NO_FREE_CHANNEL:
		//-31 IOTC_ER_SESSION_NO_FREE_CHANNEL
		printf("[Error code : %d]\n", IOTC_ER_SESSION_NO_FREE_CHANNEL);
		printf("All channels are occupied.\n");
		printf("Please release some channel.\n");
		break;
	case IOTC_ER_TCP_TRAVEL_FAILED:
		//-32 IOTC_ER_TCP_TRAVEL_FAILED
		printf("[Error code : %d]\n", IOTC_ER_TCP_TRAVEL_FAILED);
		printf("Device can't connect to Master.\n");
		printf("Don't let device use proxy.\n");
		printf("Close firewall of device.\n");
		printf("Or open device's TCP port 80, 443, 8080, 8000, 21047.\n");
		break;
	case IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED:
		//-33 IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED
		printf("[Error code : %d]\n", IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED);
		printf("Device can't connect to server by TCP.\n");
		printf("Don't let server use proxy.\n");
		printf("Close firewall of server.\n");
		printf("Or open server's TCP port 80, 443, 8080, 8000, 21047.\n");
		printf("Retry...\n");
		break;
	case IOTC_ER_NO_PERMISSION:
		//-40 IOTC_ER_NO_PERMISSION
		printf("[Error code : %d]\n", IOTC_ER_NO_PERMISSION);
		printf("This UID's license doesn't support TCP.\n");
		break;
	case IOTC_ER_NETWORK_UNREACHABLE:
		//-41 IOTC_ER_NETWORK_UNREACHABLE
		printf("[Error code : %d]\n", IOTC_ER_NETWORK_UNREACHABLE);
		printf("Network is unreachable.\n");
		printf("Please check your network.\n");
		printf("Retry...\n");
		break;
	case IOTC_ER_FAIL_SETUP_RELAY:
		//-42 IOTC_ER_FAIL_SETUP_RELAY
		printf("[Error code : %d]\n", IOTC_ER_FAIL_SETUP_RELAY);
		printf("Client can't connect to a device via Lan, P2P, and Relay mode\n");
		break;
	case IOTC_ER_NOT_SUPPORT_RELAY:
		//-43 IOTC_ER_NOT_SUPPORT_RELAY
		printf("[Error code : %d]\n", IOTC_ER_NOT_SUPPORT_RELAY);
		printf("Server doesn't support UDP relay mode.\n");
		printf("So client can't use UDP relay to connect to a device.\n");
		break;

	default:
		printf("[Unknow error code : %d]\n", error);
		break;
	}
}

//########################################################
//# Print IOTC & AV version
//########################################################
void PrintVersion()
{
	const char *iotc_ver = IOTC_Get_Version_String();
	const char *av_ver = avGetAVApiVersionString();
	printf("IOTCAPI version[%s] AVAPI version[%s]\n", iotc_ver, av_ver);
}

//########################################################
//# Search Session info
//########################################################
void *search_session_av_index(int av_index)
{
	int i = 0;
	for (i = 0; i < MAX_CLIENT_NUMBER; i++) {
		if (av_index >= 0 && gClientInfo[i].av_index == av_index)
			return &gClientInfo[i];
	}
	return NULL;
}

//########################################################
//# Enable / Disable live stream to AV client
//########################################################
void RegEditClient(int sid, int av_index)
{
	AV_Client *p = &gClientInfo[sid];
	p->av_index = av_index;
}

void RegEditClientToVideo(int sid, int av_index)
{
	AV_Client *p = &gClientInfo[sid];
	p->av_index = av_index;
	p->enable_video = 1;
}

void UnRegEditClientFromVideo(int sid)
{
	AV_Client *p = &gClientInfo[sid];
	p->enable_video = 0;
}

void RegEditClientToAudio(int sid, int av_index)
{
	(void)(av_index);
	AV_Client *p = &gClientInfo[sid];
	p->enable_audio = 1;
}

void UnRegEditClientFromAudio(int sid)
{
	AV_Client *p = &gClientInfo[sid];
	p->enable_audio = 0;
}

void RegEditClientStreamMode(int sid, int stream_mode)
{
	AV_Client *p = &gClientInfo[sid];
	p->two_way_stream = stream_mode;
}

int GetSidFromAvIndex(int av_index)
{
	for (int i = 0; i < MAX_CLIENT_NUMBER; i++) {
		if (gClientInfo[i].av_index == av_index) {
			return i;
		}
	}
	return -1;
}

static int createStreamoutThread()
{
	int ret = 0;

	pthread_t thread_video_frame_data_id;
	if ((ret = pthread_create(&thread_video_frame_data_id, NULL, thread_VideoFrameData, NULL))) {
		tutkservice_log_err("pthread_create ret=%d\n", ret);
		return -1;
	}
	pthread_detach(thread_video_frame_data_id);

	return 0;
}

//########################################################
//# Start AV server and recv IOCtrl cmd for every new av idx
//########################################################
static void *ThreadForAVServerStart(void *arg)
{
	int sid = *(int *)arg;
	free(arg);

	int chid = AV_LIVE_STREAM_CHANNEL;
	int timeout_sec = 30;
	int stream_mode = 0;
	int resend_buf_size_kbytes = 1024;

	printf("%s SID[%d]\n", __func__, sid);

	int av_index = StartAvServer(sid, chid, timeout_sec, resend_buf_size_kbytes, &stream_mode);
	if (av_index < 0) {
		tutkservice_log_err("StartAvServer SID[%d] ret[%d]!!!\n", sid, av_index);
		goto EXIT;
	}

	gOnlineNum++;
	RegEditClient(sid, av_index);
	RegEditClientStreamMode(sid, stream_mode);

	struct st_SInfoEx session_info;
	session_info.size = sizeof(struct st_SInfoEx);
	while (1) {
		int ret = IOTC_Session_Check_Ex(sid, &session_info);
		if (ret != IOTC_ER_NoERROR) {
			break;
		}
		sleep(1);
	}

EXIT:
	UnRegEditClientFromVideo(sid);
	UnRegEditClientFromAudio(sid);
	RegEditClientPlaybackMode(sid, PLAYBACK_STOP);

	if (av_index >= 0) {
		tutkservice_log_err("avServStop[%d]\n", av_index);
		avServStop(av_index);
		gOnlineNum--;
	}
	printf("IOTC_Session_Close[%d]\n", sid);
	IOTC_Session_Close(sid);
	printf("SID[%d], av_index[%d], %s exit!!\n", sid, av_index, __func__);
	pthread_exit(0);
}

static void *ThreadNebulaLogin(void *arg)
{
	NebulaDeviceCtx *device_ctx = (NebulaDeviceCtx *)arg;
	int ret = 0;
	char admin_psk[MAX_NEBULA_PSK_LENGTH + 1] = { 0 };

	while (gProgressRun) {
		int ret = Nebula_Device_Login(device_ctx, DeviceLoginStateHandle);
		printf("Nebula_Device_Login ret[%d]\n", ret);
		if (ret == NEBULA_ER_NoERROR) {
			printf("Nebula_Device_Login success...!!\n");
			if (access(BIND_FLAG, F_OK) != 0) {
				printf("create %s\n", BIND_FLAG);
				int fd = open(BIND_FLAG, O_CREAT);
				if (fd == -1) {
					perror("Unable to touch file");
				}

				close(fd);
			}

			break;
		}
		sleep(1);
	}

	while (1) {
		if (access(BIND_FLAG, F_OK) == 0) {
			break;
		}

		ret = GetPskFromFile("admin", gUserIdentitiesFilePath, admin_psk, MAX_NEBULA_PSK_LENGTH);
		if (ret != 200) {
			AppendPskToFile("admin", gUserIdentitiesFilePath, admin_psk, MAX_NEBULA_PSK_LENGTH);
		}

		ret = Nebula_Device_Bind(device_ctx, gPinCode, admin_psk, 60000, &gBindAbort);
		printf("Nebula_Device_Bind ret[%d]\n", ret);

		if (ret == NEBULA_ER_TIMEOUT) {
			continue;
		} else {
			printf("Nebula_Device exit...!!\n");
			return NULL;
		}
	}

	return NULL;
}

static void *ThreadIotcDeviceLogin(void *arg)
{
	NebulaDeviceCtx *device_ctx = (NebulaDeviceCtx *)arg;
	while (gProgressRun) {
		printf("IOTC_Device_Login_By_Nebula() start\n");
		int ret = IOTC_Device_Login_By_Nebula(device_ctx);
		printf("IOTC_Device_Login_By_Nebula() ret = %d\n", ret);
		if (ret == IOTC_ER_NoERROR) {
			printf("IOTC_Device_Login_By_Nebula success...!!\n");
			break;
		}
		sleep(1);
	}
	return NULL;
}

#define MAX_BUF_SIZE 255
static int getPIDbyProcessName(const char *proc_name)
{
	DIR *dir;
	struct dirent *entry;
	char filename[MAX_BUF_SIZE], cmdline[MAX_BUF_SIZE];
	FILE *fp;
	int pid = 0;

	dir = opendir("/proc");
	if (dir == NULL) {
		perror("opendir() failed");
		return -1;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (atoi(entry->d_name) != 0) {
			snprintf(filename, sizeof(filename), "%s/%s/cmdline", "/proc", entry->d_name);

			fp = fopen(filename, "r");
			if (fp != NULL) {
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);
				fp = NULL;

				if (strstr(cmdline, proc_name) != NULL) {
					pid = atoi(entry->d_name);
					closedir(dir);
					return pid;
				}
			}
		}
	}

	closedir(dir);
	// not found, return -1
	return -1;
}

//########################################################
//# Main function
//########################################################
int TUTK_run(char *udid, char *profile_url)
{
	(void)(udid);
	(void)(profile_url);

	int ret = 0;
	FILE *fp = NULL;
	char *profile_buf = NULL;
	const char *profile_str = NULL;
	pthread_t nebula_login_thread_id;
	pthread_t iotc_login_thread_id;
	NebulaWakeUpData *sleep_data = NULL;
	unsigned int sleep_data_count = 0;
	char wakeup_pattern[MAX_WAKEUP_PATTERN_LENGTH] = { 0 };
	NebulaSocketProtocol nebula_protocol = NEBULA_PROTO_TCP;

NEBULA_DEVICE_START:
	gProgressRun = true;
	gBindAbort = 0;

	fp = fopen(gProfilePath, "r");
	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	profile_buf = calloc(1, file_size + 1);
	fread(profile_buf, 1, file_size, fp);
	fclose(fp);
	fp = NULL;
	profile_buf[file_size] = '\0';

	ret = loadDisposableParams(gPinCode, gSecretId, NULL);
	if (ret < 0) {
		tutkservice_log_err("loadDisposableParams()=%d,exit...!!\n", ret);
		return -1;
	}

	PrintVersion();

	//Initialize
	InitAVInfo();

	ret = createStreamoutThread();
	if (ret < 0) {
		tutkservice_log_err("exit...!!\n");
		return -1;
	}

	// Nebula Initialize
	ret = Nebula_Initialize();
	printf("Nebula_Initialize ret[%d]\n", ret);
	if (ret != NEBULA_ER_NoERROR) {
		printf("Nebula_Device exit...!!\n");
		return -1;
	}

	IOTC_Set_Max_Session_Number(MAX_CLIENT_NUMBER);
	// IOTC Initialize
	ret = IOTC_Initialize2(0);
	printf("IOTC_Initialize() ret[%d]\n", ret);
	if (ret != IOTC_ER_NoERROR) {
		printf("exit...!!\n");
		return -1;
	}

	ret = Nebula_Json_Obj_Create_From_String(profile_buf, &gProfileJsonObj);
	if (ret != IOTC_ER_NoERROR) {
		printf("profile format error,  exit...!!\n");
		return -1;
	}
	profile_str = Nebula_Json_Obj_To_String(gProfileJsonObj);
	ret = Nebula_Device_New(gUdid, gSecretId, profile_str, CommandHandle, IdentityHandle, SettingsChangeHandle,
	                        &gDeviceCtx);
	printf("Nebula_Device_New ret[%d]\n", ret);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("exit...!!\n");
		return -1;
	}

	free(profile_buf);

	TUTK_startADCnotificationThread(gDeviceCtx);
	TUTK_startPIRnotificationThread(gDeviceCtx);

	//AV Initialize
	avInitialize(MAX_CLIENT_NUMBER);

	if (pthread_create(&nebula_login_thread_id, NULL, ThreadNebulaLogin, gDeviceCtx) < 0) {
		printf("create ThreadNebulaLogin failed!, exit...!!\n");
		return -1;
	}
	pthread_join(nebula_login_thread_id, NULL);

	if (pthread_create(&iotc_login_thread_id, NULL, ThreadIotcDeviceLogin, gDeviceCtx) < 0) {
		printf("create ThreadIotcDeviceLogin failed!, exit...!!\n");
		gBindAbort = 1;
		return -1;
	}
	pthread_join(iotc_login_thread_id, NULL);

	ResetNoSessionTime();

	while (ShouldDeviceGoToSleep() == 0 && gProgressRun) {
		ret = IOTC_Device_Listen_By_Nebula(gDeviceCtx, 10000);
		printf("IOTC_Device_Listen_By_Nebula ret[%d], no session sec: %d\n", ret, getNoSessionTime());
		if (ret < 0) {
			PrintErrHandling(ret);
			UpdateNoSessionTime();
			continue;
		}

		/* user connect */
		if (ret >= 0) {
			ResetNoSessionTime();

			printf("Session[%d] connect!\n", ret);
			struct st_SInfoEx session_info;
			session_info.size = sizeof(session_info);
			if (IOTC_Session_Check_Ex(ret, &session_info) == IOTC_ER_NoERROR) {
				char *mode[3] = { "P2P", "RLY", "LAN" };
				if (isdigit(session_info.RemoteIP[0]))
					printf("Client is from[IP:%s, Port:%d] Mode[%s] VPG[%d:%d:%d] VER[%X] NAT[%d] AES[%d]\n",
					       session_info.RemoteIP, session_info.RemotePort,
					       mode[(int)session_info.Mode], session_info.VID, session_info.PID,
					       session_info.GID, session_info.IOTCVersion, session_info.RemoteNatType,
					       session_info.isSecure);

				int *sid = (int *)malloc(sizeof(int));
				*sid = ret;
				pthread_t thread_id;
				ret = pthread_create(&thread_id, NULL, ThreadForAVServerStart, (void *)sid);
				if (ret < 0) {
					tutkservice_log_err("pthread_create ThreadForAVServerStart failed ret[%d]\n",
					                    ret);
				} else {
					pthread_detach(thread_id);
				}
			}
		}
	}

	TUTK_exeSystemCmd("/etc/init.d/S00aon_watchd.sh stop");
	gProgressRun = false;
	gBindAbort = 1;
	Nebula_Json_Obj_Release(gProfileJsonObj);

	if (gEnableWakeUp) {
		// Prepare wakeup data before deinitialize modules
		PrepareWakeupDataBeforeSleep(gDeviceCtx, nebula_protocol, wakeup_pattern, &sleep_data,
		                             &sleep_data_count);
	}

	printf("wake-up data prepared\n");
	Nebula_Device_Delete(gDeviceCtx);
	avDeInitialize();
	printf("avDeInitialize\n");
	IOTC_DeInitialize();
	printf("IOTC_DeInitialize\n");
	Nebula_DeInitialize();
	printf("Nebula_DeInitialize\n");
	DeInitAVInfo();

	TUTK_exeSystemCmd("killall -2 /system/bin/mpi_stream");

	while (-1 != getPIDbyProcessName("mpi_stream")) {
		//tutkservice_log_info("wait for mpi_stream end\n");
		usleep(100000);
	}

	if (ginterpolation_level <= 1) {
		if (gEnableWakeUp) {
			WaitForBatteryChargeSleeping();
		}
	} else {
		TUTK_exeSystemCmd("cp -f /tmp/record/* /mnt/sdcard; sync");
		if (gEnableWakeUp) {
			WaitForWakeupPatternWhenSleeping(nebula_protocol, wakeup_pattern, sleep_data, sleep_data_count,
			                                 ginterpolation_level);
		}
	}

	return 0;
}
