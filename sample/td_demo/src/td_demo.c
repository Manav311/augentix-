#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "vftr.h"
#include "vftr_td.h"
#include "avftr_conn.h"
#include "log.h"

#include "td_demo.h"

#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

extern int g_td_running;
TD_CTX_S g_td_ctx;

#ifdef CONFIG_APP_TD_SUPPORT_SEI
extern AVFTR_CTX_S *avftr_res_shm;
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

static int findTdCtx(const MPI_WIN idx, const AVFTR_TD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_TD_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].reg) {
			find_idx = i;
		} else if (emp_idx == -1 && !ctx[i].reg) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int clearTdCtx(MPI_WIN idx, AVFTR_TD_CTX_S *ctx)
{
	int ctx_idx = findTdCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The TD ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(AVFTR_TD_CTX_S));
	return 0;
}

static int findVftrBufCtx(MPI_WIN idx, const AVFTR_VIDEO_BUF_INFO_S *ctx, int *empty)
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

static int clearVftrBufCtx(MPI_WIN idx, AVFTR_VIDEO_BUF_INFO_S *info)
{
	int info_idx = findVftrBufCtx(idx, info, NULL);
	if (info_idx < 0) {
		log_err("The VFTR buffer ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&info[info_idx], 0, sizeof(AVFTR_VIDEO_BUF_INFO_S));
	return 0;
}

static int updateBufferIdx(AVFTR_VIDEO_BUF_INFO_S *buf_info, UINT32 timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;
	return buf_info->buf_cur_idx;
}
#endif

static int updateMpiInfo(MPI_WIN win_idx, TD_CTX_S *ctx)
{
	int ret;

	ret = MPI_getStatistics(ctx->path, &ctx->mpi_input.dip_stat);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get DIP stat on path %u. err: %d", ctx->path.value, ret);
		return ret;
	}

	ret = MPI_DEV_getIspMvHist(win_idx, ctx->mv_hist_cfg_idx, &ctx->mpi_input.mv_hist);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get MV hist on win %u. err: %d", win_idx.value, ret);
		return ret;
	}

	ret = MPI_DEV_getIspVar(win_idx, ctx->var_cfg_idx, &ctx->mpi_input.var);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get Var on win %u. err: %d", win_idx.value, ret);
		return ret;
	}

	return 0;
}

int detectTdObject(MPI_WIN win_idx, MPI_CHN_LAYOUT_S *chn_layout __attribute__((unused)), VFTR_TD_PARAM_S *td_param)
{
	UINT32 timestamp = 0;
	const INT32 timeout = 0;
	INT32 ret = 0;
	g_td_running = 1;
	MPI_RECT_S ini_roi;
	MPI_WIN_ATTR_S win_attr;
	TD_CTX_S ctx;
	VFTR_TD_INSTANCE_S *instance;

	/* Check win state */
	ret = MPI_DEV_getWindowAttr(win_idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get win %u attributes. err: %d", win_idx.value, ret);
		return ret;
	}

	/* register path */
	const UINT32 dev_idx = MPI_GET_VIDEO_DEV(win_idx);

	if (win_attr.path.bit.path0_en) {
		ctx.path = MPI_INPUT_PATH(dev_idx, 0);
	} else if (win_attr.path.bit.path1_en) {
		ctx.path = MPI_INPUT_PATH(dev_idx, 1);
	} else if (win_attr.path.bit.path2_en) {
		ctx.path = MPI_INPUT_PATH(dev_idx, 2);
	} else if (win_attr.path.bit.path3_en) {
		ctx.path = MPI_INPUT_PATH(dev_idx, 3);
	} else {
		log_err("Wrong path bmp %d setting.", win_attr.path.bmp);
		return -EINVAL;
	}

#ifdef CONFIG_APP_TD_SUPPORT_SEI
	int empty_idx, set_idx;
	set_idx = findTdCtx(win_idx, vftr_res_shm->td_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("TD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create TD instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	AVFTR_TD_CTX_S *td_ctx = &vftr_res_shm->td_ctx[empty_idx];
	td_ctx->en = 1;
	td_ctx->reg = 1;
	td_ctx->idx = win_idx;
	td_ctx->cb = NULL;

	// init vftr buffer ctx
	int buf_info_idx;
	set_idx = findVftrBufCtx(win_idx, vftr_res_shm->buf_info, &empty_idx);
	if (set_idx >= 0) {
		buf_info_idx = set_idx;
	} else if (empty_idx >= 0) {
		buf_info_idx = empty_idx;
	} else {
		log_err("Failed to register vftr buffer ctx on win(%u, %u, %u).", win_idx.dev, win_idx.chn,
		        win_idx.win);
		return -ENOMEM;
	}

	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	buf_info->idx = win_idx;
#endif
	VFTR_init(NULL);

	/* create TD instance */
	instance = VFTR_TD_newInstance();
	if (!instance) {
		log_err("Failed to create TD instance");
		return -EINVAL;
	}

	/* get input resolution & luma roi for TD */
	MPI_ROI_ATTR_S roi_attr;
	MPI_PATH_ATTR_S path_attr;

	ret = MPI_DEV_getPathAttr(ctx.path, &path_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get path 0x%08x attr. err: %d", ctx.path.value, ret);
		return ret;
	}

	ret = MPI_getRoiAttr(ctx.path, &roi_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get path 0x%08x roi attr. err: %d", win_idx.value, ret);
		return ret;
	}

	ini_roi.width = (roi_attr.luma_roi.ex - roi_attr.luma_roi.sx) * path_attr.res.width / 1024;
	ini_roi.height = (roi_attr.luma_roi.ey - roi_attr.luma_roi.sy) * path_attr.res.height / 1024;

	ret = VFTR_TD_init(instance, &ini_roi, (int)win_attr.fps);
	if (ret) {
		log_err("Init TD failed! err: %d", ret);
		return -EINVAL;
	}

	/* set parameter to td */
	ret = VFTR_TD_setParam(instance, td_param);
	if (ret) {
		log_err("Failed to set TD params. err: %d", ret);
		return -EINVAL;
	}

	/* add isp mv hist & var configs*/
	MPI_RECT_POINT_S roi = { .sx = chn_layout->window[0].x,
		                 .sy = chn_layout->window[0].y,
		                 .ex = (chn_layout->window[0].x + chn_layout->window[0].width - 1),
		                 .ey = (chn_layout->window[0].y + chn_layout->window[0].height - 1) };

	MPI_ISP_MV_HIST_CFG_S mv_hist_cfg = { .roi = roi };
	MPI_ISP_VAR_CFG_S var_cfg = { .roi = roi };

	ret = MPI_DEV_addIspMvHistCfg(win_idx, &mv_hist_cfg, &ctx.mv_hist_cfg_idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to add MvHist cfg on win %u. err: %d", win_idx.value, ret);
		return ret;
	}

	ret = MPI_DEV_addIspVarCfg(win_idx, &var_cfg, &ctx.var_cfg_idx);
	if (ret != 0) {
		log_err("Failed to add Var cfg on win %u. err: %d", win_idx.value, ret);
		return ret;
	}

	while (g_td_running) {
		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(win_idx, &timestamp, timeout);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", win_idx.dev, win_idx.chn,
			        win_idx.win, ret);
			continue;
		}

#ifdef CONFIG_APP_TD_SUPPORT_SEI
		/* Find cur buffer idx */
		int buf_idx = updateBufferIdx(buf_info, timestamp);
#endif
		/* Update MPI info */
		ret = updateMpiInfo(win_idx, &ctx);
		if (ret != 0) {
			return ret;
		}

		VFTR_TD_STATUS_S *td_res = NULL;

#ifdef CONFIG_APP_TD_SUPPORT_SEI
		td_res = &td_ctx->td_res[buf_idx];
#else
		VFTR_TD_STATUS_S status;
		td_res = &status;
#endif
		ret = VFTR_TD_detect(instance, &ctx.mpi_input, td_res);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to detect tamper. err: %d", ret);
			continue;
		}

		if (td_res->alarm == (1 << VFTR_TD_ALARM_BLOCK)) {
			log_info("Camera blocked!!");
		} else if (td_res->alarm == (1 << VFTR_TD_ALARM_REDIRECT)) {
			log_info("Camera redirected!!");
		} else if (td_res->alarm == (1 << VFTR_TD_ALARM_NUM)) {
			log_info("Both camera tamper occurred!!");
		}

		/* mark buffer ready for display */
#ifdef CONFIG_APP_TD_SUPPORT_SEI
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	clearTdCtx(win_idx, vftr_res_shm->td_ctx);

#ifdef CONFIG_APP_TD_SUPPORT_SEI
	clearVftrBufCtx(win_idx, vftr_res_shm->buf_info);
#endif

	/* delete TD instance */
	VFTR_TD_deleteInstance(&instance);

	VFTR_exit();

	return 0;
}
