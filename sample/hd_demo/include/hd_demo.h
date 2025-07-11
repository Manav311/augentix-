#ifndef HD_DEMO_H_
#define HD_DEMO_H_

#include "mpi_index.h"
#include "mpi_iva.h"
#include "eaif.h"

#define HD_ROI_MAX_NUM (5)

typedef struct {
	MPI_RECT_POINT_S rect;
	MPI_SIZE_S max;
	MPI_SIZE_S min;
} HD_ROI_FILTER_S;

typedef struct {
	int size;
	HD_ROI_FILTER_S rois[HD_ROI_MAX_NUM];
} HD_SCENE_PARAM_S;

int runHumanDetection(MPI_WIN win_idx, EAIF_PARAM_S *hd_param);
void HD_runSceneRoiFilter(const HD_SCENE_PARAM_S *param, const MPI_IVA_OBJ_LIST_S *src_list,
                          MPI_IVA_OBJ_LIST_S *dst_list);

#endif /* !HD_DEMO_H_ */