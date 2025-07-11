#ifndef AGTX_DIP_CTRL_CONF_H_
#define AGTX_DIP_CTRL_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef struct {
	AGTX_INT32 is_ae_en;
	AGTX_INT32 is_awb_en;
	AGTX_INT32 is_coring_en;
	AGTX_INT32 is_csm_en;
	AGTX_INT32 is_dip_en;
	AGTX_INT32 is_dms_en;
	AGTX_INT32 is_dpc_en;
	AGTX_INT32 is_enh_en;
	AGTX_INT32 is_fcs_en;
	AGTX_INT32 is_dhz_en;
	AGTX_INT32 is_gamma_en;
	AGTX_INT32 is_iso_en;
	AGTX_INT32 is_me_en;
	AGTX_INT32 is_nr_en;
	AGTX_INT32 is_pta_en;
	AGTX_INT32 is_shp_en;
	AGTX_INT32 is_te_en;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_CTRL_CONF_S;


#endif /* AGTX_DIP_CTRL_CONF_H_ */
