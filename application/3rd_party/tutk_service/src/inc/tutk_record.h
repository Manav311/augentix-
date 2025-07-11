
#ifndef TUTK_RECORD_H_
#define TUTK_RECORD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "NebulaJsonAPIs.h"

#define MP4_SAVE_PATH "/mnt/sdcard/record/"
#define TMP_MP4_FILE_0 "/mnt/nfs/tutk/record/tmp0.mp4"

#define CLIP_TMP_PATH "/tmp/save/"
#define CLIP_SAVE_PATH "/tmp/"

#define CLIP_SIZE (10)
#define CLIP_TMP_CNT (11) /*store max 10 clip*/

#define VIDEO_FPS (30)
#define VIDEO_TIME_SCALE (90000)
#define VIDEO_SAMPLE_DURATION (VIDEO_TIME_SCALE / VIDEO_FPS)

#define SPS_LEN 16
#define SPS_START SPS_LEN - 4

#define PPS_LEN 8
#define PPS_START PPS_LEN - 4

#define AUDIO_BUF_SIZE_RECORD (1280)
#define VIDEO_BUF_SIZE_RECORD (512000)
#define SD_MIN_AVAIL (100)

enum trigger_stat { NOTRIGGER, TRIGGERRED, RECORDED };

int TUTK_record_stop(void);
int TUTK_record_init(void);
int HandlePlaybackControl(int sid, int ctrl_value, const char *file_name);
void RegEditClientPlaybackMode(int sid, int playback_mode);
void GetRecordFileList(unsigned int start_time, unsigned int end_time, const char *event_type,
                       NebulaJsonObject **response);

#ifdef __cplusplus
}
#endif

#endif