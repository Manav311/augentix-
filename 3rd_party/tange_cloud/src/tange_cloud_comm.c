/**
 * This file integrates the tange cloud demo. code "demo.c".
*/
#include <linux/reboot.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <agtx_lpw.h>

#include "tange_cloud_video.h"
#include "tange_cloud_comm.h"

#include "ec_const.h"
#include "logfile.h"

#if defined(TANGE_USE_AGT_MPI_STREAM)
extern lpw_handle gWifihd;
extern struct timeval gNoSessionTime;
#endif

/**
 * Definition
*/
#define CallSystem(s) printf("call: %s\n", s);
typedef struct {
	int val;
	char *name;
} ValName;
#define _D(v)         \
	{             \
		v, #v \
	}

/**
 * Variables
*/

/**
 * Static Variables
*/
static char _cwd[128] = { 0 };
static int g_mode = 0;
struct TciCB _tci_cb = { 0 };

/**
 * Static Function Prototype
*/
static void setTciCallback(void);
static int _Handle_P2p_Cmd(p2phandle_t handle, int cmd, const void *buf, int size);
static int Handle_P2p_Cmd(p2phandle_t handle, int cmd, const void *buf, int size);
static int get_info(TCIDEVICEINFO *info);
static int get_device_feature(const char *key, char *buf, int bytes);
static int get_device_state(const char *key, char *buf, int bytes);
static int on_apmode_login(const char *user, const char *key);
static int on_qrcode_start(void);
static int on_get_y_data(uint8_t **ppYBuff, int *width, int *height);
static void on_qrcode_end(uint8_t *pYBuff);
static int on_ota_download_start(const char *new_version, unsigned int size);
static int on_ota_download_data(const uint8_t *buff, int size);
static int on_ota_download_finished(int status);
static int on_talkback_start(void);
static int talkback(TCMEDIA at, const uint8_t *audio, int len);
static void on_talkback_stop(void);
static int snapshot(int type, uint8_t **ppJpg, int *size);
#if defined(TANGE_USE_AGT_MPI_STREAM)
static int set_wifi(int is_switching, const char *ssid, const char *key);
#endif
static int tzoffset(const char *tzs);
static int set_timezone(const char *tzs);
static int set_time(time_t time);
static int on_status(int status, const void *pData, int len);
static int request_iframe(int vstream);
static int on_log(int action, char *path);
static int request_iframe_ex(int channel, int vstream);
static void switch_quality(int channel, int stream, const char *qstr);
static int Remove(const char *fn);
static int Touch(const char *fn);
static int loadY(unsigned char **ppY, int *width, int *height);
static int msleep(long msec);
static int goto_idlestate(void);
static int initLpwPowerMgmt(lpw_handle hd);
static int accessLpwTcp(lpw_handle hd, in_addr_t ser_ip, in_port_t ser_port);
static int prepareTgTcpWakeup(lpw_handle hd);
static int getTgWkServerList(Ipv4Addr *servers);
static int sendTgAuthString(lpw_handle hd, Ipv4Addr *servers);
static int setLpwWkPattern(lpw_handle hd);
static int goToSleep(lpw_handle hd, int timeout, int response);

int get_mode()
{
	return g_mode;
}

void set_mode(int mode)
{
	g_mode = mode;
}

/**
 * Function
*/
int TGC_initComm(char *uuid)
{
	int ret;

	//getcwd(_cwd, sizeof(_cwd));
	sprintf(_cwd, "/usrdata/root");
	LogI("CWD: %s\n", _cwd);

	setTciCallback();

	TciSetCallback(&_tci_cb);
	TciSetCmdHandler(_Handle_P2p_Cmd);

#if defined(TANGE_USE_AGT_MPI_STREAM)
	ret = TciInit(_cwd, NULL);
#elif defined(TANGE_USE_AV_MAIN2)
	ret = TciInit(_cwd, uuid);
#endif
	if (ret) {
		LogE("TciInit return %d\n", ret);
		return 1;
	}
	g_mode = 0;

#if defined(TANGE_USE_AV_MAIN2)
	if (uuid) {
		if (access("/usrdata/root/registered.ini", F_OK) == 0) {
			g_mode = 2;
		}
	}
#endif

	return 0;
};

void TGC_deinitComm(void)
{
	int ret = 0;
	LogI("Stop tci service and cleanup...\n");
	if ((ret = TciStop())) {
		LogE("TciStop failure. 0x%x\n", ret);
	}
	TciCleanup();
	TGC_releaseConfig();
	LogI("TGC_releaseConfig done...\n");
}

int TGC_startComm(void)
{
	return (TciStart(g_mode == 2, (1 << 20)));
}

void TGC_sendFrame(int channel, int stream, TCMEDIA tm, const uint8_t *pFrame, unsigned int length, uint32_t ts,
                   unsigned int flages)
{
	int err;
	if (TCMEDIA_IS_AUDIO(tm) && ((flages >> 2) & 0x03) == AUDIO_SAMPLE_16K) {
		uint8_t *f = (uint8_t *)malloc(length * 2);
		int i;
		for (i = 0; i < length; i++)
			f[2 * i] = f[2 * i + 1] = pFrame[i];
		err = TciSendFrameEx(channel, stream, tm, f, 2 * length, ts, flages);
		free(f);
	} else
		err = TciSendFrameEx(channel, stream, tm, pFrame, length, ts, flages);
	if (err) {
		_err("TciSendFrame failed. channel:%d, media:%d\n", stream, tm);
	}
}

void TGC_sendPbFrame(p2phandle_t handle, TCMEDIA mt, const uint8_t *frame, unsigned int length, uint32_t ts,
                     unsigned int flages)
{
	int err;
	if (TCMEDIA_IS_AUDIO(mt & 0xff) && ((flages >> 2) & 0x03) == AUDIO_SAMPLE_16K) {
		uint8_t *f = (uint8_t *)malloc(length * 2);
		int i;
		for (i = 0; i < length; i++)
			f[2 * i] = f[2 * i + 1] = frame[i];
		err = TciSendPbFrame(handle, mt, f, 2 * length, ts, flages);
		free(f);
	} else
		err = TciSendPbFrame(handle, mt, frame, length, ts, flages);
	if (err < 0) {
		_err("TciSendPbFrame failed:%d. mt:%d, len:%d\n", err, mt, length);
	}
}

/* IPC 取得实时流后调用接口将数据给cloud SDK */
void ipc_send_stream(int channel, int stream, TCMEDIA tm, const uint8_t *pFrame, int length, uint32_t ts,
                     int isKeyFrame)
{
	(int)channel;
	(int)stream;
	(TCMEDIA) tm;
	(const uint8_t *)pFrame;
	(int)length;
	(uint32_t) ts;
	(int)isKeyFrame;
}

/*回放时IPC从文件中读取帧然后发送*/
void ipc_send_pb_frame(p2phandle_t handle, TCMEDIA mt, const uint8_t *frame, int length, uint32_t ts, int isKeyFrame)
{
	(p2phandle_t) handle;
	(TCMEDIA) mt;
	(const uint8_t *)frame;
	(int)length;
	(uint32_t) ts;
	(int)isKeyFrame;
}
/**
 * Static Function
*/

static int _Handle_P2p_Cmd(p2phandle_t handle, int cmd, const void *buf, int size)
{
	pthread_t thread_audio_frame_data_id;
	switch (cmd) {
	case TCI_CMD_LISTEVENT_REQ:
	case TCI_CMD_RECORD_PLAYCONTROL:
	case TCI_CMD_GET_EXTERNAL_STORAGE_REQ:
	case TCI_CMD_FORMATEXTSTORAGE_REQ:
		printf("p2p command : 0x%x\n", cmd);
		// TciSendCmdResp(handle, cmd, (const char *)&resp, sizeof(resp));
		break;
	case TCI_CMD_LISTWIFIAP_REQ: {
		printf("p2p command list WIFI\n");
	} break;
	case TCI_CMD_GETWIFI_REQ: {
		printf("p2p command get WIFI Info\n");
	} break;
	case TCI_CMD_GET_VIDEOMODE_REQ: {
		printf("p2p command get video mode.\n");
	} break;
	case TCI_CMD_SET_VIDEOMODE_REQ: {
		printf("p2p command set video mode.\n");
	} break;
	case TCI_CMD_GET_ENVIRONMENT_REQ: {
		printf("p2p command get environment info..\n");
	} break;
	case TCI_CMD_SET_ENVIRONMENT_REQ: {
		printf("p2p command set environment info..\n");
	} break;
	case TCI_CMD_GETRECORD_REQ: {
		printf("p2p command record get.\n");
	} break;
	case TCI_CMD_SETRECORD_REQ: {
		printf("p2p command record set.\n");
	} break;
	case TCI_CMD_PTZ_SHORT_COMMAND: {
		printf("p2p command PTZ short control.\n");
	} break;
	case TCI_CMD_PTZ_LONG_COMMAND: {
		printf("p2p command PTZ long control.\n");
	} break;
	case TCI_CMD_SET_ENABLE_BT: {
		printf("p2p command set BT.\n");
	} break;
	case TCI_CMD_GET_ENABLE_BT: {
		printf("p2p command get BT.\n");
		// TciSendCmdResp(handle, cmd, (const char *)&resp, sizeof(resp));
	} break;
	case TCI_CMD_SET_AI: {
	} break;
	case TCI_CMD_GET_AI: {
	} break;
	case TCI_CMD_SET_PTZ_POS: {
	} break;
	case TCI_CMD_GET_PTZ_POS: {
	} break;
	case TCI_CMD_DEV_REBOOT_REQ: {
	} break;
	case TCI_CMD_GET_DOUBLELIGHT_REQ: {
	} break;
	case TCI_CMD_SET_DOUBLELIGHT_REQ: {
	} break;
	case TCI_CMD_GET_MOTION_TRACKER_REQ: {
	} break;
	case TCI_CMD_SET_MOTION_TRACKER_REQ: {
	} break;
	case TCI_CMD_GET_MICROPHONE_REQ: {
		//sdk 内部会拦截此命令
	} break;
	case TCI_CMD_SET_MICROPHONE_REQ: {
	} break;
	case TCI_CMD_GET_BUZZER_REQ: {
	} break;
	case TCI_CMD_SET_BUZZER_REQ: {
	} break;
	//p2p 异常断开时候，应用可做相关处理
	case TCI_CMD_SESSION_CLOSE: {
		// //如停止正在进行的SD回放
		// LogI("TCI_CMD_SESSION_CLOSE\n");
		// IpcSimulatorSessionClose(handle);
	} break;
	case TCI_CMD_SETMOTIONDETECT_REQ: {
	} break;
	case TCI_CMD_GETMOTIONDETECT_REQ: {
	} break;
	case TCI_CMD_GET_DEFENCE_REQ: {
	} break;
	case TCI_CMD_SET_DEFENCE_REQ: {
	} break;
	case TCI_CMD_SET_DEVICE_STATUS: {
	} break;
	case TCI_CMD_GET_ALARMTONE_CAP: {
	} break;
	case TCI_CMD_PLAY_ALARMTONE: {
	} break;
	case TCI_CMD_SET_ALARMTONE: {
	} break;
	case TCI_CMD_PLAY_AUDIO: {
	} break;
	case TCI_CMD_SET_HINTTONE:
		break;
	case TCI_CMD_GET_HINTTONE: {
	} break;
	case TCI_CMD_SET_LED_STATUS: {
	} break;
	case TCI_CMD_GET_LED_STATUS: {
	} break;
	case TCI_CMD_GET_BATTERY_STATUS: {
	} break;
	case TCI_CMD_GET_WIFI_SIGNALLEVEL: {
	} break;
	case TCI_CMD_GET_MAX_AWAKE_TIME: {
	} break;
	case TCI_CMD_SET_MAX_AWAKE_TIME: {
	} break;
	case TCI_CMD_SET_CLOSE_PLAN: {
	} break;
	case TCI_CMD_GET_CLOSE_PLAN: {
	} break;
	case TCI_CMD_SET_PRIMARY_VIEW: {
		// Tcis_SetPrimaryViewReq *req = (Tcis_PrimaryView *)buf;
		// LogI("TCI_CMD_SET_PRIMARY_VIEW: %d\n", req->channel);
		// TGC_saveSetting(cmd, buf, size);
		// TciSendCmdRespStatus(handle, cmd, 0);
		// TciSetCloudStream(req->channel, 0);
	} break;
	case TCI_CMD_GET_PRIMARY_VIEW: {
		// Tcis_GetPrimaryViewResp resp;
		// Tcis_GetPrimaryViewReq *req = (Tcis_GetPrimaryViewReq *)buf;
		// resp.id = 0;
		// resp.channel = 0; //default
		// TGC_loadSetting(TCI_CMD_SET_PRIMARY_VIEW, (char *)&resp, sizeof(resp));
		// LogI("TCI_CMD_GET_PRIMARY_VIEW: id in req:%d, channel in resp: %d\n", req->id, resp.channel);
		// TciSendCmdResp(handle, cmd, (char *)&resp, sizeof(resp));
	} break;

/* GET/SET 命令响应的简化写法： set时保存一下，get时取出来 */
#define _SETTER_(cmd, index, S, _action_, fmt, args...) \
	case cmd: {                                     \
	} break;
	// case cmd: {                                                   \
	// 	S *req = (S *)buf;                                    \
	// 	_action_;                                             \
	// 	LogI(#cmd ": " fmt "\n", args);                       \
	// 	TGC_saveSetting(cmd | (index << 16), req, sizeof(S)); \
	// 	TciSendCmdRespStatus(handle, cmd, 0);                 \
	// } break

//请求为不定长结构
#define _SETTER2_(cmd, index, S, size, _action_, fmt, args...) \
	case cmd: {                                            \
	} break;
	// case cmd: {                                              \
	// 	S *req = (S *)buf;                               \
	// 	_action_;                                        \
	// 	LogI(#cmd ": " fmt "\n", args);                  \
	// 	TGC_saveSetting(cmd | (index << 16), req, size); \
	// 	TciSendCmdRespStatus(handle, cmd, 0);            \
	// } break

//SET 等于 GET-2
#define _GETDEF_(cmd, index, R, S, _init) \
	case cmd: {                       \
	} break;
	// case cmd: {                                                                      \
	// 	LogI(#cmd "\n");                                                         \
	// 	R *req = (R *)buf;                                                       \
	// 	S resp = {};                                                             \
	// 	memset(&resp, 0, sizeof(S));                                             \
	// 	_init;                                                                   \
	// 	TGC_loadSetting((cmd - 2) | (index << 16), (char *)&resp, sizeof(resp)); \
	// 	TciSendCmdResp(handle, cmd, &resp, sizeof(resp));                        \
	// } break

//SET 不等于 GET-2
#define _GETDEF3_(cmd, setcmd, index, R, S, _init) \
	case cmd: {                                \
	} break;
	// case cmd: {                                                                     \
	// 	LogI(#cmd "\n");                                                        \
	// 	R *req = (R *)buf;                                                      \
	// 	S resp = {};                                                            \
	// 	memset(&resp, 0, sizeof(S));                                            \
	// 	_init;                                                                  \
	// 	TGC_loadSetting((setcmd) | (index << 16), (char *)&resp, sizeof(resp)); \
	// 	TciSendCmdResp(handle, cmd, &resp, sizeof(resp));                       \
	// } break

//除保存之外还有别的动作
#define _GETDEFex_(cmd, index, R, S, _init, _post) \
	case cmd: {                                \
	} break;
	// case cmd: {                                                                            \
	// 	LogI(#cmd "\n");                                                               \
	// 	R *req = (R *)buf;                                                             \
	// 	S resp = {};                                                                   \
	// 	memset(&resp, 0, sizeof(S));                                                   \
	// 	_init;                                                                         \
	// 	if (TGC_loadSetting((cmd - 2) | (index << 16), (char *)&resp, sizeof(resp))) { \
	// 		_post;                                                                 \
	// 	}                                                                              \
	// 	TciSendCmdResp(handle, cmd, &resp, sizeof(resp));                              \
	// } break

//Variable response length
#define _GETDEF2_(cmd, index, R, S, S_size, _init, initLen) \
	case cmd: {                                         \
	} break;
		// case cmd: {                                                                         \
	// 	LogI(#cmd "\n");                                                            \
	// 	R *req = (R *)buf;                                                          \
	// 	S *resp = (S *)malloc(S_size);                                              \
	// 	int len = TGC_loadSetting((cmd - 2) | (index << 16), (char *)resp, S_size); \
	// 	if (len == 0) {                                                             \
	// 		_init;                                                              \
	// 		len = initLen;                                                      \
	// 	}                                                                           \
	// 	TciSendCmdResp(handle, cmd, resp, len);                                     \
	// 	free(resp);                                                                 \
	// } break

		_SETTER_(
		        TCI_CMD_SET_DAYNIGHT_REQ, 0, Tcis_SetDayNightReq, do {} while (0), "mode=%d", req->mode);
		_GETDEF_(TCI_CMD_GET_DAYNIGHT_REQ, 0, void, Tcis_GetDayNightResp, resp.support = 2; resp.mode = 2);

		_SETTER_(
		        TCI_CMD_SET_MIC_LEVEL, 0, Tcis_SetMicLevelReq, do {} while (0), "sensitivity=%d",
		        req->sensitivity);
		_GETDEF_(TCI_CMD_GET_MIC_LEVEL, 0, void, Tcis_GetMicLevelResp, resp.sensitivity = 80);

		_SETTER_(
		        TCI_CMD_SET_VOLUME, 0, Tcis_SetVolumeReq, do {} while (0), "flags=%d, volume=%d", req->flags,
		        req->volume);
		_GETDEF_(TCI_CMD_GET_VOLUME, 0, void, Tcis_GetVolumeResp, resp.volume = 90; resp.flags = 1);

		_SETTER_(
		        TCI_CMD_SET_ALARMLIGHT, 0, Tcis_SetAlarmLightStateReq, do {} while (0), "state=%d", req->state);
		_GETDEF_(TCI_CMD_GET_ALARMLIGHT, 0, void, Tcis_GetAlarmLightStateResp, resp.state = 2);

		_SETTER_(
		        TCI_CMD_SET_PIR, 0, Tcis_SetPirSensReq, do {} while (0), "sens:%d", req->sens);
		_GETDEF_(TCI_CMD_GET_PIR, 0, void, Tcis_GetPirSensResp, resp.sens = 2);

		_SETTER_(
		        TCI_CMD_SET_ENABLE_DORMANCY, 0, Tcis_DormancyState, do {} while (0), "enabled=%d", req->enable);
		_GETDEF_(TCI_CMD_GET_ENABLE_DORMANCY, 0, void, Tcis_DormancyState, resp.enable = 1);

		_SETTER_(
		        TCI_CMD_SET_TIMELAPSE_RECORD, 0, Tcis_SetTimelapseRecordModeReq, do {} while (0), "status=%d",
		        req->status);
		_GETDEF_(TCI_CMD_GET_TIMELAPSE_RECORD, 0, void, Tcis_GetTimelapseRecordModeResp, resp.status = 0);

		_SETTER_(
		        TCI_CMD_SET_MDAREA_STATE, 0, Tcis_MdAreaState, do {} while (0), "state=%d", req->state);
		_GETDEF_(TCI_CMD_GET_MDAREA_STATE, 0, void, Tcis_GetMdAreaStateResp, resp.state = 0);

		_SETTER_(
		        TCI_CMD_SET_GSENSOR, req->scene, Tcis_SetGsensorReq, do {} while (0), "scene=%d,sens=%d",
		        req->scene, req->sensitivity);
		_GETDEF_(TCI_CMD_GET_GSENSOR, req->scene, Tcis_GetGsensorReq, Tcis_GetGsensorResp,
		         resp.scene = 0 /*req->scene*/;
		         resp.sensitivity = 2;
		         _info("req->scene=%d;resp:scene=%d,sens=%d", req->scene, resp.scene, resp.sensitivity));

		_SETTER_(
		        TCI_CMD_SET_SHOW_BOX, req->ai_type, Tcis_SetShowBoxReq, do {} while (0), "ai_type=%d,show=%d",
		        req->ai_type, req->show_box);
		_GETDEF_(TCI_CMD_GET_SHOW_BOX, req->ai_type, Tcis_GetShowBoxReq, Tcis_GetShowBoxResp,
		         resp.ai_type = req->ai_type;
		         resp.show_box = 0);

		_SETTER_(
		        TCI_CMD_SET_EVENT_STATE, req->event, Tcis_SetEventStateReq, do {} while (0),
		        "event=%d,enabled=%d", req->event, req->enabled);
		_GETDEF_(TCI_CMD_GET_EVENT_STATE, req->event, Tcis_GetEventStateReq, Tcis_GetEventStateResp,
		         resp.event = req->event;
		         resp.enabled = 1);

		_SETTER_(
		        TCI_CMD_SET_WATCHPOS, req->channel, Tcis_SetWatchPosReq, do {} while (0),
		        "num=%d, idle_time=%d", req->num, req->idle_time);
		_GETDEF_(TCI_CMD_GET_WATCHPOS, req->channel, Tcis_GetWatchPosReq, Tcis_GetWatchPosResp,
		         resp.channel = req->channel;
		         resp.num = 1; resp.idle_time = 30);

#if 1
		_SETTER_(
		        TCI_CMD_SET_TEMPERATURE_THRESHOLD, req->sensor_id, Tcis_SetTemperatureThresholdReq,
		        do {} while (0), "lo:%d:%d, hi:%d:%d", req->lo_en, req->lo_threshold, req->hi_en,
		        req->hi_threshold);
		_GETDEFex_(
		        TCI_CMD_GET_TEMPERATURE_SETTING, req->sensor_id, Tcis_GetTemperatureSettingReq,
		        Tcis_GetTemperatureSettingResp, resp.sensor_id = req->sensor_id;
		        resp.lo_en = 1; resp.hi_en = 1; resp.hi_limit = 65000; resp.lo_limit = -20000;
		        resp.lo_threshold = 25000; resp.hi_threshold = 55000, do {
			        memmove(&resp.lo_en, &resp.lo_limit, 16);
			        resp.hi_limit = 65000;
			        resp.lo_limit = -20000;
			        printf("hi_threshold=%d\n", resp.hi_threshold);
		        } while (0));

		_SETTER_(
		        TCI_CMD_SET_HUMIDITY_THRESHOLD, req->sensor_id, Tcis_SetHumidityThresholdReq, do {} while (0),
		        "lo:%d:%d, hi:%d:%d", req->lo_en, req->lo_threshold, req->hi_en, req->hi_threshold);
		_GETDEFex_(TCI_CMD_GET_HUMIDITY_SETTING, req->sensor_id, Tcis_GetHumiditySettingReq,
		           Tcis_GetHumiditySettingResp, resp.sensor_id = req->sensor_id;
		           resp.lo_en = 1; resp.hi_en = 1; resp.hi_limit = 850; resp.lo_limit = 10;
		           resp.lo_threshold = 250; resp.hi_threshold = 550, memmove(&resp.lo_en, &resp.lo_limit, 16);
		           resp.hi_limit = 850; resp.lo_limit = 10);
#endif
		_SETTER_(
		        TCI_CMD_SET_ENABLE_CLOSEUP, 0, Tcis_EnableCloseup, do {} while (0), "channel=%d,enabled=%d",
		        req->channel, req->enabled);
		_GETDEF3_(TCI_CMD_GET_ENABLE_CLOSEUP, TCI_CMD_SET_ENABLE_CLOSEUP, 0, Tcis_EnableCloseup,
		          Tcis_EnableCloseup, resp.enabled = 1);

		_SETTER_(
		        TCI_CMD_SET_SITPOSE_SENS, 0, Tcis_SitPoseSens, do {} while (0), "mode=%d\n", req->mode);
		_GETDEF_(TCI_CMD_GET_SITPOSE_SENS, 0, void, Tcis_SitPoseSens, resp.mode = 1);

		_SETTER_(
		        TCI_CMD_SET_VOICE_PROMPT_STATUS, 0, Tcis_VoicePromptStatus, do {} while (0), "status=%d",
		        req->status);
		_GETDEF_(TCI_CMD_GET_VOICE_PROMPT_STATUS, 0, void, Tcis_VoicePromptStatus, resp.status = 1);

		_SETTER_(
		        TCI_CMD_SET_PARKING_MONITOR, 0, Tcis_ParkingMonitorSwitch, do {} while (0), "enabled=%d",
		        req->enabled);
		_GETDEF_(TCI_CMD_GET_PARKING_MONITOR, 0, void, Tcis_ParkingMonitorSwitch, resp.enabled = 1);

		_SETTER2_(TCI_CMD_SET_TIMER_TASK, req->object, Tcis_TimerTask, size, print_timer_task(req), "", "");
		_GETDEF2_(TCI_CMD_GET_TIMER_TASK, req->object, Tcis_GetTimerTask, Tcis_TimerTask, 1024,
		          resp->object = req->object;
		          resp->nItems = 0, sizeof(*resp));

	case TCI_CMD_GET_PSP:
		LogV("TCI_CMD_GET_PSP, flags=%d\n", ((Tcis_GetPresetPointsReq *)buf)->flags);
		// get_psp_handler(handle, (Tcis_GetPresetPointsReq *)buf);
		break;
	case TCI_CMD_SET_PSP:
		LogV("TCI_CMD_SET_PSP\n");
		// set_psp_handler(handle, (Tcis_SetPresetPointsReq *)buf);
		break;
	case TCI_CMD_GET_PTZ_TRACK:
		LogD("TCI_CMD_GET_PTZ_TRACK: flags=0x%x\n", ((Tcis_GetPtzTrackReq *)buf)->flags);
		// get_ptz_track_handler(handle, (Tcis_GetPtzTrackReq *)buf);
		break;
	case TCI_CMD_SET_PTZ_TRACK:
		LogD("TCI_CMD_SET_PTZ_TRACK\n");
		// set_ptz_track_handler(handle, (Tcis_SetPtzTrackReq *)buf);
		break;
	case TCI_CMD_GET_OSD_REQ:
		// get_osd_handler(handle);
		break;
	case TCI_CMD_SET_OSD_REQ:
		// set_osd_handler(handle, (Tcis_SetOsdReq *)buf);
		break;

		_SETTER_(
		        TCI_CMD_SET_IPCONFIG, 0, struct IPCONFIG, do {} while (0),
		        "dhcp=%d, ip=%s, gw=%s, dns1=%s, dns2=%s", req->bDhcpEnabled, req->ip, req->gateway, req->dns1,
		        req->dns2);
		_GETDEF_(TCI_CMD_GET_IPCONFIG, 0, void, struct IPCONFIG, strcpy(resp.intf, "eth0");
		         resp.bDhcpEnabled = 1; strcpy(resp.dns1, "8.8.8.8"); strcpy(resp.dns2, "192.168.1.1");
		         strcpy(resp.ip, "192.168.1.30"); strcpy(resp.netmask, "255.255.255.0");
		         strcpy(resp.gateway, "192.168.1.1"););

	case TCI_CMD_SET_LIGHT: {
		Tcis_LightState t, *req = (Tcis_LightState *)buf;
		_info("TCI_CMD_SET_LIGHT: fMask:0x%x, on=%d, mode=%d, intensity=%d\n", req->fMask, req->on, req->mode,
		      req->intensity);
		memset(&t, 0, sizeof(t));
		TGC_loadSetting(TCI_CMD_SET_LIGHT, &t, sizeof(t));
		if (req->fMask & 1)
			t.on = req->on;
		if (req->fMask & 2)
			t.mode = req->mode;
		if (req->fMask & SETLIGHT_F_DELAYSHUT)
			t.delay_shutdown = req->delay_shutdown;
		if (req->fMask & SETLIGHT_F_INTENSITY)
			t.intensity = req->intensity;
		TGC_saveSetting(TCI_CMD_SET_LIGHT, req, sizeof(req));
		TciSendCmdRespStatus(handle, cmd, 0);
	} break;

		_GETDEF_(TCI_CMD_GET_LIGHT, 0, Tcis_GetLightReq, Tcis_LightState, resp.fMask = 0x03; resp.on = 1;
		         resp.intensity = 50);

	case TCI_CMD_ANSWERTOCALL:
		_info("TCI_CMD_ANSWERTOCALL, state=%d, more=%d\n", ((Tcis_AnswerToCall *)buf)->state,
		      ((Tcis_AnswerToCall *)buf)->more);
		break;
	case TCI_CMD_GET_RUNTIME_STATE:
		if (((Tcis_GetRuntimeStateReq *)buf)->state_name == RT_STATE_RECORDING) {
			Tcis_RuntimeStateResp state;
			state.state_name = RT_STATE_RECORDING;
			state.uState.iState = 0;
			TciSendCmdResp(handle, cmd, &state, sizeof(state));
		} else
			TciSendCmdRespStatus(handle, cmd, TCI_E_INVALID_PARAM);
		break;
	case TCI_CMD_LOCATE_IN_PIC:
		_info("TCI_CMD_LOCATE_IN_PIC, channel:%d, x:%f, y:%f\n", ((Tcis_LocateInPic *)buf)->channel,
		      ((Tcis_LocateInPic *)buf)->pos.x, ((Tcis_LocateInPic *)buf)->pos.y);
		break;

	case TCI_CMD_SET_PARKING_DET: {
		Tcis_ParkingDet *req = (Tcis_ParkingDet *)buf;
		_info("TCI_CMD_SET_PARKING_DET, flags=%d;sensitivity=%d,work_time=%d", req->flags, req->sensitivity,
		      req->work_time);
		Tcis_ParkingDet v = { 0, 3, 1, 12 };
		TGC_loadSetting(TCI_CMD_SET_PARKING_DET | (req->id << 16), &v, sizeof(v));
		if (req->flags & PARKINGDET_F_SENS)
			v.sensitivity = req->sensitivity;
		if (req->flags & PARKINGDET_F_WORKTIME)
			v.work_time = req->work_time;
		TGC_saveSetting(cmd, (const char *)&v, sizeof(v));
		TciSendCmdRespStatus(handle, cmd, 0);
	} break;

		_GETDEF_(TCI_CMD_GET_PARKING_DET, req->id, Tcis_GetParkingDetReq, Tcis_ParkingDet,
		         if (!(resp.flags = TGC_getInt("pd_flags"))) resp.flags = 3;
		         resp.sensitivity = 1; resp.work_time = 12);

	case 0x80000000: //自定义的命令
	{
		char resp[512];
		memset(resp, 'A', 511);
		resp[511] = 0;
		TciSendCmdResp(handle, cmd, resp, 512);
	} break;
	case TCI_CMD_AUDIOSTART: {
		TGC_muteAudio(0);
	} break;
	case TCI_CMD_AUDIOSTOP: {
		TGC_muteAudio(1);
	} break;
	case TCI_CMD_VIDEOSTART: {
	} break;
	case TCI_CMD_VIDEOSTOP: {
	} break;
	default:
		TciSendCmdRespStatus(handle, cmd, TCI_E_UNSUPPORTED_CMD);
		printf("non-handle cmd 0x%X, len=%d: ", cmd, size);
		{
			int i;
			for (i = 0; i < size; i++)
				printf("%02X ", ((unsigned char *)buf)[i]);
			printf("\n");
		}
	}

	return 0;
}

/**
 * Static Function
*/
static void setTciCallback(void)
{
	_tci_cb.get_info = get_info;
	_tci_cb.get_feature = get_device_feature;
	_tci_cb.get_state = get_device_state, _tci_cb.on_apmode_login = on_apmode_login;

	// _tci_cb.qrcode_start = on_qrcode_start;
	// _tci_cb.qrcode_get_y_data = on_get_y_data;
	// _tci_cb.qrcode_end = on_qrcode_end;

	// _tci_cb.on_ota_download_start = on_ota_download_start;
	// _tci_cb.on_ota_download_data = on_ota_download_data;
	// _tci_cb.on_ota_download_finished = on_ota_download_finished;

	_tci_cb.on_talkback_start = on_talkback_start;
	_tci_cb.talkback = talkback;
	_tci_cb.on_talkback_stop = on_talkback_stop;

	_tci_cb.snapshot = snapshot;
#if defined(TANGE_USE_AGT_MPI_STREAM)
	_tci_cb.set_wifi = set_wifi;
#endif
	_tci_cb.set_timezone = set_timezone;
	_tci_cb.set_time = set_time;
	_tci_cb.on_status = on_status;

	_tci_cb.request_iframe = request_iframe;
	_tci_cb.log = on_log;
	_tci_cb.request_iframe_ex = request_iframe_ex;
	_tci_cb.switch_quality = switch_quality;
}

static int get_info(TCIDEVICEINFO *info)
{
	const char *module_id = (char *)TGC_getSetting("module_id");
	strcpy(info->firmware_id, module_id ? module_id : "company_plat_device_001"); //要先向平台申请
	strcpy(info->vendor, "OEM");
	strncpy(info->device_type, "IPC", sizeof(info->device_type)); //设备类型为IPC, 目前没有使用
	const char *ver = (char *)TGC_getSetting("version");
	strcpy(info->firmware_ver, ver ? ver : "01000208"); //版本码: 8个数字
	const char *model = (char *)TGC_getSetting("model");
	strcpy(info->model, model ? model : "TG-IPC-001"); //产品型号
	return 0;
}

/* 设备能力级. 完整描述参见文档 */
static int get_device_feature(const char *key, char *buf, int bytes)
{
	const char *val = (char *)TGC_getFeature(key);
	if (val) {
		strncpy(buf, val, bytes);
		return 0;
	}
#if 1
	else
		return -1;
#else //Example for feature. More details can be found in testcase.ini and document.
	{
		/* default value */
		//printf("%s, get %s\n", __FUNCTION__, key);
		if (strcmp(key, "SupportPTZ") == 0) {
			strncpy(buf, "Yes", bytes);
		} else if (strcmp(key, "AutoTracking") == 0) {
			strncpy(buf, "Yes", bytes);
		} else if (strcmp(key, "DayNight") == 0) {
			strncpy(buf, "Yes", bytes);
		} else if (strcmp(key, "DoubleLight") == 0) {
			strncpy(buf, "Yes", bytes);
		} else if (strcmp(key, "Microphone") == 0) {
			strncpy(buf, "Yes", bytes);
		} else if (strcmp(key, "AlertSound") == 0) {
			strncpy(buf, "Yes", bytes);
		} else if (strcmp(key, "MD-Capabilities") == 0) {
			strncpy(buf, "Sensitivity", bytes);
		} else if (strcmp(key, "Cap-Defence") == 0) {
			strncpy(buf, "bundle", bytes);
		} else if (strcasecmp(key, "4G") == 0) {
			strcpy(buf, "No");
			//strcpy(buf, "iccid:8986031846206443846H");
		} else if (strcasecmp(key, "ExtInstructions") == 0) {
			strcpy(buf, "close-device");
		} else if (strcasecmp(key, "BatteryCam") == 0) {
			strcpy(buf, "No");
		} else if (strcasecmp(key, "RecordConf") == 0) {
			strcpy(buf, "Yes");
		} else if (strcasecmp(key, "DeviceType") == 0) {
			strcpy(buf, "IPC");
			//strcpy(buf, "DriveRec");
		} else {
			return -1;
		}
	}
	return 0;
#endif
}

/* 设备参数默认值 */
static int get_device_state(const char *key, char *buf, int bytes)
{
	const char *val = (char *)TGC_getState(key);
	if (val)
		strncpy(buf, val, bytes);
	//缺省云存储视频通道和码流
	else if (strcasecmp(key, "CVideoQuality") == 0) {
		//"stream:channel"
		//stream: 0-main, 1-sub
		strcpy(buf, "0:0");
	}
	//缺省p2p视频通道和码流
	else if (strcasecmp(key, "streamquality") == 0) {
		//"stream:channel"
		//stream: 0-main, 1-sub
		strcpy(buf, "1:0");
	}
	//返回 MAC,  这个是必须的
	else if (strcasecmp(key, "mac") == 0) {
		strcpy(buf, "112233445566");
	} else {
		return -1;
	}

	return 0;
}

//AP模式下的初始密码, user 未使用
static int on_apmode_login(const char *user, const char *key)
{
	LogI("user:%s, key:%s\n", user, key);
	return 0;
}

static int on_qrcode_start(void)
{
	LogI("return 0");
	return 0;
}

static int on_get_y_data(uint8_t **ppYBuff, int *width, int *height)
{
	if (loadY(ppYBuff, width, height) != 0) {
		SA_Sleep(1000);
		return -1;
	}

	SA_Sleep(100);
	return 0;
}

static void on_qrcode_end(uint8_t *pYBuff)
{
	LogI("");
	if (pYBuff)
		free(pYBuff);
}

FILE *ota_down_fp = NULL;
char firmware_path[48];
static int on_ota_download_start(const char *new_version, unsigned int size)
{
	return 0;
}

static int on_ota_download_data(const uint8_t *buff, int size)
{
	LogI("size:%d\n", size);
	if (ota_down_fp) {
		fwrite(buff, 1, size, ota_down_fp);
	}
	return 0;
}

static int on_ota_download_finished(int status)
{
	if (ota_down_fp)
		fclose(ota_down_fp);
	ota_down_fp = NULL;
	LogI("status:%d\n", status);
	if (status == 0) //ok
	{
		_ok("success!!\n");
		//.关闭看门狗；
		//关闭应用
		kill(getpid(), SIGKILL); //关闭应用
	} else //if(status == 1)
	{
		remove(firmware_path);
	}
	return 0;
}

static int on_talkback_start(void)
{
	TGC_initTalkback();
	return 0;
}

FILE *_fpTalk;
static int talkback(TCMEDIA at, const uint8_t *audio, int len)
{
	if (TCMEDIA_IS_AUDIO(at)) {
		TGC_AudioTalkCallback((char *)audio, len);
		return 0;
	} else {
		printf(TCMEDIA_IS_AUDIO(at) ? "." : "#");
		fflush(stdout);
		LogI("Play audio data. format:%d, length:%d", at, len);
		return 0;
	}
}

static void on_talkback_stop(void)
{
	if (_fpTalk) {
		fclose(_fpTalk);
		_fpTalk = NULL;
	}
	TGC_deinitTalkback();
}

static int snapshot(int type, uint8_t **ppJpg, int *size)
{
	return 0;
}

#if defined(TANGE_USE_AGT_MPI_STREAM)
static int set_wifi(int is_switching, const char *ssid, const char *key)
{
	LogI("is_switching:%d. ssid:%s, key:%s", is_switching, ssid, key);
	printf("----------0-0-Stop SoftAP Init-0-0--------------\n");
	int ret = -1;
	/* wifi status */
	lpw_wifi_state_t lpw_state;

	ret = lpw_wifi_set_sta_mode(gWifihd);
	if (ret < 0) {
		LogE("Set wifi STA mode fail!\n");
	}

	ret = lpw_wifi_get_status(gWifihd, &lpw_state);
	if (ret < 0) {
		LogE("LPW get status fail\n");
	}
	// else (ret == 0) {
	// 	LogI("ip %d:%d:%d:%d, gw:%d:%d:%d:%d, conn: %d\n", lpw_state.ip[0], lpw_state.ip[1], lpw_state.ip[2], lpw_state.ip[3],
	// 			lpw_state.gw[0], lpw_state.gw[1], lpw_state.gw[2], lpw_state.gw[3], lpw_state.conn);
	// }

	printf("----------0-0-Start STA mode Init-0-0--------------\n");

	/* scan target and result */
	lpw_wifi_scan_t sc;
	memset(&sc, 0, sizeof(lpw_wifi_scan_t));
	sc.ssid = calloc(WIFI_MAX_SSID_LEN + 1, sizeof(char));
	lpw_wifi_scan_result_t result;

	/* connect info */
	lpw_wifi_conn_t conn;
	memset(&conn, 0, sizeof(lpw_wifi_conn_t));
	conn.ssid = calloc(WIFI_MAX_SSID_LEN + 1, sizeof(char));
	conn.key = calloc(WIFI_MAX_KEY_LEN + 1, sizeof(char));

	strcpy(sc.ssid, ssid);
	sc.scan_type = WIFI_SSID_SCAN;

	int sc_ret = -1;
	int retry_times = 0;
	while (sc_ret != 0) {
		sc_ret = lpw_wifi_scan(gWifihd, &sc, &result);
		if (sc_ret == 0) {
			LogI("%s ...present\n", sc.ssid);

			strcpy(conn.ssid, ssid);
			strncpy(conn.key, key, WIFI_MAX_KEY_LEN);
			conn.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX;
			conn.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX;
			memset(&conn.bssid, 0, WIFI_MAC_LEN);
		} else {
			LogE("...doesn't exist\n");
		}

		if (retry_times > MAX_WIFI_RETRY_TIMES) {
			free(conn.ssid);
			free(conn.key);
			free(sc.ssid);
			return -EINVAL;
		}
		retry_times++;
		LogI(" scan retry: %d\n", retry_times);
	}

	retry_times = 0;
	while (1) {
		int result = lpw_wifi_connect_to(gWifihd, &conn);
		if (result != 0) {
			LogE("LPW connect Fail!\n");
		} else {
			LogI("LPW connect OK!\n");
			break;
		}

		if (retry_times > MAX_WIFI_RETRY_TIMES) {
			free(conn.ssid);
			free(conn.key);
			free(sc.ssid);
			return -EINVAL;
		}
		retry_times++;
		LogI(" TCP connect retry: %d\n", retry_times);
	}

	ret = lpw_wifi_get_status(gWifihd, &lpw_state);
	if (ret == 0) {
		LogI("ip %d:%d:%d:%d, gw:%d:%d:%d:%d, conn: %d\n", lpw_state.ip[0], lpw_state.ip[1], lpw_state.ip[2],
		     lpw_state.ip[3], lpw_state.gw[0], lpw_state.gw[1], lpw_state.gw[2], lpw_state.gw[3],
		     lpw_state.conn);
	} else {
		LogE("LPW get status fail\n");
	}

	// Save Wi-Fi connection information.

	char cmd_ssid[128];
	char cmd_psk[128];
	memset(cmd_ssid, 0, 128);
	memset(cmd_psk, 0, 128);

	sprintf(cmd_ssid, "sed -i 's/\\(ssid=\"\\).*/\\1%s\"/' /usrdata/root/wpa_supplicant.conf", ssid);
	sprintf(cmd_psk, "sed -i 's/\\(psk=\"\\).*/\\1%s\"/' /usrdata/root/wpa_supplicant.conf", key);

	ret = system("cp -af /etc/wpa_supplicant.conf /usrdata/root");
	if (ret != 0) {
		LogE(" Copy wpa_supplicant.conf fail\n");
	}
	ret = system("sed -i '1,13!d' /usrdata/root/wpa_supplicant.conf");
	if (ret != 0) {
		LogE(" write wpa_supplicant.conf fail\n");
	}
	ret = system(cmd_ssid);
	if (ret != 0) {
		LogE(" write wifi ssid fail\n");
	}
	ret = system(cmd_psk);
	if (ret != 0) {
		LogE(" write wifi psk fail\n");
	}

	free(conn.ssid);
	free(conn.key);
	free(sc.ssid);
	free(cmd_ssid);
	free(cmd_psk);

	printf("---------------0-0-STA mode Done-0-0---------------\n");
	LogI(" set wifi Done !!!\n");

	return 0;
}
#endif

static int tzoffset(const char *tzs)
{
	char *ptr;
	int o = strtol(tzs + 3, &ptr, 10) * 60;
	if (*ptr == ':') {
		o += strtol(ptr, NULL, 10);
	}
	o *= 60;
	_info("tz=%s, offset=%d\n", tzs, o);
	return o;
}

static int set_timezone(const char *tzs)
{
	LogI("timezone: %s\n", tzs);
	char buff[16] = { 0 };
	if (strlen(tzs) < strlen("GMT-8"))
		return 0;

	sprintf(buff, "%s\n", tzs);
	if (buff[3] == '-')
		buff[3] = '+';
	else if (buff[3] == '+')
		buff[3] = '-';

#ifdef __UCLIBC__
#define TZ_ETC_FILE "/etc/TZ"
	remove(TZ_ETC_FILE);
	FILE *fd = fopen(TZ_ETC_FILE, "wb");
	if (!fd) {
		perror("set timezone");
		return -1;
	}
	fwrite(buff, 1, strlen(buff), fd);
	fclose(fd);
#else
	setenv("TZ", buff, 1);
#endif
	tzset();
	return 0;
}

static int set_time(time_t time)
{
	struct timeval tv;
	tv.tv_sec = time;
	tv.tv_usec = 0;
	settimeofday(&tv, NULL);

	printf("after set time, time=%d\n", (int)SA_time(NULL));

	return 0;
}

static char *wk_reason = NULL;
static int on_status(int status, const void *pData, int len)
{
	switch (status) {
	case STATUS_LOGON:
		LogI("STATUS_LOGON\n");
		Touch("registered.ini");
		resetIdleTime();
		break;
	case STATUS_LOGOFF:
		LogI("STATUS_LOGOFF\n");
		break;
	case STATUS_DELETED:
		LogI("STATUS_DELETED\n");
		Remove("registered.ini");
		Remove("config.ini");
		Remove("config.dat");
		kill(getpid(), SIGINT);
		break;
	case STATUS_UPDATE_SERVICE: {
		LogI("STATUS_UPDATE_SERVICE\n");
		TCISERVICEINFO *svc = (TCISERVICEINFO *)pData;
		printf("STATUS_UPDATE_SERVICE type: %d, will expiration at utc time %ld", svc->serviceType,
		       svc->expiration);
		if (svc->serviceType == ECGS_TYPE_AI && svc->objects) {
			const char *o = svc->objects;
			printf(", supported objects: ");
			while (*o) {
				printf("%s ", o);
				o += strlen(o) + 1;
			}
		}
		printf("\n");
	} break;
	case STATUS_AP_CONNECT:
		LogI("STATUS_AP_CONNECT\n");
		break;
	case STATUS_STREAMING:
		LogI("Num of clients in streaming: %d\n", (int)(long)pData);
		resetIdleTime();
		break;
	case STATUS_START_TELNETD: {
		static int _telnetd_started = 0;
		if (!_telnetd_started) {
			CallSystem("telnetd");
			_telnetd_started = 1;
		}
	} break;
	case STATUS_IDLE:
#if defined(TANGE_USE_AGT_MPI_STREAM)
		/* Open LPW device */
		do {
			gWifihd = openLpwHandler();
		} while (gWifihd == 0);

		/* Go to sleep */
		if (len == 0) {
			printf(">>>> STATUS_IDLE, sleep now ...");
			/* Force reboot if the system is not going to sleep (the 1st attempt) */
			msleep(5000);
			LogE("The device is not sleeping (timeout=5s). Rebooting...\n");
			sync();
			if (reboot(LINUX_REBOOT_CMD_RESTART) == -1) {
				LogE("Failed to reboot the system.\n");
			}
			/* Force reboot if the system is not going to sleep (the 2nd attempt) */
			msleep(10000);
			LogE("The device is not sleeping (timeout=10s). Rebooting...\n");
			if (reboot(LINUX_REBOOT_CMD_RESTART) == -1) {
				LogE("Still failed to reboot the system!!!\n");
			}
			break;
		}

		printf(">>>> STATUS_IDLE, query for sleepable ...\n");
		/* Query per 30 seconds */
		if (!shouldSleep()) {
			return 30;
		}

		/* Prepare Tange TCP wake-up. Retry per second if it is failed to prepare. */
		LogI(">>>> STATUS_IDLE, setting up Tange TCP wake-up event.\n");
		TciSetWakeupReason(0, ECEVENT_USER_DEFINED, wk_reason, 50, 50);
		if (prepareTgTcpWakeup(gWifihd) < 0) {
			return 1;
		}
		if (goto_idlestate() < 0) {
			return 1;
		}
		/* Ready to sleep */
		return 0;
#else
		return 180;
#endif
	case STATUS_AI:
		printf("<<%s>> is detected.\n", ((struct AiResult *)pData)->name);
		break;

	case STATUS_SWD_TIMEOUT:
		printf("soft watchdog for %s timeouted.\n", (const char *)pData);
		break;

	case STATUS_USER_DATA:
		break;

	case STATUS_INCALL:
		break;

	case STATUS_TRANSFER_MONITOR:
		break;

	default:
		LogI("unkown status [%d]\n", status);
	}
	return 0;
}

static int request_iframe(int vstream)
{
	// To switch the resolution
	LogI("Request I-Frame of stream %d", vstream);
	return 0;
}

static int on_log(int action, char *path)
{
	switch (action) {
	case 0:
		printf("start long log\n");
		break;
	case 1:
		printf("stop long log\n");
		break;
	case 2: //if you have other file than /tmp/icam365.log, return its path
		strcpy(path, "/tmp/my.log");
		printf("return my private file: %s\n", path);
		break;
	}
	return 0;
}

static int request_iframe_ex(int channel, int vstream)
{
	// To switch the resolution
	LogI("Request I-Frame of stream %d:%d", channel, vstream);
	return 0;
}

static void switch_quality(int channel, int stream, const char *qstr)
{
	LogI("Switch quality of stream:%d to qstr:%s\n", stream, qstr);
	TGC_switchVideoSystem(stream, qstr);
}

static int Remove(const char *fn)
{
	char path[128];
	sprintf(path, "%s/%s", _cwd, fn);
	return remove(path);
}

static int Touch(const char *fn)
{
	char path[128];
	sprintf(path, "%s/%s", _cwd, fn);
	int fd = open(path, O_RDONLY | O_CREAT, 0666);
	if (fd > 0)
		close(fd);
	return fd > 0;
}

static int loadY(unsigned char **ppY, int *width, int *height)
{
	if (!*ppY) {
		*ppY = (uint8_t *)malloc(1280 * 720);
		*width = 1280;
		*height = 720;
	}
	//fill Y-data into *ppBuff...
	return 0;
}

static int msleep(long msec)
{
	struct timespec ts;
	int res;

	if (msec < 0) {
		errno = EINVAL;
		return -1;
	}

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;

	do {
		res = nanosleep(&ts, &ts);
	} while (res && errno == EINTR);

	return res;
}

static char _cmdline[256];
static int goto_idlestate(void)
{
	int ret = 0;

#if defined(TANGE_USE_AGT_MPI_STREAM)
	static int sleep_timeout = 180;
	static int sleep_response = 50;
	ret = goToSleep(gWifihd, sleep_timeout, sleep_response);
	if (ret < 0) {
		LogE("LPW is failed to start the sleep procedure.\n");
		goto error;
	}

error:
#endif
	return ret;
}

static int initLpwPowerMgmt(lpw_handle hd)
{
	int ret = 0;

	static int gpio = 10;
	static int direction = 1;
	static int active_status = 0;
	static int adc_ch = 3;
	static int off_thre = 640;
	static int on_thre = 747;

	ret = lpw_pm_init(hd, gpio, active_status, adc_ch, off_thre, on_thre);
	if (ret < 0) {
		goto error;
	}

	ret = lpw_gpio_set_dir(hd, gpio, direction, active_status);
	if (ret < 0) {
		goto error;
	}

error:
	return ret;
}

static int accessLpwTcp(lpw_handle hd, in_addr_t ser_ip, in_port_t ser_port)
{
	int ret = 0;

	/* print human-readable TCP server address and port */
	char addr_str[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN is typically 16 bytes
	struct in_addr addr;
	addr.s_addr = ser_ip;
	if (inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN) == NULL) {
		ret = -1;
	}

	/* connect to the server via TCP */
	/* !!! important notice !!!
	 * We must pass the port number in host byte-order due to liblpw's bug.
	 */
	if (ret != -1) {
		LogI("Connecting to the TCP Server: %s:%d...\n", addr_str, ntohs(ser_port));
		LogI("hd: %d, ser_ip: %u, ser_port: %d\n", hd, ser_ip, ser_port);
	}
	ret = lpw_tcp_connect_to(hd, ser_ip, ntohs(ser_port));
	if (ret < 0) {
		LogE("Failed to connect to the TCP Server.\n");
		goto error;
	}

error:
	return ret;
}

static int prepareTgTcpWakeup(lpw_handle hd)
{
	int ret = 0;
	int i = 0;

	LogI("Step 1: Getting the Tange wake-up server list...\n");
	Ipv4Addr servers[3];
	ret = getTgWkServerList(servers);
	if (ret < 0) {
		LogE("SoC is failed to get the Tange wake-up servers.\n");
		goto error;
	}
	LogI("Received the Tange wake-up server list.\n");
	for (i = 0; i < 3; ++i) {
		LogI("Server %d = %s:%d\n", i + 1, inet_ntoa(*((struct in_addr *)&servers[i].ip)),
		     ntohs(servers[i].port));
	}

	LogI("Step 2: Sending the auth string to the Tange wake-up server.\n");
	ret = sendTgAuthString(hd, servers);
	if (ret == -1) {
		LogE("LPW is failed to connect to the Tange wake-up server.\n");
		goto error;
	}
	if (ret == -2) {
		LogE("LPW is failed to get the rand key from the Tange wake-up server.\n");
		goto error;
	}
	if (ret == -3) {
		LogE("SoC is failed to prepare the auth string for the Tange wake-up server.\n");
		goto error;
	}
	if (ret == -4) {
		LogE("LPW is failed to send the auth string to the Tange wake-up server.\n");
		goto error;
	}

	LogI("Step 3: Setting up the Tange heartbeat and wake-up pattern.\n");
	ret = setLpwWkPattern(hd);
	if (ret == -1) {
		LogE("Failed to initialise the LPW power control.\n");
		goto error;
	}
	if (ret == -2) {
		LogE("LPW is failed to configure the Tange wake-up event.\n");
		goto error;
	}

	LogI("Step 4: Syncing cached writes to persistent storage before going to sleep.\n");
	sync();

error:
	return ret;
}

static int getTgWkServerList(Ipv4Addr *servers)
{
	int server_num = 0;
	server_num = TciGetWakeupServers(servers);
	if (server_num <= 0) {
		return -1;
	}
	return 0;
}

static int sendTgAuthString(lpw_handle hd, Ipv4Addr *servers)
{
	int ret = 0;
	int i = 0;

	unsigned char session_key[64];
	int session_key_len = 0;
	int auth_str_len = 0;
	int send_len = 0;

	/* Step 1: Accessing the Tange wake-up server */
	ret = accessLpwTcp(hd, servers[0].ip, servers[0].port);
	if (ret < 0) {
		return -1;
	}
	/* Waiting for the LPW to be ready */
	msleep(50);

	/* Step 2: Receiving the Tange session key */
	for (i = 0; i < 100; ++i) {
		ret = lpw_tcp_recv(hd, session_key, sizeof(session_key));
		if (ret == 0) {
			msleep(50);
			continue;
		}
		break;
	}
	if (ret <= 0) {
		lpw_tcp_disconnect(hd);
		return -2;
	}

	/* Step 3: Preparing the auth string */
	session_key_len = ret;
	session_key[session_key_len] = 0;
	const unsigned char *auth_str = TciPrepareAuthString((const char *)session_key, &auth_str_len);
	if (auth_str == NULL || auth_str_len <= 0) {
		lpw_tcp_disconnect(hd);
		return -3;
	}

	/* Step 4: Sending the auth string to the Tange wake-up server */
	send_len = lpw_tcp_send(hd, (unsigned char *)auth_str, auth_str_len);
	if (send_len <= 0) {
		lpw_tcp_disconnect(hd);
		return -4;
	}

	return 0;
}

static int setLpwWkPattern(lpw_handle hd)
{
	int ret = 0;

#if defined(TANGE_USE_AGT_MPI_STREAM)
	static unsigned char hb_pack[64] = { 0 };
	static unsigned char wk_pack[6] = { 0x98, 0x3b, 0x16, 0xf8, 0xf3, 0x9c };
	static tcp_wake_t tcp_wk_pattern = {
		.keep_alive_pack_len = 64,
		.keep_alive_period = 15,
		.wake_pack_len = 6,
		.bad_conn_handle = 1,
		.keep_alive_pack = hb_pack,
		.wake_pack = wk_pack
	};

	ret = initLpwPowerMgmt(gWifihd);
	if (ret < 0) {
		ret = -1;
		goto error;
	}

	ret = lpw_sleep_tcp_wake_en(hd, &tcp_wk_pattern);
	if (ret < 0) {
		ret = -2;
		goto error;
	}

error:
#endif
	return ret;
}

static int goToSleep(lpw_handle hd, int timeout, int response)
{
	int ret = 0;

	ret = lpw_sleep_start(hd, timeout, response);

	return ret;
}

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

/* return lpw device handle or 0 for failure */
lpw_handle openLpwHandler(void)
{
	return lpw_open();
}

void closeLpwHandler(lpw_handle hd)
{
	lpw_tcp_disconnect(hd);
	lpw_close(hd);
}

void resetIdleTime(void)
{
#if defined(TANGE_USE_AGT_MPI_STREAM)
	LogI("Reset idle time.\n");
	gettimeofday(&gNoSessionTime, NULL);
#endif
}

int shouldSleep(void)
{
#if defined(TANGE_USE_AGT_MPI_STREAM)
	struct timeval now;
	static time_t sleep_timeout = 10;
	gettimeofday(&now, NULL);

	/* always awake if there is a keep-alive file */
	if (access(KEEP_ALIVE_FILE, F_OK) == 0) {
		return 0;
	}

	if (now.tv_sec - gNoSessionTime.tv_sec > sleep_timeout) {
		LogI("No session for a while. Going to sleep...\n");
		return 1;
	}
#endif
	return 0;
}
