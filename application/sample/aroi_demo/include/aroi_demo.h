#ifndef AROI_DEMO_H_
#define AROI_DEMO_H_

#include "mpi_index.h"
#include "vftr_aroi.h"
#include "video_ptz.h"

int detectAroi(MPI_RECT_S *pWinRect, MPI_RECT_S *pWinRectRatio, MPI_WIN win_idx, MPI_SIZE_S *res,
               VFTR_AROI_PARAM_S *aroi_attr, VIDEO_FTR_PTZ_PARAM_S *ptz_param);

#endif /* AROI_DEMO_H_ */
