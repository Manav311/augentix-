#define _GNU_SOURCE /* For pthread_setname_np */

#include "avftr.h"

#include "mpi_base_types.h"

#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "ado_ctrl.h"

#include "avftr_log.h"
#include "mpi_dev.h"


#include "ac.h"

#include "vftr.h"
#include "avftr_dk.h"
#include "video_od.h"
#include "avftr_md.h"
#include "avftr_ld.h"
#include "avftr_td.h"
#include "avftr_fld.h"
#include "avftr_ef.h"
#include "video_rms.h"
#include "avftr_aroi.h"
#include "avftr_pd.h"
#include "video_vdbg.h"
#include "video_ptz.h"
#include "avftr_eaif.h"
#include "avftr_pfm.h"
#include "avftr_bm.h"
#include "avftr_common.h"

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

static bool g_layout_updated = 1;

#define AVFTR_IVA_THREAD_NAME "avftr_iva%d"
#define AVFTR_IAA_THREAD_NAME "avftr_iaa%d"

#define abs(x) (((x) > 0) ? (x) : (-x))

typedef struct {
	pthread_mutex_t lock;
	pthread_t tid_vftr;
	MPI_WIN idx;
	char name[16];
} AVFTR_VIDEO_CTL_CTX_S;

static AVFTR_VIDEO_CTL_CTX_S g_vftr_ctx[AVFTR_VIDEO_MAX_SUPPORT_NUM] = { [0 ... AVFTR_VIDEO_MAX_SUPPORT_NUM -
	                                                                  1] = { .lock = PTHREAD_MUTEX_INITIALIZER } };

typedef struct {
	pthread_mutex_t lock;
	pthread_t tid_aftr;
	int running;
	MPI_DEV idx;
	char name[16];
} AVFTR_AUDIO_CTL_CTX_S;

static AVFTR_AUDIO_CTL_CTX_S g_aftr_ctx[AVFTR_AUDIO_MAX_SUPPORT_NUM] = {
	[0 ... AVFTR_AUDIO_MAX_SUPPORT_NUM - 1] = { .running = 0, .lock = PTHREAD_MUTEX_INITIALIZER }
};

static int AVFTR_resumeVideo(MPI_WIN idx)
{
	AVFTR_TD_resume(idx);
	AVFTR_LD_resume(idx); //Do nothing now

	return 0;
}

static int AVFTR_resumeAudio(MPI_DEV idx)
{
	AVFTR_SD_resume(idx);

	return 0;
}

static int getWinLayout(MPI_WIN idx, MPI_RECT_S *rect)
{
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn;
	uint8_t i;
	int ret;

	chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Cannot get channel layout for chn%d, err: %d", chn.chn, ret);
		return -1;
	}
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			break;
		}
	}
	if (i == layout_attr.window_num) {
		avftr_log_err("Window %d does not exist in channel %d", idx.win, idx.chn);
		return -1;
	}

	rect->x = layout_attr.window[i].x;
	rect->y = layout_attr.window[i].y;
	rect->width = layout_attr.window[i].width;
	rect->height = layout_attr.window[i].height;
	return 0;
}

/* For iva thread create & exit searching */
static int findVftrCtx(MPI_WIN idx, const AVFTR_VIDEO_CTL_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_VIDEO_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

/* For iaa thread create & exit searching */
static int findAftrCtx(MPI_DEV idx, const AVFTR_AUDIO_CTL_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_AUDIO_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int findVftrBufIdx(AVFTR_VIDEO_BUF_INFO_S *buf_info, UINT32 timestamp)
{
	int time_diff_prev = (int)timestamp - (int)buf_info->buf_cur_time;
	time_diff_prev = abs(time_diff_prev);
	int time_diff = 0;
	int i;
	int buf_idx;
	int ret_idx = buf_info->buf_cur_idx;

	if ((time_diff_prev == 0) && (buf_info->buf_ready[ret_idx])) {
		return ret_idx;
	}
	for (i = 1; i < AVFTR_VIDEO_RING_BUF_SIZE; i++) {
		buf_idx = (buf_info->buf_cur_idx - i);
		buf_idx = (buf_idx < 0) ? (AVFTR_VIDEO_RING_BUF_SIZE + buf_idx) : buf_idx;
		time_diff = (int)timestamp - (int)buf_info->buf_time[buf_idx];
		time_diff = abs(time_diff);
		if (time_diff >= time_diff_prev) {
			if (buf_info->buf_ready[ret_idx]) {
				return ret_idx;
			} else {
				time_diff_prev = time_diff;
			}
		} else if (time_diff < time_diff_prev) {
			time_diff_prev = time_diff;
		}
		ret_idx = buf_idx;
	}

	return ret_idx;
}

static int __attribute__((unused)) findAftrBufIdx(AVFTR_AUDIO_BUF_INFO_S *buf_info, UINT64 timestamp)
{
	int time_diff_prev = buf_info->buf_cur_time - timestamp;
	time_diff_prev = abs(time_diff_prev);
	int time_diff = 0;
	int i;
	int buf_idx;
	int ret_idx = buf_info->buf_cur_idx;

	for (i = 1; i < AVFTR_AUDIO_RING_BUF_SAFE_SIZE; i++) {
		if ((time_diff_prev == 0) && (buf_info->buf_ready[ret_idx])) {
			return ret_idx;
		}
		buf_idx = (buf_info->buf_cur_idx - i);
		buf_idx = (buf_idx < 0) ? (AVFTR_AUDIO_RING_BUF_SIZE + buf_idx) : buf_idx;
		time_diff = buf_info->buf_time[buf_idx] - timestamp;
		time_diff = abs(time_diff);

		if (time_diff > time_diff_prev) {
			if (buf_info->buf_ready[ret_idx]) {
				return ret_idx;
			} else {
				time_diff_prev = time_diff;
				ret_idx = buf_idx;
			}
		} else if (time_diff < time_diff_prev) {
			time_diff_prev = time_diff;
			ret_idx = buf_idx;
		}
	}

	return ret_idx;
}

/* For result translation */
static int findVftrBufCtx(MPI_WIN idx, const AVFTR_VIDEO_BUF_INFO_S *ctx)
{
	for (int i = 0; i < AVFTR_VIDEO_MAX_SUPPORT_NUM; i++) {
		if (ctx[i].idx.value == idx.value) {
			return i;
		}
	}

	return -1;
}

/* For result translation */
static int __attribute__((unused)) findAftrBufCtx(MPI_DEV idx, const AVFTR_AUDIO_BUF_INFO_S *ctx)
{
	int i = 0;
	int find_idx = -1;

	for (i = 0; i < AVFTR_AUDIO_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
		}
	}
	return find_idx;
}

static int updateVftrBufferInfo(AVFTR_VIDEO_BUF_INFO_S *buf_info, UINT32 timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;

	return buf_info->buf_cur_idx;
}

static int updateAftrBufferInfo(AVFTR_AUDIO_BUF_INFO_S *buf_info, UINT64 timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_AUDIO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;

	return buf_info->buf_cur_idx;
}

static AUDIO_CODEC_TYPE_E getACCodecEnum(codec_mode_t pcm_codec)
{
	AUDIO_CODEC_TYPE_E codec;
	switch (pcm_codec) {
	case RAW:
		codec = AUDIO_CODEC_TYPE_PCM16LE;
		break;
	case A_LAW:
		codec = AUDIO_CODEC_TYPE_PCMA;
		break;
	case MU_LAW:
		codec = AUDIO_CODEC_TYPE_PCMU;
		break;
	case G726_2_BE:
		codec = AUDIO_CODEC_TYPE_G726_16_BE;
		break;
	case G726_2_LE:
		codec = AUDIO_CODEC_TYPE_G726_16_LE;
		break;
	case G726_4_BE:
		codec = AUDIO_CODEC_TYPE_G726_32_BE;
		break;
	case G726_4_LE:
		codec = AUDIO_CODEC_TYPE_G726_32_LE;
		break;
	default:
		assert(0);
	}
	return codec;
}

static int AVFTR_writeVideoParam(MPI_WIN idx)
{
	int ret = 0;
	ret |= AVFTR_DK_writeParam(idx);
	ret |= AVFTR_MD_writeParam(idx);
	ret |= AVFTR_TD_writeParam(idx);
	ret |= AVFTR_FLD_writeParam(idx);
	ret |= AVFTR_EF_writeParam(idx);
	ret |= AVFTR_AROI_writeParam(idx);
	ret |= AVFTR_LD_writeParam(idx);
	ret |= AVFTR_BM_writeParam(idx);
	ret |= AVFTR_PFM_writeParam(idx);
	return ret;
}

static int AVFTR_writeAudioParam(MPI_DEV idx)
{
	int ret = 0;
	ret |= AVFTR_SD_writeParam(idx);
	return ret;
}

static int AVFTR_updateMpiInfo(MPI_WIN idx)
{
	int ret = 0;
	ret |= AVFTR_TD_updateMpiInfo(idx);
	ret |= AVFTR_LD_updateMpiInfo(idx);
	ret |= AVFTR_BM_updateMpiInfo(idx);
	ret |= AVFTR_PFM_updateMpiInfo(idx);
	return ret;
}

/**
 * @brief Query whether any module is enabled.
 * @param[in] idx    video window index.
 * @param[in] res
 * @return enable status of all video features.
 * @retval true     Some module is enabled.
 * @retval false    All modules are disabled.
 * @see AVFTR_tranVideoRes()
 * @see AVFTR_tranVideoResV2()
 */
// This API is designed for AVFTR clients.
int AVFTR_getVideoStat(MPI_WIN idx, AVFTR_VIDEO_CTX_S *res)
{
	if (res == NULL) {
		avftr_log_err("Args should not be NULL.");
		return 0;
	}

	return VIDEO_FTR_getOdStat(idx, res->od_ctx) || VIDEO_FTR_getRmsStat(idx, res->rms_ctx) ||
	       AVFTR_TD_getStat(idx, res->td_ctx) || AVFTR_AROI_getStat(idx, res->aroi_ctx) ||
	       AVFTR_LD_getStat(idx, res->ld_ctx) || AVFTR_MD_getStat(idx, res->md_ctx) ||
	       AVFTR_EF_getStat(idx, res->ef_ctx) || AVFTR_EAIF_getStat(idx, res->eaif_ctx) ||
	       AVFTR_PFM_getStat(idx, res->pfm_ctx) || AVFTR_BM_getStat(idx, res->bm_ctx) ||
	       AVFTR_DK_getStat(idx, res->dk_ctx) || AVFTR_FLD_getStat(idx, res->fld_ctx)
#ifdef AVFTR_VDBG
	       || VIDEO_FTR_getVdbgStat(&res->vdbg_ctx)
#endif /* AVFTR_VDBG */
	        ;
}

/**
 * @brief Get enable status of audio features.
 * @param[in] dev_idx         audio device index.
 * @see none
 * @retval enable status of all audio features.
 */
int AVFTR_getAudioStat(MPI_DEV idx, AVFTR_AUDIO_CTX_S *res)
{
	return AVFTR_SD_getStat(idx, res->sd_ctx);
}

/**
 * @brief Get metadata of video features.
 * @param[in]  idx             video window index.
 * @param[in]  frame_cnt       frame index of target frame.
 * @param[in]  timestamp
 * @param[out] data            metadata string buffer.
 * @see none
 * @retval length of metadata.
 */
int AVFTR_tranVideoRes(MPI_WIN src, MPI_WIN dst, AVFTR_VIDEO_CTX_S *iva_ctx, UINT32 timestamp, char *data)
{
	if (iva_ctx == NULL || data == NULL) {
		avftr_log_err("Args should not be NULL.");
		return 0;
	}

	uint32_t chn_idx = MPI_GET_VIDEO_CHN(src);
	int dataoffset = 0;
	int buf_idx = 0;
	int enable_idx = 0;
	MPI_RECT_S src_rect, dst_rect;
	MPI_RECT_S src_roi, dst_roi;

	if (getWinLayout(src, &src_rect)) {
		return 0;
	}

	if (getWinLayout(dst, &dst_rect)) {
		return 0;
	}

	if (MPI_DEV_getWindowRoi(src, &src_roi)) {
		return 0;
	}

	if (MPI_DEV_getWindowRoi(dst, &dst_roi)) {
		return 0;
	}

	enable_idx = findVftrBufCtx(src, iva_ctx->buf_info);
	buf_idx = findVftrBufIdx(&iva_ctx->buf_info[enable_idx], timestamp);

	//TODO: Failure handling
	dataoffset += sprintf(&data[dataoffset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                      "<AGTX TIME=\"%d.%d\" CHN=\"%d\">"
#else /* IVA_FORMAT_JSON */
	                      "{\"agtx\":{\"time\":%u,\"chn\":%d,\"iva\":{"
#endif /* IVA_FORMAT */
	                      ,
	                      timestamp, chn_idx);

	dataoffset += AVFTR_MD_transRes(iva_ctx->md_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_EF_transRes(iva_ctx->ef_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_AROI_transRes(iva_ctx->aroi_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                  (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_TD_transRes(iva_ctx->td_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_PFM_transRes(iva_ctx->pfm_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                 (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_BM_transRes(iva_ctx->bm_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_FLD_transRes(iva_ctx->fld_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                 (char *)&data[dataoffset], buf_idx);
	dataoffset += VIDEO_FTR_transRmsRes(iva_ctx->rms_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                    (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_DK_transRes(iva_ctx->dk_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += VIDEO_FTR_transOdRes(iva_ctx->od_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                   (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_EAIF_transRes(iva_ctx->eaif_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                  (char *)&data[dataoffset], buf_idx);

#ifdef AVFTR_VDBG
	dataoffset += VIDEO_FTR_transVdbgRes(iva_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                     (char *)&data[dataoffset], buf_idx);
#endif /* !AVFTR_VDBG */

	dataoffset += AVFTR_LD_transRes(iva_ctx->ld_ctx, src, dst, &src_rect, &dst_rect, &src_roi, &dst_roi,
	                                (char *)&data[dataoffset], buf_idx);

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	dataoffset += sprintf(&data[dataoffset], "</AGTX>");
#else /* IVA_FORMAT_JSON */
	int deduct = (data[dataoffset - 1] == ',') ? 1 : 0;
	dataoffset += (sprintf(&data[dataoffset - deduct], "}}}") - deduct);
#endif /* IVA_FORMAT */
	return dataoffset;
}

static int AVFTR_getVideoRes(MPI_WIN idx, UINT32 timestamp)
{
	time_t curr;
	double diff_t;
	AVFTR_VIDEO_NOTIFY_CTX *nf_ctx = &vftr_res_shm->nf_ctx;
	AVFTR_VIDEO_CTL_CTX_S *vftr_ctx = g_vftr_ctx;
	VIDEO_FTR_OBJ_LIST_S obj_list;
	memset(&obj_list, 0, sizeof(VIDEO_FTR_OBJ_LIST_S));

	AVFTR_VIDEO_BUF_INFO_S *buf_info;
	/* Find cur buffer idx */
	const int enable_idx = findVftrCtx(idx, vftr_ctx, NULL);
	buf_info = &vftr_res_shm->buf_info[enable_idx];
	const int buf_idx = updateVftrBufferInfo(buf_info, timestamp);

	// Other modules are based on object detection.
	// Therefore, VIDEO_FTR_getObjList() always first.
	VIDEO_FTR_getObjList(idx, timestamp, &obj_list);
	AVFTR_EAIF_getRes(idx, &obj_list, buf_idx);
	AVFTR_MD_getRes(idx, &obj_list, buf_idx);
	AVFTR_EF_getRes(idx, &obj_list, buf_idx);
	AVFTR_AROI_getRes(idx, &obj_list, buf_idx);
	VIDEO_FTR_getOdRes(idx, &obj_list, buf_idx);
	AVFTR_DK_getRes(idx, &obj_list, buf_idx);
	AVFTR_FLD_getRes(idx, &obj_list, buf_idx);
	VIDEO_FTR_getRmsRes(idx, buf_idx);

	if (nf_ctx->notify == AVFTR_VIDEO_NOTIFY_OFF) {
		AVFTR_LD_getRes(idx, buf_idx);
		AVFTR_TD_getRes(idx, buf_idx);
		AVFTR_PFM_getRes(idx, &obj_list, buf_idx);
		AVFTR_BM_getRes(idx, &obj_list, buf_idx);
	} else {
		// Delay some frames then resume inference.
		time(&curr);

		diff_t = difftime(curr, nf_ctx->start_time);
		if (diff_t >= nf_ctx->wait_time) {
			AVFTR_resumeVideo(idx);
			nf_ctx->notify = AVFTR_VIDEO_NOTIFY_OFF;
		}
	}

#ifdef AVFTR_VDBG
	VIDEO_FTR_getVdbgRes(idx, buf_idx);
#endif /* AVFTR_VDBG */
	VIDEO_FTR_setPtzResult(timestamp);
	buf_info->buf_ready[buf_idx] = 1;

	return 0;
}

static int AVFTR_getAudioRes(MPI_DEV idx, UINT64 timestamp, const char *bit_buffer, int size_of_bit)
{
	time_t curr;
	double diff_t;
	AVFTR_AUDIO_NOTIFY_CTX *nf_ctx = &aftr_res_shm->nf_ctx;
	AVFTR_AUDIO_CTL_CTX_S *aftr_ctx = g_aftr_ctx;
	char *raw_buffer = NULL;
	AVFTR_AUDIO_BUF_INFO_S *buf_info;
	/* Find cur buffer idx */
	const int enable_idx = findAftrCtx(idx, aftr_ctx, NULL);
	buf_info = &aftr_res_shm->buf_info[enable_idx];
	const int buf_idx = updateAftrBufferInfo(buf_info, timestamp);
	int size_of_raw = 0;

	// TODO: Failure handling
	AC_decode(bit_buffer, size_of_bit, &raw_buffer, &size_of_raw);
	if (nf_ctx->notify == AVFTR_AUDIO_NOTIFY_OFF) {
		AVFTR_SD_getRes(idx, raw_buffer, size_of_raw, buf_idx);
	} else {
		time(&curr);

		diff_t = difftime(curr, nf_ctx->start_time);
		if (diff_t >= nf_ctx->wait_time) {
			AVFTR_resumeAudio(idx);
			nf_ctx->notify = AVFTR_AUDIO_NOTIFY_OFF;
		}
	}

	buf_info->buf_ready[buf_idx] = 1;
	if (raw_buffer != bit_buffer) {
		free(raw_buffer);
		raw_buffer = NULL;
	}

	return 0;
}

/**
 * @brief Get metadata of video features with preload layout info
 * @param[in]  src             source video window index.
 * @param[in]  dst             dest video window index.
 * @param[in]  src_rect        source video window layout in rect
 * @param[in]  dst_rect        dest video window layout in rect
 * @param[in]  src_roi         source video window roi
 * @param[in]  dst_roi         dest video window roi
 * @param[in]  timestamp       jiffy timestamp from encoder
 * @param[out] data            metadata string buffer.
 * @see none
 * @retval length of metadata.
 */
// This API is designed for AVFTR clients.
int AVFTR_tranVideoResV2(MPI_WIN src, MPI_WIN dst, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                         const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, AVFTR_VIDEO_CTX_S *iva_ctx,
                         UINT32 timestamp, char *data)
{
	if (iva_ctx == NULL || data == NULL) {
		avftr_log_err("Args should not be NULL.");
		return 0;
	}

	const uint32_t chn_idx = MPI_GET_VIDEO_CHN(src);
	const int enable_idx = findVftrBufCtx(src, iva_ctx->buf_info);
	const int buf_idx = findVftrBufIdx(&iva_ctx->buf_info[enable_idx], timestamp);

	int dataoffset = 0;

	// TODO: Failure handling
	dataoffset += sprintf(&data[dataoffset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                      "<AGTX TIME=\"%d.%d\" CHN=\"%d\">"
#else /* IVA_FORMAT_JSON */
	                      "{\"agtx\":{\"time\":%u,\"chn\":%d,\"iva\":{ "
#endif /* IVA_FORMAT */
	                      ,
	                      timestamp, chn_idx);

	dataoffset += AVFTR_MD_transRes(iva_ctx->md_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_EF_transRes(iva_ctx->ef_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_AROI_transRes(iva_ctx->aroi_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                  (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_TD_transRes(iva_ctx->td_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_PFM_transRes(iva_ctx->pfm_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                 (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_BM_transRes(iva_ctx->bm_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_FLD_transRes(iva_ctx->fld_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                 (char *)&data[dataoffset], buf_idx);
	dataoffset += VIDEO_FTR_transRmsRes(iva_ctx->rms_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                    (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_DK_transRes(iva_ctx->dk_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                (char *)&data[dataoffset], buf_idx);
	dataoffset += VIDEO_FTR_transOdRes(iva_ctx->od_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                   (char *)&data[dataoffset], buf_idx);
	dataoffset += AVFTR_EAIF_transRes(iva_ctx->eaif_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                  (char *)&data[dataoffset], buf_idx);

#ifdef AVFTR_VDBG
	dataoffset += VIDEO_FTR_transVdbgRes(iva_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                     (char *)&data[dataoffset], buf_idx);
#endif /* !AVFTR_VDBG */

	dataoffset += AVFTR_LD_transRes(iva_ctx->ld_ctx, src, dst, src_rect, dst_rect, src_roi, dst_roi,
	                                (char *)&data[dataoffset], buf_idx);

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	dataoffset += sprintf(&data[dataoffset], "</AGTX>");
#else /* IVA_FORMAT_JSON */
	int deduct = (data[dataoffset - 1] == ',') ? 1 : 0;
	dataoffset += (sprintf(&data[dataoffset - deduct], "}}}") - deduct);
#endif /* IVA_FORMAT */
	return dataoffset;
}

// Need to re-design mechanism for notifying running modules.
// Some changes on dynamic configuration cannot be aware by application, such as FPS.
// Application layer need to regularly query info to aware current FPS.
__attribute__ ((deprecated)) int AVFTR_notifyVideo(MPI_WIN idx, int wait_time)
{
#define AVFTR_NOTIFY_DEFAULT_WAIT_TIME 3
	// Set suppress alarm/stop updating result of features(TD/LD)
	AVFTR_VIDEO_NOTIFY_CTX *ctx = &vftr_res_shm->nf_ctx;

	time(&ctx->start_time);

	if (wait_time == -1) {
		ctx->wait_time = AVFTR_NOTIFY_DEFAULT_WAIT_TIME;
	} else {
		ctx->wait_time = wait_time;
	}

	ctx->notify = AVFTR_VIDEO_NOTIFY_ON;
	g_layout_updated = 1;

	AVFTR_TD_resetShm(idx);
	AVFTR_LD_resetShm(idx);
	VIDEO_FTR_resetBmShm(idx);

	return 0;
}

// Need to re-design mechanism for notifying running modules.
// Currently this API have no users.
__attribute__ ((deprecated)) int AVFTR_notifyAudio(MPI_DEV idx, int wait_time)
{
#define AFTR_NOTIFY_DEFAULT_WAIT_TIME 3
	// Set suppress alarm/stop updating result of features(SD)
	AVFTR_AUDIO_NOTIFY_CTX *ctx = &aftr_res_shm->nf_ctx;

	time(&ctx->start_time);

	if (wait_time == -1) {
		ctx->wait_time = AFTR_NOTIFY_DEFAULT_WAIT_TIME;
	} else {
		ctx->wait_time = wait_time;
	}

	ctx->notify = AVFTR_AUDIO_NOTIFY_ON;

	AVFTR_SD_resetShm(idx);

	return 0;
}

static MPI_WIN searchFastestWin(MPI_CHN chn)
{
	MPI_CHN_LAYOUT_S layout;
	MPI_WIN_ATTR_S win_attr;
	MPI_WIN win = MPI_VIDEO_WIN(chn.dev, chn.chn, 0);
	MPI_WIN win_temp;
	int i;
	int ret = MPI_DEV_getChnLayout(chn, &layout);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Cannot get channel layout for chn:0x%x", chn.value);
		return MPI_VIDEO_WIN(0, 0, 0);
	}

	FLOAT fps = 0;
	for (i = 0; i < layout.window_num; i++) {
		win_temp = layout.win_id[i];
		ret = MPI_DEV_getWindowAttr(win_temp, &win_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Cannot get window attr for win:0x%x", win_temp.value);
			return MPI_VIDEO_WIN(0, 0, 0);
		}

		if (win_attr.fps > fps) {
			win = win_temp;
			fps = win_attr.fps;
		}
	}

	return win;
}

static void *runAllIva(void *args)
{
	AVFTR_VIDEO_CTL_CTX_S *ctx = (AVFTR_VIDEO_CTL_CTX_S *)args;
	UINT32 timestamp = 0;
	const INT32 timeout = 0; /* Wait forever */

	// Call MPI_DEV_waitWin() on the video win with highest fps
	const MPI_CHN chn = MPI_VIDEO_CHN(ctx->idx.dev, ctx->idx.chn);
	MPI_WIN win = MPI_VIDEO_WIN(ctx->idx.dev, ctx->idx.chn, 0);
	int err;

	VFTR_init(NULL);

	while (1) {
		if (g_layout_updated) {
			win = searchFastestWin(chn);
			g_layout_updated = 0;
		}

		if ((err = MPI_DEV_waitWin(win, &timestamp, timeout)) != 0) {
			avftr_log_err("MPI_DEV_waitWin(idx: (%d, %d, %d)) failed. err: %d", win.dev, win.chn, win.win,
			              err);
			return NULL;
		}

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		AVFTR_writeVideoParam(ctx->idx);

		AVFTR_updateMpiInfo(ctx->idx);

		AVFTR_getVideoRes(ctx->idx, timestamp);

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		pthread_testcancel();
	}

	return NULL;
}

static void *runAllIaa(void *args)
{
#define AVFTR_AUDIO_BUFFER_SIZE 1400

	AVFTR_AUDIO_CTL_CTX_S *ctx = (AVFTR_AUDIO_CTL_CTX_S *)args;
	UINT64 timestamp = 0;
	int size = 0;
	struct timeval _time;
	char audioBuf[AVFTR_AUDIO_BUFFER_SIZE];

	while (ctx->running) {
		size = ADOI_getBitStream(audioBuf);
		if (size < 0) {
			usleep(50 * 1000);
			continue;
		}

		gettimeofday(&_time, NULL);
		timestamp = ((UINT64)(_time.tv_sec)) * 1000000 + (_time.tv_usec);

		AVFTR_writeAudioParam(ctx->idx);

		AVFTR_getAudioRes(ctx->idx, timestamp, audioBuf, size);
	}

	return NULL;
}

/**
 * @brief Run the iva thread
 * @param[in]  idx video window index.
 * @see none
 * @retval start AVFTR_runIva thread
 */
int AVFTR_runIva(MPI_WIN idx)
{
	AVFTR_VIDEO_CTL_CTX_S *vftr_ctx = g_vftr_ctx;
	AVFTR_VIDEO_CTL_CTX_S *ctx;

	int enable_idx;
	int empty_idx;
	const int set_idx = findVftrCtx(idx, vftr_ctx, &empty_idx);
	int err;

	if (set_idx >= 0) {
		enable_idx = set_idx;
		sprintf(vftr_ctx[enable_idx].name, AVFTR_IVA_THREAD_NAME, enable_idx);
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		vftr_ctx[enable_idx].idx = idx;
		sprintf(vftr_ctx[enable_idx].name, AVFTR_IVA_THREAD_NAME, enable_idx);
		vftr_res_shm->buf_info[enable_idx].idx = idx;
		vftr_res_shm->buf_info[enable_idx].buf_cur_idx = 0;
		vftr_res_shm->buf_info[enable_idx].buf_cur_time = 0;
	} else {
		avftr_log_err("Create routine for runAllIva(%d) failed. No resources.", idx.win);
		return -ENOMEM;
	}

	ctx = &vftr_ctx[enable_idx];

	g_layout_updated = 1;
	if ((err = pthread_create(&ctx->tid_vftr, NULL, runAllIva, (void *)ctx)) != 0) {
		avftr_log_err("Create thread for routine runAllIva(idx: %u) failed. err: %s.", idx.value,
		              strerror(err));
		return -1;
	}

	avftr_log_info("Create thread for runAllIva() success.");

	// Be care of buffer overrun bug when ctx->name exceed length 16.
	pthread_setname_np(ctx->tid_vftr, ctx->name);

	return 0;
}

/**
 * @brief Run the iaa thread
 * @param[in]  idx audio device index.
 * @see none
 * @retval start AVFTR_runIaa thread
 */
int AVFTR_runIaa(MPI_DEV idx)
{
	AVFTR_AUDIO_CTL_CTX_S *aftr_ctx = g_aftr_ctx;
	int enable_idx;
	int empty_idx;
	int set_idx = findAftrCtx(idx, aftr_ctx, &empty_idx);
	codec_mode_t pcm_codec = RAW;

	unsigned int format = SND_PCM_FORMAT_S16_LE;
	unsigned int channels = 1;
	unsigned int rate = 8000;
	int volume = 100;
	snd_pcm_uframes_t frames = FRAME_SIZE;
	int err;

	ADOI_initSystem();
	ADOI_setCodecMode(pcm_codec);
	ADOI_setChannels(channels);
	ADOI_setRate(rate);
	ADOI_setPeriodSize(frames);
	ADOI_setFormat(format);
	ADOI_startSystem();
	ADOI_setVolume(volume);
	ADOI_getCodecMode(&pcm_codec);
	AC_setParam(MPI_VIDEO_DEV(0), &(AC_PARAM_S){ .codec = getACCodecEnum(pcm_codec) });

	if (set_idx >= 0) {
		enable_idx = set_idx;
		sprintf(aftr_ctx[enable_idx].name, AVFTR_IAA_THREAD_NAME, enable_idx);
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		aftr_ctx[enable_idx].idx = idx;
		sprintf(aftr_ctx[enable_idx].name, AVFTR_IAA_THREAD_NAME, enable_idx);
		aftr_res_shm->buf_info[enable_idx].idx = idx;
		aftr_res_shm->buf_info[enable_idx].buf_cur_idx = 0;
		aftr_res_shm->buf_info[enable_idx].buf_cur_time = 0;
	} else {
		avftr_log_err("Create routine for runAllIaa(%d) failed. No resources.", idx.dev);
		return -1;
	}

	AVFTR_AUDIO_CTL_CTX_S *ctx = &aftr_ctx[enable_idx];
	ctx->running = 1;

	if ((err = pthread_create(&ctx->tid_aftr, NULL, runAllIaa, (void *)ctx)) != 0) {
		avftr_log_err("Create thread for routine runAllIaa(idx: %u) failed. err: %d.", idx.value, err);
		return -1;
	}

	avftr_log_info("Create thread for runAllIaa() success.");

	// Be care of buffer overrun bug when ctx->name exceed length 16.
	pthread_setname_np(ctx->tid_aftr, ctx->name);

	return 0;
}

/**
 * @brief Exit iva thread
 * @param[in]  idx video window index.
 * @see none
 * @retval exit Iva thread w.r.t. window index
 */
int AVFTR_exitIva(MPI_WIN idx)
{
	AVFTR_VIDEO_CTL_CTX_S *vftr_ctx = g_vftr_ctx;
	void *res;
	int enable_idx = findVftrCtx(idx, vftr_ctx, NULL);
	int ret;

	avftr_log_info("Exit IVA thread for WIN:0x%x.", idx.value);
	if (enable_idx < 0) {
		avftr_log_err("Cannot find AVFTR_runIva thread for WIN:%d.", idx.value);
		return -1;
	}

	/* TODO: Clean up memory idx on AVFTR_VIDEO_CTL_CTX_S and vftr_res_shm*/
	AVFTR_VIDEO_CTL_CTX_S *ctx = &vftr_ctx[enable_idx];

	ret = pthread_cancel(ctx->tid_vftr);
	if (ret != 0) {
		avftr_log_err("Cancel AVFTR_runIva thread failed for WIN:%d.", idx.value);
		return -1;
	}

	ret = pthread_join(ctx->tid_vftr, (void *)&res);
	if (ret != 0) {
		avftr_log_err("Join thread to AVFTR_runIva process failed.");
		return -1;
	}

	VFTR_exit();

	if (res == PTHREAD_CANCELED) {
		avftr_log_err("AVFTR_runIva thread for win:0x%x was canceled!", idx.value);
	} else {
		avftr_log_err("AVFTR_runIva thread wasn't canceled (shouldn't happen!)");
		return -1;
	}

	VFTR_exit();

	return 0;
}

/**
 * @brief Exit iaa thread
 * @param[in]  idx audio device index.
 * @see none
 * @retval exit Iaa thread w.r.t. device index
 */
int AVFTR_exitIaa(MPI_DEV idx)
{
	AVFTR_AUDIO_CTL_CTX_S *aftr_ctx = g_aftr_ctx;
	void *res;
	int enable_idx = findAftrCtx(idx, aftr_ctx, NULL);
	int ret;

	avftr_log_info("Exit IAA thread for DEV:0x%x.", idx.value);
	if (enable_idx < 0) {
		avftr_log_err("Cannot find AVFTR_runIaa thread for DEV:%d.", idx.value);
		return -1;
	}

	/* TODO: Clean up memory idx on AVFTR_AUDIO_CTL_CTX_S and aftr_res_shm*/
	AVFTR_AUDIO_CTL_CTX_S *ctx = &aftr_ctx[enable_idx];
	ctx->running = 0;

	ret = pthread_join(ctx->tid_aftr, (void *)&res);
	if (ret != 0) {
		avftr_log_err("Join thread to AVFTR_runIaa process failed.");
		return -1;
	}

	ADOI_stopSystem();
	ADOI_closeSystem();

	avftr_log_err("AVFTR_runIaa thread for dev:0x%x was joined!", idx.value);
	return 0;
}
