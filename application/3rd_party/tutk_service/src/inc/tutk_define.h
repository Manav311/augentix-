#ifndef TUTK_DEFINE_H_
#define TUTK_DEFINE_H_

#include "NebulaDevice.h"
#include "IOTCDevice.h"
#include "AVServer.h"
#include "AVAPIs.h"
#include "VSaaS.h"
#include "P2PCam/AVFRAMEINFO.h"
#include "P2PCam/AVIOCTRLDEFs.h"
#include "disposable_params.h"
//#include "playback/playback.h"
#include "Nebula_User_PSK_Manager/pskManager.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "log_define.h"

#define ENABLE_DTLS 0 // 0:disable DTLS; 1:enable DTLS
#define ENABLE_RESEND 1 // 0:disable RESEND; 1:enable RESEND

#define BIND_FLAG "/usrdata/active_setting/bind_flag"
#define DEFAULT_DEVICE_PROFILE "/usrdata/active_setting/profile.txt"
#define DEFAULT_DEVICE_SETTING "/usrdata/active_setting/device_settings.txt"
#define DEFAULT_IDENTITY_FILE "/usrdata/active_setting/identities_list.txt"
#define DEFAULT_UDID "/usrdata/active_setting/UDID"
#define ADC_WARING_FILE "/usrdata/active_setting/adc_warning"
#define KEEP_ALIVE_FILE "/tmp/augentix/keep_alive"

#define MAX_LEVEL_CNT 11
#define DEFAULT_LOW_BATTERY_WARNING (50)

/**
 * @brief structure to store the power management information
 * @details
 */
typedef struct {
	uint8_t gpio; // power control gpio
	uint8_t active_status; // 0: active low; 1: active high
	uint8_t battery_adc_ch; // adc channel for power management
	uint16_t off_thre; // power off threshold
	uint16_t on_thre; // power on threshold
} PWR_MGMT;

#define GPIO_OUTPUT 1

typedef struct {
	unsigned int adc_val;
	unsigned int level;
} BatteryLevel;

typedef struct {
	unsigned int adc_chn;
	unsigned int thres;
	int detect_event_gpio;
} PIRADCLevel;

typedef struct tutk_configs {
	PWR_MGMT pwr_mgmt;
	unsigned int no_session_timeout;
	unsigned int sleep_timeout;
	unsigned int sleep_response;
	unsigned int video_chn;
	BatteryLevel battery_level[MAX_LEVEL_CNT];
	unsigned int low_battery_warning;
	PIRADCLevel pir_adc;
} TutkConfigs;

#define SDCARD_RECORD_DIR "/mnt/sdcard/"

/* wifi pair */
#define WLAN_DEV "wlan0"
#define WPA_SUPPLICANT_CONF_FILE "/etc/wpa_supplicant.conf"

/* Audio Flow control */
#define RESEND_FRAME_COUNT_THRESHOLD_TO_ENABLE_I_FRAME_ONLY 30
#define RESEND_FRAME_COUNT_THRESHOLD_TO_DISABLE_I_FRAME_ONLY 10

#define RECORD_BUF_SIZE (1024 * 300)

#define AUDIO_BUF_SIZE 1024
#define AUDIO_CODEC MEDIA_CODEC_AUDIO_PCM

#define MAX_CLIENT_NUMBER 10
#define SERVTYPE_STREAM_SERVER 0

#define GO_TO_SLEEP_AFTER_NO_SESSION_WITH_SEC 10 /*connect after 30 sec, device will go to sleep*/
#define MAX_WAKEUP_PATTERN_LENGTH 100
#define AV_LIVE_STREAM_CHANNEL 0

/* SD card related settings */
#define LINUX_SD_DEV_FILE "/dev/mmcblk0"

#define SD_MOUNT_PATH "/mnt/sdcard"
#define CLIP_STORED_PATH "/mnt/sdcard/record"
#define CLIP_UPLOAD_PATH "/mnt/sdcard/record/uploaded"
#define LINUX_SD_DEVP1_FILE "/dev/mmcblk0p1"
#define LINUX_MOUNT_INFO "/proc/mounts"

/* Playback control */
#define PLAYBACK_PAUSE 0
#define PLAYBACK_START 1
#define PLAYBACK_STOP 2
#define AV_PLAYBACK_CHANNEL 1

#define MAX_WIFI_LIST_NUM (5)

enum BindType { DISABLE_BIND, SERVER_BIND, LOCAL_BIND, BIND_TYPE_NUM };

typedef struct {
	int av_index;
	unsigned char enable_video;
	unsigned char enable_audio;
	unsigned char two_way_stream;
	unsigned char enable_record_video;
	unsigned char enable_speaker;
	pthread_rwlock_t lock;
	char playback_file[64];
	FILE *playback_file_fd;
	FILE *playback_file_info_fd;
	FILE *playback_audio_fd;
	int do_clean_buffer;
	int do_clean_buffer_done;
	int wait_key_frame;
	int send_iframe_only;
	/*playback*/
	pthread_rwlock_t sLock;
	int playback_a_sample;
	int playback_v_sample;
	int aIdx;
	int vIdx;
	char filename_with_path[128];
} AV_Client;

int nebula_device_push_notification(const char *push_data, int event);

/**************************************************
 * TUTK functions
***************************************************/
void TUTK_runSystemCmdWithRetry(char *excmd);
int TUTK_exeSystemCmd(char *excmd);
int TUTK_forkIndependentProc(char *prog, char **arg_list);

unsigned int GetTimeStampMs();
int StartAvServer(int sid, unsigned char chid, unsigned int timeout_sec, unsigned int resend_buf_size_kbyte,
                  int *stream_mode);

#endif /* TUTK_DEFINE_H_ */
