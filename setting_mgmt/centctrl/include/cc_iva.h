/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CC_IVA_H_
#define CC_IVA_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "json.h"

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_iva_td_conf.h"
#include "agtx_iva_md_conf.h"
#include "agtx_iva_aroi_conf.h"
#include "agtx_iva_pd_conf.h"
#include "agtx_iva_ld_conf.h"
#include "agtx_iva_od_conf.h"
#include "agtx_iva_rms_conf.h"
#include "agtx_iva_ef_conf.h"
#include "agtx_vdbg_conf.h"
#include "agtx_video_ptz_conf.h"
#include "agtx_iva_shd_conf.h"
#include "agtx_iva_eaif_conf.h"
#include "agtx_iva_pfm_conf.h"
#include "agtx_iva_bm_conf.h"
#include "agtx_iva_dk_conf.h"
#include "agtx_iva_pdm_conf.h"
#include "agtx_iva_fld_conf.h"

int set_iva_td_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_TD_CONF_S *pdata);
int set_iva_md_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_MD_CONF_S *pdata);
int set_iva_aroi_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_AROI_CONF_S *pdata);
int set_iva_pd_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_PD_CONF_S *pdata);
int set_iva_ld_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_LD_CONF_S *pdata);
int set_iva_od_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_OD_CONF_S *pdata);
int set_iva_rms_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_RMS_CONF_S *pdata);
int set_iva_ef_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_EF_CONF_S *pdata);
int set_vdbg_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_VDBG_CONF_S *pdata);
int set_video_ptz_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_VIDEO_PTZ_CONF_S *pdata);
int set_iva_shd_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_SHD_CONF_S *pdata);
int set_iva_eaif_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id, AGTX_IVA_EAIF_CONF_S *pdata);
int set_iva_pfm_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id,
                     AGTX_IVA_PFM_CONF_S *pdata);
int set_iva_bm_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id,
                    AGTX_IVA_BM_CONF_S *pdata);
int set_iva_dk_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id,
                    AGTX_IVA_DK_CONF_S *pdata);
int set_iva_pdm_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id,
                     AGTX_IVA_PDM_CONF_S *pdata);
int set_iva_fld_conf(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id,
                     AGTX_IVA_FLD_CONF_S *pdata);

int get_iva_td_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_TD_CONF_S *pdata);
int get_iva_md_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_MD_CONF_S *pdata);
int get_iva_aroi_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_AROI_CONF_S *pdata);
int get_iva_pd_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_PD_CONF_S *pdata);
int get_iva_ld_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_LD_CONF_S *pdata);
int get_iva_od_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_OD_CONF_S *pdata);
int get_iva_rms_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_RMS_CONF_S *pdata);
int get_iva_ef_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_EF_CONF_S *pdata);
int get_vdbg_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_VDBG_CONF_S *pdata);
int get_video_ptz_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_VIDEO_PTZ_CONF_S *pdata);
int get_iva_shd_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_SHD_CONF_S *pdata);
int get_iva_eaif_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_EAIF_CONF_S *pdata);
int get_iva_pfm_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_PFM_CONF_S *pdata);
int get_iva_bm_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_BM_CONF_S *pdata);
int get_iva_dk_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_dk_CONF_S *pdata);
int get_iva_pdm_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_PDM_CONF_S *pdata);
int get_iva_fld_conf(struct json_object *ret_obj, AGTX_UINT32 cmd_id, int sd, AGTX_IVA_FLD_CONF_S *pdata);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CC_IVA_H_ */

