#ifndef AVFTR_H_
#define AVFTR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_base_types.h"

#include "avftr_sd.h"

#include "avftr_common.h"
#include "video_od.h"
#include "avftr_md.h"
#include "avftr_ld.h"
#include "avftr_td.h"
#include "avftr_ef.h"
#include "avftr_pd.h"
#include "video_rms.h"
#include "avftr_aroi.h"
#include "avftr_eaif.h"
#include "avftr_shd.h"
#include "avftr_pfm.h"
#include "avftr_bm.h"
#include "avftr_dk.h"
#include "avftr_fld.h"
#include "video_vdbg.h"

#define VIDEO_OD_MAX_SUPPORT_NUM 4
#define VIDEO_RMS_MAX_SUPPORT_NUM 4
#define AVFTR_PD_MAX_SUPPORT_NUM 4
#define AVFTR_TD_MAX_SUPPORT_NUM 4
#define AVFTR_AROI_MAX_SUPPORT_NUM 4
#define AVFTR_EF_MAX_SUPPORT_NUM 4
#define AVFTR_MD_MAX_SUPPORT_NUM 4
#define AVFTR_LD_MAX_SUPPORT_NUM 4
#define AVFTR_EAIF_MAX_SUPPORT_NUM 4
#define VIDEO_PFM_MAX_SUPPORT_NUM 4
#define VIDEO_BM_MAX_SUPPORT_NUM 4
#define AVFTR_DK_MAX_SUPPORT_NUM 4
#define AVFTR_FLD_MAX_SUPPORT_NUM 4
#define AVFTR_SHD_MAX_SUPPORT_NUM 4
#define VIDEO_VDBG_MAX_SUPPORT_NUM 1

#define AVFTR_SD_MAX_SUPPORT_NUM 1

typedef enum {
	AVFTR_VIDEO_NOTIFY_ON,
	AVFTR_VIDEO_NOTIFY_OFF,
} AVFTR_VIDEO_NOTIFY_E;

typedef enum {
	AVFTR_AUDIO_NOTIFY_ON,
	AVFTR_AUDIO_NOTIFY_OFF,
} AVFTR_AUDIO_NOTIFY_E;

typedef struct {
	int notify;
	int wait_time;
	time_t start_time;
} AVFTR_VIDEO_NOTIFY_CTX;

typedef struct {
	int notify;
	int wait_time;
	time_t start_time;
} AVFTR_AUDIO_NOTIFY_CTX;

typedef struct {
	AVFTR_VIDEO_BUF_INFO_S buf_info[AVFTR_VIDEO_MAX_SUPPORT_NUM];
	AVFTR_VIDEO_NOTIFY_CTX nf_ctx;
	VIDEO_OD_CTX_S od_ctx[VIDEO_OD_MAX_SUPPORT_NUM];
	AVFTR_MD_CTX_S md_ctx[AVFTR_MD_MAX_SUPPORT_NUM];
	AVFTR_AROI_CTX_S aroi_ctx[AVFTR_AROI_MAX_SUPPORT_NUM];
	AVFTR_EF_CTX_S ef_ctx[AVFTR_EF_MAX_SUPPORT_NUM];
	AVFTR_PD_CTX_S pd_ctx[AVFTR_PD_MAX_SUPPORT_NUM];
	AVFTR_EAIF_CTX_S eaif_ctx[AVFTR_EAIF_MAX_SUPPORT_NUM];
	VIDEO_RMS_CTX_S rms_ctx[VIDEO_RMS_MAX_SUPPORT_NUM];
	AVFTR_LD_CTX_S ld_ctx[AVFTR_LD_MAX_SUPPORT_NUM];
	AVFTR_TD_CTX_S td_ctx[AVFTR_TD_MAX_SUPPORT_NUM];
	AVFTR_PFM_CTX_S pfm_ctx[VIDEO_PFM_MAX_SUPPORT_NUM];
	AVFTR_BM_CTX_S bm_ctx[VIDEO_BM_MAX_SUPPORT_NUM];
	AVFTR_DK_CTX_S dk_ctx[AVFTR_DK_MAX_SUPPORT_NUM];
	AVFTR_FLD_CTX_S fld_ctx[AVFTR_FLD_MAX_SUPPORT_NUM];
	AVFTR_SHD_CTX_S shd_ctx[AVFTR_SHD_MAX_SUPPORT_NUM];
#ifdef AVFTR_VDBG
	VIDEO_VDBG_CTX_S vdbg_ctx;
#endif /* !AVFTR_VDBG */
} AVFTR_VIDEO_CTX_S;

typedef struct {
	AVFTR_AUDIO_NOTIFY_CTX nf_ctx;
	AVFTR_AUDIO_BUF_INFO_S buf_info[AVFTR_AUDIO_MAX_SUPPORT_NUM];
	AVFTR_SD_CTX_S sd_ctx[AVFTR_SD_MAX_SUPPORT_NUM];
} AVFTR_AUDIO_CTX_S;

int AVFTR_getVideoStat(MPI_WIN idx, AVFTR_VIDEO_CTX_S *res);
int AVFTR_tranVideoRes(MPI_WIN src, MPI_WIN dst, AVFTR_VIDEO_CTX_S *iva_ctx, UINT32 timestamp, char *data);
int AVFTR_tranVideoResV2(MPI_WIN src, MPI_WIN dst, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                         const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, AVFTR_VIDEO_CTX_S *iva_ctx,
                         UINT32 timestamp, char *data);
int AVFTR_runIva(MPI_WIN idx);
int AVFTR_exitIva(MPI_WIN idx);

int AVFTR_getAudioStat(MPI_DEV idx, AVFTR_AUDIO_CTX_S *res);
int AVFTR_runIaa(MPI_DEV idx);
int AVFTR_exitIaa(MPI_DEV idx);

__attribute__ ((deprecated)) int AVFTR_notifyVideo(MPI_WIN idx, int wait_time);
__attribute__ ((deprecated)) int AVFTR_notifyAudio(MPI_DEV idx, int wait_time);

#ifdef __cplusplus
}
#endif

#endif /* !AVFTR_H_ */