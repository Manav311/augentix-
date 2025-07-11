#ifndef TGC_AGT_CMD_UTILS_H_
#define TGC_AGT_CMD_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "agtx_cmd.h"

#include "agtx_video_strm_conf.h"
#include "agtx_img_pref.h"
#include "agtx_anti_flicker_conf.h"
#include "agtx_adv_img_pref.h"
#include "agtx_osd_conf.h"
#include "agtx_iva_od_conf.h"
#include "agtx_iva_md_conf.h"
#include "agtx_iva_pd_conf.h"
#include "agtx_video_ptz_conf.h"
#include "agtx_voice_conf.h"
#include "agtx_video_layout_conf.h"
#include "agtx_pir_conf.h"
#include "agtx_siren_conf.h"
#include "agtx_local_record_conf.h"
#include "agtx_event_param.h"
#include "agtx_color_conf.h"

#define PR_ERR(x, ...) fprintf(stderr, "%s() in %s: " x, __func__, __FILE__, ##__VA_ARGS__)
#define PR_WARN(x, ...) fprintf(stderr, "%s() in %s: " x, __func__, __FILE__, ##__VA_ARGS__)
#define PR_NOTICE(x, ...) fprintf(stderr, "%s() in %s: " x, __func__, __FILE__, ##__VA_ARGS__)
#define PR_INFO(x, ...) fprintf(stderr, "%s() in %s: " x, __func__, __FILE__, ##__VA_ARGS__)
#define PR_DEBUG(x, ...) fprintf(stderr, "%s() in %s: " x, __func__, __FILE__, ##__VA_ARGS__)

typedef struct {
	int cmd;
	void *data;
	int *update;
} AGTX_CMD_DATA_S;

typedef struct {
	int update;
	AGTX_STRM_CONF_S data;
} TGC_STRM_CONF_S;

typedef struct {
	int update;
	AGTX_IMG_PREF_S data;
} TGC_IMG_PREF_S;

typedef struct {
	int update;
	AGTX_ADV_IMG_PREF_S data;
} TGC_ADV_IMG_PREF_S;

typedef struct {
	int update;
	AGTX_OSD_CONF_S data;
} TGC_OSD_CONF_S;

typedef struct {
	int update;
	AGTX_IVA_OD_CONF_S data;
} TGC_IVA_OD_CONF_S;

typedef struct {
	int update;
	AGTX_IVA_MD_CONF_S data;
} TGC_IVA_MD_CONF_S;

typedef struct {
	int update;
	AGTX_IVA_PD_CONF_S data;
} TGC_IVA_PD_CONF_S;

typedef struct {
	int update;
	AGTX_VIDEO_PTZ_CONF_S data;
} TGC_VIDEO_PTZ_CONF_S;

typedef struct {
	int update;
	AGTX_VOICE_CONF_S data;
} TGC_VOICE_CONF_S;

typedef struct {
	int update;
	AGTX_SIREN_CONF_S data;
} TGC_SIREN_CONF_S;

typedef struct {
	int update;
	AGTX_LOCAL_RECORD_CONF_S data;
} TGC_LOCAL_RECORD_CONF_S;

typedef struct {
	int update;
	AGTX_LAYOUT_CONF_S data;
} TGC_LAYOUT_CONF_S;

typedef struct {
	int update;
	AGTX_PIR_CONF_S data;
} TGC_PIR_CONF_S;

typedef struct {
	int update;
	AGTX_EVENT_PARAM_S data;
} TGC_EVT_CONF_S;

typedef struct {
	int update;
	AGTX_ANTI_FLICKER_CONF_S data;
} TGC_ANTI_FLICKER_CONF_S;

typedef struct {
	TGC_STRM_CONF_S strm;
	TGC_LAYOUT_CONF_S layout;
	TGC_IMG_PREF_S img;
	TGC_ADV_IMG_PREF_S pref;
	TGC_OSD_CONF_S osd;
	TGC_IVA_OD_CONF_S od;
	TGC_IVA_MD_CONF_S md;
	TGC_IVA_PD_CONF_S pd;
	TGC_VIDEO_PTZ_CONF_S ptz;
	TGC_VOICE_CONF_S voice;
	TGC_SIREN_CONF_S siren;
	TGC_LOCAL_RECORD_CONF_S local_record;
	TGC_PIR_CONF_S pir;
	TGC_EVT_CONF_S led;
	TGC_ANTI_FLICKER_CONF_S flicker;
} TGC_AGTX_CONF_S;

int TGC_initCcClient(void);
void TGC_exitCcClient(void);
int TGC_getCcConfig(int cmdId, void *data);
int TGC_setCcConfig(int cmd_id, void *data, int *cc_ret);
int TGC_getConf(TGC_AGTX_CONF_S **conf);

#ifndef VOID
#define VOID void
#endif

//typedef unsigned int UINT_T;
//typedef char CHAR_T;
//typedef int BOOL_T;

#ifdef __cplusplus
}
#endif

#endif /* TGC_AGT_CMD_UTILS_H_ */
