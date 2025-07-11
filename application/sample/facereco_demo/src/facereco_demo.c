#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "log.h"

#ifdef CONFIG_APP_FACERECO_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include "video_od.h"

#include "facereco_demo.h"

//#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern MPI_SIZE_S g_chn_resoln;
extern MPI_RECT_POINT_S g_chn_bdry;
extern MPI_IVA_OD_PARAM_S g_od_param;
extern int g_facereco_running;
int g_facereco_roi = 1;

#ifdef CONFIG_APP_FACERECO_SUPPORT_SEI
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
 * @brief fillin data for od with facereco result for rtsp iva display
 */
static int GetOdRes(MPI_WIN idx __attribute__((unused)), MPI_IVA_OBJ_LIST_S *ol, EAIF_STATUS_S *status, int buf_idx)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];

	if (vftr_od_ctx->en) {
		VIDEO_FTR_OBJ_LIST_S *obj_list = &vftr_od_ctx->ol[buf_idx];
		obj_list->basic_list = *ol;
		for (int i = 0; i < ol->obj_num; i++) {
			int id = obj_list->basic_list.obj[i].id;
			EAIF_OBJ_ATTR_S *eobj = NULL;
			int j;
			for (j = 0; (UINT32)j < status->obj_cnt; j++) {
				if (id == status->obj_attr[j].id) {
					eobj = &status->obj_attr[j];
					break;
				}
			}
			if (eobj) {
				if (eobj->label_num) {
					strncpy(obj_list->obj_attr[i].cat, eobj->category[0],
				        VFTR_OBJ_CAT_LEN - 1);
					strncpy(obj_list->obj_attr[i].conf, eobj->prob[0], VFTR_OBJ_CAT_LEN - 1);
				} else {
					obj_list->obj_attr[i].cat[0] = 0;
					obj_list->obj_attr[i].conf[0] = 0;
				}
			} else {
				obj_list->obj_attr[i].cat[0] = 0;
				obj_list->obj_attr[i].conf[0] = 0;
			}
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

static void GetFaceRecoResult(MPI_WIN idx __attribute__((unused)), const EAIF_STATUS_S *status, int buf_idx)
{
	int eaif_idx = findEaifCtx(idx, vftr_res_shm->eaif_ctx, NULL);
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = &vftr_res_shm->eaif_ctx[eaif_idx];

	if (vftr_eaif_ctx->en) {
		vftr_eaif_ctx->stat[buf_idx] = *status;
	}
}

#else /* CONFIG_APP_HD_SUPPORT_SEI */

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

#ifdef CONFIG_APP_FACERECO_SUPPORT_SEI
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
		log_err("Disable object detect on win %u failed.", idx.win);
		goto error;
	}

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}

int runFaceRecognition(MPI_WIN idx, FACERECO_PARAM_S *facereco_param)
{
	const int timeout = 0;
	int ret = 0;
	uint32_t timestamp = 0;

#ifdef CONFIG_APP_FACERECO_SUPPORT_SEI
	// init eaif ctx
	int empty_idx, set_idx;
	set_idx = findEaifCtx(idx, vftr_res_shm->eaif_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("EAIF is already registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create EAIF instance on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	AVFTR_EAIF_CTX_S *fr_ctx = &vftr_res_shm->eaif_ctx[empty_idx];
	fr_ctx->en = 1;
	fr_ctx->reg = 1;
	fr_ctx->idx = idx;
	fr_ctx->cb = NULL;

	// init vftr buffer ctx
	int buf_info_idx;
	set_idx = findVftrBufCtx(idx, vftr_res_shm->buf_info, &empty_idx);
	if (set_idx >= 0) {
		buf_info_idx = set_idx;
	} else if (empty_idx >= 0) {
		buf_info_idx = empty_idx;
	} else {
		log_err("Failed to register vftr buffer ctx on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	buf_info->idx = idx;
#endif

	const int inf_with_obj_list = facereco_param->inf_with_obj_list;

	if (inf_with_obj_list) {
		if ((ret = VIDEO_FTR_enableOd(idx))) {
			return -EINVAL;
		}
	}

	EAIF_INSTANCE_S *facereco = EAIF_newInstance(idx);
	if (!facereco) {
		log_err("Failed to create EAIF instance!");
		return -EINVAL;
	}

	DBG("[INFO] Add FACRECO instance!\n");

	// set param
	if ((ret = EAIF_setParam(facereco, facereco_param))) {
		log_err("Failed to set facereco params! err: %d", ret);
		return -EINVAL;
	}
	DBG("[INFO] Set Face Recognition parameter!\n");

	// enable eaif
	if ((ret = EAIF_activate(facereco))) {
		log_err("Failed to activate EAIF! err: %d", ret);
		return -EINVAL;
	}
	DBG("[INFO] Enabled Face recognition!\n");
	g_facereco_running = 1;

#ifdef SEI_DRAW_ROI
	VIDEO_VDBG_CTX_S *vdbg_ctx = &vftr_res_shm->vdbg_ctx;
	vdbg_ctx->en = 1;
	vdbg_ctx->ctx |= VIDEO_VDBG_DEBUG;
#endif

	while (g_facereco_running) {
		MPI_IVA_OBJ_LIST_S obj_list_original = { 0 };
		//MPI_IVA_OBJ_LIST_S obj_list_filtered = { 0 };
		MPI_IVA_OBJ_LIST_S *obj_list = &obj_list_original;
		EAIF_STATUS_S status = { 0 };

		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(idx, &timestamp, timeout);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", idx.dev, idx.chn, idx.win, ret);
			continue;
		}

#ifdef CONFIG_APP_FACERECO_SUPPORT_SEI
		int buf_idx = updateVftrBufferInfo(buf_info, timestamp);
#endif
		if (inf_with_obj_list)
			GetObjList(idx, timestamp, &obj_list_original);

		// get classification result from eaif client module
		EAIF_testRequestV2(facereco, obj_list, &status);

		if (inf_with_obj_list) {
			if (obj_list->obj_num) {
				DBG("[DEB] FACERECO result timestamp: %u\n", timestamp);
				for (int i = 0; i < obj_list->obj_num; i++) {
					DBG("[DEB] FACERECO idx:%d life:%3d [%d,%d,%d,%d] class:%s\n",
					    obj_list->obj[i].id, obj_list->obj[i].life, obj_list->obj[i].rect.sx,
					    obj_list->obj[i].rect.sy, obj_list->obj[i].rect.ex,
					    obj_list->obj[i].rect.ey, status.obj_attr[i].category[0]);
				}
				DBG("\n");
			}
		} else {
			DBG("[DEB] FACERECO result timestamp: %u\n", timestamp);
			for (int i = 0; (UINT32)i < status.obj_cnt; i++) {
				DBG("[DEB] FACERECO idx:%d [%d,%d,%d,%d] class:%s\n", status.obj_attr[i].id,
				    status.obj_attr[i].rect.sx, status.obj_attr[i].rect.sy, status.obj_attr[i].rect.ex,
				    status.obj_attr[i].rect.ey,
				    status.obj_attr[i].label_num ? status.obj_attr[i].category[0] : "");
			}
			DBG("\n");
		}

#ifdef CONFIG_APP_FACERECO_SUPPORT_SEI
		// copy to object list result to share memory
		if (inf_with_obj_list) {
			GetOdRes(idx, obj_list, &status, buf_idx);
			GetFaceRecoResult(idx, &status, buf_idx);
		} else {
			GetFaceRecoResult(idx, &status, buf_idx);
		}
#ifdef SEI_DRAW_ROI
		char *vdbg_buff = &vdbg_ctx->data[buf_idx][0];
		if (g_facereco_roi) {
			int data_size = 0;
			MPI_RECT_POINT_S *roi = &facereco_param->inf_utils.roi;
			data_size += sprintf(&vdbg_buff[data_size],
			                     "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":2,\"lncr\":[0,0,255]}},",
			                     roi->sx * g_chn_resoln.width / 100, roi->sy * g_chn_resoln.height / 100,
			                     roi->ex * g_chn_resoln.width / 100, roi->ey * g_chn_resoln.height / 100);
			vdbg_buff[data_size] = 0;
			vdbg_ctx->data_len[buf_idx] = data_size;
		}
#endif // SEI_DRAW_ROI
		buf_info->buf_ready[buf_idx] = 1;

#endif /* CONFIG_APP_FACERECO_SUPPORT_SEI */
	}

	DBG("[INFO] Exiting running loop!\n");

	ret = EAIF_deactivate(facereco);
	if (ret) {
		log_err("Failed to deactivate EAIF! err: %d", ret);
		return -EINVAL;
	}

	DBG("[INFO] Disabled FACRECO!\n");

	ret = EAIF_deleteInstance(&facereco);
	if (ret && facereco) {
		log_err("Failed to delete EAIF instance! err: %d", ret);
		return -EINVAL;
	}

	DBG("[INFO] Deleted EAIF!\n");

	if (inf_with_obj_list) {
		if ((ret = VIDEO_FTR_disableOd(idx))) {
			return -EINVAL;
		}
	}

	return 0;
}
