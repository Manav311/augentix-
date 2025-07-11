#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "hd_demo.h"

#include "avftr.h"

AVFTR_VIDEO_CTX_S *vftr_res_shm;

int MPI_DEV_queryChnState(MPI_CHN idx, MPI_CHN_STAT_S *stat)
{
	return 0;
};
int MPI_DEV_getChnLayout(MPI_CHN idx, MPI_CHN_LAYOUT_S *layout)
{
	return 0;
};
int MPI_IVA_setObjParam(MPI_WIN idx, const MPI_IVA_OD_PARAM_S *param)
{
	return 0;
};
int MPI_IVA_enableObjDet(MPI_WIN idx)
{
	return 0;
};
int MPI_IVA_disableObjDet(MPI_WIN idx)
{
	return 0;
};
int MPI_IVA_getBitStreamObjList(MPI_WIN idx, uint32_t timestamp, MPI_IVA_OBJ_LIST_S *obj_list)
{
	return 0;
};

int MPI_DEV_waitWin(MPI_WIN idx, UINT32 *timestamp, INT32 timeout)
{
	return 0;
}

EAIF_INSTANCE_S *EAIF_newInstance(MPI_WIN idx)
{
	return 0;
}

int EAIF_deleteInstance(EAIF_INSTANCE_S **instance)
{
	return 0;
}

int EAIF_activate(EAIF_INSTANCE_S *instance)
{
	return 0;
}
int EAIF_deactivate(EAIF_INSTANCE_S *instance)
{
	return 0;
}
int EAIF_checkParam(const EAIF_PARAM_S *param)
{
	return 0;
}
int EAIF_setParam(EAIF_INSTANCE_S *instance, const EAIF_PARAM_S *param)
{
	return 0;
}
int EAIF_getParam(EAIF_INSTANCE_S *instance, EAIF_PARAM_S *param)
{
	return 0;
}

int EAIF_testRequest(EAIF_INSTANCE_S *instance, const MPI_IVA_OBJ_LIST_S *obj_list, EAIF_STATUS_S *status)
{
	return 0;
}
int EAIF_testRequestV2(EAIF_INSTANCE_S *instance, const MPI_IVA_OBJ_LIST_S *obj_list, EAIF_STATUS_S *status)
{
	return 0;
}

EAIF_PARAM_S g_hd_param;
MPI_WIN g_win_idx;
MPI_IVA_OD_PARAM_S g_od_param;
MPI_RECT_POINT_S g_chn_bdry;
int g_hd_running;
HD_SCENE_PARAM_S g_hd_scene_param = {};

extern void HD_runSceneRoiFilter(const HD_SCENE_PARAM_S *param, const MPI_IVA_OBJ_LIST_S *src_list,
                                 MPI_IVA_OBJ_LIST_S *dst_list);
int case0_angle30()
{
	HD_SCENE_PARAM_S param = { .size = 3,
		                   .rois = {
		                           { .rect = { 0, 0, 1919, 259 },
		                             .max = { 260, 260 },
		                             .min = { 50, 70 } }, // filter big object, small object
		                           { .rect = { 0, 260, 1919, 819 },
		                             .max = { 9999, 9999 },
		                             .min = { 50, 70 } }, // filter small object
		                           { .rect = { 0, 820, 1919, 1079 },
		                             .max = { 520, 520 },
		                             .min = { 260, 260 } } // always filter
		                   } };
	MPI_IVA_OBJ_LIST_S src_list, dst_list;
	src_list.obj_num = 8;
	src_list.timestamp = 123456;
	// filter noise
	// filter by roi[0] max
	src_list.obj[0] = (MPI_IVA_OBJ_ATTR_S){ .id = 1, .rect = { -1, -2, -1 + 50, -2 + 51 }, .life = 160 };
	// filter by roi[0] min
	src_list.obj[1] = (MPI_IVA_OBJ_ATTR_S){ .id = 2, .rect = { 120, 120, 120 + 49, 120 + 51 }, .life = 160 };

	// pass on roi[0]
	src_list.obj[2] = (MPI_IVA_OBJ_ATTR_S){ .id = 3, .rect = { 220, 10, 220 + 60, 10 + 71 }, .life = 160 };
	// pass on roi[1]
	src_list.obj[3] = (MPI_IVA_OBJ_ATTR_S){ .id = 4, .rect = { 360, 450, 360 + 120, 450 + 240 }, .life = 160 };

	// filter by roi[1] min
	src_list.obj[4] = (MPI_IVA_OBJ_ATTR_S){ .id = 5, .rect = { 1280, 420, 1280 + 50, 420 + 68 }, .life = 160 };
	// filter by roi[2] max
	src_list.obj[5] = (MPI_IVA_OBJ_ATTR_S){ .id = 6, .rect = { 360, 660, 360 + 640, 660 + 519 }, .life = 160 };
	// filter by roi[2] min
	src_list.obj[6] = (MPI_IVA_OBJ_ATTR_S){ .id = 7, .rect = { 1084, 840, 1084 + 240, 840 + 240 }, .life = 160 };

	// pass on roi[2]
	src_list.obj[7] = (MPI_IVA_OBJ_ATTR_S){ .id = 8, .rect = { 840, 660, 840 + 320, 660 + 519 }, .life = 160 };

	HD_runSceneRoiFilter(&param, &src_list, &dst_list);

	//for (int i = 0; i < dst_list.obj_num; i++) {
	//	MPI_IVA_OBJ_ATTR_S *obj = &dst_list.obj[i];
	//	printf("dst: id:%d rect[%d %d %d %d]\n",
	//		obj->id, obj->rect.sx, obj->rect.sy, obj->rect.ex,obj->rect.ey);
	//}
	assert(dst_list.obj_num == 3);
	assert(!memcmp(&dst_list.obj[0], &src_list.obj[2], sizeof(MPI_IVA_OBJ_ATTR_S)));
	assert(!memcmp(&dst_list.obj[1], &src_list.obj[3], sizeof(MPI_IVA_OBJ_ATTR_S)));
	assert(!memcmp(&dst_list.obj[2], &src_list.obj[7], sizeof(MPI_IVA_OBJ_ATTR_S)));
	return 0;
}

int main()
{
	case0_angle30();
	return 0;
}