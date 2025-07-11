#ifndef VIDEO_VIEW_API_H_
#define VIDEO_VIEW_API_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "agtx_panorama_conf.h"
#include "agtx_panning_conf.h"
#include "agtx_surround_conf.h"
#include "agtx_stitch_conf.h"
#include "agtx_video_ldc_conf.h"
#include "agtx_video_view_type.h"

#include "mpi_index.h"

INT32 APP_VIEW_setLdcConf(MPI_PATH path_idx, const AGTX_LDC_CONF_S *ldc_cfg);
INT32 APP_VIEW_setPanoramaConf(MPI_PATH path_idx, const AGTX_PANORAMA_CONF_S *pano_cfg);
INT32 APP_VIEW_setPanningConf(MPI_PATH path_idx, const AGTX_PANNING_CONF_S *pann_cfg);
INT32 APP_VIEW_setSurroundConf(MPI_PATH path_idx, const AGTX_SURROUND_CONF_S *surr_cfg);
INT32 APP_VIEW_setStitchConf(MPI_PATH path_idx, const AGTX_STITCH_CONF_S *stitch_cfg);
INT32 APP_VIEW_getLdcConf(MPI_PATH path_idx, AGTX_LDC_CONF_S *ldc_cfg);
INT32 APP_VIEW_getPanoramaConf(MPI_PATH path_idx, AGTX_PANORAMA_CONF_S *pano_cfg);
INT32 APP_VIEW_getPanningConf(MPI_PATH path_idx, AGTX_PANNING_CONF_S *pann_cfg);
INT32 APP_VIEW_getSurroundConf(MPI_PATH path_idx, AGTX_SURROUND_CONF_S *surr_cfg);
INT32 APP_VIEW_getStitchConf(MPI_PATH path_idx, AGTX_STITCH_CONF_S *stitch_cfg);
INT32 APP_VIEW_getWinViewType(MPI_PATH path_idx, AGTX_VIEW_TYPE_INFO_S *view_type_info);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif
