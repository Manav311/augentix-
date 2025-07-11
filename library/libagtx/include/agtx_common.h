/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_COMMON_H_
#define AGTX_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"

#define STITCH_CONF_JSON_FILE           "/usrdata/factory_default/stitching.json"
#define COLOR_DIFF_JSON_FILE            "/usrdata/factory_default/color_diff.json"
#define PANORAMA_JSON_FILE              "/usrdata/factory_default/panorama.json"
#define ANTI_FLICKER_JSON_FILE          "/usrdata/factory_default/anti_flicker.json"
#define LIGHT_SENSOR_JSON_FILE          "/usrdata/factory_default/light_sensor.json"

#define STITCH_CONF_UPD_FILE            "/usrdata/update_stitch_conf"
#define COLOR_DIFF_UPD_FILE             "/usrdata/update_color_diff"
#define PANORAMA_UPD_FILE               "/usrdata/update_panorama"
#define ANTI_FLICKER_UPD_FILE           "/usrdata/update_anti_flicker"
#define LIGHT_SENSOR_UPD_FILE           "/usrdata/update_light_sensor"

#define DB_MERGE_DONE_FILE              "/tmp/db_merge_done"

#define AGTX_MAX_PUB_POOL_NUM           (32)    /**< Max number of vodeo buffer public pool. */
#define AGTX_MAX_INPUT_PATH_NUM         (4)     /**< Max number of input path. */
#define AGTX_MAX_VIDEO_DEV_NUM          (1)     /**< Max number of video device. */
#define AGTX_MAX_VIDEO_STRM_NUM         (4)     /**< Max number of video channel. */


#define AGTX_MAX_NTP_SERVER_NUM         (1)
#define AGTX_MAX_STR_LEN                (128)

#define AGTX_MAX_PRIV_MASK_NUM          (4)
#define AGTX_MAX_EVT_NUM                (16)


#define AGTX_BAYER_CHN_NUM              (4)
#define AGTX_K_TABLE_ENTRY_NUM          (4)
#define AGTX_COLOR_CHN_NUM              (3)
#define AGTX_PTA_CURVE_ENTRY_NUM        (33)
#define AGTX_ISO_LUT_ENTRY_NUM          (11)

#define AGTX_MAX_STITCH_TABLE_NUM       (3)


/**
 * @brief Enumeration of position.
 */
typedef enum {
	AGTX_POS_NONE,  /**< Un-defined. */
	AGTX_POS_UP,    /**< Up. */
	AGTX_POS_DOWN,  /**< Down. */
	AGTX_POS_LEFT,  /**< Left. */
	AGTX_POS_RIGHT, /**< Right. */
	AGTX_POS_NUM,   /**< Position number. */
} AGTX_POS_E;

/**
 * @brief Enumeration of rotation type.
 */
typedef enum {
	AGTX_ROTATE_0,   /**< Rotate 0 degree. */
	AGTX_ROTATE_90,  /**< Rotate 90 degree. */
	AGTX_ROTATE_180, /**< Rotate 180 degree. */
	AGTX_ROTATE_270, /**< Rotate 270 degree. */
	AGTX_ROTATE_NUM, /**< The number of rotation type. */
} AGTX_ROTATE_E;

/**
 * @brief Enumeration of Bayer phase.
 */
typedef enum {
	AGTX_BAYER_G0,  /**< Bayer phase G0.*/
	AGTX_BAYER_R,   /**< Bayer phase R.*/
	AGTX_BAYER_B,   /**< Bayer phase B.*/
	AGTX_BAYER_G1,  /**< Bayer phase G1.*/
	AGTX_BAYER_NUM, /**< The number of Bayer phase. */
} AGTX_BAYER_E;

/**
 * @brief Enumeration of raw bit width.
 */
typedef enum {
	AGTX_BITS_16,  /**< 16 bits. */
	AGTX_BITS_14,  /**< 14 bits. */
	AGTX_BITS_12,  /**< 12 bits. */
	AGTX_BITS_10,  /**< 10 bits. */
	AGTX_BITS_9,   /**<  9 bits. */
	AGTX_BITS_8,   /**<  8 bits. */
	AGTX_BITS_7,   /**<  7 bits. */
	AGTX_BITS_6,   /**<  6 bits. */
	AGTX_BITS_NUM, /**< The number of bit width type. */
} AGTX_BIT_WIDTH_E;

/**
 * @brief Enumeration of device.
 */
typedef enum {
	AGTX_MODE_ONLINE,  /**< Online mode. */
	AGTX_MODE_OFFLINE, /**< Offline mode. */
	AGTX_MODE_NUM,     /**< The number of device/channel mode. */
} AGTX_MODE_E;

/**
 * @brief Enumeration of weekday.
 */
typedef enum {
	AGTX_WEEKDAY_SUN,
	AGTX_WEEKDAY_MON,
	AGTX_WEEKDAY_TUE,
	AGTX_WEEKDAY_WED,
	AGTX_WEEKDAY_THU,
	AGTX_WEEKDAY_FRI,
	AGTX_WEEKDAY_SAT,
	AGTX_WEEKDAY_NUM,
} AGTX_WEEKDAY_E;

/**
 * @brief Enumeration of video codec.
 */
typedef enum {
	AGTX_VENC_TYPE_H264,
	AGTX_VENC_TYPE_H265,
	AGTX_VENC_TYPE_MJPEG,
	AGTX_VENC_TYPE_NONE
} AGTX_VENC_TYPE_E;

/**
 * @brief Enumeration of profile.
 */
typedef enum {
	AGTX_PRFL_BASELINE,
	AGTX_PRFL_MAIN,
	AGTX_PRFL_HIGH,
	AGTX_PRFL_NONE
} AGTX_PRFL_E;

/**
 * @brief Enumeration of rate control mode.
 */
typedef enum {
	AGTX_RC_MODE_VBR,
	AGTX_RC_MODE_CBR,
	AGTX_RC_MODE_SBR,
	AGTX_RC_MODE_CQP,
	AGTX_RC_MODE_NONE
} AGTX_RC_MODE_E;

/**
 * @brief Enumeration of NR LUT type.
 */
typedef enum { AGTX_NR_LUT_TYPE_0, AGTX_NR_LUT_TYPE_1, AGTX_NR_LUT_TYPE_2, AGTX_NR_LUT_TYPE_3 } AGTX_NR_LUT_TYPE_E;

/**
* @brief Struct for a date.
*/
typedef struct {
	AGTX_UINT16  year;
	AGTX_UINT8   month;
	AGTX_UINT8   day;
} AGTX_DATE_S;

/**
* @brief Struct for a time.
*/
typedef struct {
	AGTX_UINT8  hour;
	AGTX_UINT8  min;
	AGTX_UINT8  sec;
} AGTX_TIME_S;

/**
* @brief Struct for a range.
*/
typedef struct {
	AGTX_UINT32  min; /**< Min value of the range.*/
	AGTX_UINT32  max; /**< Max value of the range.*/
} AGTX_RANGE_S;

/**
 * @brief Struct for an pixel position.
 */
typedef struct {
	AGTX_UINT16  x; /**< X coodinate of left corner corresponding to input. */
	AGTX_UINT16  y; /**< Y coodinate of left corner corresponding to input. */
} AGTX_POINT_S;

/**
 * @brief Struct for a porch.
 */
typedef struct {
	AGTX_UINT16  hor; /**< Porch in horizontal direction. */
	AGTX_UINT16  ver; /**< Porch in vertical direction. */
} AGTX_PORCH_S;

/**
 * @brief Struct for an image size.
 */
typedef struct {
	AGTX_UINT16  width;  /**< Image width. */
	AGTX_UINT16  height; /**< Image height. */
} AGTX_SIZE_S;

/**
 * @brief Struct for an rectangle.
 */
typedef struct {
	AGTX_UINT16  x;      /**< X coodinate of left corner corresponding to input. */
	AGTX_UINT16  y;      /**< Y coodinate of left corner corresponding to input. */
	AGTX_UINT16  width;  /**< Image width. */
	AGTX_UINT16  height; /**< Image height. */
} AGTX_RECT_S;

/**
 * @brief Struct for a rectangle start and end point.
 */
typedef struct {
	AGTX_INT16   sx;  /**< X coodinate of start point corresponding to input. */
	AGTX_INT16   sy;  /**< Y coodinate of start point corresponding to input. */
	AGTX_INT16   ex;  /**< X coodinate of end point corresponding to input. */
	AGTX_INT16   ey;  /**< Y coodinate of end point corresponding to input. */
} AGTX_RECT_POINT_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_COMMON_H_ */

