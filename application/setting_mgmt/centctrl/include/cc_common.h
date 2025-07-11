/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CC_COMMON_H_
#define CC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_cmd.h"
#include "agtx_sys_info.h"
#include "agtx_sys_feature_option.h"
#include "agtx_sys_db_info.h"
#include "agtx_product_option_list.h"
#include "agtx_video_dev_conf.h"
#include "agtx_video_view_type.h"
#include "agtx_video_strm_conf.h"
#include "agtx_video_layout_conf.h"
#include "agtx_stitch_conf.h"
#include "agtx_awb_pref.h"
#include "agtx_img_pref.h"
#include "agtx_adv_img_pref.h"
#include "agtx_dip_cal_conf.h"
#include "agtx_dip_dbc_conf.h"
#include "agtx_dip_dcc_conf.h"
#include "agtx_dip_lsc_conf.h"
#include "agtx_dip_ctrl_conf.h"
#include "agtx_dip_ae_conf.h"
#include "agtx_dip_iso_conf.h"
#include "agtx_dip_awb_conf.h"
#include "agtx_dip_pta_conf.h"
#include "agtx_dip_pca_conf.h"
#include "agtx_dip_csm_conf.h"
#include "agtx_dip_dms_conf.h"
#include "agtx_dip_shp_conf.h"
#include "agtx_dip_nr_conf.h"
#include "agtx_dip_roi_conf.h"
#include "agtx_dip_te_conf.h"
#include "agtx_dip_gamma_conf.h"
#include "agtx_dip_enh_conf.h"
#include "agtx_dip_coring_conf.h"
#include "agtx_dip_fcs_conf.h"
#include "agtx_dip_dhz_conf.h"
#include "agtx_dip_hdr_synth_conf.h"
#include "agtx_dip_stat_conf.h"
#include "agtx_dip_exp_info.h"
#include "agtx_dip_wb_info.h"
#include "agtx_dip_te_info.h"
#include "agtx_color_conf.h"
#include "agtx_audio_conf.h"
#include "agtx_voice_conf.h"
#include "agtx_siren_conf.h"
#include "agtx_product_option.h"
#include "agtx_res_option.h"
#include "agtx_venc_option.h"
#include "agtx_osd_conf.h"
#include "agtx_osd_pm_conf.h"
#include "agtx_iva.h"
#include "agtx_iaa.h"
#include "agtx_event_conf.h"
#include "agtx_gpio_conf.h"
#include "agtx_event_param.h"
#include "agtx_local_record_conf.h"
#include "agtx_video_ldc_conf.h"
#include "agtx_panorama_conf.h"
#include "agtx_panning_conf.h"
#include "agtx_surround_conf.h"
#include "agtx_anti_flicker_conf.h"
#include "agtx_pwm_conf.h"
#include "agtx_pir_conf.h"
#include "agtx_floodlight_conf.h"
#include "agtx_dip_shp_win_conf.h"
#include "agtx_dip_nr_win_conf.h"
#include "agtx_private_mode_conf.h"
#include "agtx_light_sensor_conf.h"

#define CC_SUCCESS                    (0)
#define CC_FAILURE                    (-1)
#define CC_DEPRECATED                 (-2)

#define CC_JSON_STR_BUF_SIZE 36000
#define CC_ERR_STR_BUF_SIZE           64

#define CC_MAX_CLIENT_NUM             8
#define CC_SKT_TIMEOUT_SEC            10
#define CC_SKT_TIMEOUT_USEC           0

#define CC_JSON_KEY_CLIENT_NAME       "name"
#define CC_JSON_KEY_MASTER_ID         "master_id"
#define CC_JSON_KEY_CMD_ID            "cmd_id"
#define CC_JSON_KEY_CMD_TYPE          "cmd_type"
#define CC_JSON_KEY_RET_VAL           "rval"

#define CC_SESSION_ID_MIN             0x0000000B
#define CC_SESSION_ID_MAX             0x7FFFFFFF
#define CC_SESSION_ID_INVALID         0xFFFFFFFF

#define CC_CLIENT_STATE_NONE          0x0000
#define CC_CLIENT_STATE_CONNECTING    0x0001
#define CC_CLIENT_STATE_CONNECTED     0x0002

#define CC_CMD_ID_INVALID             AGTX_CMD_INVALID

#define SYSUPD_FILE_PATH              "/usrdata/update_file"
#define RESET_FILE_PATH               "/usrdata/reset_file"

#define DBMONITOR_PID_FILE            "/var/run/dbmonitor.pid"

#define SOCKET_FILE_PATH              "/tmp/ccUnxSkt"
#define LOCK_FILE_PATH                "/tmp/ccserver.lock"
#define SOCKET_READY_FILE_PATH        "/tmp/ccserver_ready"

#define CC_UPDATE_CAL_DONE_FILE       "/usrdata/update_cal_done"

#define FACTORY_DEFAULT_DB            "/system/factory_default/ini.db"
#define USRDATA_ACTIVE_DB             "/usrdata/active_setting/ini.db"
#define TMP_ACTIVE_DB                 "/tmp/ini.db"

#define CC_NELEMS(x)                 (sizeof(x) / sizeof((x)[0]))


typedef enum {
	CC_CLIENT_ID_ONVIF,
	CC_CLIENT_ID_CGI,
	CC_CLIENT_ID_UNICORN,
	CC_CLIENT_ID_COLOR_CTRL,
	CC_CLIENT_ID_APP_HOST,
	/* Add host client before AV_MAIN */
	CC_CLIENT_ID_AV_MAIN,
	CC_CLIENT_ID_EVT_MAIN,
	CC_CLIENT_ID_APP,
	CC_CLIENT_ID_INVALID,
} CC_CLIENT_ID_E;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CC_COMMON_H_ */

