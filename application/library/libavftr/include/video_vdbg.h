#ifndef VIDEO_VDBG_H_
#define VIDEO_VDBG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_DEBUG_MODULE
#define AVFTR_VDBG
#include "video_od.h"

#define DEBUG_BUFFER_SIZE 2048
#define DEBUG_BUFFER_LIM (2048 - 128)
#define VIDEO_VDBG_DATA_SIZE 4096

typedef enum {
	VIDEO_VDBG_OD = 1,
	VIDEO_VDBG_TD = 1 << 1,
	VIDEO_VDBG_MD = 1 << 2,
	VIDEO_VDBG_EF = 1 << 3,
	VIDEO_VDBG_LD = 1 << 4,
	VIDEO_VDBG_RMS = 1 << 5,
	VIDEO_VDBG_AROI = 1 << 6,
	VIDEO_VDBG_FLD = 1 << 7,
	VIDEO_VDBG_EXPO = 1 << 8,
	VIDEO_VDBG_DEBUG = 1 << 9,
	VIDEO_VDBG_NUM = 1 << 10,
} VIDEO_VDBG_CTX_E;

typedef struct {
	UINT32 ctx;
	UINT8 en;
	char data[AVFTR_VIDEO_RING_BUF_SIZE][VIDEO_VDBG_DATA_SIZE];
	int data_len[AVFTR_VIDEO_RING_BUF_SIZE];
} VIDEO_VDBG_CTX_S;

int AVFTR_VDBG_debugInit(void);
int AVFTR_VDBG_debugExit(void);
int VIDEO_FTR_getVdbgStat(VIDEO_VDBG_CTX_S *vftr_vdbg_ctx);
int VIDEO_FTR_getVdbgRes(MPI_WIN idx, int buf_idx);
int VIDEO_FTR_transVdbgRes(void *vftr_ctx_ptr, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                           const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str, int buf_idx);
int VIDEO_FTR_enableVdbg(void);
int VIDEO_FTR_disableVdbg(void);
int VIDEO_FTR_getVdbg(UINT32 *enable_flag);
int VIDEO_FTR_setVdbg(UINT32 enable_flag);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_VDBG_H_ */
