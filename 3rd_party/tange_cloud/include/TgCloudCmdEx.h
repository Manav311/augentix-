/**
 * @file TgCloudCmdEx.h
 * @brief P2p Commands and Structures
 */
#ifndef __TgCloudCmdExt_h__
#define __TgCloudCmdExt_h__
//#include <stdio.h>
#include "basedef.h"

/** @addtogroup p2pcmds
 * @{*/

/** @name 设备重启
  @{*/
#define TCI_CMD_DEV_REBOOT_REQ    0x8010  ///< 重启请求命令 see @ref Tcis_DevRebootReq
#define TCI_CMD_DEV_REBOOT_RESP   0x8011  ///< 重启应答 see @ref Tcis_DevRebootResp
/**@}*/

/** @name 白光灯 
  @{*/
#define TCI_CMD_GET_WHITELIGHT_REQ    0x8012  ///< 获取设备双光状态请求 see @ref Tcis_GetWhiteLightReq
#define TCI_CMD_GET_WHITELIGHT_RESP   0x8013  ///< 双光状态应答 see @ref Tcis_GetWhiteLightResp
#define TCI_CMD_SET_WHITELIGHT_REQ    0x8014  ///< 设置双光状态请求 see @ref Tcis_SetWhiteLightReq
#define TCI_CMD_SET_WHITELIGHT_RESP   0x8015  ///< 设置双光状态应答 see @ref Tcis_SetWhiteLightResp
#define TCI_CMD_GET_DOUBLELIGHT_REQ TCI_CMD_GET_WHITELIGHT_REQ     ///< 重命名
#define TCI_CMD_GET_DOUBLELIGHT_RESP TCI_CMD_GET_WHITELIGHT_RESP   ///< 重命名
#define TCI_CMD_SET_DOUBLELIGHT_REQ  TCI_CMD_SET_WHITELIGHT_REQ    ///< 重命名
#define TCI_CMD_SET_DOUBLELIGHT_RESP TCI_CMD_SET_WHITELIGHT_RESP   ///< 重命名
/**@}*/

/** @name 设备夜视 
  @{*/
#define TCI_CMD_GET_DAYNIGHT_REQ    0x8016  ///<查询夜视状态命令 see @ref Tcis_GetDayNightReq
#define TCI_CMD_GET_DAYNIGHT_RESP   0x8017  ///< 夜视状态应答命令 see @ref Tcis_GetDayNightResp
#define TCI_CMD_SET_DAYNIGHT_REQ    0x8018  ///< 设置夜视状态命令 see @ref Tcis_SetDayNightReq
#define TCI_CMD_SET_DAYNIGHT_RESP   0x8019 ///< 夜视状态设置应答命令 see @ref Tcis_SetDayNightResp
/**@}*/

/** @name 设备移动追踪 
  @{*/
#define TCI_CMD_GET_MOTION_TRACKER_REQ    0x8020  ///< 获取设备移动追踪开关状态请求 see @ref Tcis_GetMotionTrackerReq
#define TCI_CMD_GET_MOTION_TRACKER_RESP   0x8021  ///< 设备移动追踪开关状态应答 see @ref Tcis_GetMotionTrackerResp
#define TCI_CMD_SET_MOTION_TRACKER_REQ    0x8022  ///< 设置移动追踪开关状态请求 see @ref Tcis_SetMotionTrackerReq
#define TCI_CMD_SET_MOTION_TRACKER_RESP   0x8023  ///< 设置移动追踪开关状态应答 see @ref Tcis_SetMotionTrackerResp
/**@}*/

/** @name 云录像视频清晰度 
  @{*/
#define TCI_CMD_GET_CLOUD_VIDEO_QUALITY_REQ    0x8026  ///< 获取云录像清晰度请求 see @ref Tcis_GetCloudVideoQualityReq
#define TCI_CMD_GET_CLOUD_VIDEO_QUALITY_RESP   0x8027  ///< 获取云录像清晰度的应答 see @ref Tcis_GetCloudVideoQualityResp
/**@}*/

/**@name 设置/查询麦克风开关 
  @{*/
#define TCI_CMD_GET_MICROPHONE_REQ		0x8032  ///< 查询麦克风开关状态请求 see @ref Tcis_GetMicroPhoneReq -- sdk sdk内部处理此命令
#define TCI_CMD_GET_MICROPHONE_RESP		0x8033  ///< 设备麦克风开关状态的应答 see @ref Tcis_GetMicroPhoneResp
#define TCI_CMD_SET_MICROPHONE_REQ		0x8034  ///< 设置设备麦克风开关请求 see @ref Tcis_SetMicroPhoneReq
#define TCI_CMD_SET_MICROPHONE_RESP		0x8035  ///< 设置设备麦克风开关的应答
/**@}*/

/** @name 设置/查询蜂鸣器开关 
  @{*/
#define TCI_CMD_GET_BUZZER_REQ		0x8036  ///< 查询蜂鸣器状态请求 see @ref Tcis_GetBuzzerReq 
#define TCI_CMD_GET_BUZZER_RESP     0x8037  ///< 查询蜂鸣器状态应答 see @ref Tcis_GetBuzzerResp
#define TCI_CMD_SET_BUZZER_REQ		0x8038  ///< 设置蜂鸣器状态请求 see @ref Tcis_SetBuzzerReq
#define TCI_CMD_SET_BUZZER_RESP		0x8039  ///< 设置蜂鸣器状态应答 see @ref Tcis_SetBuzzerResp
/**@}*/

/** @name 温湿度控制
 * @{*/
#define TCI_CMD_SET_TEMPERATURE_THRESHOLD   0x0464   ///< 设置温度报警阈值 req: @ref Tcis_SetTemperatureThresholdReq; resp: generic
#define TCI_CMD_GET_TEMPERATURE_SETTING     0x0466   ///< 获取温度设置. req: @ref Tcis_GetTemperatureSettingReq; resp: @ref Tcis_GetTemperatureSettingResp
#define TCI_CMD_SET_HUMIDITY_THRESHOLD      0x0468   ///< 设置湿度报警阈值 req: @ref Tcis_SetHumidityThresholdReq; resp: generic
#define TCI_CMD_GET_HUMIDITY_SETTING        0x046A   ///< 获取湿度设置 req: @ref Tcis_GetHumiditySettingReq; resp: @ref Tcis_GetHumiditySettingResp
/**@}*/

/**@}*/ //end of: addtogroup p2pcmds

__BEGIN_PACKED__

/**\brief 重启请求*/
/**@ref TCI_CMD_DEV_REBOOT_REQ
 * */
typedef struct Tcis_DevRebootReq
{
    unsigned int channel; 	///< Camera Index
    unsigned char reserved[4]; ///< 0
} __PACKED__ Tcis_DevRebootReq;


/** \brief 重启请求应答.*/
/**   设备端应在发送答后延时2秒，以使APP可以收到应答 
 *   @ref TCI_CMD_DEV_REBOOT_RESP
 *   */
typedef struct Tcis_DevRebootResp
{
    unsigned int result;				///< 1 ok , !1	no ok
    unsigned char reserved[4];
} __PACKED__ Tcis_DevRebootResp;



/** \brief 白光状态请求 */
/** @ref TCI_CMD_GET_WHITELIGHT_REQ
 * */
typedef struct Tcis_GetWhiteLightReq
{
    unsigned int channel; 	///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetWhiteLightReq;
#define Tcis_GetDoubleLightReq Tcis_GetWhiteLightReq


/** \brief 白光灯设置结构体 */
/** @ref TCI_CMD_GET_WHITELIGHT_RESP
 * */
typedef struct Tcis_GetWhiteLightResp
{
    unsigned int channel; 		///< Camera Index
    unsigned int support;       ///< 0:不支持，2:支持两种模式，3:支持三种模式； --- ???
    /** 当前模式: 
     * - \c 0 - 关闭（白光不工作）
     * - \c 1 - 打开（全彩色）
     * - \c 2 - 智能模式（事件触发白光）
     * - <span style="text-decoration:line-through"> \c 3 - 定时开关(@ref Feature_DoubleLight 能力要有 "Timer" 属性)</span>
     *   <em>在定时范围内按定时规定(开), 定时范围之外按模式设定执行. 不需要一个单独的"定时开关"模式(2014/1/5)</em>
     */
    unsigned char mode;
    unsigned char reserved[3];
} __PACKED__ Tcis_GetWhiteLightResp;
#define Tcis_GetDoubleLightResp Tcis_GetWhiteLightResp


/** \brief 设置白光请求结构体 */
/** @ref TCI_CMD_SET_WHITELIGHT_REQ
 * */
typedef struct Tcis_SetWhiteLightReq {
    unsigned int channel; 	///< Camera Index
    unsigned int mode;		///< 当前模式，0-关闭（白光不工作）;1-打开（全彩色）;2-智能模式（移动侦测触发自动白光）
    unsigned char reserved[4];
} __PACKED__ Tcis_SetWhiteLightReq;
#define Tcis_SetDoubleLightReq Tcis_SetWhiteLightReq


/** \brief 设置白光状态应答结构体*/ 
/** @ref TCI_CMD_SET_WHITELIGHT_RESP
 * */
typedef struct Tcis_SetWhiteLightResp {
    unsigned int result;				///< 1 ok , !1	no ok
    unsigned char reserved[4];
} __PACKED__ Tcis_SetWhiteLightResp;
#define Tcis_SetDoubleLightResp Tcis_SetWhiteLightResp



/** \brief 查询夜视状态 */
/** @ref TCI_CMD_GET_DAYNIGHT_REQ
 * */
typedef struct Tcis_GetDayNightReq{
    unsigned int channel; 	///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetDayNightReq;


/** \brief 返回夜视状态 */
/** @ref TCI_CMD_GET_DAYNIGHT_RESP
 * */
typedef struct Tcis_GetDayNightResp {
    unsigned int channel; 		///< Camera Index
    unsigned int support;       ///< 0不支持，2支持两种模式，3支持三种模式；  --- ???
    unsigned char mode;			///< 当前模式，mode: 0- auto 1- day,2- night
    unsigned char reserved[3];
} __PACKED__ Tcis_GetDayNightResp;


/** \brief 设置夜视状态 */
/** @ref TCI_CMD_SET_DAYNIGHT_REQ
 * */
typedef struct Tcis_SetDayNightReq {
    unsigned int channel; 	///< Camera Index
    unsigned int mode;		///< 当前模式，mode: 0- auto 1- day,2- night
    unsigned char reserved[4];
} __PACKED__ Tcis_SetDayNightReq;


/** \brief 设置夜视状态应答结构体 */
/** @ref TCI_CMD_SET_DAYNIGHT_RESP
 * */
typedef struct Tcis_SetDayNightResp {
    unsigned int result;			///< 1 ok , !1	no ok
    unsigned char reserved[4];
} __PACKED__ Tcis_SetDayNightResp;


/** \brief 获取设备移动追踪开关状态请求 */
/** @ref TCI_CMD_GET_MOTION_TRACKER_REQ
 * */
typedef struct Tcis_GetMotionTrackerReq {
    unsigned int channel; 		///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetMotionTrackerReq;


/** \brief 设备移动追踪开关状态应答结构体 */
/** @ref TCI_CMD_GET_MOTION_TRACKER_RESP
 * */
typedef struct Tcis_GetMotionTrackerResp {
    unsigned int channel;       ///< Camera Index
    unsigned int support;       ///< 0不支持，非0支持    --- ??? not used in get_setting
    unsigned char mode;         ///< 当前模式，mode: 1- on, 0- off
    unsigned char reserved[3];
} __PACKED__ Tcis_GetMotionTrackerResp;


/** \brief 设置移动追踪开关状态请求结构体 */
/** @ref TCI_CMD_SET_MOTION_TRACKER_REQ
 * */
typedef struct Tcis_SetMotionTrackerReq {
    unsigned int channel;       ///< Camera Index
    unsigned int mode;          ///< 需要设置的模式，1 - on, 0 - off
    unsigned char reserved[4];  ///< 保留字段，未使用
} __PACKED__ Tcis_SetMotionTrackerReq;


/** \brief 设置移动追踪应答开关状态应答结构体 */
/** @ref TCI_CMD_SET_MOTION_TRACKER_RESP
 * */
typedef struct Tcis_SetMotionTrackerResp {
    unsigned int result;        ///< 1 ok , !1	no ok
    unsigned char reserved[4];  
} __PACKED__ Tcis_SetMotionTrackerResp;


/** \brief 获取云录像清晰度请求结构体 */
/** @ref TCI_CMD_GET_CLOUD_VIDEO_QUALITY_REQ
 * */
typedef struct Tcis_GetCloudVideoQualityReq {
    unsigned int channel; 	///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetCloudVideoQualityReq;


/** \brief 获取云录像清晰度应答结构体 */
/** @ref TCI_CMD_GET_CLOUD_VIDEO_QUALITY_RESP
 * */
typedef struct Tcis_GetCloudVideoQualityResp { 
    unsigned int channel;       ///< Camera Index
    unsigned char quality;      ///< 当前清晰度，mode: 0- 高清, 1- 标清
    unsigned char reserved[3];  ///< 保留字段，未使用
} __PACKED__ Tcis_GetCloudVideoQualityResp;

/** @name 麦克风 
 * @{*/
/** \brief 查询麦克风状态请求结构体. sdk内部处理 */
/** @ref TCI_CMD_GET_MICROPHONE_REQ
 * */
typedef struct Tcis_GetMicroPhoneReq {
    unsigned int channel; 	///< Camera Index: 0
    unsigned char reserved[4];
} __PACKED__ Tcis_GetMicroPhoneReq;


/** \brief 麦克风开关状态应答结构体 */
/** @ref TCI_CMD_GET_MICROPHONE_RESP
 * */
typedef struct Tcis_GetMicroPhoneResp {
    unsigned int channel;       ///< Camera Index
    unsigned char status;       ///< 当前当前麦克风状态，status: 0- 关, 1- 开
    unsigned char reserved[3]; 
} __PACKED__ Tcis_GetMicroPhoneResp;


/** \brief 设置麦克风开关状态请求 */
/** @ref TCI_CMD_SET_MICROPHONE_REQ
 * */
typedef struct Tcis_SetMicroPhoneReq {
    unsigned int channel;       ///< Camera Index
    unsigned int status;       ///< 设置麦克风状态，status: 0- 关, 1- 开
} __PACKED__ Tcis_SetMicroPhoneReq;

/**@}*/

/** @name 蜂鸣器
 * @{*/
/** \brief 查询蜂鸣器状态请求结构体 */
/** @ref TCI_CMD_GET_BUZZER_REQ
 * */
typedef struct Tcis_GetBuzzerReq {
    unsigned int channel; 		///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetBuzzerReq;


/** \brief 蜂鸣器状态应答结构体 */
/** @ref TCI_CMD_GET_BUZZER_RESP
 * */
typedef struct Tcis_GetBuzzerResp {
    unsigned int channel;       ///< Camera Index
    unsigned char status;       ///< 当前蜂鸣器状态，status: 0- 关; 1- 开; 2- 按定时器设定
    unsigned char reserved[3];
} __PACKED__ Tcis_GetBuzzerResp;


/** \brief 设置蜂鸣器状态请求结构体 */
/** @ref TCI_CMD_SET_BUZZER_REQ
 * */
typedef struct Tcis_SetBuzzerReq {
    unsigned int channel;       ///< Camera Index
    unsigned char status;       ///< 当前蜂鸣器状态，status: 0- 关; 1- 开; 2- 按定时器设定
    unsigned char reserved[3];
} __PACKED__ Tcis_SetBuzzerReq;


/** \brief 设置蜂鸣器状态应答参数结构体*/ 
/** @ref TCI_CMD_GET_BUZZER_RESP
 * */
typedef struct Tcis_SetBuzzerResp {
    unsigned int result;        ///< 0失败，1成功
    unsigned char reserved[4];
} __PACKED__ Tcis_SetBuzzerResp;
/**@}*/

/** @name 温湿度控制
 * @{*/
/** 获取温度设置. 
 *  @ref TCI_CMD_GET_TEMPERATURE_SETTING  =   0x0466
 */
typedef struct Tcis_GetTemperatureSettingReq {
    int sensor_id;  ///< 温度传感器标识 0~N-1
} Tcis_GetTemperatureSettingReq;

/** 传感值温度报警设置 
 *  温度单位为 0.001摄氏度。例如:
 *   - -40050 表示 -40.050 摄像度
 *   - 100123 表示 100.123 摄氏度
 */
typedef struct Tcis_GetTemperatureSettingResp {
    int sensor_id;  ///< 温度传感器标识 0~N-1
    int lo_limit;  ///< 感温能力下限。 
    int hi_limit;  ///< 感温能力上限。
    int lo_en;   ///< 低温报警使能标志. 1:enable; 0:disable
    int hi_en;   ///< 高温报警使能标志
    int lo_threshold; ///< 低温报警阈值
    int hi_threshold; ///< 高温报警阈值
} __PACKED__ Tcis_GetTemperatureSettingResp;


/** 设置温度传感器的报警阈值.
 * @ref TCI_CMD_SET_TEMPERATURE_THRESHOLD =  0x0464 \n
 *  温度单位为 0.001摄氏度。例如:
 *   - -40050 表示 -40.050 摄像度
 *   - 100123 表示 100.123 摄氏度
 */
typedef struct Tcis_SetTemperatureThresholdReq {
    int sensor_id;  ///< 温度传感器标识 0~N-1
    int lo_en;   ///< 低温报警使能标志. 1:enable; 0:disable
    int hi_en;   ///< 高温报警使能标志
    int lo_threshold; ///< 低温报警阈值
    int hi_threshold; ///< 高温报警阈值
} __PACKED__ Tcis_SetTemperatureThresholdReq;

/** 获取湿度传感器设置 
 * @ref TCI_CMD_GET_HUMIDITY_SETTING   =     0x046A \n
 */
typedef struct Tcis_GetHumiditySettingReq {
    int sensor_id;  ///< 湿度传感器标识 0~N-1
} __PACKED__ Tcis_GetHumiditySettingReq;

/** 湿度传感器设置
 *  湿度单位为 0.1%. 即 281 示湿度为 28.1%
 */
typedef struct Tcis_GetHumiditySettingResp {
    int sensor_id;        ///< 湿度传感器标识 0~N-1
    int lo_limit;         ///< 传感器检测下限
    int hi_limit;         ///< 传感器检测上限
    int lo_en;            ///< 湿度低值报警使能标志。1:enable; 0:disabled
    int hi_en;            ///< 湿度高值报警使能标志。1:enable; 0:disabled
    int lo_threshold;     ///< 低湿度报警阈值
    int hi_threshold;     ///< 高湿度报警阈值
} __PACKED__ Tcis_GetHumiditySettingResp;

/** 设置湿度传感器报警阈值.
 *  @ref TCI_CMD_SET_HUMIDITY_THRESHOLD  =    0x0468 \n
 *  湿度单位为 0.1%. 即 281 示湿度为 28.1%
 */
typedef struct Tcis_SetHumidityThresholdReq {
    int sensor_id;        ///< 湿度传感器标识 0~N-1
    int lo_en;            ///< 湿度低值报警使能标志。1:enable; 0:disabled
    int hi_en;            ///< 湿度高值报警使能标志。1:enable; 0:disabled
    int lo_threshold;     ///< 低湿度报警阈值
    int hi_threshold;     ///< 高湿度报警阈值
} __PACKED__ Tcis_SetHumidityThresholdReq;
/**@}*/

__END_PACKED__

//内部处理的命令。会告知应用层，应用层不要应答。
//参数为 channel (整数)
#define	TCI_CMD_VIDEOSTART 				0x01FF
#define	TCI_CMD_VIDEOSTOP	 			0x02FF
#define	TCI_CMD_AUDIOSTART 				0x0300
#define	TCI_CMD_AUDIOSTOP 				0x0301

#endif
