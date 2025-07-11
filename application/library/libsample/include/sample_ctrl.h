#ifndef SAMPLE_CTRL_H_
#define SAMPLE_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "sample_stream.h"

#include "mpi_index.h"
#include "mpi_dev.h"
#include "mpi_enc.h"

INT32 SAMPLE_reconfigResolution(const SAMPLE_CONF_S *conf, MPI_CHN idx, const MPI_SIZE_S *res);
INT32 SAMPLE_reconfigLayout(const SAMPLE_CONF_S *conf, MPI_CHN idx, const MPI_CHN_LAYOUT_S *layout);
INT32 SAMPLE_updateFps(MPI_CHN idx, FLOAT fps);
INT32 SAMPLE_updateWindowRoi(MPI_WIN idx, const MPI_RECT_S *roi);

INT32 SAMPLE_updateMirrorAttr(UINT8 enable);
INT32 SAMPLE_updateFlipAttr(UINT8 enable);

INT32 SAMPLE_reconfigWindowViewType(const SAMPLE_CONF_S *conf, MPI_WIN idx, MPI_WIN_VIEW_TYPE_E type);
INT32 SAMPLE_updateStitchAttr(const MPI_STITCH_ATTR_S *attr);
INT32 SAMPLE_updatePanoramaAttr(const MPI_PANORAMA_ATTR_S *attr);
INT32 SAMPLE_updatePanningAttr(const MPI_PANNING_ATTR_S *attr);
INT32 SAMPLE_updateSurroundAttr(const MPI_SURROUND_ATTR_S *attr);
INT32 SAMPLE_updateLdcAttr(const MPI_LDC_ATTR_S *attr);

INT32 SAMPLE_reconfigCodec(const SAMPLE_CONF_S *conf, MPI_ECHN idx, const MPI_VENC_ATTR_S *attr);

INT32 SAMPLE_updateVbrParams(MPI_ECHN idx, const MPI_MCVC_VBR_PARAM_S *param);
INT32 SAMPLE_updateCbrParams(MPI_ECHN idx, const MPI_MCVC_CBR_PARAM_S *param);
INT32 SAMPLE_updateSbrParams(MPI_ECHN idx, const MPI_MCVC_SBR_PARAM_S *param);
INT32 SAMPLE_updateCqpParams(MPI_ECHN idx, const MPI_MCVC_CQP_PARAM_S *param);

INT32 SAMPLE_updateGopAttr(MPI_ECHN idx, UINT32 gop);
INT32 SAMPLE_updateBitRate(MPI_ECHN idx, UINT32 rate);
INT32 SAMPLE_updateSpsVuiFpsInfo(MPI_ECHN idx, UINT32 fps);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /**< SAMPLE_CTRL_H_ */
