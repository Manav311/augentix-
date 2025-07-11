#include "rtsp_app_cb.h"
#include "rtsp_shm.h"
#include "eaif_dump_define.h"
#include "avftr_conn.h"
#include <assert.h>
#include <errno.h>

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

static int updateEaifResult(const MPI_WIN idx, const MPI_IVA_OBJ_LIST_S *ol, const EAIF_STATUS_S *status, int buf_idx)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];
	VIDEO_FTR_OBJ_LIST_S *obj_list;
	const EAIF_OBJ_ATTR_S *eaif_obj = NULL;

	int i;
	unsigned int j;
	int id;

	obj_list = &vftr_od_ctx->ol[buf_idx];
	for (i = 0; i < ol->obj_num; i++) {
		id = obj_list->basic_list.obj[i].id;
		eaif_obj = NULL;
		for (j = 0; j < status->obj_cnt; j++) {
			if (id == status->obj_attr[j].id) {
				eaif_obj = &status->obj_attr[j];
				break;
			}
		}

		if (eaif_obj && eaif_obj->label_num) {
			strncpy(obj_list->obj_attr[i].cat, eaif_obj->category[0], VFTR_OBJ_CAT_LEN - 1);
			strncpy(obj_list->obj_attr[i].conf, eaif_obj->prob[0], VFTR_OBJ_CAT_LEN - 1);
		} else {
			obj_list->obj_attr[i].cat[0] = 0;
			obj_list->obj_attr[i].conf[0] = 0;
		}
	}

	return MPI_SUCCESS;
}

static int updateHdStatus(MPI_WIN idx, void *buf, UINT32 count, UINT32 flag, struct timespec ts)
{
	VFTR_DUMP_FLAG_V1_U msg = (VFTR_DUMP_FLAG_V1_U)flag;

	if (msg.field.category != FLAG_CAT_EAIF) {
		return 0;
	}

	static MPI_IVA_OBJ_LIST_S obj_list = { 0 };
	static int buf_index = 0;

	if (msg.field.id == EAIF_ID_MPI_IVA_OBJ_LIST_S) {
		assert(sizeof(MPI_IVA_OBJ_LIST_S) == count);
		memcpy(&obj_list, buf, count);
		buf_index = updateBufIndex(idx, (UINT32)ts.tv_sec);
		refineObjList(&obj_list);
		updateOdStatus(idx, &obj_list, buf_index);
	} else if (msg.field.id == EAIF_ID_EAIF_STATUS_S) {
		EAIF_STATUS_S *status = buf;
		assert(sizeof(EAIF_STATUS_S) == count);
		updateEaifResult(idx, &obj_list, status, buf_index);
		setBufReady(idx, buf_index);
	} else {
		/* do nothing */
	}

	return 0;
}

int initHdCb(MPI_WIN win_idx)
{
	int ret = initOdCtx(win_idx);
	if (ret) {
		fprintf(stderr, "[ERROR] init OD context fail\n");
		return -EINVAL;
	}

	g_app_cb = updateHdStatus;

	return 0;
}
