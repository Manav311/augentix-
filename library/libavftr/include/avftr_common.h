#ifndef AVFTR_COMMON_H_
#define AVFTR_COMMON_H_

/* Comment below line to enable JSON format */
//#define IVA_FORMAT_XML
#include <string.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_index.h"
#include "mpi_iva.h"

#define AVFTR_VIDEO_JIF_HZ 100
#define AVFTR_VIDEO_RING_BUF_SIZE (10)
#define AVFTR_VIDEO_RING_BUF_SAFE_SIZE \
	(8) /* Should always less than AVFTR_VIDEO_RING_BUF_SIZE to prevent ring buffer override*/
#define AVFTR_VIDEO_MAX_SUPPORT_NUM (4)
#define AVFTR_VIDEO_YAVG_ROI_RESOURCE_NUM (MPI_MAX_ISP_Y_AVG_CFG_NUM)

//#define AVFTR_AUDIO_FTR_JIF_HZ 100
#define AVFTR_AUDIO_RING_BUF_SIZE (10)
#define AVFTR_AUDIO_RING_BUF_SAFE_SIZE \
	(8) /* Should always less than AVFTR_AUDIO_RING_BUF_SIZE to prevent ring buffer override*/
#define AVFTR_AUDIO_MAX_SUPPORT_NUM (1)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SIGN(a) ((a) < 0 ? -1 : 1)
#define CLAMP(x, low, high)                                                                                            \
	({                                                                                                             \
		__typeof__(x) __x = (x);                                                                               \
		__typeof__(low) __low = (low);                                                                         \
		__typeof__(high) __high = (high);                                                                      \
		__x > __high ? __high : (__x < __low ? __low : __x);                                                   \
	})

#ifdef IVA_FORMAT_XML
#define print_meta(x, xml_fmt, json_fmt, args...) sprintf(x, xml_fmt, ##args)
#else /* IVA_FORMAT_JSON */
#define print_meta(x, xml_fmt, json_fmt, args...) sprintf(x, json_fmt, ##args)
#endif /* !IVA_FORMAT_XML */

typedef struct {
	UINT32 en;
	UINT32 buf_cur_time;
	UINT32 buf_cur_idx;
	MPI_WIN idx;
	UINT32 buf_time[AVFTR_VIDEO_RING_BUF_SIZE];
	UINT32 buf_ready[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_VIDEO_BUF_INFO_S;

typedef struct {
	UINT64 buf_cur_time;
	UINT32 buf_cur_idx;
	MPI_DEV idx;
	UINT64 buf_time[AVFTR_AUDIO_RING_BUF_SIZE];
	UINT32 buf_ready[AVFTR_AUDIO_RING_BUF_SIZE];
} AVFTR_AUDIO_BUF_INFO_S;

int checkMpiDevValid(const MPI_WIN idx);

int getMpiSize(const MPI_WIN idx, MPI_SIZE_S *res);

int getRoi(const MPI_WIN idx, MPI_RECT_S *roi);

void rescaleMpiRectPoint(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                         const MPI_RECT_S *dst_roi, MPI_RECT_POINT_S *roi);

UINT8 cropRect(const MPI_RECT_S *roi, const MPI_IVA_OBJ_ATTR_S *src_obj, MPI_IVA_OBJ_ATTR_S *dst_obj);

static inline void copy_obj_list(const MPI_IVA_OBJ_LIST_S *src_list, MPI_IVA_OBJ_LIST_S *dst_list)
{
	dst_list->timestamp = dst_list->timestamp;
	dst_list->obj_num = src_list->obj_num;
	memcpy(dst_list->obj, src_list->obj, dst_list->obj_num * sizeof(MPI_IVA_OBJ_ATTR_S));
}

void vftrYAvgResInc(void);

int vftrYAvgResDec(void);

#define TfindCtx(Tfeat, Tfeat_ctx, Tsupport_num)                               \
                                                                               \
	static int find##Tfeat##Ctx(MPI_WIN idx, Tfeat_ctx *ctx, int *empty)   \
	{                                                                      \
		int i = 0;                                                     \
		int find_idx = -1;                                             \
		int emp_idx = -1;                                              \
                                                                               \
		if (empty == NULL) {                                           \
			emp_idx = -2;                                          \
		} else {                                                       \
			emp_idx = -1;                                          \
		}                                                              \
                                                                               \
		for (i = 0; i < Tsupport_num; i++) {                           \
			if (find_idx == -1 && ctx[i].idx.value == idx.value) { \
				find_idx = i;                                  \
			} else if (emp_idx == -1 && !ctx[i].en) {              \
				emp_idx = i;                                   \
			}                                                      \
		}                                                              \
                                                                               \
		if (empty != NULL) {                                           \
			*empty = emp_idx;                                      \
		}                                                              \
                                                                               \
		return find_idx;                                               \
	}

#define DEBUG_MODULE

#endif /* !AVFTR_COMMON_H_ */