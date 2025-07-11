#include "rtsp_app_cb.h"
#include "rtsp_shm.h"
#include "eaif_dump_define.h"
#include "avftr_conn.h"
#include <assert.h>
#include <errno.h>

extern AVFTR_CTX_S *avftr_res_shm;
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

static int findMdCtx(const MPI_WIN idx, const AVFTR_MD_CTX_S *ctx, int *empty)
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

static int updateMdResult(MPI_WIN idx, VFTR_MD_STATUS_S *status, int buf_index)
{
	int md_idx = findMdCtx(idx, vftr_res_shm->md_ctx, NULL);
	AVFTR_MD_CTX_S *md_ctx = &vftr_res_shm->md_ctx[md_idx];
	VFTR_MD_STATUS_S *md_res = &md_ctx->md_res[buf_index];

	memcpy(md_res, status, sizeof(*md_res));

	return MPI_SUCCESS;
}

static int updateMdStatus(MPI_WIN idx, void *buf, UINT32 count, UINT32 flag, struct timespec ts)
{
	VFTR_DUMP_FLAG_V1_U msg = (VFTR_DUMP_FLAG_V1_U)flag;

	if (msg.field.category != FLAG_CAT_VFTR) {
		return 0;
	}

	static MPI_IVA_OBJ_LIST_S obj_list = { 0 };
	static int buf_index = 0;

	if (msg.field.id == VFTR_ID_MPI_IVA_OBJ_LIST_S) {
		assert(sizeof(MPI_IVA_OBJ_LIST_S) == count);
		memcpy(&obj_list, buf, count);
		buf_index = updateBufIndex(idx, (UINT32)ts.tv_sec);
		refineObjList(&obj_list);
		updateOdStatus(idx, &obj_list, buf_index);
	} else if (msg.field.id == VFTR_ID_VFTR_SHD_STATUS_S) {
		assert(sizeof(VFTR_SHD_STATUS_S) == count);
		VFTR_SHD_STATUS_S *shd = buf;
		updateShdStatus(idx, shd, buf_index);
	} else if (msg.field.id == VFTR_ID_VFTR_MD_STATUS_S) {
		VFTR_MD_STATUS_S *status = buf;
		assert(sizeof(VFTR_MD_STATUS_S) == count);
		updateMdResult(idx, status, buf_index);
		setBufReady(idx, buf_index);
	} else {
		/* do nothing */
	}

	return 0;
}

int initMdCb(MPI_WIN win_idx)
{
	initOdCtx(win_idx);
	g_app_cb = updateMdStatus;

	int empty_idx, set_idx;
	set_idx = findMdCtx(win_idx, vftr_res_shm->md_ctx, &empty_idx);
	if (set_idx >= 0) {
		printf("MD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		fprintf(stderr, "Failed to create MD instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn,
		        win_idx.win);
		return -ENOMEM;
	}

	AVFTR_MD_CTX_S *md_ctx = &vftr_res_shm->md_ctx[empty_idx];
	md_ctx->en = 1;
	md_ctx->reg = 1;
	md_ctx->idx = win_idx;
	md_ctx->cb = NULL;

	return 0;
}
