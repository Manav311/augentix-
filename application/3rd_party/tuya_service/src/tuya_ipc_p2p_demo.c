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
  *Copyright(C),2015-2020, TUYA www.tuya.comm
  *FileName:    tuya_ipc_p2p_demo
**********************************************************************************/

/*
 * Caution:
 *   Include mpi_base_types.h in the very first one.
 *   In order to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_media.h"
#include "tuya_ipc_media_demo.h"
#include "tuya_ipc_system_control_demo.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_p2p.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

extern IPC_MEDIA_INFO_S s_media_info;

typedef struct {
	BOOL_T enabled;
	TRANSFER_VIDEO_CLARITY_TYPE_E live_clarity;
	UINT_T max_users;
	TUYA_CODEC_ID p2p_audio_codec;
} TUYA_APP_P2P_MGR;

STATIC TUYA_APP_P2P_MGR s_p2p_mgr = { 0 };

STATIC VOID __TUYA_APP_media_frame_TO_trans_video(IN CONST MEDIA_FRAME_S *p_in, INOUT TRANSFER_VIDEO_FRAME_S *p_out)
{
	UINT_T codec_type = 0;
	codec_type = (p_in->type & 0xff00) >> 8;
	p_out->video_codec = (codec_type == 0 ? TUYA_CODEC_VIDEO_H264 : TUYA_CODEC_VIDEO_H265);
	p_out->video_frame_type = (p_in->type && 0xff) == E_VIDEO_PB_FRAME ? TY_VIDEO_FRAME_PBFRAME :
	                                                                     TY_VIDEO_FRAME_IFRAME;
	p_out->p_video_buf = p_in->p_buf;
	p_out->buf_len = p_in->size;
	p_out->pts = p_in->pts;
	p_out->timestamp = p_in->timestamp;
}

STATIC VOID __TUYA_APP_media_frame_TO_trans_audio(IN CONST MEDIA_FRAME_S *p_in, INOUT TRANSFER_AUDIO_FRAME_S *p_out)
{
	p_out->audio_codec = s_media_info.audio_codec[E_CHANNEL_AUDIO];
	p_out->audio_sample = s_media_info.audio_sample[E_CHANNEL_AUDIO];
	p_out->audio_databits = s_media_info.audio_databits[E_CHANNEL_AUDIO];
	p_out->audio_channel = s_media_info.audio_channel[E_CHANNEL_AUDIO];
	p_out->p_audio_buf = p_in->p_buf;
	p_out->buf_len = p_in->size;
	p_out->pts = p_in->pts;
	p_out->timestamp = p_in->timestamp;
}

STATIC VOID __TUYA_APP_ss_pb_event_cb(IN UINT_T pb_idx, IN SS_PB_EVENT_E pb_event,
                                      IN PVOID_T args __attribute__((unused)))
{
	PR_DEBUG("ss pb rev event: %u %d", pb_idx, pb_event);
	if (pb_event == SS_PB_FINISH) {
		tuya_ipc_playback_send_finish(pb_idx);
	}
}

#define SS_PB_NICENESS (-19)
int ss_pb_niceness = 0;
int ss_pb_niceness_old = 0;

STATIC VOID __TUYA_APP_ss_pb_get_video_cb(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_S *p_frame)
{
	if(ss_pb_niceness != ss_pb_niceness_old) {
		ss_pb_niceness_old = ss_pb_niceness;
		setpriority(PRIO_PROCESS, 0, ss_pb_niceness);
		printf("Apply ss_pb_niceness = %d\n", ss_pb_niceness);
	}

	TRANSFER_VIDEO_FRAME_S video_frame = { 0 };
	__TUYA_APP_media_frame_TO_trans_video(p_frame, &video_frame);
	tuya_ipc_playback_send_video_frame(pb_idx, &video_frame);
}

STATIC VOID __TUYA_APP_ss_pb_get_audio_cb(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_S *p_frame)
{
	TRANSFER_AUDIO_FRAME_S audio_frame = { 0 };
	__TUYA_APP_media_frame_TO_trans_audio(p_frame, &audio_frame);
	tuya_ipc_playback_send_audio_frame(pb_idx, &audio_frame);
}

STATIC VOID __depereated_online_cb(IN TRANSFER_ONLINE_E status __attribute__((unused)))
{
}

/* Callback functions for transporting events */
STATIC INT_T __TUYA_APP_p2p_event_cb(IN CONST TRANSFER_EVENT_E event, IN CONST PVOID_T args)
{
	PR_NOTICE("p2p rev event cb=[%d] ", event);
	PR_DEBUG("p2p rev event cb=[%d] ", event);
	switch (event) {
	case TRANS_LIVE_VIDEO_START: {
		C2C_TRANS_CTRL_VIDEO_START *parm = (C2C_TRANS_CTRL_VIDEO_START *)args;
		PR_DEBUG("chn[%u] type[%d]video start", parm->channel, parm->type);
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_LIVE_VIDEO_START);
		break;
	}
	case TRANS_LIVE_VIDEO_STOP: {
		C2C_TRANS_CTRL_VIDEO_STOP *parm = (C2C_TRANS_CTRL_VIDEO_STOP *)args;
		PR_DEBUG("chn[%u] type[%d] video stop", parm->channel, parm->type);
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_LIVE_VIDEO_STOP);
		break;
	}
	case TRANS_LIVE_AUDIO_START: {
		C2C_TRANS_CTRL_AUDIO_START *parm = (C2C_TRANS_CTRL_AUDIO_START *)args;
		PR_DEBUG("chn[%u] audio start", parm->channel);
		break;
	}
	case TRANS_LIVE_AUDIO_STOP: {
		C2C_TRANS_CTRL_AUDIO_STOP *parm = (C2C_TRANS_CTRL_AUDIO_STOP *)args;
		PR_DEBUG("chn[%u] audio stop", parm->channel);
		break;
	}
	case TRANS_SPEAKER_START: {
		TRANSFER_SOURCE_TYPE_E type = *(TRANSFER_SOURCE_TYPE_E *)args;
		PR_DEBUG("enbale audio speaker by %d", type);
		TUYA_APP_Enable_Speaker_CB(TRUE);
		TRANSFER_EVENT_RETURN_E ret = TRANS_EVENT_SUCCESS;
		//if start failed, return TRANS_EVENT_SPEAKER_ISUSED/TRANS_EVENT_SPEAKER_REPSTART
		return ret;
	}
	case TRANS_SPEAKER_STOP: {
		TRANSFER_SOURCE_TYPE_E type = *(TRANSFER_SOURCE_TYPE_E *)args;
		PR_DEBUG("disable audio speaker by %d", type);
		TUYA_APP_Enable_Speaker_CB(FALSE);
		TRANSFER_EVENT_RETURN_E ret = TRANS_EVENT_SUCCESS;
		//if start failed, return TRANS_EVENT_SPEAKER_STOPFAILED
		return ret;
	}
	case TRANS_LIVE_LOAD_ADJUST: {
		C2C_TRANS_LIVE_LOAD_PARAM_S *quality = (C2C_TRANS_LIVE_LOAD_PARAM_S *)args;
		PR_DEBUG("live quality %d -> %d", quality->curr_load_level, quality->new_load_level);
		break;
	}
	case TRANS_PLAYBACK_LOAD_ADJUST: {
		C2C_TRANS_PB_LOAD_PARAM_S *quality = (C2C_TRANS_PB_LOAD_PARAM_S *)args;
		PR_DEBUG("pb idx:%d quality %d -> %d", quality->client_index, quality->curr_load_level,
		         quality->new_load_level);
		break;
	}
	case TRANS_ABILITY_QUERY: {
		C2C_TRANS_QUERY_FIXED_ABI_REQ *pAbiReq;
		pAbiReq = (C2C_TRANS_QUERY_FIXED_ABI_REQ *)args;
		pAbiReq->ability_mask = TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_VIDEO |
		                        TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_SPEAKER |
		                        TY_CMD_QUERY_IPC_FIXED_ABILITY_TYPE_MIC;
		break;
	}
	case TRANS_PLAYBACK_QUERY_MONTH_SIMPLIFY: {
		C2C_TRANS_QUERY_PB_MONTH_REQ *p = (C2C_TRANS_QUERY_PB_MONTH_REQ *)args;
		PR_DEBUG("pb query by month: %d-%d", p->year, p->month);

		OPERATE_RET ret = tuya_ipc_pb_query_by_month(p->year, p->month, &(p->day));
		if (OPRT_OK != ret) {
			PR_ERR("pb query by month: %d-%d ret:%d", p->year, p->month, ret);
		}

		break;
	}
	case TRANS_PLAYBACK_QUERY_DAY_TS: {
		C2C_TRANS_QUERY_PB_DAY_RESP *pquery = (C2C_TRANS_QUERY_PB_DAY_RESP *)args;
		PR_DEBUG("pb_ts query by day: idx[%d] %d-%d-%d", pquery->channel, pquery->year, pquery->month,
		         pquery->day);
		SS_QUERY_DAY_TS_ARR_S *p_day_ts = NULL;
		OPERATE_RET ret =
		        tuya_ipc_pb_query_by_day(pquery->channel, pquery->year, pquery->month, pquery->day, &p_day_ts);
		if (OPRT_OK != ret) {
			PR_ERR("pb_ts query by day: idx[%d] %d-%d-%d Fail", pquery->channel, pquery->year,
			       pquery->month, pquery->day);
			break;
		}
		if (p_day_ts) {
			PR_INFO("%s %d count = %d\n", __FUNCTION__, __LINE__, p_day_ts->file_count);
			PLAY_BACK_ALARM_INFO_ARR *pResult = (PLAY_BACK_ALARM_INFO_ARR *)malloc(
			        sizeof(PLAY_BACK_ALARM_INFO_ARR) +
			        p_day_ts->file_count * sizeof(PLAY_BACK_ALARM_FRAGMENT));
			if (NULL == pResult) {
				PR_INFO("%s %d malloc failed \n", __FUNCTION__, __LINE__);
				free(p_day_ts);
				pquery->alarm_arr = NULL;
				return TRANS_EVENT_SPEAKER_INVALID;
			}

			INT_T i;
			pResult->file_count = p_day_ts->file_count;
			for (i = 0; (UINT_T)i < p_day_ts->file_count; i++) {
				pResult->file_arr[i].type = p_day_ts->file_arr[i].type;
				pResult->file_arr[i].time_sect.start_timestamp = p_day_ts->file_arr[i].start_timestamp;
				pResult->file_arr[i].time_sect.end_timestamp = p_day_ts->file_arr[i].end_timestamp;
			}
			pquery->alarm_arr = pResult;
			free(p_day_ts);

		} else {
			pquery->alarm_arr = NULL;
		}
		break;
	}
	case TRANS_PLAYBACK_START_TS: {
		/* Client will bring the start time when playback.
            For the sake of simplicity, only log printing is done. */
		C2C_TRANS_CTRL_PB_START *pParam = (C2C_TRANS_CTRL_PB_START *)args;
		PR_DEBUG("PB StartTS idx:%d %u [%u %u]", pParam->channel, pParam->playTime,
		         pParam->time_sect.start_timestamp, pParam->time_sect.end_timestamp);

		/* Engineering mode to adjust SD card playback thread niceness */
		FILE *fp = fopen("/tmp/ss_pb_niceness", "r");
		char nice[16];
		if(fp) {
			fgets(nice, 16, fp);
			ss_pb_niceness = atoi(nice);
			if(ss_pb_niceness > 20) {
				ss_pb_niceness = 20;
			} else if ( ss_pb_niceness < -19 ) {
				ss_pb_niceness = -19;
			}
			fclose(fp);
			printf("Read ss_pb_niceness = %d\n", ss_pb_niceness);
		} else {
			ss_pb_niceness = SS_PB_NICENESS;
			printf("Read ss_pb_niceness = %d (default)\n", ss_pb_niceness);
		}
		ss_pb_niceness_old = ss_pb_niceness - 1;

		SS_FILE_TIME_TS_S pb_file_info;
		int ret;
		memset(&pb_file_info, 0x00, sizeof(SS_FILE_TIME_TS_S));
		//memcpy(&pb_file_info, &pParam->time_sect, sizeof(SS_FILE_TIME_TS_S));
		pb_file_info.start_timestamp = pParam->time_sect.start_timestamp;
		pb_file_info.end_timestamp = pParam->time_sect.end_timestamp;
		ret = tuya_ipc_ss_pb_start(pParam->channel, __TUYA_APP_ss_pb_event_cb, __TUYA_APP_ss_pb_get_video_cb,
		                           __TUYA_APP_ss_pb_get_audio_cb);
		if (0 != ret) {
			PR_INFO("%s %d pb_start failed\n", __FUNCTION__, __LINE__);
			tuya_ipc_playback_send_finish(pParam->channel);
		} else {
			if (0 != tuya_ipc_ss_pb_seek(pParam->channel, &pb_file_info, pParam->playTime)) {
				PR_INFO("%s %d pb_seek failed\n", __FUNCTION__, __LINE__);
				tuya_ipc_playback_send_finish(pParam->channel);
			}
		}

		break;
	}
	case TRANS_PLAYBACK_PAUSE: {
		C2C_TRANS_CTRL_PB_PAUSE *pParam = (C2C_TRANS_CTRL_PB_PAUSE *)args;
		PR_DEBUG("PB Pause idx:%d", pParam->channel);

		tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_PAUSE);
		break;
	}
	case TRANS_PLAYBACK_RESUME: {
		C2C_TRANS_CTRL_PB_RESUME *pParam = (C2C_TRANS_CTRL_PB_RESUME *)args;
		PR_DEBUG("PB Resume idx:%d", pParam->channel);

		tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_RESUME);
		break;
	}
	case TRANS_PLAYBACK_MUTE: {
		C2C_TRANS_CTRL_PB_MUTE *pParam = (C2C_TRANS_CTRL_PB_MUTE *)args;
		PR_DEBUG("PB idx:%d mute", pParam->channel);

		tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_MUTE);
		break;
	}
	case TRANS_PLAYBACK_UNMUTE: {
		C2C_TRANS_CTRL_PB_UNMUTE *pParam = (C2C_TRANS_CTRL_PB_UNMUTE *)args;
		PR_DEBUG("PB idx:%d unmute", pParam->channel);

		tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_UN_MUTE);
		break;
	}
	case TRANS_PLAYBACK_STOP: {
		C2C_TRANS_CTRL_PB_STOP *pParam = (C2C_TRANS_CTRL_PB_STOP *)args;
		PR_DEBUG("PB Stop idx:%d", pParam->channel);

		tuya_ipc_ss_pb_stop(pParam->channel);
		break;
	}
	case TRANS_LIVE_VIDEO_CLARITY_SET: {
		C2C_TRANS_LIVE_CLARITY_PARAM_S *pParam = (C2C_TRANS_LIVE_CLARITY_PARAM_S *)args;
		PR_DEBUG("set clarity:%d", pParam->clarity);
		if ((pParam->clarity == TY_VIDEO_CLARITY_STANDARD) || (pParam->clarity == TY_VIDEO_CLARITY_HIGH)) {
			PR_DEBUG("set clarity:%d OK", pParam->clarity);
			s_p2p_mgr.live_clarity = pParam->clarity;
		}
		break;
	}
	case TRANS_LIVE_VIDEO_CLARITY_QUERY: {
		C2C_TRANS_LIVE_CLARITY_PARAM_S *pParam = (C2C_TRANS_LIVE_CLARITY_PARAM_S *)args;
		pParam->clarity = s_p2p_mgr.live_clarity;
		PR_DEBUG("query larity:%d", pParam->clarity);
		break;
	}
	case TRANS_DOWNLOAD_START: {
		C2C_TRANS_CTRL_DL_START *pParam = (C2C_TRANS_CTRL_DL_START *)args;
#if 0  
        // user to do 
        //fill strFrameHead and dlBuff
        int index = pParam->channel;
        STORAGE_FRAME_HEAD_S strFrameHead;
        CHAR_T * dlBuff =NULL;
        // send frame 1, fill strFrameHead and dlBuff
        tuya_ipc_4_app_download_data(index, &strFrameHead, dlBuff);
    	// send frame 2, fill strFrameHead and dlBuff
        tuya_ipc_4_app_download_data(index, &strFrameHead, dlBuff);
        // send last frame, fill strFrameHead and dlBuff
        OPERATE_RET tuya_ipc_4_app_download_status(index, IN CONST UINT_T percent);// only 0,100 in use
#else
		SS_FILE_TIME_TS_S strParm;
		strParm.start_timestamp = pParam->time_sect.start_timestamp;
		strParm.end_timestamp = pParam->time_sect.end_timestamp;
		if (OPRT_OK == tuya_ipc_ss_donwload_pre(pParam->channel, &strParm)) {
			tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_START);
		}
#endif
		break;
	}
	case TRANS_DOWNLOAD_STOP: {
		C2C_TRANS_CTRL_DL_STOP *pParam = (C2C_TRANS_CTRL_DL_STOP *)args;
		tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_STOP);
		break;
	}
	case TRANS_DOWNLOAD_PAUSE: {
		C2C_TRANS_CTRL_DL_PAUSE *pParam = (C2C_TRANS_CTRL_DL_PAUSE *)args;
		tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_PAUSE);
		break;
	}
	case TRANS_DOWNLOAD_RESUME: {
		C2C_TRANS_CTRL_DL_RESUME *pParam = (C2C_TRANS_CTRL_DL_RESUME *)args;
		tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_RESUME);
		break;
	}
	case TRANS_DOWNLOAD_CANCLE: {
		C2C_TRANS_CTRL_DL_CANCLE *pParam = (C2C_TRANS_CTRL_DL_CANCLE *)args;
		tuya_ipc_ss_download_set_status(pParam->channel, SS_DL_CANCLE);
		break;
	}
	case TRANS_STREAMING_VIDEO_START: {
		TRANSFER_SOURCE_TYPE_E *pSrcType = (TRANSFER_SOURCE_TYPE_E *)args;
		PR_DEBUG("streaming start type %d", *pSrcType);
		break;
	}
	case TRANS_STREAMING_VIDEO_STOP: {
		TRANSFER_SOURCE_TYPE_E *pSrcType = (TRANSFER_SOURCE_TYPE_E *)args;
		PR_DEBUG("streaming stop type %d", *pSrcType);
		break;
	}
	default:
		break;
	}
	return 0;
}

STATIC VOID __TUYA_APP_rev_audio_cb(IN CONST TRANSFER_AUDIO_FRAME_S *p_audio_frame,
                                    IN CONST UINT_T frame_no __attribute__((unused)))
{
	MEDIA_FRAME_S audio_frame = { 0 };
	audio_frame.p_buf = p_audio_frame->p_audio_buf;
	audio_frame.size = p_audio_frame->buf_len;

	PR_DEBUG("Rev Audio. size:%u audio_codec:%d audio_sample:%d audio_databits:%d audio_channel:%d",
	         p_audio_frame->buf_len, p_audio_frame->audio_codec, p_audio_frame->audio_sample,
	         p_audio_frame->audio_databits, p_audio_frame->audio_channel);

	TUYA_APP_Rev_Audio_CB(&audio_frame, TUYA_AUDIO_SAMPLE_8K, TUYA_AUDIO_DATABITS_16, TUYA_AUDIO_CHANNEL_MONO);
}

OPERATE_RET TUYA_APP_Enable_P2PTransfer(IN UINT_T max_users)
{
	if (s_p2p_mgr.enabled == TRUE) {
		PR_DEBUG("P2P Is Already Inited");
		return OPRT_OK;
	}

	PR_DEBUG("Init P2P With Max Users:%u", max_users);

	s_p2p_mgr.enabled = TRUE;
	s_p2p_mgr.max_users = max_users;
	s_p2p_mgr.p2p_audio_codec = s_media_info.audio_codec[E_CHANNEL_AUDIO];

	TUYA_IPC_TRANSFER_VAR_S p2p_var = { 0 };
	p2p_var.online_cb = __depereated_online_cb;
	p2p_var.on_rev_audio_cb = __TUYA_APP_rev_audio_cb;
	/*speak data format  app->ipc*/
	p2p_var.rev_audio_codec = TUYA_CODEC_AUDIO_PCM;
	p2p_var.audio_sample = s_media_info.audio_sample[E_CHANNEL_AUDIO];
	p2p_var.audio_databits = s_media_info.audio_databits[E_CHANNEL_AUDIO];
	p2p_var.audio_channel = s_media_info.audio_channel[E_CHANNEL_AUDIO];
	/*end*/
	p2p_var.on_event_cb = __TUYA_APP_p2p_event_cb;
	p2p_var.live_quality = TRANS_LIVE_QUALITY_MAX;
	p2p_var.max_client_num = max_users;
	p2p_var.defLiveMode = TRANS_DEFAULT_HIGH; //TRANS_DEFAULT_HIGH;// TRANS_DEFAULT_STANDARD;
	memcpy(&p2p_var.AVInfo, &s_media_info, sizeof(IPC_MEDIA_INFO_S));
	tuya_ipc_tranfser_init(&p2p_var);

	return OPRT_OK;
}
