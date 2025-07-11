#ifndef FACEDET_DEMO_H_
#define FACEDET_DEMO_H_

#include <time.h>
#include "mpi_index.h"
#include "inf_face.h"
#include "eaif.h"

#define FACEDET_TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#define FACEDET_TOC(str, start)                                                                          \
	do {                                                                                             \
		struct timespec end;                                                                     \
		float delta_s;                                                                           \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                \
		delta_s = (float)(end.tv_sec - start.tv_sec) + (float)(end.tv_nsec - start.tv_nsec) / 1000000000; \
		printf("%s Elapsed time: %.8f (s).\n", str, delta_s);                                    \
	} while (0)


#define fd_err(fmt, args...) printf("[ERROR] %s:%d " fmt, __func__, __LINE__, ##args)
#define fd_info(fmt, args...) printf("[INFO] " fmt, ##args)

// utils function
#define FIXED_POINT_BS (8)

typedef MPI_SIZE_S FixedPointSize;
typedef EAIF_PARAM_S FACEDET_PARAM_S;

int CalcScaleFactor(int dst_width, int dst_height, const MPI_SIZE_S *src, FixedPointSize *scale_factor);
int AssignFrameInfo(int fr_width, int fr_height, MPI_VIDEO_FRAME_INFO_S *frame_info);
int FilterAndCopyScaledListWithBoundary(const MPI_SIZE_S *resoln, const FixedPointSize *scale_factor,
                               const MPI_IVA_OBJ_LIST_S *src,  int life_th, MPI_IVA_OBJ_LIST_S *dst);
int FillImageDataSnapshot(MPI_WIN idx, MPI_VIDEO_FRAME_INFO_S *frame_info);
int ReverseScaledResult(const FixedPointSize *scale_factor, const InfDetList *src, MPI_IVA_OBJ_LIST_S *dst);

int runFaceDetection(MPI_WIN win_idx, MPI_SIZE_S *chn_resoln, FACEDET_PARAM_S *facedet_param);

#endif /* !FACEDET_DEMO_H_ */