#ifndef AGTX_IVA_OD_CONF_H_
#define AGTX_IVA_OD_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum { AGTX_IVA_OD_VERSION_OD_V4, AGTX_IVA_OD_VERSION_OD_V5 } AGTX_IVA_OD_VERSION_E;

typedef struct {
	AGTX_INT32 en_crop_outside_obj;
	AGTX_INT32 en_gmv_det;
	AGTX_INT32 en_motor;
	AGTX_INT32 en_shake_det;
	AGTX_INT32 en_stop_det;
	AGTX_INT32 enabled;
	AGTX_INT32 od_conf_th;
	AGTX_INT32 od_iou_th;
	AGTX_INT32 od_qual;
	AGTX_INT32 od_sen;
	AGTX_INT32 od_size_th;
	AGTX_INT32 od_snapshot_h;
	AGTX_INT32 od_snapshot_type;
	AGTX_INT32 od_snapshot_w;
	AGTX_INT32 od_track_refine;
	AGTX_IVA_OD_VERSION_E version;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_OD_CONF_S;


#endif /* AGTX_IVA_OD_CONF_H_ */
