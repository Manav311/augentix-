#ifndef VIDE_DIP_API_H_
#define VIDE_DIP_API_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "agtx_dip_all_conf.h"
#include "mpi_index.h"

INT32 APP_DIP_setCal(MPI_PATH path_idx, const AGTX_DIP_CAL_CONF_S *cal_cfg);
INT32 APP_DIP_setDbc(MPI_PATH path_idx, const AGTX_DIP_DBC_CONF_S *dbc_cfg);
INT32 APP_DIP_setDcc(MPI_PATH path_idx, const AGTX_DIP_DCC_CONF_S *dcc_cfg);
INT32 APP_DIP_setLsc(MPI_PATH path_idx, const AGTX_DIP_LSC_CONF_S *lsc_cfg);
INT32 APP_DIP_setRoi(MPI_PATH path_idx, const AGTX_DIP_ROI_CONF_S *roi_cfg);
INT32 APP_DIP_setCtrl(MPI_PATH path_idx, const AGTX_DIP_CTRL_CONF_S *ctrl_cfg);
INT32 APP_DIP_setAe(MPI_PATH path_idx, const AGTX_DIP_AE_CONF_S *ae_cfg);
INT32 APP_DIP_setIso(MPI_PATH path_idx, const AGTX_DIP_ISO_CONF_S *iso_cfg);
INT32 APP_DIP_setAwb(MPI_PATH path_idx, const AGTX_DIP_AWB_CONF_S *awb_cfg);
INT32 APP_DIP_setCsm(MPI_PATH path_idx, const AGTX_DIP_CSM_CONF_S *csm_cfg);
INT32 APP_DIP_setPta(MPI_PATH path_idx, const AGTX_DIP_PTA_CONF_S *pta_cfg);
INT32 APP_DIP_setPca(MPI_PATH path_idx, const AGTX_DIP_PCA_CONF_S *pca_cfg);
INT32 APP_DIP_setShp(MPI_PATH path_idx, const AGTX_DIP_SHP_CONF_S *shp_cfg);
INT32 APP_DIP_setNr(MPI_PATH path_idx, const AGTX_DIP_NR_CONF_S *nr_cfg);
INT32 APP_DIP_setDms(MPI_PATH path_idx, const AGTX_DIP_DMS_CONF_S *dms_cfg);
INT32 APP_DIP_setWinShp(MPI_WIN win_idx, const AGTX_SHP_WINDOW_PARAM_S *shp_cfg);
INT32 APP_DIP_setWinNr(MPI_WIN win_idx, const AGTX_NR_WINDOW_PARAM_S *nr_cfg);
INT32 APP_DIP_setTe(MPI_PATH path_idx, const AGTX_DIP_TE_CONF_S *te_cfg);
INT32 APP_DIP_setGamma(MPI_PATH path_idx, const AGTX_DIP_GAMMA_CONF_S *gamma_cfg);
INT32 APP_DIP_setEnh(MPI_PATH path_idx, const AGTX_DIP_ENH_CONF_S *enh_cfg);
INT32 APP_DIP_setCoring(MPI_PATH path_idx, const AGTX_DIP_CORING_CONF_S *coring_cfg);
INT32 APP_DIP_setFcs(MPI_PATH path_idx, const AGTX_DIP_FCS_CONF_S *fcs_cfg);
INT32 APP_DIP_setDhz(MPI_PATH path_idx, const AGTX_DIP_DHZ_CONF_S *dhz_cfg);
INT32 APP_DIP_setHdrSynth(MPI_PATH path_idx, const AGTX_DIP_HDR_SYNTH_CONF_S *hdr_synth_cfg);
INT32 APP_DIP_setStat(MPI_PATH path_idx, const AGTX_DIP_STAT_CONF_S *stat_cfg);

INT32 APP_DIP_getCal(MPI_PATH path_idx, AGTX_DIP_CAL_CONF_S *cal_cfg);
INT32 APP_DIP_getDbc(MPI_PATH path_idx, AGTX_DIP_DBC_CONF_S *dbc_cfg);
INT32 APP_DIP_getDcc(MPI_PATH path_idx, AGTX_DIP_DCC_CONF_S *dcc_cfg);
INT32 APP_DIP_getLsc(MPI_PATH path_idx, AGTX_DIP_LSC_CONF_S *lsc_cfg);
INT32 APP_DIP_getRoi(MPI_PATH path_idx, AGTX_DIP_ROI_CONF_S *roi_cfg);
INT32 APP_DIP_getCtrl(MPI_PATH path_idx, AGTX_DIP_CTRL_CONF_S *ctrl_cfg);
INT32 APP_DIP_getAe(MPI_PATH path_idx, AGTX_DIP_AE_CONF_S *ae_cfg);
INT32 APP_DIP_getIso(MPI_PATH path_idx, AGTX_DIP_ISO_CONF_S *iso_cfg);
INT32 APP_DIP_getAwb(MPI_PATH path_idx, AGTX_DIP_AWB_CONF_S *awb_cfg);
INT32 APP_DIP_getCsm(MPI_PATH path_idx, AGTX_DIP_CSM_CONF_S *csm_cfg);
INT32 APP_DIP_getPta(MPI_PATH path_idx, AGTX_DIP_PTA_CONF_S *pta_cfg);
INT32 APP_DIP_getPca(MPI_PATH path_idx, AGTX_DIP_PCA_CONF_S *pca_cfg);
INT32 APP_DIP_getShp(MPI_PATH path_idx, AGTX_DIP_SHP_CONF_S *shp_cfg);
INT32 APP_DIP_getNr(MPI_PATH path_idx, AGTX_DIP_NR_CONF_S *nr_cfg);
INT32 APP_DIP_getDms(MPI_PATH path_idx, AGTX_DIP_DMS_CONF_S *dms_cfg);
INT32 APP_DIP_getWinShp(MPI_WIN win_idx, AGTX_SHP_WINDOW_PARAM_S *shp_cfg);
INT32 APP_DIP_getWinNr(MPI_WIN win_idx, AGTX_NR_WINDOW_PARAM_S *nr_cfg);
INT32 APP_DIP_getTe(MPI_PATH path_idx, AGTX_DIP_TE_CONF_S *te_cfg);
INT32 APP_DIP_getGamma(MPI_PATH path_idx, AGTX_DIP_GAMMA_CONF_S *gamma_cfg);
INT32 APP_DIP_getEnh(MPI_PATH path_idx, AGTX_DIP_ENH_CONF_S *enh_cfg);
INT32 APP_DIP_getCoring(MPI_PATH path_idx, AGTX_DIP_CORING_CONF_S *coring_cfg);
INT32 APP_DIP_getFcs(MPI_PATH path_idx, AGTX_DIP_FCS_CONF_S *fcs_cfg);
INT32 APP_DIP_getDhz(MPI_PATH path_idx, AGTX_DIP_DHZ_CONF_S *dhz_cfg);
INT32 APP_DIP_getHdrSynth(MPI_PATH path_idx, AGTX_DIP_HDR_SYNTH_CONF_S *hdr_synth_cfg);
INT32 APP_DIP_getStat(MPI_PATH path_idx, AGTX_DIP_STAT_CONF_S *stat_cfg);
INT32 APP_DIP_getExposureInfo(MPI_PATH path_idx, AGTX_DIP_EXPOSURE_INFO_S *exp_info);
INT32 APP_DIP_getWhiteBalanceInfo(MPI_PATH path_idx, AGTX_DIP_WHITE_BALANCE_INFO_S *wb_info);
INT32 APP_DIP_getTeInfo(MPI_PATH path_idx, AGTX_DIP_TE_INFO_S *te_info);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /**< !DIP_H_ */
