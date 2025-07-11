#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"

#include "inf_face.h"
#include "inf_model.h"

#include "facedet_demo.h"
#include "log.h"

#ifdef CONFIG_APP_FACEDET_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include "video_od.h"

//#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

extern MPI_RECT_POINT_S g_chn_bdry;
extern MPI_IVA_OD_PARAM_S g_od_param;
extern int g_facedet_running;

#ifdef CONFIG_APP_FACEDET_SUPPORT_SEI
extern AVFTR_CTX_S *avftr_res_shm;
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

static int findOdCtx(MPI_WIN idx, VIDEO_OD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_OD_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].en) {
			find_idx = i;
		} else if (emp_idx == -1 && !(ctx[i].en || ctx[i].en_implicit)) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int findEaifCtx(const MPI_WIN idx, const AVFTR_EAIF_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}
	for (i = 0; i < AVFTR_EAIF_MAX_SUPPORT_NUM; i++) {
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

/**
 * @brief update ring buffer for iva sei display
 */
static int updateVftrBufferInfo(AVFTR_VIDEO_BUF_INFO_S *buf_info, uint32_t timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;
	return buf_info->buf_cur_idx;
}

/**
 * @brief fillin data for od with facedet result for rtsp iva display
 */
static int GetOdRes(MPI_WIN idx, MPI_IVA_OBJ_LIST_S *ol, int buf_idx)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];

	if (vftr_od_ctx->en) {
		VIDEO_FTR_OBJ_LIST_S *obj_list = &vftr_od_ctx->ol[buf_idx];
		obj_list->basic_list = *ol;
	}

	return MPI_SUCCESS;
}

static int GetFaceRes(MPI_WIN idx, MPI_IVA_OBJ_LIST_S *face_list, int buf_idx)
{
	int eaif_idx = findEaifCtx(idx, vftr_res_shm->eaif_ctx, NULL);
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = &vftr_res_shm->eaif_ctx[eaif_idx];

	if (vftr_eaif_ctx->en) {
		EAIF_STATUS_S *list = &vftr_eaif_ctx->stat[buf_idx];
		list->obj_cnt = face_list->obj_num;
		list->timestamp = face_list->timestamp;
		for (int i = 0; i < face_list->obj_num; i++) {
			list->obj_attr[i].id = face_list->obj[i].id;
			list->obj_attr[i].rect = face_list->obj[i].rect;
		}
	}
	return MPI_SUCCESS;
}

/**
 * @brief fill object context
 */
static int GetObjList(MPI_WIN idx, UINT32 timestamp, MPI_IVA_OBJ_LIST_S *obj_list)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];

	if (vftr_od_ctx->en) {
		int ret = 0;
		MPI_RECT_POINT_S *bd = &vftr_od_ctx->bdry;
		MPI_RECT_POINT_S *obj;
		MPI_RECT_POINT_S *final_obj;
		int obj_cnt = 0;

		ret = MPI_IVA_getBitStreamObjList(idx, timestamp, obj_list);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to get object list. err: %d", ret);
			goto err;
		}

		/* Below segment is to remove out of boundary object list */
		for (int i = 0; i < obj_list->obj_num; i++) {
			/* Limit OL boundary */
			obj = &obj_list->obj[i].rect;
			final_obj = &obj_list->obj[obj_cnt].rect;

			/* NOTICE: the following code remove the object that out of boundary
             *         or crop the object to fit the output image */
			if (bd->sx != -1) {
				if (obj->ex < bd->sx) {
					continue;
				}
				final_obj->sx = obj->sx > bd->sx ? obj->sx : bd->sx;
			} else {
				final_obj->sx = obj->sx;
			}

			if (bd->sy != -1) {
				if (obj->ey < bd->sy) {
					continue;
				}
				final_obj->sy = obj->sy > bd->sy ? obj->sy : bd->sy;
			} else {
				final_obj->sy = obj->sy;
			}

			if (bd->ex != -1) {
				if (obj->sx > bd->ex) {
					continue;
				}
				final_obj->ex = obj->ex > bd->ex ? bd->ex : obj->ex;
			} else {
				final_obj->ex = obj->ex;
			}

			if (bd->ey != -1) {
				if (obj->sy > bd->ey) {
					continue;
				}
				final_obj->ey = obj->ey > bd->ey ? bd->ey : obj->ey;
			} else {
				final_obj->ey = obj->ey;
			}

			obj_list->obj[obj_cnt].id = obj_list->obj[i].id;
			obj_list->obj[obj_cnt].life = obj_list->obj[i].life;
			obj_list->obj[obj_cnt].mv = obj_list->obj[i].mv;

			/* increase the index */
			obj_cnt++;
		}

		obj_list->obj_num = obj_cnt;
	}

	return MPI_SUCCESS;
err:
	return MPI_FAILURE;
}

#else /* CONFIG_APP_FACEDET_SUPPORT_SEI */

static int GetObjList(MPI_WIN idx, UINT32 timestamp, MPI_IVA_OBJ_LIST_S *obj_list)
{
	const int ret = MPI_IVA_getBitStreamObjList(idx, timestamp, obj_list);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get object list. err: %d", ret);
		return MPI_FAILURE;
	}

	return 0;
}

#endif

/**
 * @brief declaration of od enable in enable OD ()
 */
int VIDEO_FTR_enableOd(MPI_WIN idx)
{
	const MPI_IVA_OD_PARAM_S *od = &g_od_param;
	int ret;

	ret = MPI_IVA_setObjParam(idx, od);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to set OD param. err: %d", ret);
		goto error;
	}

	ret = MPI_IVA_enableObjDet(idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to enable OD. err: %d", ret);
		goto error;
	}

#ifdef CONFIG_APP_FACEDET_SUPPORT_SEI
	/* OD information */
	int empty_idx, set_idx;
	set_idx = findOdCtx(idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_err("OD is already registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -EINVAL;
	}
	if (empty_idx < 0) {
		log_err("Failed to create OD on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	VIDEO_OD_CTX_S *od_ctx = &vftr_res_shm->od_ctx[empty_idx];
	od_ctx->en = 1;
	od_ctx->en_shake_det = 0;
	od_ctx->en_crop_outside_obj = 0;
	od_ctx->idx = idx;
	od_ctx->cb = NULL;
	od_ctx->bdry = g_chn_bdry;
#endif

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}

/**
 * @brief declaration of od disable
 */
int VIDEO_FTR_disableOd(MPI_WIN idx)
{
	const int ret = MPI_IVA_disableObjDet(idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to disable OD. err: %d", ret);
		goto error;
	}

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}

static int GetImage(const FACEDET_PARAM_S *param, MPI_VIDEO_FRAME_INFO_S *frame_info, InfImage *image)
{
	MPI_WIN idx = param->target_idx;
	image->c = 3;
	image->dtype = Inf8UC3;
	image->w = param->snapshot_width;
	image->h = param->snapshot_height;
	frame_info->width = param->snapshot_width;
	frame_info->height = param->snapshot_height;

	switch (param->data_fmt) {
	case EAIF_DATA_MPI_Y:
	case EAIF_DATA_Y: {
		frame_info->type = MPI_SNAPSHOT_Y;
		image->c = 1;
		image->dtype = Inf8UC1;
		break;
	}
	case EAIF_DATA_MPI_YUV:
	case EAIF_DATA_YUV: {
		frame_info->type = MPI_SNAPSHOT_NV12;
		break;
	}
	case EAIF_DATA_MPI_RGB:
	case EAIF_DATA_RGB: {
		frame_info->type = MPI_SNAPSHOT_RGB;
		break;
	}
	default: {
		frame_info->type = MPI_SNAPSHOT_Y;
		image->c = 1;
		image->dtype = Inf8UC1;
	}
	};

	int ret = FillImageDataSnapshot(idx, frame_info);
	if (ret) {
		return -1;
	}

	UINT32 expected_data_size = frame_info->height * frame_info->width * image->c;
	if (expected_data_size != frame_info->size) {
		fd_err("Invalid retrieved window snapshot size (%u) vs expected (%u)\n", frame_info->size,
		       expected_data_size);
		return -1;
	}
	image->data = frame_info->uaddr;
	image->buf_owner = 0;
	return 0;
}

int runFaceDetection(MPI_WIN win_idx, MPI_SIZE_S *chn_resoln, FACEDET_PARAM_S *facedet_param)
{
	int ret = 0;
	uint32_t timestamp = 0;
	const int timeout = 0;
	const int inf_with_obj_list = facedet_param->inf_with_obj_list;
	const int detection_period = facedet_param->detection_period;
	const int life_th = facedet_param->obj_life_th;
	g_facedet_running = 1;
	int detection_counter = 0;
	FixedPointSize scale_factor = { .width = 0, .height = 0 };
	MPI_IVA_OBJ_LIST_S obj_list_raw = { 0 };
	MPI_IVA_OBJ_LIST_S obj_list = { 0 };
	MPI_IVA_OBJ_LIST_S obj_list_result = { 0 };
	InfModelCtx ctx = { .info = NULL, .model = NULL };

#ifdef CONFIG_APP_FACEDET_SUPPORT_SEI
	// init eaif ctx
	int empty_idx, set_idx;
	set_idx = findEaifCtx(win_idx, vftr_res_shm->eaif_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("EAIF is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create EAIF instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	AVFTR_EAIF_CTX_S *eaif_ctx = &vftr_res_shm->eaif_ctx[empty_idx];
	eaif_ctx->reg = 1;
	eaif_ctx->en = 1;
	eaif_ctx->idx = win_idx;
	eaif_ctx->cb = NULL;

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

	// 1. Enable od if needed
	if (inf_with_obj_list) {
		if ((ret = VIDEO_FTR_enableOd(win_idx))) {
			return -EINVAL;
		}
	}

	// 1.a calculate chn resoln to snapshot scale factor
	CalcScaleFactor(facedet_param->snapshot_width, facedet_param->snapshot_height, chn_resoln,
		                &scale_factor);

	// 2. Init face detect model
	ret = Inf_InitModel(&ctx, facedet_param->face_detect_model);
	if (ret || !ctx.model) {
		log_err("Cannot create face detection instance! err: %d", ret);
		goto release_od;
	}

	InfImage image = { 0 };
	InfDetList result = { 0 };
	MPI_VIDEO_FRAME_INFO_S frame_info = { 0 };

	while (g_facedet_running) {
		frame_info = (MPI_VIDEO_FRAME_INFO_S){ 0 };

		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(win_idx, &timestamp, timeout);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", win_idx.dev, win_idx.chn,
			        win_idx.win, ret);
			continue;
		}

		detection_counter++;
		if (detection_counter < detection_period) {
			continue;
		}
		detection_counter = 0;

#ifdef CONFIG_APP_FACEDET_SUPPORT_SEI
		int buf_idx = updateVftrBufferInfo(buf_info, timestamp);
#endif

		if (inf_with_obj_list) {
			// 3. Get obj list
			if ((ret = GetObjList(win_idx, timestamp, &obj_list_raw))) {
				goto release_model;
			}

			// 3.5 Filter Scale objects
			FilterAndCopyScaledListWithBoundary(chn_resoln, &scale_factor, &obj_list_raw, life_th, &obj_list);
		}

		// 4. Get Snapshot
		ret = GetImage(facedet_param, &frame_info, &image);
		if (ret || !image.data) {
			goto release_image;
		}

		// 5. Invoke face detection inference
		if (inf_with_obj_list) {
			// 5.a.1 Object list based detection
			ret = Inf_InvokeFaceDetObjList(&ctx, &image, &obj_list, &result);
		} else {
			// 5.a.2 Full scale frame based detection
			ret = Inf_InvokeFaceDet(&ctx, &image, &result);
		}

		if (ret) {
			log_err("Failed to invoke face detection inference. err: %d", ret);
			goto release_image;
		}

		// 5.b release window frame
		ret = MPI_DEV_releaseWinFrame(win_idx, &frame_info);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to release window frames. err: %d", ret);
			goto release_result;
		}

		// 5.c Scale results
		ReverseScaledResult(&scale_factor, &result, &obj_list_result);

		// 5.d Release Result
		ret = Inf_ReleaseDetResult(&result);
		if (ret) {
			log_err("Failed to release detection result. err: %d", ret);
			goto release_model;
		}

#ifdef CONFIG_APP_FACEDET_SUPPORT_SEI
		// copy to object list result to share memory
		GetOdRes(win_idx, &obj_list_raw, buf_idx);
		GetFaceRes(win_idx, &obj_list_result, buf_idx);
		buf_info->buf_ready[buf_idx] = 1;
#endif /* CONFIG_APP_FACEDET_SUPPORT_SEI */

	}

	goto release_model;

release_result:

	ret = Inf_ReleaseDetResult(&result);
	if (ret) {
		log_err("Failed to release detection result. err: %d", ret);
	}

release_image:
	ret = MPI_DEV_releaseWinFrame(win_idx, &frame_info);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to release window frames. err: %d", ret);
	}

release_model:

	DBG("[INFO] Exiting running loop!\n");
	// 6. Exit
	// 6.a Release model
	ret = Inf_ReleaseModel(&ctx);
	if (ret || ctx.model) {
		log_err("Failed to release model. err: %d", ret);
	}

release_od:

	if (inf_with_obj_list) {
		DBG("[INFO] Deleted EAIF!\n");
		// 6.b Disable od
		ret = VIDEO_FTR_disableOd(win_idx);
		if (ret) {
			log_err("Fail to disable OD. err: %d", ret);
		}
	}
	return ret;
}