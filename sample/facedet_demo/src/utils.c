#include <errno.h>

#include "mpi_dev.h"
#include "facedet_demo.h"

static inline int clamp(int v, int l, int h)
{
	return (v < l) ? l : ((v > h) ? h : v);
}

int CalcScaleFactor(int dst_width, int dst_height, const MPI_SIZE_S *src, FixedPointSize *scale_factor)
{
	scale_factor->width = (dst_width << FIXED_POINT_BS) / src->width;
	scale_factor->height = (dst_height << FIXED_POINT_BS) / src->height;
	return 0;
}

int AssignFrameInfo(int fr_width, int fr_height, MPI_VIDEO_FRAME_INFO_S *frame_info)
{
	frame_info->width = fr_width;
	frame_info->height = fr_height;
	return 0;
}

int FilterAndCopyScaledListWithBoundary(const MPI_SIZE_S *resoln, const FixedPointSize *scale_factor,
                               const MPI_IVA_OBJ_LIST_S *src,  int life_th, MPI_IVA_OBJ_LIST_S *dst)
{
#define MIN_OBJ_SIZE (10)

	dst->timestamp = src->timestamp;
	dst->obj_num = 0;
	MPI_IVA_OBJ_ATTR_S *left = NULL;
	const MPI_IVA_OBJ_ATTR_S *right = NULL;

	for (int i = 0; i < src->obj_num; i++) {
		right = &src->obj[i];

		if (right->life < life_th)
			continue;

		MPI_RECT_POINT_S rect = right->rect;
		rect.sx = clamp(rect.sx, 0, rect.ex);
		rect.sy = clamp(rect.sy, 0, rect.ey);
		rect.ex = clamp(rect.ex, rect.sx, resoln->width);
		rect.ey = clamp(rect.ey, rect.sy, resoln->height);
		int obj_w = rect.ex - rect.sx + 1;
		int obj_h = rect.ey - rect.sy + 1;

		if (obj_w < MIN_OBJ_SIZE || obj_h < MIN_OBJ_SIZE)
			continue;

		left = &dst->obj[dst->obj_num];
		left->id = right->id;
		left->life = right->life;
		left->mv = right->mv;
		left->rect.sx = ((int)rect.sx * scale_factor->width) >> FIXED_POINT_BS;
		left->rect.ex = ((int)rect.ex * scale_factor->width) >> FIXED_POINT_BS;
		left->rect.sy = ((int)rect.sy * scale_factor->height) >> FIXED_POINT_BS;
		left->rect.ey = ((int)rect.ey * scale_factor->height) >> FIXED_POINT_BS;
		dst->obj_num++;
	}
	return 0;
}

struct timespec g_start __attribute__((unused));

int FillImageDataSnapshot(MPI_WIN idx, MPI_VIDEO_FRAME_INFO_S *frame_info)
{
#define GETWINFRAME_TRY_MAX (3)
	int err = 0;
	int repeat = 0;
	int try_time = 0;

	do {
		repeat = 0;
		//FACEDET_TIC(g_start);
		err = MPI_DEV_getWinFrame(idx, frame_info, 1200);
		//FACEDET_TOC("Snapshot time", g_start);
		if (err == -EAGAIN) {
			MPI_DEV_releaseWinFrame(idx, frame_info);
			fprintf(stderr, "[WARN] GetWinFrame Timeout retry ... #%d/%d\n", try_time + 1,
			        GETWINFRAME_TRY_MAX);
			if (try_time == GETWINFRAME_TRY_MAX) {
				fprintf(stderr, "[WARN] MPI_DEV_getWinFrame is too Busy! exit request\n");
				break;
			}
			try_time += 1;
			repeat = 1;
		} else if (err == -ENODATA) {
			fprintf(stderr, "[ERROR] No Data from MPI!\n");
			return -1;
		}
	} while (repeat);

	if (err != MPI_SUCCESS) {
		//MPI_DEV_releaseWinFrame(param->target_idx, frame_info);
		fprintf(stderr, "[ERROR] Failed to take snapshot for target channel (c=%d) (w=%d). errno %d\n", idx.chn,
		        idx.win, err);
		return -1;
	}
#undef GETWINFRAME_TRY_MAX
	return 0;
}

int ReverseScaledResult(const FixedPointSize *scale_factor, const InfDetList *src, MPI_IVA_OBJ_LIST_S *dst)
{
	dst->obj_num = src->size;
	MPI_IVA_OBJ_ATTR_S *left = NULL;
	const InfDetResult *right = NULL;

	for (int i = 0; i < src->size; i++) {
		right = &src->data[i];
		left = &dst->obj[i];
		left->id = i;
		left->rect.sx = ((int)right->rect.sx << FIXED_POINT_BS) / scale_factor->width;
		left->rect.ex = ((int)right->rect.ex << FIXED_POINT_BS) / scale_factor->width;
		left->rect.sy = ((int)right->rect.sy << FIXED_POINT_BS) / scale_factor->height;
		left->rect.ey = ((int)right->rect.ey << FIXED_POINT_BS) / scale_factor->height;
	}
	return 0;
}