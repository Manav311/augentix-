/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/*********************************************************************************
  *Copyright(C),2015-2020,
  *TUYA
  *www.tuya.comm
  *FileName:    tuya_ipc_motion_detect_demo
**********************************************************************************/

/*
 * Caution:
 *   Include mpi_base_types.h in the very first one.
 *   In order to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_system_control_demo.h"
#include "tuya_ipc_dp_handler.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_motion_detect_demo.h"

//AI detect should enable SUPPORT_AI_DETECT
#define SUPPORT_AI_DETECT 0
#if SUPPORT_AI_DETECT
#include "tuya_ipc_ai_detect_storage.h"
#endif

#define SUPPORT_CLOUD_STORAGE 0
#if SUPPORT_CLOUD_STORAGE
#include "tuya_ipc_cloud_storage.h"
#endif

#include "mpi_enc.h"
#include "avftr_conn.h"
#include "avftr.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


typedef struct _event_record_s {
	int post_record_time;
	int min_event_time;
	int max_event_time;
} event_record_s;

int g_get_jpeg_snapshot = 0;

extern StateDataPoint requested;
extern StateDataPoint acknowledged;
extern StateDataPoint active;

extern int vftrUnxSktClientFD;
extern AVFTR_CTX_S *avftr_res_shm_client;
int gmotion_alarm_is_triggerd = FALSE;
pthread_mutex_t g_snapshot_mutex = PTHREAD_MUTEX_INITIALIZER;
AVFTR_MD_CTX_S *vftr_md_ctx = NULL;
event_record_s gevent_record = { 0 };

#define MOTION_DIR_REPORT_CNT_NUM (10)
#define MOTION_DIR_REG_OFFSET (4)

typedef struct {
	union {
		unsigned int val;
		struct {
			unsigned char dir;
			unsigned char reg_left;
			unsigned char reg_right;
		};
	};
} motion_info_s;

typedef struct {
	unsigned char dir_cnt;
	unsigned char reg_left_cnt;
	unsigned char reg_right_cnt;
} event_timer_s;

motion_info_s motion_info = { { 0 } };
motion_info_s prev_motion_info = { { 0 } };
event_timer_s event_timer = { .dir_cnt = 0, .reg_left_cnt = 3, .reg_right_cnt = 3 };

extern TUYA_AG_CONF_S g_ag_conf;
#define AG_GET_CONF() (&g_ag_conf)

static int findMdCtx(MPI_WIN idx, AVFTR_MD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_MD_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1 && !ctx[i].en) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static inline int genMdAlarm(AVFTR_MD_CTX_S *md_ctx)
{
	return md_ctx->total_alarm;
}

//#ifdef TUYA_DP_MOTION_DIR
static int findAroiCtx(MPI_WIN idx, AVFTR_AROI_CTX_S *ctx)
{
	int i = 0;
	int find_idx = -1;

	for (i = 0; i < AVFTR_AROI_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
			break;
		}
	}
	return find_idx;
}

static void getMotionDir(MPI_WIN idx, motion_info_s *info)
{
	AVFTR_AROI_CTX_S *aroi_ctx = avftr_res_shm_client->vftr.aroi_ctx;
	int enable_idx = findAroiCtx(idx, aroi_ctx);
	if (enable_idx < 0) {
		*info = (motion_info_s) { { 0 } };
		return;
	}
	if (aroi_ctx[enable_idx].en) {
		info->dir = aroi_ctx[enable_idx].motion_dir;
		info->reg_left = (aroi_ctx[enable_idx].motion_reg & AVFTR_MOTION_LEFT) ? 1 : 0;
		info->reg_right = (aroi_ctx[enable_idx].motion_reg & AVFTR_MOTION_RIGHT) ? 1 : 0;
	}
	//printf("%s:%d motion: %02x\n", __func__, __LINE__, motion_dir);
	return;

}

static void report_motion_dir(const motion_info_s *motion, motion_info_s *prev_motion, int *motion_cnt)
{
	// report every MOTION_DIR_REPORT_CNT_NUM loops
	if (*motion_cnt & (1 << MOTION_DIR_REPORT_CNT_NUM)) {
		//printf("%s[%d] motion info dir:%d l:%d r:%d %d\n", __func__, __LINE__, motion->dir, motion->reg_left, motion->reg_right, *motion_cnt);
		if ((motion->dir != prev_motion->dir) && (event_timer.dir_cnt == 0)) {
			prev_motion->dir = motion->dir;
			IPC_APP_report_motion_direction(motion->dir);
		}

		if ((motion->reg_left != prev_motion->reg_left) && (event_timer.reg_left_cnt == 0)) {
			prev_motion->reg_left = motion->reg_left;
			IPC_APP_report_motion_reg_left(motion->reg_left);
			event_timer.reg_left_cnt = 3;
		}

		if ((motion->reg_right != prev_motion->reg_right) && (event_timer.reg_right_cnt == 0)) {
			prev_motion->reg_right = motion->reg_right;
			IPC_APP_report_motion_reg_right(motion->reg_right);
			event_timer.reg_right_cnt = 3;
		}

		if (event_timer.dir_cnt > 0)
			--event_timer.dir_cnt;
		if (event_timer.reg_left_cnt > 0)
			--event_timer.reg_left_cnt;
		if (event_timer.reg_right_cnt > 0)
			--event_timer.reg_right_cnt;

		*motion_cnt = 1;
	} else {
		*motion_cnt <<= 1;
	}
}

//According to different chip platforms, users need to implement whether there is motion alarm in the current frame.
int fake_md_status = 0;

int get_motion_status(int *enable)
{
	int alarm = 0;
	MPI_WIN iva_idx = MPI_VIDEO_WIN(0, 0, 0);
	//int ret = 0;
	//	PR_INFO("%s:[%s:%d] retval:%d\n",__FILE__,__func__,__LINE__, fake_md_status);
	*enable = 0;
	vftr_md_ctx = (AVFTR_MD_CTX_S *)avftr_res_shm_client->vftr.md_ctx;
	int enable_idx = findMdCtx(iva_idx, vftr_md_ctx, NULL);
	if (enable_idx < 0) {
		PR_ERR("Can't find iva idx %d\n",iva_idx.chn);
		return 0;
	}
	*enable = vftr_md_ctx[enable_idx].en;
	if (vftr_md_ctx[enable_idx].en) {
		alarm = genMdAlarm(&vftr_md_ctx[enable_idx]);
	} else {
		alarm = 0;
	}

	//	PR_INFO("%s:[%s:%d] retval:%d enable:%d mden;%d\n",__FILE__,__func__,__LINE__, fake_md_status, enable_idx, vftr_md_ctx[enable_idx].en);
	return alarm;
}

//According to different chip platforms, users need to implement the interface of capture.
void get_motion_snapshot(char *snap_addr, unsigned int *snap_size)
{
	int i = 0;
	unsigned int offset = 0;
	MPI_ECHN snapshot_chn;
	MPI_STREAM_PARAMS_S param;
	MPI_BCHN bchn;
	UINT32 err;

	pthread_mutex_lock(&g_snapshot_mutex);

	PR_INFO("Taking snapshot of encoder channel (c=%d)\n", 1);

	snapshot_chn = MPI_ENC_CHN(3);
	bchn = MPI_createBitStreamChn(snapshot_chn);

	err = MPI_ENC_getChnFrame(MPI_ENC_CHN(1), &param, -1);
	if (err != MPI_SUCCESS) {
		PR_ERR("Failed to take snapshot.\n");
		pthread_mutex_unlock(&g_snapshot_mutex);
		return;
	}

	for (i = 0; (UINT32)i < param.seg_cnt; i++) {
		memcpy(snap_addr + offset, param.seg[i].uaddr, param.seg[i].size);
		offset += param.seg[i].size;
	}

	*snap_size = offset;

	err = MPI_ENC_releaseChnFrame(MPI_ENC_CHN(1), &param);

	err = MPI_destroyBitStreamChn(bchn);

	pthread_mutex_unlock(&g_snapshot_mutex);
	return;
}

//According to different chip platforms, users need to implement the interface of capture.
VOID tuya_ipc_get_snapshot_cb(char *pjbuf, unsigned int *size)
{
	get_motion_snapshot(pjbuf, size);
}

VOID *thread_md_proc(VOID *arg __attribute__((unused)))
{
	int motion_flag = 0;
	int siren_is_triggerd = FALSE;
	char snap_addr[100 * 1024] = { 0 }; //Snapshot maximum size is 100KB
	unsigned int snap_size = 0;
	int event_start = 0;
	int event_end = 0;
	int event_renew = 0;
	int md_enable = 0;
	struct timeval t1 = { 0 };
	event_record_s *ev_rec = &gevent_record;
	MPI_WIN iva_idx = MPI_VIDEO_WIN(0, 0, 0);
	int motion_dir_report_cnt = 1;
	TIME_T current_time;
#if SUPPORT_CLOUD_STORAGE
	EVENT_ID event_id;
#endif
	while (1) {
		usleep(100 * 1000);
		tuya_ipc_get_utc_time(&current_time);
		motion_flag = get_motion_status(&md_enable);
		getMotionDir(iva_idx, &motion_info);
		report_motion_dir(&motion_info, &prev_motion_info, &motion_dir_report_cnt);
		//PR_DEBUG("motion info dir:%d l:%d r:%d\n", motion->dir, motion->reg_left, motion->reg_right);

        if ((eventDetectCntr > 0) &&
            (gmotion_alarm_is_triggerd == TRUE)) { // Keep track of events when wlan0 disapper for reboot
            eventDetectCntr += 1;
        }
	if (motion_flag) {
		/*reset event end timer*/
		event_end = 0;
		if (!gmotion_alarm_is_triggerd) {
			gettimeofdayMonotonic(&t1, NULL);
			event_start = t1.tv_sec;
			event_renew = t1.tv_sec + ev_rec->max_event_time;
			siren_is_triggerd = TRUE;
			IPC_APP_Notify_Siren(1);
			gmotion_alarm_is_triggerd = TRUE;
			//start Local SD Card Event Storage
			tuya_ipc_ss_start_event();
			get_motion_snapshot(snap_addr, &snap_size);
			if (snap_size > 0) {
#if SUPPORT_CLOUD_STORAGE
				//Report Cloud Storage Events
				//whether order type is event or continuous, a list of events is presented on APP for quick jumps.
				event_id = tuya_ipc_cloud_storage_event_add(
					snap_addr, snap_size, EVENT_TYPE_MOTION_DETECT, MAX_CLOUD_EVENT_DURATION);
				if (event_id == INVALID_EVENT_ID) {
					printf("fail to add cloud storage event\n");
					// abnormal process here
				}
#endif
				//					md_enable = IPC_APP_get_alarm_function_onoff();

				//NOTE!! this md_enable is ONLY used for md notification message control. Not for cloud or local storage
				if (md_enable) {
#if SUPPORT_AI_DETECT
					// ai detect process will send several snapshots to cloud AI server,
					// and automatically send alarm message only if target is detected.
					// return NON-0 if no ai detect service bill exists or any other failure, then use nomal md notification api
					if (0 != tuya_ipc_ai_detect_storage_start()) {
						tuya_ipc_notify_motion_detect(snap_addr, snap_size,
							                      NOTIFICATION_CONTENT_JPEG);
					}
#else
					//When user opens the detection alarm on the APP, it pushes pictures and messages to the message center (the API is a blocking interface).
					tuya_ipc_notify_motion_detect(snap_addr, snap_size, NOTIFICATION_CONTENT_JPEG);
#endif
				}
			}

			/*NOTE:
                ONE:Considering the real-time performance of push and storage, the above interfaces can be executed asynchronously in different tasks.
                TWO:When event cloud storage is turned on, it will automatically stop beyond the maximum event time in SDK.
                THREE:If you need to maintain storage for too long without losing it, you can use the interface (tuya_ipc_ss_get_status and tuya_ipc_cloud_storage_get_event_status).
                      to monitor whether there are stop event videos in SDK and choose time to restart new events
                */
		} else {
			//Storage interruption caused by maximum duration of internal events, restart new events
			if (SS_WRITE_MODE_EVENT == tuya_ipc_ss_get_write_mode() &&
			    E_STORAGE_STOP == tuya_ipc_ss_get_status()) {
				tuya_ipc_ss_start_event();
			}
#if SUPPORT_CLOUD_STORAGE
			if (ClOUD_STORAGE_TYPE_EVENT == tuya_ipc_cloud_storage_get_store_mode() &&
			    tuya_ipc_cloud_storage_get_event_status_by_id(event_id) == EVENT_NONE) {
				get_motion_snapshot(snap_addr, &snap_size);
				event_id = tuya_ipc_cloud_storage_event_add(
					snap_addr, snap_size, EVENT_TYPE_MOTION_DETECT, MAX_CLOUD_EVENT_DURATION);
			}
#endif

			/*renew event*/
			gettimeofdayMonotonic(&t1, NULL);
			if (t1.tv_sec >= event_renew) {
				event_start = t1.tv_sec;
				event_renew = t1.tv_sec + ev_rec->max_event_time;
				IPC_APP_Notify_Siren(1);
#if SUPPORT_AI_DETECT
				tuya_ipc_ai_detect_storage_stop();
#endif
				tuya_ipc_ss_stop_event();
#if SUPPORT_CLOUD_STORAGE
				tuya_ipc_cloud_storage_event_stop();
#endif
				tuya_ipc_ss_start_event();
				get_motion_snapshot(snap_addr, &snap_size);
				if (snap_size > 0) {
#if SUPPORT_CLOUD_STORAGE
					tuya_ipc_cloud_storage_event_start(snap_addr, snap_size,
						                           EVENT_TYPE_MOTION_DETECT);
#endif
					if (md_enable) {
#if SUPPORT_AI_DETECT
						//use motion detect, ONLY ai_detect start failed
						if (0 != tuya_ipc_ai_detect_storage_start()) {
							tuya_ipc_notify_motion_detect(snap_addr, snap_size,
								                      NOTIFICATION_CONTENT_JPEG);
						}
#else
						tuya_ipc_notify_motion_detect(snap_addr, snap_size,
							                      NOTIFICATION_CONTENT_JPEG);
#endif
					}
				}
			}
		}
	} else {
		if (gmotion_alarm_is_triggerd) {
			gettimeofdayMonotonic(&t1, NULL);

			/*If event re-trigger, force close current event and start new event.*/
			event_renew = 0;

			if (!event_end) {
				event_end = ((t1.tv_sec - event_start) < ev_rec->min_event_time) ?
					            t1.tv_sec + ev_rec->post_record_time +
					                    (ev_rec->min_event_time - (t1.tv_sec - event_start)) :
					            t1.tv_sec + ev_rec->post_record_time;
			}

			//Stand still for more than n seconds, stop the event
			if (t1.tv_sec >= event_end) {
#if SUPPORT_AI_DETECT
				tuya_ipc_ai_detect_storage_stop();
#endif
				tuya_ipc_ss_stop_event();
#if SUPPORT_CLOUD_STORAGE
				tuya_ipc_cloud_storage_event_delete(event_id);
#endif
				event_end = 0;
				gmotion_alarm_is_triggerd = FALSE;
			}
		}

		if (siren_is_triggerd) {
			IPC_APP_Notify_Siren(0);
			siren_is_triggerd = FALSE;
		}
	}
	}

	return NULL;
}
#if SUPPORT_AI_DETECT
extern IPC_MEDIA_INFO_S s_media_info;
OPERATE_RET TUYA_APP_Enable_AI_Detect()
{
	tuya_ipc_ai_detect_storage_init(&s_media_info);

	return OPRT_OK;
}
#endif

BOOL_T TUYA_APP_Check_Event_Triggerd()
{
	return gmotion_alarm_is_triggerd;
}

VOID TUYA_APP_Update_Md_Parameter()
{
	TUYA_AG_CONF_S *conf = NULL;
	event_record_s *ev_rec = &gevent_record;

	AG_Get_Conf(&conf);

	ev_rec->min_event_time = conf->local_record.data.min_event_time;
	ev_rec->post_record_time = conf->local_record.data.post_record_time;
	ev_rec->max_event_time = conf->local_record.data.max_event_time;

	PR_NOTICE("event max %d min %d post %d\n", ev_rec->min_event_time, ev_rec->post_record_time,
	          ev_rec->max_event_time);
}
