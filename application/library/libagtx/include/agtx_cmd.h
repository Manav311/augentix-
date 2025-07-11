/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_CMD_H
#define AGTX_CMD_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"


/**
 * @brief Enumeration of access priority.
 */
typedef enum {
	AGTX_PROT_NOR,  /**< Normal priority. */
	AGTX_PROT_HIGH, /**< High priority. */
} AGTX_PROT_E;

/**
 * @brief Enumeration of category.
 */
typedef enum {
	AGTX_CAT_NONE,  /**< Dummy. */
	AGTX_CAT_SYS,   /**< System. */
	AGTX_CAT_NET,   /**< Network. */
	AGTX_CAT_VIDEO, /**< Video. */
	AGTX_CAT_AUDIO, /**< Audio. */
	AGTX_CAT_EVT,   /**< Event. */
	AGTX_CAT_OSD,   /**< OSD. */
	AGTX_CAT_IVA,   /**< IVA. */
	AGTX_CAT_IAA,   /**< IAA. */
	AGTX_CAT_NUM,   /**< # of category. */
} AGTX_CAT_E;

/**
 * @brief Enumeration of item for none.
 */
typedef enum {
	AGTX_ITEM_NONE_NONE,        /**< Dummy item. */
	AGTX_ITEM_NONE_NUM,         /**< # of item for none. */
} AGTX_ITEM_NONE_E;

/**
 * @brief Enumeration of item for system.
 */
typedef enum {
	AGTX_ITEM_SYS_NONE,                /**< Dummy item. */
	AGTX_ITEM_SYS_REG_CLIENT,          /**< Register client. >*/
	AGTX_ITEM_SYS_SESS_START,          /**< System control grant. */
	AGTX_ITEM_SYS_INFO,                /**< General info. */
	AGTX_ITEM_SYS_FEATURE_OPTION,      /**< Feature option. */
	AGTX_ITEM_SYS_PRODUCT_OPTION_LIST, /**< Product option list. */
	AGTX_ITEM_SYS_DATETIME,            /**< Datetime. */
	AGTX_ITEM_SYS_DB_INFO,             /**< Database info. */
	AGTX_ITEM_SYS_NUM,                 /**< # of item for system. */
} AGTX_ITEM_SYS_E;

/**
 * @brief Enumeration of item for network.
 */
typedef enum {
	AGTX_ITEM_NET_NONE, /**< Dummy item. */
	AGTX_ITEM_NET_INTF, /**< Network interface. */
	AGTX_ITEM_NET_NUM,  /**< # of item for network. */
} AGTX_ITEM_NET_E;

/* clang-format off */
/**
 * @brief Enumeration of item for video.
 */
typedef enum {
	AGTX_ITEM_VIDEO_NONE,         /**< Dummy item. */
	AGTX_ITEM_VIDEO_BUF_CONF,     /**< Video buffer configuration. */
	AGTX_ITEM_VIDEO_DEV_CONF,     /**< Video device configuration. */
	AGTX_ITEM_VIDEO_STRM_CONF,    /**< Video stream configuration. */
	AGTX_ITEM_VIDEO_STITCH_CONF,  /**< Stitching configuration. >*/
	AGTX_ITEM_VIDEO_AWB_PREF,     /**< AWB preference. */
	AGTX_ITEM_VIDEO_IMG_PREF,     /**< Image preference. */
	AGTX_ITEM_VIDEO_ADV_IMG_PREF, /**< Advanced image preference. */
	AGTX_ITEM_VIDEO_DIP_CAL,      /**< DIP CAL parameter. */
	AGTX_ITEM_VIDEO_DIP_DBC,      /**< DIP DBC parameter. */
	AGTX_ITEM_VIDEO_DIP_DCC,      /**< DIP DCC parameter. */
	AGTX_ITEM_VIDEO_DIP_LSC,      /**< DIP LSC parameter. */
	AGTX_ITEM_VIDEO_DIP_CTRL,     /**< DIP CTRL parameter. */
	AGTX_ITEM_VIDEO_DIP_AE,       /**< DIP AE parameter. */
	AGTX_ITEM_VIDEO_DIP_AWB,      /**< DIP AWB parameter. */
	AGTX_ITEM_VIDEO_DIP_PTA,      /**< DIP PTA parameter. */
	AGTX_ITEM_VIDEO_DIP_CSM,      /**< DIP CSM parameter. */
	AGTX_ITEM_VIDEO_DIP_SHP,      /**< DIP SHP parameter. */
	AGTX_ITEM_VIDEO_DIP_NR,       /**< DIP NR parameter. */
	AGTX_ITEM_VIDEO_DIP_ROI,      /**< DIP ROI parameter. */
	AGTX_ITEM_VIDEO_DIP_TE,       /**< DIP TE parameter. */
	AGTX_ITEM_VIDEO_DIP_GAMMA,    /**< DIP GAMMA parameter. */
	AGTX_ITEM_VIDEO_DIP_ISO,      /**< DIP ISO parameter. */
	AGTX_ITEM_VIDEO_COLOR_CONF,   /**< Color configuration. */
	AGTX_ITEM_VIDEO_PRODUCT_OPTION, /**< Product option. */
	AGTX_ITEM_VIDEO_RES_OPTION,     /**< Resolution option. */
	AGTX_ITEM_VIDEO_VENC_OPTION,    /**< Video Encoder option. */
	AGTX_ITEM_VIDEO_LDC,            /**< Video LDC option. */
	AGTX_ITEM_VIDEO_LAYOUT_CONF,    /**< Video layout configuration. */
	AGTX_ITEM_VIDEO_PANORAMA,          /**< Video PANORAMA option. */
	AGTX_ITEM_VIDEO_PANNING,           /**< Video PANORAMA option. */
	AGTX_ITEM_VIDEO_SURROUND,          /**< Video PANORAMA option. */
	AGTX_ITEM_VIDEO_ANTI_FLICKER_CONF, /**< Video PANORAMA option. */
	AGTX_ITEM_VIDEO_DIP_SHP_WIN,  /**< DIP SHP window based parameter. */
	AGTX_ITEM_VIDEO_DIP_NR_WIN,   /**< DIP NR window based option. */
	AGTX_ITEM_VIDEO_PRIVATE_MODE, /**< video private mode setting. */
	AGTX_ITEM_VIDEO_DIP_ENH,      /**< DIP ENH parameter. */
	AGTX_ITEM_VIDEO_DIP_CORING,   /**< DIP CORING parameter. */
	AGTX_ITEM_VIDEO_DIP_STAT,     /**< DIP STAT parameter. */
	AGTX_ITEM_VIDEO_DIP_EXP_INFO,     /**< DIP STAT parameter. */
	AGTX_ITEM_VIDEO_DIP_WB_INFO,     /**< DIP STAT parameter. */
	AGTX_ITEM_VIDEO_DIP_FCS,      /**< DIP FCS parameter. */
	AGTX_ITEM_VIDEO_DIP_DHZ,      /**< DIP DHZ parameter. */
	AGTX_ITEM_VIDEO_DIP_HDR_SYNTH,      /**< DIP HDR_SYNTH parameter. */
	AGTX_ITEM_VIDEO_DIP_TE_INFO,     /**< DIP STAT parameter. */
	AGTX_ITEM_VIDEO_DIP_DMS,      /**< DIP DMS parameter. */
	AGTX_ITEM_VIDEO_DIP_PCA_TABLE,      /**< DIP PCA parameter. */
	AGTX_ITEM_VIDEO_VIEW_TYPE,     /**< Video window configuration, */
	AGTX_ITEM_VIDEO_NUM          /**< # of item for video. */
} AGTX_ITEM_VIDEO_E;

/* clang-format on */

/**
 * @brief Enumeration of item for audio.
 */
typedef enum {
	AGTX_ITEM_AUDIO_NONE, /**< Dummy item. */
	AGTX_ITEM_AUDIO_CONF, /**< General configuration. */
	AGTX_ITEM_VOICE_CONF, /**< Voice configuration. */
	AGTX_ITEM_SIREN_CONF, /**< Siren configuration. */
	AGTX_ITEM_AUDIO_NUM,  /**< # of item for audio. */
} AGTX_ITEM_AUDIO_E;

/**
 * @brief Enumeration of item for Event.
 */
typedef enum {
	AGTX_ITEM_EVT_NONE,        /**< Dummy item. */
	AGTX_ITEM_EVT_CONF,        /**< Event configuration. */
	AGTX_ITEM_EVT_GPIO_CONF,   /**< GPIO configuration. */
	AGTX_ITEM_EVT_PARAM,       /**< Event parameter. */
	AGTX_ITEM_LOCAL_RECORD_CONF, /**< Event record parameter. */
	AGTX_ITEM_PWM_CONF,        /**< PWM configuration. */
	AGTX_ITEM_PIR_CONF,        /**< PIR configuration. */
	AGTX_ITEM_FLOODLIGHT_CONF, /**< FLOODLIGHT configuration. */
	AGTX_ITEM_LIGHT_SENSOR_CONF, /**< Light sensor configuration. */
	AGTX_ITEM_EVT_NUM,         /**< # of item for event. */
} AGTX_ITEM_EVT_E;

/**
 * @brief Enumeration of item for OSD.
 */
typedef enum {
	AGTX_ITEM_OSD_NONE,      /**< Dummy item. */
	AGTX_ITEM_OSD_CONF,      /**< General configuration. */
	AGTX_ITEM_OSD_PM_CONF,   /**< OSD Privacy Mask. */
	AGTX_ITEM_OSD_NUM,       /**< # of item for OSD. */
} AGTX_ITEM_OSD_E;

/**
 * @brief Enumeration of item for IVA.
 */
typedef enum {
	AGTX_ITEM_IVA_NONE, /**< Dummy item. */
	AGTX_ITEM_IVA_TD_CONF, /**< Tamper detection configuration. */
	AGTX_ITEM_IVA_MD_CONF, /**< Motion detection configuration. */
	AGTX_ITEM_IVA_AROI_CONF, /**< Automatic region of interest configuration. */
	AGTX_ITEM_IVA_PD_CONF, /**< Pedestrian detection configuration. */
	AGTX_ITEM_IVA_OD_CONF, /**< Object detection configuration. */
	AGTX_ITEM_IVA_RMS_CONF, /**< Regional motion sensor configuration. */
	AGTX_ITEM_IVA_LD_CONF, /**< Light on off detection configuration. */
	AGTX_ITEM_IVA_EF_CONF, /**< Electric fence configuration. */
	AGTX_ITEM_VDBG_CONF, /**< Visualize Debugger */
	AGTX_ITEM_VIDEO_PTZ_CONF, /**< ePTZ */
	AGTX_ITEM_IVA_SHD_CONF, /**< Shaking object detection configuration */
	AGTX_ITEM_IVA_EAIF_CONF, /**< Edge AI assisted feature configuration */
	AGTX_ITEM_IVA_PFM_CONF, /**< Pet Feeding Monitor configuration */
	AGTX_ITEM_IVA_BM_CONF, /**< Baby Monitor configuration */
	AGTX_ITEM_IVA_DK_CONF, /**< Door Keeper configuration */
	AGTX_ITEM_IVA_FLD_CONF, /**< Fall detection configuration. */
	AGTX_ITEM_IVA_NUM, /**< # of item for IVA. */
} AGTX_ITEM_IVA_E;

/**
 * @brief Enumeration of item for IAA.
 */
typedef enum {
	AGTX_ITEM_IAA_NONE,    /**< Dummy item. */
	AGTX_ITEM_IAA_LSD_CONF, /**< Loud sound detection configuration. */
	AGTX_ITEM_IAA_NUM,     /**< # of item for IAA. */
} AGTX_ITEM_IAA_E;

/**
 * @brief Enumeration of command type.
 */
typedef enum {
	AGTX_CMD_TYPE_NOTIFY,
	AGTX_CMD_TYPE_CTRL,
	AGTX_CMD_TYPE_SET,
	AGTX_CMD_TYPE_GET,
	AGTX_CMD_TYPE_REPLY,
	AGTX_CMD_TYPE_NONE,
} AGTX_CMD_TYPE_E;



#define AGTX_ITEM_BIT_LEN                       (20)
#define AGTX_CAT_BIT_LEN                        (8)
#define AGTX_PROT_BIT_LEN                       (4)

#define AGTX_ITEM_BIT_SHIFT                     (0)
#define AGTX_CAT_BIT_SHIFT                      (AGTX_ITEM_BIT_SHIFT + AGTX_ITEM_BIT_LEN)
#define AGTX_PROT_BIT_SHIFT                     (AGTX_CAT_BIT_SHIFT  + AGTX_CAT_BIT_LEN )

#define AGTX_ITEM_BIT_MASK                      ((1 << AGTX_ITEM_BIT_LEN) - 1)
#define AGTX_CAT_BIT_MASK                       ((1 << AGTX_CAT_BIT_LEN ) - 1)
#define AGTX_PROT_BIT_MASK                      ((1 << AGTX_PROT_BIT_LEN) - 1)


#define AGTX_CMD_ID(p,c,i)                      ((((p) & AGTX_PROT_BIT_MASK) << AGTX_PROT_BIT_SHIFT)  | \
                                                 (((c) & AGTX_CAT_BIT_MASK ) << AGTX_CAT_BIT_SHIFT )  | \
                                                 (((i) & AGTX_ITEM_BIT_MASK) << AGTX_ITEM_BIT_SHIFT))

#define AGTX_CMD_ITEM(d)                        (((d) >> AGTX_ITEM_BIT_SHIFT) & AGTX_ITEM_BIT_MASK)
#define AGTX_CMD_CAT(d)                         (((d) >> AGTX_CAT_BIT_SHIFT ) & AGTX_CAT_BIT_MASK )
#define AGTX_CMD_PROT(d)                        (((d) >> AGTX_PROT_BIT_SHIFT) & AGTX_PROT_BIT_MASK)

/* clang-format off */

/* Invalid command */
#define AGTX_CMD_INVALID                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_NONE, AGTX_ITEM_NONE_NONE);

/* System command */
#define AGTX_CMD_REG_CLIENT                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_REG_CLIENT)
#define AGTX_CMD_SESS_START                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_SESS_START)
#define AGTX_CMD_SYS_INFO                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_INFO)
#define AGTX_CMD_SYS_FEATURE_OPTION             AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_FEATURE_OPTION)
#define AGTX_CMD_PRODUCT_OPTION_LIST            AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_PRODUCT_OPTION_LIST)
#define AGTX_CMD_DATETIME                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_DATETIME) /* Unused */
#define AGTX_CMD_SYS_DB_INFO                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_SYS, AGTX_ITEM_SYS_DB_INFO)


/* Network command */
#define AGTX_CMD_NET_INTF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_NET, AGTX_ITEM_NET_INTF) /* Unused */


/* Video command */
#define AGTX_CMD_VIDEO_BUF_CONF                 AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_BUF_CONF) /* Unused */
#define AGTX_CMD_VIDEO_DEV_CONF                 AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DEV_CONF)
#define AGTX_CMD_VIDEO_VIEW_TYPE                AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_VIEW_TYPE)
#define AGTX_CMD_VIDEO_STRM_CONF                AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_STRM_CONF)
#define AGTX_CMD_VIDEO_LAYOUT_CONF              AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_LAYOUT_CONF)
#define AGTX_CMD_STITCH_CONF                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_STITCH_CONF)
#define AGTX_CMD_AWB_PREF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_AWB_PREF)
#define AGTX_CMD_IMG_PREF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_IMG_PREF)
#define AGTX_CMD_ADV_IMG_PREF                   AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_ADV_IMG_PREF)
#define AGTX_CMD_DIP_CAL                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_CAL)
#define AGTX_CMD_DIP_DBC                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_DBC)
#define AGTX_CMD_DIP_DCC                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_DCC)
#define AGTX_CMD_DIP_LSC                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_LSC)
#define AGTX_CMD_DIP_CTRL                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_CTRL)
#define AGTX_CMD_DIP_AE                         AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_AE)
#define AGTX_CMD_DIP_ISO                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_ISO)
#define AGTX_CMD_DIP_AWB                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_AWB)
#define AGTX_CMD_DIP_PTA                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_PTA)
#define AGTX_CMD_DIP_PCA                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_PCA_TABLE)
#define AGTX_CMD_DIP_CSM                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_CSM)
#define AGTX_CMD_DIP_SHP                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_SHP)
#define AGTX_CMD_DIP_NR                         AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_NR)
#define AGTX_CMD_DIP_ROI                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_ROI)
#define AGTX_CMD_DIP_TE                         AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_TE)
#define AGTX_CMD_DIP_GAMMA                      AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_GAMMA)
#define AGTX_CMD_DIP_ENH                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_ENH)
#define AGTX_CMD_DIP_CORING                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_CORING)
#define AGTX_CMD_DIP_FCS                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_FCS)
#define AGTX_CMD_DIP_DHZ                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_DHZ)
#define AGTX_CMD_DIP_DMS                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_DMS)
#define AGTX_CMD_DIP_HDR_SYNTH                  AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_HDR_SYNTH)
#define AGTX_CMD_DIP_STAT                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_STAT)
#define AGTX_CMD_DIP_EXP_INFO                   AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_EXP_INFO)
#define AGTX_CMD_DIP_WB_INFO                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_WB_INFO)
#define AGTX_CMD_DIP_TE_INFO                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_TE_INFO)
#define AGTX_CMD_COLOR_CONF                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_COLOR_CONF)
#define AGTX_CMD_PRODUCT_OPTION                 AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_PRODUCT_OPTION)
#define AGTX_CMD_RES_OPTION                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_RES_OPTION)
#define AGTX_CMD_VENC_OPTION                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_VENC_OPTION)
#define AGTX_CMD_LDC_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_LDC)
#define AGTX_CMD_PANORAMA_CONF                  AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_PANORAMA)
#define AGTX_CMD_PANNING_CONF                   AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_PANNING)
#define AGTX_CMD_SURROUND_CONF                  AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_SURROUND)
#define AGTX_CMD_ANTI_FLICKER_CONF              AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_ANTI_FLICKER_CONF)
#define AGTX_CMD_DIP_SHP_WIN                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_SHP_WIN)
#define AGTX_CMD_DIP_NR_WIN                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_DIP_NR_WIN)
#define AGTX_CMD_PRIVATE_MODE                   AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_VIDEO, AGTX_ITEM_VIDEO_PRIVATE_MODE)

/* Audio command */
/*start cmd id : 4194305*/
#define AGTX_CMD_AUDIO_CONF                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_AUDIO, AGTX_ITEM_AUDIO_CONF)
#define AGTX_CMD_VOICE_CONF                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_AUDIO, AGTX_ITEM_VOICE_CONF)
#define AGTX_CMD_SIREN_CONF                     AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_AUDIO, AGTX_ITEM_SIREN_CONF)


/* OSD command */
#define AGTX_CMD_OSD_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_OSD, AGTX_ITEM_OSD_CONF)
#define AGTX_CMD_OSD_PM_CONF                    AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_OSD, AGTX_ITEM_OSD_PM_CONF)


/* IVA command */
#define AGTX_CMD_TD_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_TD_CONF)
#define AGTX_CMD_MD_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_MD_CONF)
#define AGTX_CMD_AROI_CONF                      AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_AROI_CONF)
#define AGTX_CMD_PD_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_PD_CONF)
#define AGTX_CMD_OD_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_OD_CONF)
#define AGTX_CMD_RMS_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_RMS_CONF)
#define AGTX_CMD_LD_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_LD_CONF)
#define AGTX_CMD_EF_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_EF_CONF)
#define AGTX_CMD_VDBG_CONF                      AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_VDBG_CONF)
#define AGTX_CMD_VIDEO_PTZ_CONF                 AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_VIDEO_PTZ_CONF)
#define AGTX_CMD_SHD_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_SHD_CONF)
#define AGTX_CMD_EAIF_CONF                      AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_EAIF_CONF)
#define AGTX_CMD_PFM_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_PFM_CONF)
#define AGTX_CMD_BM_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_BM_CONF)
#define AGTX_CMD_DK_CONF                        AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_DK_CONF)
#define AGTX_CMD_FLD_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IVA, AGTX_ITEM_IVA_FLD_CONF)

/* IAA command */
#define AGTX_CMD_LSD_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_IAA, AGTX_ITEM_IAA_LSD_CONF)

/* Event command */
#define AGTX_CMD_EVT_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_EVT_CONF)
#define AGTX_CMD_GPIO_CONF                      AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_EVT_GPIO_CONF)
#define AGTX_CMD_EVT_PARAM                      AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_EVT_PARAM)
#define AGTX_CMD_LOCAL_RECORD_CONF              AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_LOCAL_RECORD_CONF)
#define AGTX_CMD_PWM_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_PWM_CONF)
#define AGTX_CMD_PIR_CONF                       AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_PIR_CONF)
#define AGTX_CMD_FLOODLIGHT_CONF                AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_FLOODLIGHT_CONF)
#define AGTX_CMD_LIGHT_SENSOR_CONF              AGTX_CMD_ID(AGTX_PROT_NOR, AGTX_CAT_EVT, AGTX_ITEM_LIGHT_SENSOR_CONF)

/* clang-format off */

typedef struct {
	AGTX_UINT32  cid; // Command ID
	AGTX_UINT32  sid; // Unused now
	AGTX_UINT32  len; // Data length
} AGTX_MSG_HEADER_S;


/* Error code */
#define AGTX_ERR_LARGE_JSON_STR                 (2)
#define AGTX_ERR_INVALID_MASTER_ID              (3)
#define AGTX_ERR_INVALID_CMD_ID                 (4)
#define AGTX_ERR_INVALID_CMD_TYPE               (5)
#define AGTX_ERR_UNDEF_CLIENT_ID                (6)
#define AGTX_ERR_UNDEF_STATE                    (7)
#define AGTX_ERR_NOT_PERM_OPT                   (8)
#define AGTX_ERR_NON_JSON_STR                   (9)


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_CMD_H */
