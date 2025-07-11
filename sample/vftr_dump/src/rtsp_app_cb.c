#include "rtsp_app_cb.h"

#include <assert.h>
#include <errno.h>
#include "avftr_conn.h"
#include "mpi_dev.h"
#include "mpi_sys.h"

extern AVFTR_CTX_S *avftr_res_shm;
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

app_cb_t g_app_cb = NULL;

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
		if (find_idx == -1 && ctx[i].idx.value == idx.value && (ctx[i].en || ctx[i].en_implicit)) {
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

static int findVftrBufCtx(MPI_WIN idx, const AVFTR_VIDEO_BUF_INFO_S *ctx)
{
	for (int i = 0; i < AVFTR_VIDEO_MAX_SUPPORT_NUM; i++) {
		if (ctx[i].idx.value == idx.value) {
			return i;
		}
	}

	return -1;
}

/**
 * @brief update ring buffer for iva sei display
 */
int updateBufIndex(MPI_WIN idx, UINT32 timestamp)
{
	int buf_info_idx = findVftrBufCtx(idx, vftr_res_shm->buf_info);
	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;

	return buf_info->buf_cur_idx;
}

/**
 * @brief update ring buffer for iva sei display
 */
int setBufReady(MPI_WIN idx, int buf_index)
{
	int buf_info_idx = findVftrBufCtx(idx, vftr_res_shm->buf_info);
	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	buf_info->buf_ready[buf_index] = 1;

	return 0;
}

/**
 * @brief fill object context
 */
int refineObjList(MPI_IVA_OBJ_LIST_S *obj_list)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	MPI_RECT_POINT_S *bd = &vftr_od_ctx[0].bdry;
	MPI_RECT_POINT_S *obj;
	MPI_RECT_POINT_S *final_obj;
	int obj_cnt = 0;

	/* Below segment is to remove out of boundary object list */
	for (int i = 0; i < obj_list->obj_num; i++) {
		/* Limit OL boundary */
		obj = &obj_list->obj[i].rect;
		final_obj = &obj_list->obj[obj_cnt].rect;

#define CLIP_BD_S(bs, be, os, oe, result)     \
	if (bs != -1) {                       \
		if (oe < bs)                  \
			continue;             \
		result = (os > bs) ? os : bs; \
	} else {                              \
		result = os;                  \
	}
#define CLIP_BD_E(bs, be, os, oe, result)     \
	if (be != -1) {                       \
		if (os > be)                  \
			continue;             \
		result = (oe > be) ? be : oe; \
	} else {                              \
		result = oe;                  \
	}

		/* NOTICE: the following code remove the object that out of boundary
		 *         or crop the object to fit the output image */
		CLIP_BD_S(bd->sx, bd->ex, obj->sx, obj->ex, final_obj->sx);
		CLIP_BD_S(bd->sy, bd->ey, obj->sy, obj->ey, final_obj->sy);
		CLIP_BD_E(bd->sx, bd->ex, obj->sx, obj->ex, final_obj->ex);
		CLIP_BD_E(bd->sy, bd->ey, obj->sy, obj->ey, final_obj->ey);
		obj_list->obj[obj_cnt].id = obj_list->obj[i].id;
		obj_list->obj[obj_cnt].life = obj_list->obj[i].life;
		obj_list->obj[obj_cnt].mv = obj_list->obj[i].mv;

		/* increase the index */
		obj_cnt++;
	}

	obj_list->obj_num = obj_cnt;

	return MPI_SUCCESS;
}

int initOdCtx(MPI_WIN win_idx)
{
	/* OD information */
	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(0, win_idx.chn);

	/* Initialize MPI system */
	MPI_SYS_init();

	int ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to get video channel attribute. err: %d", ret);
		return -EINVAL;
	}

	/* TODO: please check the following code */
	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to get video layout attribute. err: %d", ret);
		return -EINVAL;
	}

	MPI_SYS_exit();

	/* NOTICE: we only support the case that only one window in a channel */
	assert(layout_attr.window_num == 1);
	assert(layout_attr.window[0].x == 0);
	assert(layout_attr.window[0].y == 0);
	assert(layout_attr.window[0].width == chn_attr.res.width);
	assert(layout_attr.window[0].height == chn_attr.res.height);

	/* init OD ctx */
	int empty_idx, set_idx;
	set_idx = findOdCtx(win_idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		fprintf(stderr, "OD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -EINVAL;
	}
	if (empty_idx < 0) {
		fprintf(stderr, "Failed to create OD on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	VIDEO_OD_CTX_S *od_ctx = &vftr_res_shm->od_ctx[empty_idx];
	od_ctx->bdry =
	        (MPI_RECT_POINT_S){ .sx = 0, .sy = 0, .ex = chn_attr.res.width - 1, .ey = chn_attr.res.height - 1 };

	od_ctx->en = 1;
	od_ctx->en_shake_det = 0;
	od_ctx->en_crop_outside_obj = 0;
	od_ctx->idx = win_idx;
	od_ctx->cb = NULL;

	return 0;
}

int updateOdStatus(MPI_WIN idx, MPI_IVA_OBJ_LIST_S *ol, int buf_index)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];
	VIDEO_FTR_OBJ_LIST_S *vftr_obj_list = &vftr_od_ctx->ol[buf_index];

	/**
	 * get object list from MPI
	 */
	memset(vftr_obj_list, 0, sizeof(*vftr_obj_list));
	memcpy(&vftr_obj_list->basic_list, ol, sizeof(MPI_IVA_OBJ_LIST_S));

	return 0;
}

int updateShdStatus(MPI_WIN idx, VFTR_SHD_STATUS_S *status, int buf_index)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];
	VIDEO_FTR_OBJ_LIST_S *obj_list = &vftr_od_ctx->ol[buf_index];
	int obj_cnt = obj_list->basic_list.obj_num;

	for (int i = 0; i < obj_cnt; i++) {
		obj_list->obj_attr[i].shaking = status->shaking[i];
	}

	return 0;
}
