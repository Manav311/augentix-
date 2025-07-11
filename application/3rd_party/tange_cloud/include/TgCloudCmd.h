/** \file TgCloudCmd.h
 *  \brief P2p Commands and Structures
 */

#ifndef __TgCloudCmd_h__
#define __TgCloudCmd_h__

#include "basedef.h"
/////////////////////////////////////////////////////////////////////////////////
/////////////////// Message Type Define//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/** @defgroup p2pcmds P2P命令和请求/应答
 * 命令标注约定:
 * - `Req: none`   表示命令请求没有数据
 * - `Resp: none`  表示命令没有应答 
 * - `Resp: generic` 命令响应为状态码
 * @{*/

/** @name 录像和回放
  @{*/
#define    TCI_CMD_SETRECORD_REQ              0x0310 ///< 设置SD卡录像模式命令 @ref Tcis_SetRecordReq. \n能力 @ref Feature_RecordConf
#define    TCI_CMD_SETRECORD_RESP             0x0311 ///< 设置SD卡录像模式应答命令 @ref Tcis_SetRecordResp
#define    TCI_CMD_GETRECORD_REQ              0x0312 ///< 获取SD卡录像模式. 能力 @ref Feature_RecordConf
#define    TCI_CMD_GETRECORD_RESP             0x0313 ///< 获取SD卡录像模式应答 @ref Tcis_GetRecordResp

#define    TCI_CMD_SET_TIMELAPSE_RECORD       0x0314 ///< 设置延时摄像录像模式. 
                                                     ///< Req: @ref Tcis_SetTimelapseRecordModeReq \n
                                                     ///< Resp: generic. \n
                                                     ///< Require: 能力 @ref Feature_RecordConf 包含 time-lapse
#define    TCI_CMD_GET_TIMELAPSE_RECORD       0x0316 ///< 获取延时摄像设置. 
                                                     ///< Req: @ref Tcis_GetTimelapseRecordModeReq; \n
                                                     ///< Resp: @ref Tcis_GetTimelapseRecordModeResp. \n
                                                     ///< Require: 能力 @ref Feature_RecordConf 包含 time-lapse

#define TCI_CMD_LISTEVENT_REQ 0x8000 ///< 查询SD卡录像 Req: @ref Tcis_ExListEventReq
#define    TCI_CMD_LISTEVENT_RESP		0x8001 ///< SD卡录像查询应答命令 see @ref Tcis_ExListEventResp

#define TCI_CMD_LIST_RECORDDAYS 0x800A ///< 返回SD卡上有录像的日期. req: none; resp: @ref Tcis_DaysList

#define    TCI_CMD_RECORD_PLAYCONTROL          0x031A  ///< 回放控制命令   @ref Tcis_PlayRecord
#define    TCI_CMD_RECORD_PLAYCONTROL_RESP     0x031B  ///< 回放控制应答命令 @ref Tcis_PlayRecordResp
/**@}*/


/** @name 移动侦测
 * @{*/
#define    TCI_CMD_SETMOTIONDETECT_REQ         0x0324 ///< 设置移动侦测. see @ref Tcis_SetMotionDetect. \n@ref Feature_MD-Capabilities
#define    TCI_CMD_SETMOTIONDETECT_RESP        0x0325 ///< 设置移动侦测应答 @ref Tcis_SetMotionDetectResp
#define    TCI_CMD_GETMOTIONDETECT_REQ         0x0326 ///< 获取移动侦测配置请求 @ref Tcis_GetMotionDetectReq
#define    TCI_CMD_GETMOTIONDETECT_RESP        0x0327 ///< 获取移动侦测配置应答 @ref Tcis_SetMotionDetect
#define    TCI_CMD_SET_MDAREA_STATE            0x033A ///< 设置侦测区域状态（显示|隐藏）Req: @ref Tcis_MdAreaState, Resp: generic
#define    TCI_CMD_GET_MDAREA_STATE            0x033C ///< 获取侦测区域状态 Req: @ref Tcis_GetMdAreaStateReq, Resp: Tcis_MdAreaState
/**@}*/

/** @name 布防
  @{*/
#define    TCI_CMD_SET_DEFENCE_REQ              0x0328 ///< 布防设置 参数:@ref Tcis_SetDefenceReq
#define    TCI_CMD_GET_DEFENCE_REQ              0x032A ///< 获取布防设置 请求: @ref Tcis_GetDefenceReq, 应答:@ref Tcis_SetDefence
#define    TCI_CMD_SET_EVENT_STATE              0x031C ///< 禁用或使能报警事件 请求: @ref Tcis_SetEventStateReq, 应答: generic
#define    TCI_CMD_GET_EVENT_STATE              0x031E ///< 获取事件使能状态 请求: @ref Tcis_GetEventStateReq, 应答: @ref Tcis_GetEventStateResp
/**@}*/

/** @name WiFi 相关操作
  @{*/
#define    TCI_CMD_LISTWIFIAP_REQ              0x0340  ///< WiFi列表请求 see @ref Tcis_ListWifiApReq
#define    TCI_CMD_LISTWIFIAP_RESP             0x0341  ///< WiFi 列表应答 see @ref Tcis_ListWifiApResp
#define    TCI_CMD_SETWIFI_REQ                 0x0342  ///< 设置WiFi. password 32bytes see @ref Tcis_SetWifiReq
#define    TCI_CMD_SETWIFI_RESP                0x0343  ///< 设置WiFi应答. password 32bytes see @ref Tcis_SetWifiResp
/** 获取WiFi配置  see @ref Tcis_GetWifiReq. 
 * App 通过 0x8006 命令(token为空) 修改wifi，应用层收到的是 set_wifi() 回调, \n
 * 所以没有给应用的设置命令
 */
#define TCI_CMD_GETWIFI_REQ 0x0344
#define    TCI_CMD_GETWIFI_RESP                0x0345  ///< WiFi配置   see @ref Tcis_GetWifiResp
//#define    TCI_CMD_SETWIFI_REQ_2               0x0346  ///< password 64字节. 未使用 see @ref Tcis_SetWifiReq2
//#define    TCI_CMD_GETWIFI_RESP_2              0x0347  ///< Not used
/**@}*/

/** @name 50/60Hz选择
 *@{ */
#define    TCI_CMD_SET_ENVIRONMENT_REQ         0x0360  ///< 设置电源频率 see @ref Tcis_SetEnvironmentReq
#define    TCI_CMD_SET_ENVIRONMENT_RESP        0x0361  ///< 设置电源频率应答命令 see @ref Tcis_SetEnvironmentReq
#define    TCI_CMD_GET_ENVIRONMENT_REQ         0x0362  ///< 获取电源频率 see @ref Tcis_GetEnvironmentReq
#define    TCI_CMD_GET_ENVIRONMENT_RESP        0x0363  ///< 获取电源频率应答 see @ref Tcis_GetEnvironmentResp
/**@}*/

/** @name 图像翻转
 * @{*/
#define    TCI_CMD_SET_VIDEOMODE_REQ           0x0370   ///< Set Video Flip Mode see @ref Tcis_SetVideoModeReq 
#define    TCI_CMD_SET_VIDEOMODE_RESP          0x0371   ///< 设置图像翻转模式应答 see @ref Tcis_SetVideoModeResp
#define    TCI_CMD_GET_VIDEOMODE_REQ           0x0372   ///< Get Video Flip Mode see @ref Tcis_GetVideoModeReq
#define    TCI_CMD_GET_VIDEOMODE_RESP          0x0373   ///< 获取图像翻转模式应答 see @ref Tcis_GetVideoModeResp
/**@}*/

/** @name OSD
 * @{*/
#define    TCI_CMD_SET_OSD_REQ                 0x0374   ///< 设置OSD。 参数: @ref Tcis_SetOsdReq
#define    TCI_CMD_SET_OSD_RESP                0x0375   ///< 设置OSD 应答: generic
#define    TCI_CMD_GET_OSD_REQ                 0x0376   ///< 获取OSD 设置. 参数: @ref Tcis_GetOsdReq
#define    TCI_CMD_GET_OSD_RESP                0x0377   ///< 获取OSD 应答. 参数: @ref Tcis_GetOsdResp
/**@}*/

/** @name 其它 
 * @{*/
#define TCI_CMD_SESSION_CLOSE 0x0386 ///< 关闭连接
#define    TCI_CMD_GET_RUNTIME_STATE           0x0388  ///< 获取设备端运行状态. 参数: @ref Tcis_GetRuntimeStateReq . 应答: @ref Tcis_RuntimeStateResp
/**@}*/

/** @name SD卡状态和格式化
 * @{*/
#define TCI_CMD_FORMATEXTSTORAGE_REQ 0x0380 ///< Format external storage see @ref Tcis_FormatExtStorageReq
#define TCI_CMD_FORMATEXTSTORAGE_RESP 0x0381 ///< SD卡格式化应答 see @ref Tcis_FormatExtStorageResp
#define TCI_CMD_GET_EXTERNAL_STORAGE_REQ 0x8030 ///< SD卡状态查询命令
#define    TCI_CMD_GET_EXTERNAL_STORAGE_RESP   0x8031  ///< SD卡状态应答命令 see @ref Tcis_SDCapResp
/**@}*/

/** @name 带屏IPC
 * @{*/
#define TCI_CMD_SET_SCREEN_DISPLAY 0x0382 ///< 设置屏幕显示。 req: @ref Tcis_ScreenDisplay; resp: generic
#define TCI_CMD_GET_SCREEN_DISPLAY 0x0384 ///< 获取屏幕显示设置。 req: none; resp: @ref Tcis_ScreenDisplay
/**@}*/

/** @name AI功能
 * @{*/
#define    TCI_CMD_SET_AI                      0x032C ///< 设置AI功能开关的通用命令。可代替 TCI_CMD_SET_ENABLE_BT. 参数 @ref Tcis_AiStatus
#define    TCI_CMD_GET_AI                      0x032E ///< 获取AI功能开关状态的通用命令. 可代替 TCI_CMD_GET_ENABLE_BT. 返回 @ref Tcis_AiStatus

#define    TCI_CMD_SET_SHOW_BOX                0x0348 ///< 显示人形/人脸边框. req: @ref Tcis_SetShowBoxReq; resp: generic. @see Feature_Cap-AI
#define    TCI_CMD_GET_SHOW_BOX                0x034A ///< 获取人形/人脸边框设置状态. req: @ref Tcis_GetShowBoxReq; resp: @ref Tcis_GetShowBoxResp. @see Feature_Cap-AI

#define    TCI_CMD_SET_ENABLE_BT               0x0410 ///< 设置人形跟踪开关  see @ref Tcis_SetEnableBtReq
#define    TCI_CMD_GET_ENABLE_BT               0x0412 ///< 获取人形跟踪开关状态 see @ref Tcis_GetEnableBtResp

#define    TCI_CMD_SET_ENABLE_CLOSEUP          0x033E ///< 跟踪功能开启特写   req: @ref Tcis_EnableCloseup; resp: generic. @see Feature_Cap-AI
#define    TCI_CMD_GET_ENABLE_CLOSEUP          0x034C ///< 获取跟踪功能特写设置. req: none; resp @ref Tcis_EnableCloseup.  \see Feature_Cap-AI

#define    TCI_CMD_SET_SITPOSE_SENS            0x0478 ///< 设置坐姿检测灵敏度  req: @ref Tcis_SitPoseSens; resp: generic
#define    TCI_CMD_GET_SITPOSE_SENS            0x047A ///< 获取从姿检测灵敏度  req: none; resp @ref Tcis_SitPoseSens
/**@}*/

/** @name 设备开关
 * @{*/
#define    TCI_CMD_SET_DEVICE_STATUS           0x0414 ///< 设置设备开关   see @ref Tcis_SetDeviceStatusReq
#define    TCI_CMD_GET_DEVICE_STATUS           0x0416 ///< 获取设备开关状态 see @ref Tcis_GetDeviceStatusResp
/**@}*/

/** @name 报警音(Buzzer功能的扩展)
 * @{*/
#define    TCI_CMD_SET_ALARM_BELL              0x0418 ///< 警铃开关 see @ref Tcis_SetAlarmBell
#define    TCI_CMD_GET_ALARM_BELL              0x041A ///< 取警铃开关状态 see @ref Tcis_SetAlarmBell

#define    TCI_CMD_GET_ALARMTONE_CAP           0x041C    ///< 获取报警音频文件格式信息 see @ref Tcis_GetAlarmToneCap_Resp
#define    TCI_CMD_SET_ALARMTONE               0x041E    ///< 设置报警音频 see @ref Tcis_SetAlarmTone_Req
#define    TCI_CMD_PLAY_ALARMTONE              0x0420    ///< 播放报警音
/**@}*/

/** @name 状态灯和语音提示
 * @{*/
#define    TCI_CMD_SET_LED_STATUS              0x0422    ///< 设置状态灯的模式 req: @ref Tcis_SetLedStatusReq; resp: generic. 能力: ExtInstructions包含led-status
#define    TCI_CMD_GET_LED_STATUS              0x0424    ///< 获取状态灯的模式 req: none; resp: @ref Tcis_GetLedStatusResp
#define    TCI_CMD_SET_VOICE_PROMPT_STATUS     0x0358    ///< 设置提示音开关 req: @ref Tcis_VoicePromptStatus; resp: generic. 能力: ExtInstructions包含voice-prompt
#define    TCI_CMD_GET_VOICE_PROMPT_STATUS     0x035A    ///< 获取提示音开关 req: none; resp: @ref Tcis_VoicePromptStatus
/**@}*/

/** @name 电池电量和信号强度
 * @{*/
#define    TCI_CMD_GET_BATTERY_STATUS          0x0426 ///< 获取电池电量 see @ref Tcis_GetBatteryStatusResp
#define    TCI_CMD_GET_WIFI_SIGNALLEVEL        0x0428 ///< 获取WiFi/4G信号强度 see @ref Tcis_GetWifiLevelResp
/**@}*/

/** @name 低功耗相关
 * @{*/
/** @deprecated
 * 设置被动唤醒后无操作最大工作时长 @ref Tcis_SetMaxAwakeTimeReq.
 * 用 TCI_CMD_SET_POWER_STRATEGY 代替
 */
#define TCI_CMD_SET_MAX_AWAKE_TIME 0x042A
/** @deprecated
 * 返回被动唤醒后无操作最大工作时长 @ref Tcis_GetMaxAwakeTimeResp.
 * 用 TCI_CMD_GET_POWER_STRATEGY 代替
 */
#define TCI_CMD_GET_MAX_AWAKE_TIME 0x042C
#define    TCI_CMD_SET_ENABLE_DORMANCY         0x044A ///< 设置允许或禁止休眠. 参数 @ref Tcis_DormancyState
#define    TCI_CMD_GET_ENABLE_DORMANCY         0x044C ///< 获取当前休眠开关, 返回 @ref Tcis_DormancyState
#define    TCI_CMD_SET_AWAKE_TIME              0x0470 ///< 设置主动唤醒时间。参数 @ref Tcis_SetAwakeTimeReq, 应答: general
#define    TCI_CMD_GET_AWAKE_TIME              0x0472 ///< 获取设备的主动唤醒时间设置。参数 @ref Tcis_GetAwakeTimeReq, 应答 @ref Tcis_GetAwakeTimeResp

/** 设置电池电源策略, 本指令代替 @ref TCI_CMD_SET_MAX_AWAKE_TIME. req: @ref Tcis_PowerStrategy. resp: generic */
#define TCI_CMD_SET_POWER_STRATEGY 0x048C
/** 获取电池电源策略, 本指令代替 @ref TCI_CMD_GET_MAX_AWAKE_TIME. req: none. resp: @ref Tcis_PowerStrategy */
#define TCI_CMD_GET_POWER_STRATEGY 0x048E
/**@}*/

/** @name 设备关闭计划
 * @{*/
#define    TCI_CMD_SET_CLOSE_PLAN              0x042E ///< 设置设备关闭计划 see @ref Tcis_SetClosePlanReq
#define    TCI_CMD_GET_CLOSE_PLAN              0x0430 ///< 获取设备关闭计划 see @ref Tcis_GetClosePlanResp
/**@}*/

/** @name 云台操作
 * @{*/
#define    TCI_CMD_SET_PTZ_POS                 0x0408 ///< 设置云台位置 req: @ref Tcis_SetPtzPosReq;
#define    TCI_CMD_GET_PTZ_POS                 0x040A ///< 获取云台位置 req: @ref Tcis_GetPtzPosReq; resp: @ref Tcis_GetPtzPosResp
#define    TCI_CMD_PTZ_SHORT_COMMAND           0x1000 ///< 云台短按命令 see @ref Tcis_PtzShortCmd
#define    TCI_CMD_PTZ_LONG_COMMAND            0x1001 ///< 云台长按命令 see @ref Tcis_PtzCmd . 本命令除control=TCIC_MORTOR_RESET_POSITION 外都无应答.
                                                      ///< WARNING: 这个命令为奇数，要特别处理

#define    TCI_CMD_LOCATE_IN_PIC               0x1002 ///< 图像内云台定位. req: Tcis_LocateInPic. resp: none

//#define    TCI_CMD_PTZ_RESET                   0x1004 ///< 云台复位 req: 空; resp: 要求在复位完成后给一个通过结构的应
#define    TCI_CMD_GET_PSP                     0x0452 ///< 获取设备端的预置点或预置点能力. 请求:Tcis_GetPresetPointsReq; 应答: @ref Tcis_GetPresetPointsResp
#define    TCI_CMD_SET_PSP                     0x0454 ///< 设置预置位。 请求: Tcis_SetPresetPointsReq; 应答: generic
#define    TCI_CMD_SET_WATCHPOS                0x0456 ///< 设置守望位. req: Tcis_SetWatchPosReq; resp: generic
#define    TCI_CMD_GET_WATCHPOS                0x0458 ///< 获取守望位. req: Tcis_GetWatchPosReq; resp: @ref Tcis_GetWatchPosResp
#define    TCI_CMD_SET_PTZ_TRACK               0x0474 ///< 巡航设置. req: Tcis_SetPtzTrackReq; resp: generic
#define    TCI_CMD_GET_PTZ_TRACK               0x0476 ///< 获取巡航设置. req: Tcis_GetPtzTrackReq; resp: Tcis_GetPtzTrackResp

/**@}*/

/** @name G-sensor
 * @{*/
#define TCI_CMD_SET_GSENSOR                    0x0432 ///< 设置g-sensor配置.
                                                      ///< req: @ref Tcis_SetGsensorReq; Resp: generic \n
                                                      ///< Require: @ref Feature_G-Sensor
#define TCI_CMD_GET_GSENSOR                    0x0434 ///< 获取g-sensor配置.
                                                      ///< 请求: Tcis_GetGsensorReq, 应答:@ref Tcis_GetGsensorResp
                                                      ///< Require: @ref Feature_G-Sensor

#define TCI_CMD_SET_PARKING_DET                0x0364 ///< 停车监控设置. \n Req: Tcis_ParkingDet \n Resp: generic
#define TCI_CMD_GET_PARKING_DET                0x0366 ///< 获取停车监控设置. \n Req: Tcis_GetParkingDetReq\n Resp: Tcis_ParkingDet
/**@}*/

/** @name 音量
 * @{*/
#define TCI_CMD_SET_VOLUME 					0x0436  ///< 设置设备喇叭音量 see @ref Tcis_SetVolume
#define TCI_CMD_GET_VOLUME 					0x0438  ///< 获取设备当前喇叭音量 see @ref Tcis_SetVolume
#define TCI_CMD_SET_MIC_LEVEL 				0x043A 	///< 设置拾音器灵敏度 param: @ref Tcis_SetMicLevel
#define TCI_CMD_GET_MIC_LEVEL 				0x043C  	///< 获取当前设置的拾音器灵敏度 return: @ref Tcis_SetMicLevel
/**@}*/

/** @name 画中画
 * @{*/
#define TCI_CMD_SET_PRIMARY_VIEW            0x043E ///< 设置画中画主面面通道 @ref Tcis_SetPrimaryViewReq
#define TCI_CMD_GET_PRIMARY_VIEW            0x0440 ///< 获取画中画主面面通道 @ref Tcis_GetPrimaryViewReq, @ref Tcis_GetPrimaryViewResp
/**@}*/

/** @name 报警灯
 * @{*/
#define TCI_CMD_SET_ALARMLIGHT              0x0442 ///< 设置报警灯状态. 请求@ref Tcis_AlarmLightState
#define TCI_CMD_GET_ALARMLIGHT              0x0444 ///< 获取报警灯状态. 应答@ref Tcis_AlarmLightState
/**@}*/

/** @name PIR
 * @{*/
#define TCI_CMD_SET_PIR                     0x0446  ///< 设置Pir灵敏度 @ref Tcis_SetPirSensReq
#define TCI_CMD_GET_PIR                     0x0448  ///< 获取Pir灵敏度 @ref Tcis_GetPirSensResp
/**@}*/                              
 
/** @name 门铃/门锁
 * @{*/
#define TCI_CMD_ANSWERTOCALL                0x0450  ///< 呼叫应答 @ref Tcis_AnswerToCall; resp: 见请求。应用层不用处理
#define TCI_CMD_UNLOCK                      0x045A  ///< 开门 Req: @ref Tcis_UnlockReq; resp: Tcis_UnlockResp
#define TCI_CMD_GET_LOCK_STATE              0x045C  ///< 获取门(锁状态) req: empty. resp: @ref Tcis_LockState;
/**@}*/

/** @name 网络IP配置
 * @{*/
#define TCI_CMD_SET_IPCONFIG                0x0460 ///< 设置当前活动网络IP配置。 请求:IPCONFIG. 应答:空
#define TCI_CMD_GET_IPCONFIG                0x0462 ///< 获取当前活动网络IP配置。 请求:空。 应答:IPCONFIG
/**@}*/

/** @name 台灯
 * @{*/
#define TCI_CMD_SET_LIGHT                   0x0352 ///< 设置灯光  req: @ref Tcis_LightState, resp: generic
#define TCI_CMD_GET_LIGHT                   0x0354 ///< 获取灯状态  req: @ref Tcis_GetLightReq; resp: Tcis_LightState
#define TCI_CMD_PLAY_AUDIO                  0x0356 ///< 播放语音 req: Tcis_SetAlarmTone_Req, resp: generic
#define TCI_CMD_SET_HINTTONE                0x0480 ///< 设置提示音 req: Tcis_SetHintToneReq; resp: generic. @ref Feature_ExtInstructions
#define TCI_CMD_GET_HINTTONE                0x0482 ///< 获取提示音 req: Tcis_GetHintToneReq; resp: Tcis_GetHintToneResp
/**@}*/

/** @name 行车记录仪
 * @{*/
#define TCI_CMD_SET_PARKING_MONITOR         0x0484 ///< 设置停车监控总开关. req: @ref Tcis_ParkingMonitorSwitch; resp: generic
#define TCI_CMD_GET_PARKING_MONITOR         0x0486 ///< 获取停车监控总开关. req: none; resp: @ref Tcis_ParkingMonitorSwitch
/**@}*/

/** @name 定时任务
 * @{*/
#define TCI_CMD_SET_TIMER_TASK              0x0488 ///< 设置定时任务. req: @ref Tcis_TimerTask; resp: generic
#define TCI_CMD_GET_TIMER_TASK              0x048A ///< 获取定时任务. req: @ref Tcis_GetTimerTask; resp: @ref Tcis_TimerTask
/**@}*/

/**@} end of group*/

/** doxygen 里匿名变量的占位符。定义为空，仅为了生成文档用 */
//#define __NONAME__

/////////////////////////////////////////////////////////////////////////////////
/////////////////// Type ENUM Define ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/** 视频质量表示*/
typedef enum  ENUM_QUALITY_LEVEL
{
    //TCIC_QUALITY_UNKNOWN          = 0x00,    
    TCIC_QUALITY_MAX                = 0x01,  ///< 最高质量
    TCIC_QUALITY_HIGH               = 0x02,
    TCIC_QUALITY_MIDDLE             = 0x03,
    TCIC_QUALITY_LOW                = 0x04,
    TCIC_QUALITY_MIN                = 0x05,  ///< 最低质量
} ENUM_QUALITY_LEVEL;

/** Wifi 工作模式 */
typedef enum ENUM_AP_MODE
{
    TCIC_WIFIAPMODE_NULL               = 0x00,
    TCIC_WIFIAPMODE_MANAGED            = 0x01,
    TCIC_WIFIAPMODE_ADHOC              = 0x02,
} ENUM_AP_MODE;

/** AP热点的加密方式 */
typedef enum ENUM_AP_ENCTYPE
{
    TCIC_WIFIAPENC_INVALID             = 0x00, 
    TCIC_WIFIAPENC_NONE                = 0x01, //
    TCIC_WIFIAPENC_WEP                 = 0x02, ///< WEP, for no password
    TCIC_WIFIAPENC_WPA_TKIP            = 0x03, 
    TCIC_WIFIAPENC_WPA_AES             = 0x04, 
    TCIC_WIFIAPENC_WPA2_TKIP           = 0x05, 
    TCIC_WIFIAPENC_WPA2_AES            = 0x06,

    TCIC_WIFIAPENC_WPA_PSK_TKIP  = 0x07,
    TCIC_WIFIAPENC_WPA_PSK_AES   = 0x08,
    TCIC_WIFIAPENC_WPA2_PSK_TKIP = 0x09,
    TCIC_WIFIAPENC_WPA2_PSK_AES  = 0x0A,

} ENUM_AP_ENCTYPE;

/** \brief 录像模式 */
typedef enum ENUM_RECORD_TYPE
{
    TCIC_RECORDTYPE_OFF                = 0x00, ///< 不录像
    TCIC_RECORDTYPE_ALARM              = 0x01, ///< 报警录像
    TCIC_RECORDTYPE_FULLTIME           = 0x02, ///< 全天录像
    TCIC_RECORDTYPE_AUTO               = 0x03, ///< 自动录像
} ENUM_RECORD_TYPE;

/** \brief SD卡回放控制指令 */
typedef enum ENUM_PLAYCONTROL {
	TCIC_RECORD_PLAY_PAUSE = 0x00, ///< 暂停
	TCIC_RECORD_PLAY_STOP = 0x01, ///< 停止, 并结束回放session. 结束后倍速/模式等设置消失, 下次回放要重新设置
	TCIC_RECORD_PLAY_STEPFORWARD = 0x02,
	//TCIC_RECORD_PLAY_STEPBACKWARD      = 0x03,
	TCIC_RECORD_PLAY_FORWARD =
	        0x04, ///< 快进。 Tcis_PlayRecord::Param 是回放速度: <tt> 0</tt>: 1倍速; \c 1: 2倍速...
	//TCIC_RECORD_PLAY_BACKWARD          = 0x05,
	//TCIC_RECORD_PLAY_SEEKTIME          = 0x06,
	//TCIC_RECORD_PLAY_END               = 0x07,
	TCIC_RECORD_PLAY_CONTINUE = 0x08, ///< 继续
	/** 开始回放session。这是回放过程收到的第一个命令. 
     *  \c Tcis_PlayRecord::Param: 
     *      * bit0: 发送通道选择。这个标志在sdk内部处理
     *          * 0(默认) - 独立通道(aIndex=vIndex+1). 
     *          * 1 - 声音和视频在同一个通道上发送 
     *
     *      * bit1: 播放模式
     *          * 0 - 连续播放模式. 文件播放完成后自动播放下一个文件.(默认)
     *          * 1 - 事件播放模式. 当前事件播放完后暂停发送(可以接收新的PLAY_START请求)
     *
     *  回放中可以多次发送本命令实现回放跳转.
     */
	TCIC_RECORD_PLAY_START = 0x10,
} ENUM_PLAYCONTROL;

/** 防闪烁参数 */
typedef enum ENUM_ENVIRONMENT_MODE
{
    TCIC_ENVIRONMENT_INDOOR_50HZ       = 0x00, ///< 50Hz 电源
    TCIC_ENVIRONMENT_INDOOR_60HZ       = 0x01, ///< 60Hz 电源
    TCIC_ENVIRONMENT_OUTDOOR           = 0x02,
    TCIC_ENVIRONMENT_NIGHT             = 0x03,    
} ENUM_ENVIRONMENT_MODE;

/** Video Flip Mode */
typedef enum ENUM_VIDEO_MODE
{
    TCIC_VIDEOMODE_NORMAL              = 0x00, ///< 正常
    TCIC_VIDEOMODE_FLIP                = 0x01, ///< 上下翻转
    TCIC_VIDEOMODE_MIRROR              = 0x02, ///< 左右镜像
    TCIC_VIDEOMODE_FLIP_MIRROR         = 0x03, ///< 旋转180度 
} ENUM_VIDEO_MODE;

/** PTZ Command Value */
typedef enum ENUM_PTZCMD
{
    TCIC_PTZ_STOP                      = 0, ///< 停止
    TCIC_PTZ_UP                        = 1, ///< 向上
    TCIC_PTZ_DOWN                      = 2, ///< 向下
    TCIC_PTZ_LEFT                      = 3, ///< 向左
    TCIC_PTZ_LEFT_UP                   = 4, ///< 左上
    TCIC_PTZ_LEFT_DOWN                 = 5, ///< 左下
    TCIC_PTZ_RIGHT                     = 6, ///< 向右
    TCIC_PTZ_RIGHT_UP                  = 7, ///< 右上
    TCIC_PTZ_RIGHT_DOWN                = 8, ///< 右下

    TCIC_PTZ_AUTO_SCAN                 = 9, ///< 自动线扫
    TCIC_PTZ_CALL_TRACK                = 10, ///< 调用巡航. Tcis_PtzCmd::point 为巡航轨迹号. 0为默认轨迹
    //TCIC_PTZ_SET_POINT                 = 10,
    //TCIC_PTZ_CLEAR_POINT               = 11,
    TCIC_PTZ_GOTO_POINT                = 12, ///< 调用预置位. Tcis_PtzCmd::point 为预置位编号 
#if 0
    TCIC_PTZ_SET_MODE_START            = 13,
    TCIC_PTZ_SET_MODE_STOP             = 14,
    TCIC_PTZ_MODE_RUN                  = 15,

    TCIC_PTZ_MENU_OPEN                 = 16, 
    TCIC_PTZ_MENU_EXIT                 = 17,
    TCIC_PTZ_MENU_ENTER                = 18,
#endif
    TCIC_PTZ_FLIP                      = 19,
    TCIC_PTZ_START                     = 20,

    TCIC_LENS_APERTURE_OPEN            = 21,
    TCIC_LENS_APERTURE_CLOSE           = 22,

    TCIC_LENS_ZOOM_IN                  = 23, ///< Zoom In
    TCIC_LENS_ZOOM_OUT                 = 24, ///< Zoom Out

    TCIC_LENS_FOCAL_NEAR               = 25, ///< Focus Near. @ref Feature_Cap-Zoom 带 'mfocus' 属性
    TCIC_LENS_FOCAL_FAR                = 26, ///< Focus Far. @ref Feature_Cap-Zoom 带 'mfocus' 属性
#if 0
    TCIC_AUTO_PAN_SPEED                = 27,
    TCIC_AUTO_PAN_LIMIT                = 28,
    TCIC_AUTO_PAN_START                = 29,

    TCIC_PATTERN_START                 = 30,
    TCIC_PATTERN_STOP                  = 31,
    TCIC_PATTERN_RUN                   = 32,

    TCIC_SET_AUX                       = 33,
    TCIC_CLEAR_AUX                     = 34,
#endif
    TCIC_MOTOR_RESET_POSITION          = 35, ///< 云台复位. 本指令要求在复位完成后给一个通用结构的应答

} ENUM_PTZCMD;



/////////////////////////////////////////////////////////////////////////////
///////////////////////// Message Body Define ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/** @anchor generic_err_code */
/** @name 通用错误码
  Tcis_ErrorResp::err 取值, 用于 TciSendCmdRespStatus() 的 status 参数
  @{*/
#define TCI_OK                 0 ///< 命令成功执行
#define TCI_E_INPROCESSING     1 ///< 操作进行中
#define TCI_E_CMDHDR           2 ///< 错误命令头
#define TCI_E_UNSUPPORTED_CMD  3 ///< 不支持的命令
#define TCI_E_INVALID_PARAM    4 ///< 无效参数
#define TCI_E_LACKOF_RESOURCE  5 ///< 资源(内存)不足
#define TCI_E_INTERNEL         6 ///< 设备内部错误
#define TCI_E_NOT_ALLOWED      7 ///< 操作不允许
#define TCI_E_NOTREADY 8 ///< 操作对象(例如SD卡)还未准备好，可以稍后重试
#define TCI_E_BUSY 9 ///< 例如呼叫占线
#define TCI_E_REJECT 10 ///< 用户拒绝

#define __TCI_E_MAX__ 0xFFFF
/**@}*/

__BEGIN_PACKED__

/** @name 通用状态应答命令
  @{*/
#define    TCI_CMD_ERROR_RESP                  1 ///< 通用错误应答命令字 
/** 通用错误返回结构 */
typedef struct Tcis_ErrorResp {
    unsigned int cmd; ///< 请求命令字
    unsigned int err; ///< 通用错误码 @ref TCI_OK ...
}  __PACKED__ Tcis_ErrorResp;
/**@}*/

/** @name 设备消息通知
 * @{*/
#define    TCI_CMD_RTMSG                       2 ///< 设备实时事件通知
/** 实时事件通知 */
typedef struct RTMSG_t
{
    unsigned int type;        ///< 消息类型。 @ref RTMTYPE

    unsigned int data1;	      ///< 消息类型相关数据1
    unsigned int data2;	      ///< 消息类型相关数据2
    unsigned int extra_len; ///< 更多数据紧接本结构之后
} __PACKED__ RTMSG_t;
/**@}*/

/** @name 移动侦测
 * 设置开关/区域/灵敏度...
  @{*/
/** 获取移动侦测区域请求结构 
 * @ref TCI_CMD_GETMOTIONDETECT_REQ         0x0326 ///< 获取移动侦测配置
 */
typedef struct Tcis_GetMotionDetectReq {
    unsigned int channel;     ///< Camera Index: 0~N-1
    unsigned char reserved[4];
} __PACKED__ Tcis_GetMotionDetectReq;

/** \brief 移动侦测区域的表示方式

  left/width: 区域的左上角坐标/宽度转成浮点数据，除以图片宽，再乘10000后取整.\n
  top/height: 区域的右上角坐标/高度转成浮点数据，除以图片高，再乘10001后取整.\n
  图片的左上解为坐标原点. \n
  例如 { 5000,5000,5000,5000 } 表示右下角1/4矩形.
  */
typedef struct MdZone {
    int left;
    int top;
    int width;
    int height;
}  __PACKED__ MdZone;

/** 坐标点 */
typedef struct TgPOINT {
    int x;  ///< x
    int y;  ///< y
} TgPOINT;

/** @anchor md_area_type
 * @name 侦测区域表示方式 
 *
 * Tcis_SetMotionDetect::flags 低2位为设备支持的区域表示方式
 * @{*/
#define MD_AT_RECTS            0   ///< 区域用多个矩形表示
#define MD_AT_POLYGON          1   ///< 区域用多边形表示
#define MD_AT_RECTSWITHPOLYGON 2   ///< 区域用多个矩形逼近多边形，同时后面有多边形坐标

/** 最高为1时，表示支持区域排除. 仅用于从设备返回 */
#define MD_F_SUPPORT_EXCLUDE_ZONE 0x8000
/**@}*/

/** 移动侦测多边形区域的顶点坐标. */
typedef struct MdPolygon {
    int nPoints;
    /** 点的坐标为相对位置: \n
     *   \c x: X坐标 转成浮点数据，除以图片宽，再乘10000后取整.
     *   \c y: Y坐标 转成浮点数据，除以图片高，再乘10000后取整.
     */
    TgPOINT points[1];
} __PACKED__ MdPolygon;
 

/** \brief 设置移动侦测区域请求和获取移动侦测区域应答的结构(伪).
 * 注意这个结构是变长的. \n
 * @ref TCI_CMD_SETMOTIONDETECT_REQ   =     0x0324 ///< 设置移动侦测
 *
 * @note 如果结构中 enabled=1, hasZone=0，则默认为检测全部区域
 */
typedef struct Tcis_SetMotionDetect
{
    unsigned int channel;         ///< Camera Index: 0~N-1
    unsigned short enabled;       ///< 1:enabled; 0:disabled
    unsigned short flags;         ///< 低字节为 @ref md_area_type "移动侦测区域表示标志"; 最高位为是否支持[区域排除]的标志位

    /** 下面的域根据设备能力设置。如果设备本身不支持，其值会被忽略 */
    unsigned int sensitivity;     ///< 1~5
    unsigned char hasZone;        ///< 1: nZones/zones has valid setting; 0: ignore zones
    unsigned char excludeZone;    ///< 0: 检测区域内；1: 检测区域外
    
    /** @union unionMdArea
     *   区域 */
    union unionMdArea {
        /** 检测移动的变长矩形数组 */
        struct Fake_MdZoneVLA {
            unsigned short nZones;    ///< 矩形个数
            MdZone zones[1];          ///< 矩形数组
        } __PACKED__ mz; ///< (flags&0x03)== @ref MD_AT_RECTS

        /** 检测移动的变长多边形数组 */
        struct Fake_MdPolygonVLA {
            unsigned short nPolygons;    ///< 多边形个数
            MdPolygon polygons[1];       ///< 多边形数组
        } __PACKED__ mp; ///< (flags&0x03)== @ref MD_AT_POLYGON

        struct Fake_RectPolygonVLA {
            struct Fake_MdZoneVLA mz;      ///< 矩形变长数组
            struct Fake_MdPolygonVLA mp;   ///< 多边形变长数组
        } __PACKED__ zp;           ///< (flags&0x03)== @ref MD_AT_RECTSWITHPOLYGON 的伪数据结构
    } u;
} __PACKED__ Tcis_SetMotionDetectReq;
/** \see Tcis_SetMotionDetect*/
typedef Tcis_SetMotionDetectReq Tcis_GetMotionDetectResp;


/** 设置移动侦测区域应答结构体.
 * @ref TCI_CMD_SETMOTIONDETECT_RESP    = 0x0325
 */
typedef struct Tcis_SetMotionDetectResp {
    int result;    ///<  0: success; otherwise: failed.
    unsigned char reserved[4];
} __PACKED__ Tcis_SetMotionDetectResp;

/** 设置移动侦测区域状态
 * @ref TCI_CMD_SET_MDAREA_STATE    =       0x033A 
 * 设置侦测区域状态（显示|隐藏）
 */
typedef struct Tcis_MdAreaState {
    int channel; ///< 视频通道(Camera Index) 0~N-1
    int state;   ///< 0x00:隐藏；0x01: 显示
} __PACKED__ Tcis_MdAreaState;

/** 获取移动侦测区域状态
 * @ref TCI_CMD_GET_MDAREA_STATE     =      0x033C
 */
typedef struct Tcis_GetMdAreaStateReq {
    int channel; ///< 视频通道(Camera Index) 0~N-1
    int reserved; ///< 0
} Tcis_GetMdAreaStateReq;

/** \see Tcis_MdAreaState */
typedef Tcis_MdAreaState Tcis_GetMdAreaStateResp;
/**@}*/ //end of: name 移动侦测

/** @name WiFi 相关操作
  @{*/
/** \brief 获取WiFi列表请求命名参数结构体.
  @ref TCI_CMD_LISTWIFIAP_REQ        = 0x0340
  */
typedef struct Tcis_ListWifiApReq {
    unsigned char reserved[4];
} __PACKED__ Tcis_ListWifiApReq;
/**\struct SWifiAp
  WiFi信息结构体.
 */
typedef struct SWifiAp {
    char ssid[32];      ///< WiFi ssid
    char mode;          ///< refer to @ref ENUM_AP_MODE
    char enctype;       ///< refer to @ref ENUM_AP_ENCTYPE
    char signal;        ///< signal intensity 0--100%

    /** - 0 : invalid ssid or disconnected
        - 1 : connected with default gateway
        - 2 : unmatched password
        - 3 : weak signal and connected
     */
    char status;        
} __PACKED__  SWifiAp;

/** 获取WiFi列表应答参数结构体.
  @ref TCI_CMD_LISTWIFIAP_RESP        = 0x0341
  */
typedef struct Tcis_ListWifiApResp {
    unsigned int number;    ///< MAX number: 1024(IOCtrl packet size) / 36(bytes) = 28
    SWifiAp stWifiAp[1];    ///< wifi信息 @ref SWifiAp
} __PACKED__ Tcis_ListWifiApResp;

#if 1
/** 设置设备WiFi请求命令参数结构体.
  @ref TCI_CMD_SETWIFI_REQ            = 0x0342
  */
typedef struct Tcis_SetWifiReq {
    unsigned char ssid[32];             ///< WiFi ssid
    unsigned char password[32];         ///< if exist, WiFi password
    unsigned char mode;                 ///< refer to @ref ENUM_AP_MODE
    unsigned char enctype;              ///< refer to @ref ENUM_AP_ENCTYPE
    unsigned char reserved[10];         ///< 保留字段，未使用
} __PACKED__ Tcis_SetWifiReq;

/** 设置WiFi请求2.
 * 当请求为TCI_CMD_SETWIFI_REQ_2时的命令参数结构体 \n
 @ref TCI_CMD_SETWIFI_REQ_2        = 0x0346
 */
typedef struct Tcis_SetWifiReq2 {
    unsigned char ssid[32];        	///< WiFi ssid
    unsigned char password[64];    	///< if exist, WiFi password
    unsigned char mode;            	///< refer to @ref ENUM_AP_MODE
    unsigned char enctype;        	///< refer to @ref ENUM_AP_ENCTYPE
    unsigned char reserved[10];
} __PACKED__ Tcis_SetWifiReq2;

/** 设置WiFi应答命令的参数结构体.
  @ref  TCI_CMD_SETWIFI_RESP            = 0x0343,
  */
typedef struct Tcis_SetWifiResp {
    int result; 				///< 0: wifi connected; 1: failed to connect
    unsigned char reserved[4];
} __PACKED__ Tcis_SetWifiResp;
#endif

/** 获取当前连接的WiFi信息请求命的求参数结构.
  @ref TCI_CMD_GETWIFI_REQ            = 0x0344
  */
typedef struct Tcis_GetWifiReq {
    unsigned char reserved[4];
} __PACKED__ Tcis_GetWifiReq;

/** @struct Tcis_GetWifiResp
  获取设备当前连接WiFi信息结构体.
  @ref TCI_CMD_GETWIFI_RESP            = 0x0345 \n
  if no wifi connected, members of Tcis_GetWifiResp are all 0
  */
typedef struct Tcis_GetWifiResp {
    unsigned char ssid[32];        	///<  WiFi ssid
    unsigned char password[32]; 	///< WiFi password if not empty
    unsigned char mode;            	///< refer to @ref ENUM_AP_MODE
    unsigned char enctype;        	///< refer to @ref ENUM_AP_ENCTYPE
    unsigned char signal;        	///< signal intensity 0--100%
    unsigned char status;        	///< refer to SWifiAp::status
} __PACKED__ Tcis_GetWifiResp;

/** 获取当前连接WiFi应答命令的参数结构体.
changed: WI-FI Password 32bit Change to 64bit  \n
@ref TCI_CMD_GETWIFI_RESP_2    = 0x0347
*/
typedef struct Tcis_GetWifiResp2 {
    unsigned char ssid[32];     ///< WiFi ssid
    unsigned char password[64]; ///< WiFi password if not empty
    unsigned char mode;    		///< refer to @ref ENUM_AP_MODE
    unsigned char enctype; 		///< refer to @ref ENUM_AP_ENCTYPE
    unsigned char signal;  		///< signal intensity 0--100%
    unsigned char status;  		///< refer to @ref SWifiAp::status
} __PACKED__ Tcis_GetWifiResp2;
/**@}*/

/** @name 录像和回放
  @{*/
/** @struct Tcis_SetRecord
  SD卡录像模式结构体.
  @ref TCI_CMD_SETRECORD_REQ \n
  @ref TCI_CMD_GETRECORD_RESP
 */
typedef struct Tcis_SetRecord {
    unsigned int channel;       ///< Camera Index:0~N-1
    unsigned int recordType;    ///< Refer to @ref ENUM_RECORD_TYPE
    unsigned char recordStream; ///< SD卡录像清晰度:0-高清，1-标清. 要求 @ref Feature_RecordConf = "res"
    unsigned char flags;        ///< not used (2021-9-18)
    unsigned char reserved[2]; ///< 0
} __PACKED__ Tcis_SetRecord;
/** \see Tcis_SetRecord */
typedef Tcis_SetRecord Tcis_SetRecordReq;
/** \see Tcis_SetRecord */
typedef Tcis_SetRecord Tcis_GetRecordResp;

/** @struct Tcis_SetRecordResp
  设置SD卡录像模式应答结构体
  @ref TCI_CMD_SETRECORD_RESP        = 0x0311,
*/
typedef struct Tcis_SetRecordResp
{
    int result;    				///<  0: success; otherwise: failed.
    unsigned char reserved[4];
} __PACKED__ Tcis_SetRecordResp;

/** @struct Tcis_SetTimelapseRecordModeReq
 * 设置延时摄像设置.
    TCI_CMD_SET_TIMELAPSE_RECORD   =   0x0314 \n
 */
typedef struct Tcis_SetTimelapseRecordModeReq {
    int channel; ///< Camera Index: 0~N-1
    int when;    ///< 1:目前固定取值1, 表示停车状态
    int status;  ///< 延时摄影状态： 0-disable; 1-enable。
    int interval; ///< 录像间隔.单位:秒
} __PACKED__ Tcis_SetTimelapseRecordModeReq;

/** @struct Tcis_GetTimelapseRecordModeReq
 * 获取延时摄像设置.
    TCI_CMD_GET_TIMELAPSE_RECORD   =   0x0316
 */
typedef struct Tcis_GetTimelapseRecordModeReq {
    int channel;  ///< Camera Index: 0-N-1
    int when;    ///< 1:目前固定取值1, 表示停车状态
} __PACKED__ Tcis_GetTimelapseRecordModeReq;

/** \see Tcis_SetTimelapseRecordModeReq */
typedef struct Tcis_SetTimelapseRecordModeReq Tcis_GetTimelapseRecordModeResp;

/** 日期表示 */
typedef struct SDay {
	unsigned short year; ///< Year
	unsigned char month; ///< Month: 1~12
	unsigned char day; ///< Day: 1~31
} __PACKED__ SDay;

/** 时间的表示方法 */
typedef struct STimeDay {
    unsigned short year;    	///< The number of year.
    unsigned char month;    	///< The number of months since January, in the range 1 to 12.
    unsigned char day;        	///< The day of the month, in the range 1 to 31.
    unsigned char wday;        	///< The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
    unsigned char hour;     	///< The number of hours past midnight, in the range 0 to 23.
    unsigned char minute;   	///< The number of minutes after the hour, in the range 0 to 59.
    unsigned char second;   	///< The number of seconds after the minute, in the range 0 to 59.
} __PACKED__ STimeDay;

#ifdef __cplusplus
extern "C" {
#endif
time_t TcuTimeDay2T(const STimeDay *pTd);
void TcuT2TimeDay(time_t t, STimeDay *pTd);
#ifdef __cplusplus
}
#endif

/** \brief 录像查询请求结构*/ 
/** @ref TCI_CMD_LISTEVENT_REQ */
typedef struct Tcis_ExListEventReq
{
    STimeDay stStartTime; 		///< Search event from
    STimeDay stEndTime;	  		///< Search event to
    unsigned int channel; 		///< 视频通道(Camera Index) (polluted with value 3 ?)
    unsigned char event;  		///< 事件类型, 参考 @ref ECEVENT
    unsigned char reserved[3];  ///< 0
} __PACKED__ Tcis_ExListEventReq;



/** \brief 录像条目 */
typedef struct SAvExEvent {
	STimeDay start_time; ///< 录像开始时间
	unsigned int file_len; ///< time length: in second
	unsigned char event; ///< 事件类型 @ref ECEVENT
#define AVE_F_TIMELAPSE    0x01 ///< 缩时录像标志
    unsigned char flags;      ///< 录像条目其它标志. 0x01:缩时录像
    unsigned char reserved[2]; ///< 0
}  __PACKED__ SAvExEvent;

/** \brief 录像条目, 带事件的时间戳 */
typedef struct SAvEvent2 {
	STimeDay start_time; ///< 录像开始时间
	unsigned int file_len; ///< time length: in second
	unsigned char event; ///< 事件类型 @ref ECEVENT
#define AVE_F_TIMELAPSE 0x01 ///< 缩时录像标志
	unsigned char flags; ///< 录像条目其它标志. 0x01:缩时录像
	unsigned char reserved[2]; ///< =0
	unsigned int t_event; ///< 本段录像对应的事件的时间(要与上报给云端的事件时间一致). 没有事件时传0
} __PACKED__ SAvEvent2;

/** \brief SD卡录像查询返回结构 */ 
/** @ref TCI_CMD_LISTEVENT_RESP
 * */
typedef struct Tcis_ExListEventResp {
    unsigned int  channel;		///< Camera Index: 0~
    unsigned int num; ///< 录像条目总数
    unsigned char index;        ///< ignored
    unsigned char endflag;      ///< 为1是表示是最后一个包
    unsigned char count;        ///< 本包包含中的事件数
    unsigned char estype; ///< 0:录像记录为SAvExEvent数组; 1:录像记录为SAvEvent2数组
    union {
	    SAvExEvent stExEvent[1]; ///< 录像条目数组 see @ref SAvExEvent. 一次发送最多 50 条记录
	    SAvEvent2 stEvent2[1]; ///< 带事件时间戳的录像条目数组 see @ref SAvEvent2. 一次发送最多 50 条记录
    };
}  __PACKED__ Tcis_ExListEventResp;

#if 0
#define MAX_LIST_EVENT_NUM 50 ///< 一次发送最多包含的事件记录数。超过此值需要多次发送。
//send data max len is 1024
struct _lsEvtExParam{
    Tcis_ExListEventResp stListEventResp;
    SAvExEvent events[MAX_LIST_EVENT_NUM - 1];
};
#endif

/** @struct Tcis_DaysList
 * 日期列表. 
 * @ref TCI_CMD_LIST_RECORDDAYS = 0x800A
 */
typedef struct Tcis_DaysList {
	int n_day; ///< 日期数组大小
	SDay days[1]; ///< 日期数组
} __PACKED__ Tcis_DaysList;

/** @struct Tcis_PlayRecord
 * SD卡回放控制请求结构体 .
 *@ref TCI_CMD_RECORD_PLAYCONTROL
 * */
typedef struct Tcis_PlayRecord {
    unsigned int avIndex;    	///< avIndex
    unsigned int command;    	///< play record command. refer to @ref ENUM_PLAYCONTROL
    unsigned int Param;        	///< command param. Depend on \c command
    STimeDay stTimeDay;        	///< Event time from ListEventi @ref STimeDay
    unsigned int channel;    	///< Camera Index: 0~N-1
}  __PACKED__ Tcis_PlayRecord;

/** SD卡回放控制应答结构体.
 * only for play record start command
 * @ref TCI_CMD_RECORD_PLAYCONTROL_RESP
 * */
typedef struct Tcis_PlayRecordResp
{
    unsigned int command;   ///< Play record command. refer to @ref ENUM_PLAYCONTROL
    int result;     		///< Depends on command
    ///< when command is TCIC_RECORD_PLAY_START:
    ///<    result>=0   real channel no used by device for playback
    ///<    result <0    error
    ///<            -1    playback error
    ///<            -2    exceed max allow client amount
    unsigned char reserved[4];
}  __PACKED__ Tcis_PlayRecordResp;
/**@}*/ //name 录像和回放

/** @anchor AI_type*/
/** @name AI类型及掩码
 *  掩码为: (1 << AI类型值)
 * @{*/
#define AIT_BODY_DET            0  ///< 人形检测
#define AIT_BODY_TRACE          1  ///< 人形追踪
#define AIT_FACE_DET            2 ///< 人脸检测
#define AIT_FACE_RECO           3  ///< 人脸识别
#define AIT_SITPOSTURE          4 ///< 坐姿

#define AITM_BODY_DET            (1<<AIT_BODY_DET)   ///< 人形检测mask
#define AITM_BODY_TRACE          (1<<AIT_BODY_TRACE) ///< 人形追踪mask
#define AITM_FACE_DET            (1<<AIT_FACE_DET)   ///< 人脸检测mask
#define AITM_FACE_RECO           (1<<AIT_FACE_RECO)  ///< 人脸识别mask
#define AITM_SITPOSTURE          (1<<AIT_SITPOSTURE) ///< 坐姿检测mask
/**@}*/

/** @name AI功能
 * @{*/
/** @struct Tcis_SetEnableBT
 * 人形追踪开关状态结构体.
 * @ref TCI_CMD_SET_ENABLE_BT = 0x0410 \n
 * @ref TCI_CMD_GET_ENABLE_BT = 0x0412
 */
typedef struct Tcis_SetEnableBT {
    int enable;      	///< 1-打开; 0-关闭
    char resvered[4];
}  __PACKED__ Tcis_SetEnableBT;
/** \see Tcis_SetEnableBT*/
typedef Tcis_SetEnableBT Tcis_GetEnableBtResp;
/** \see Tcis_SetEnableBT*/
typedef Tcis_SetEnableBT Tcis_SetEnableBtReq;

/** \struct Tcis_AiStatus
 * 设置/获取 AI功能开关状态.
  @ref TCI_CMD_SET_AI           = 0x032C \n
  @ref TCI_CMD_GET_AI           = 0x032E \n
  设置AI模式请求和获取AI模式应答结构体  \n
*/
typedef struct Tcis_AiStatus {
    unsigned int ait_mask;      ///< <a href="#AI_type">AI类型掩码</a> 的组合。 获取时为当前支持的AI功能; 设置时为要修改状态的AI功能
    unsigned int ai_flags;      ///< 对应位为1表示使能
    unsigned char reserved[8];
} __PACKED__ Tcis_AiStatus;

/** \struct Tcis_SetShowBoxReq
 * 设置AI对象边框显示状态\n
   TCI_CMD_SET_SHOW_BOX         =      0x0348 
 */
typedef
struct Tcis_SetShowBoxReq {
    int ai_type;   ///< <a href="#AI_type">AI类型</a>
    int show_box;  ///< 1:显示边框; 0:不显示边框
} Tcis_SetShowBoxReq;

/** \struct Tcis_GetShowBoxReq
 * 获取人形/人脸边框设置状态. 
 * @ref TCI_CMD_GET_SHOW_BOX       =         0x034A 
 */
typedef
struct Tcis_GetShowBoxReq {
    int ai_type;   ///< <a href="#AI_type">AI类型/</a>
    int reserved; ///< 0
} Tcis_GetShowBoxReq;

/** \struct Tcis_EnableCloseup
 * 跟踪特写功能设置
 * @ref TCI_CMD_SET_ENABLE_CLOSEUP    =     0x033E
 * @ref TCI_CMD_GET_ENABLE_CLOSEUP    =     0x034C
 */
typedef struct Tcis_EnableCloseup {
    int channel;  ///< Camera Index: 0~N-1
    int enabled;  ///< 1: enable closeup; 0: disable closeup
} Tcis_EnableCloseup;

/** \see Tcis_GetShowBoxReq */
typedef Tcis_SetShowBoxReq Tcis_GetShowBoxResp;

/** \struct Tcis_SitPoseSens
 * 坐姿检测灵敏度.
 * TCI_CMD_SET_SITPOSE_SENS     =       0x0478 
 * TCI_CMD_GET_SITPOSE_SENS     =       0x047A 
 */
typedef struct Tcis_SitPoseSens {
    int mode;  ///< 灵敏度. 0:高(灵敏); 1:中(正常); 2:低(精准)
} __PACKED__ Tcis_SitPoseSens;

/**@}*/

/** 设备开关状态结构体.
 * @ref TCI_CMD_GET_DEVICE_STATUS = 0x0416 \n
 * @ref TCI_CMD_SET_DEVICE_STATUS = 0x0414
 */
typedef struct Tcis_SetDeviceStatus{
    int status;     	///< 1-打开; 0-关闭
    char resvered[4];
}  __PACKED__ Tcis_SetDeviceStatus;
/** \see Tcis_SetDeviceStatus */
typedef Tcis_SetDeviceStatus Tcis_SetDeviceStatusReq;
/** \see Tcis_SetDeviceStatus */
typedef Tcis_SetDeviceStatus Tcis_GetDeviceStatusResp;


/** 设置设备警铃命令和获取设备警铃设置命令的参数结构体.
 * 报警音是 Buzzer功能的扩展。要支持这些功能首先要支持 @ref Feature_AlertSound 能力
  @ref TCI_CMD_SET_ALARM_BELL             = 0x0418, \n
  @ref TCI_CMD_GET_ALARM_BELL             = 0x041A,
  */
typedef struct Tcis_SetAlarmBell {
    uint8_t      version;     ///< 0. *** 接收者要检查version的值。当前为0对应本结构定义 *** !!!!
    uint8_t      reserved[3]; ///< all 0

    /** 事件类型掩码. bit0:所有事件; bit1:移动侦测; bit2:人体检测; ...[参看 ECEVENT 定义].
     *  如果bitN为1, 表示相应事件将触发警铃 */
    uint32_t     event_mask;
    uint32_t     event_mask2; ///< 0. 用作值大于31的事件掩码
}  __PACKED__ Tcis_SetAlarmBellReq;
/** \see Tcis_SetAlarmBell*/
typedef Tcis_SetAlarmBellReq Tcis_GetAlarmBellResp;

//--------------------------------------------------------------
/** @name 云台操作
 * @{*/

/** 云台位置结构体.
 * @ref TCI_CMD_GET_PTZ_POS        = 0x040A
 */
typedef struct PtzPos {
    /** x,y,z: 取值0.0~1.0, 分别表示云台水平、垂直、纵深方向的位置点各自方向最大范围的比率。
      例如x=0.5表示去云台水平居中；z=0.0表示纵深处在x1的位置
      x,y 小于0.0或大于1.0时无意义(程序忽略)。
      z值仅在变焦倍数可知时有意义。z小于0.0或大于1.0时另行定义
      */
    float x, y, z;
}  __PACKED__ PtzPos;


/** \struct Tcis_SetPtzPosReq
 * 设置云台位置
 * @ref TCI_CMD_SET_PTZ_POS        = 0x0408 \n
 */
typedef struct Tcis_SetPtzPosReq {
    PtzPos pos;     ///< 位置
    int    channel; ///< Camera Index
    int    psp_num; ///< 如果是要转到预置位，此为预置位编号，否则为0 [2021.11.2]
} __PACKED__ Tcis_SetPtzPosReq;

/** \struct Tcis_GetPtzPosReq
 * 获取云台位置. \n
 * @ref TCI_CMD_GET_PTZ_POS = 0x040A
 */
typedef struct Tcis_GetPtzPosReq {
    int channel;   ///< Camera Index
} Tcis_GetPtzPosReq;

/** \see Tcis_SetPtzPosReq */
typedef Tcis_SetPtzPosReq Tcis_GetPtzPosResp;

/** 云台长按的参数结构体.
  @ref TCI_CMD_PTZ_LONG_COMMAND    = 0x1001,    // P2P Ptz Command Msg 
  */
typedef struct Tcis_PtzCmd {
    unsigned char control;    ///< PTZ control command, refer to @ref ENUM_PTZCMD
    unsigned char speed;      ///< PTZ control speed
    unsigned char point;      ///< 预置位(control=TCIC_PTZ_GOTO_POINT)或巡航轨迹号(control=TCIC_PTZ_CALL_TRACK)
    unsigned char limit;      ///< 0
    unsigned char aux;        ///< 0
    unsigned char channel;    ///< camera index
    unsigned char reserve[2];
} __PACKED__ Tcis_PtzCmd;

/** 云台图片内定位
    TCI_CMD_LOCATE_IN_PIC  =  0x1002
 */
typedef struct Tcis_LocateInPic {
    int channel;      ///< 定位参照通道(Refered camera-index)
    PtzPos pos; ///< 位置。 x,y为位置坐标相对图像(高,宽)的比例. 图片左上角为原点；z忽略
} Tcis_LocateInPic;

/** 云台位置 */
typedef struct PtzSpace_t {
    int x;     ///< >0: right; <0: left ?
    int y;     ///< >0: up; <0:down ?
    int zoom;  ///< >0: zoom-in; <0: zoom-out
} __PACKED__ PtzSpace_t;

/** 云台短按的参数结构体.
  @ref TCI_CMD_PTZ_SHORT_COMMAND    = 0x1001
  */
typedef struct Tcis_PtzShortCmd {
    PtzSpace_t space;  ///< see @ref PtzSpace_t
    int channel;       ///< 视频通道号(camera index)(2021.10.22)
    int focus;         ///< >0:far; <0:near (2022.10.30)
} __PACKED__ Tcis_PtzShortCmd;


/** 获取设备端的预置点.
 * TCI_CMD_GET_PSP  = 0x0452
 */
typedef struct Tcis_GetPresetPointsReq {
    /** - \c 0: 返回全部预置位
     *  - \c 1: 返回能力
     */
    uint16_t flags;    
    uint16_t channel;       ///< 视频通道号(camera index)(2022.12.15)
    uint32_t reserved; ///< 0
} Tcis_GetPresetPointsReq;

/** @anchor psp_type
 * @name 预置位类型
 * @{*/
#define PSP_BY_NO   1    ///< 预置位类型: 索引
#define PSP_BY_POS  2    ///< 预置位类型: 位置
#define PSP_BY_NO_NONAME 3 ///< 预置位用编号表示，没有名字
#define PSP_BY_POS_NONAME 4  ///< 预置位用位置表示，没有名字
/**@}*/

/** @anchor psp_flags
 * @name 预置位标志
 * 除 PSP_F_DISABLED 外，其它标志由设备返回，用于与app协商UI
 * @{*/
#define PSP_F_ZOOMONLY   0x01       ///< 本预置位仅含变焦信息(ex. 远景/近景)
#define PSP_F_SHORTCUT   0x02       ///< 表示这是个快捷位置，UI上有对应的按钮直达 (ex. 显示 远景/近景 按钮)
#define PSP_F_DISABLED   0x04       ///< 用于设置命令, 按编号删除. 单独使用
#define PSP_F_CANNOT_DELETE  0x08   ///< 设备内置，不可删除
/**@}*/

/** 预置位数组
 *
 * 预置位有两种表示方式：\n
 *   1. 传统球机用编号表示的预置位。编号对应的具体位置保存在球机内，对外是黑盒。 本命令中用 struct PresetPointArray::unionPSP::psp_by_no 表示. \n
 *   2. 用位置表示的预置位。 本命令中用 struct PresetPointArray::unionPSP::psp_by_pos 表示. \n
 *
 * 一个预置位可以对应云台的某个朝向、镜头的某个变焦倍数，或二者组合。如果仅包含变焦信息，flags 应设置 \ref PSP_F_ZOOMONLY 。
 *
 * 对仅包含变焦信息的预置位，应该放在数组的最前面，并按变焦倍数从小到大的顺序排列。这样APP容易根据排序决定UI。 \n
 * 例如，对双目+变焦摄像机，返回两个预置位（同时设置 \ref PSP_F_ZOOMONLY 和 \ref PSP_F_SHORTCUT 标志），\n
 * 第一个位置将对应[近景]按钮， 第二个对应[远景]按扭。或者，对返回多个预置位的情形，APP在一个 \n
 * 滑动轴上，按变倍值单向排列并对每个预置位置描一个驻点。
 */
typedef struct PresetPointArray {
    uint16_t n_psp; ///< 预置位数组大小
    uint8_t  type;  ///< @ref psp_type "预置位类型"
    uint8_t  channel; ///< 在TCI_CMD_GET_PSP 的应答中，为与请求匹配的通道号(Camera index)。设置时为0，因为请求结构中有定义channel字段

    /** 预置位数组 */
    union unionPSP {
        /** 用编号表示的预置位. type=PSP_BY_NO */
        struct psp_by_no { 
            uint16_t flags;       ///< 预置标志. 0 或 @ref PSP_F_DISABLED 或 其它 @ref psp_flags "PSP_F_xxx" 的组合
            uint16_t num;         ///< 预置位编号: 1~n_psp. 0保留
            char name[32];        ///< 预置位名称
        } pspn[0];     ///< type = @ref PSP_BY_NO, 用编号表示的预置位

        /** 用编号表示的预置位, 没有名字. type=PSP_BY_NO_NONAME */
        struct pspn_noname { 
            uint16_t flags;       ///< 预置标志. 0 或 @ref PSP_F_DISABLED 或 其它 @ref psp_flags "PSP_F_xxx" 的组合
            uint16_t num;         ///< 预置位编号: 1~n_psp. 0保留
        } pspn_nn[0];     ///< type = @ref PSP_BY_NO, 用编号表示的预置位

        /** 用位置表示的预置位. type=PSP_BY_POS */
        struct psp_by_pos {
            uint16_t flags;  ///< 标志. 0 或 PSP_F_DISABLED 或 @ref PSP_F_DISABLED 或 其它 @ref psp_flags "PSP_F_xxx" 的组合
            uint16_t num;     ///< ID, 用于删除或修改时作标识. 0保留不可删除(双目变焦设备用于镜头切换)
            PtzPos pos;      ///< 预置位
            char name[32];   ///< 预置位名称
        } pspp[0]; ///< type = @ref PSP_BY_POS, 用位置表示的预置位

        /** 用位置表示的预置位, 没有名字. type=PSP_BY_POS_NONAME */
        struct pspp_noname {
            uint16_t flags;  ///< 标志. 0 或 PSP_F_DISABLED 或 @ref PSP_F_DISABLED 或 其它 @ref psp_flags "PSP_F_xxx" 的组合
            uint16_t num;     ///< ID, 用于删除或修改时作标识. 0保留不可删除(双目变焦设备用于镜头切换)
            PtzPos pos;      ///< 预置位
        } pspp_nn[0]; ///< type = @ref PSP_BY_POS, 用位置表示的预置位
    } u; ///< 预置位数组
} __PACKED__ PresetPointArray;

/** 设备端返回的预置位.
 * @ref TCI_CMD_GET_PSP  = 0x0452 \n
 * 本结构用于返回设备支持的所有预置位置. 
 */
typedef struct Tcis_GetPresetPointsResp {
    union {
        /** Tcis_GetPresetPointsReq::flags == 1 */
        struct psp_cap {
            uint16_t cbSize;  ///< 本结构大小 = sizeof(struct psp_cap)。目前为12
            uint8_t  zero;    ///< 0
            uint8_t  channel; ///< 与 TCI_CMD_GET_PSP 请求匹配的通道号(Camera Index)
            uint16_t max_psp; ///< 支持的预置位数。有效预置位编号 1~max_psp. 0保留
            uint16_t type;    ///< @ref psp_type "预置位类型"
            int      flags;   ///< 0x01: 预置位保存到设备端(app要调用 TCI_CMD_SET_PSP)
        } __PACKED__ psp_cap; ///< 预置位能力(Tcis_GetPresetPointsReq::flags == 1)

        /** Tcis_GetPresetPointsReq::flags == 0 */
        PresetPointArray pspa; ///< 预置位数组
    } u;
} __PACKED__ Tcis_GetPresetPointsResp;

/** 设置设备端的预置点.
 * @ref TCI_CMD_SET_PSP = 0x0454 
 */
typedef struct Tcis_SetPresetPointsReq {
    int channel;   ///< Camera Index: 0~N-1
    struct PresetPointArray pspa;  ///< 预置位数组(一般一次设置一个位置，即数组大小为1)
} __PACKED__ Tcis_SetPresetPointsReq;

/** 设置守望位.
 * @ref TCI_CMD_SET_WATCHPOS     =           0x0456 
 */
typedef
struct Tcis_SetWatchPosReq {
    int channel; ///< Camera Index
    int num;     ///< 预置位编号. -1:禁用; >0:设置预置位num为守望位
    int idle_time; ///< 回到守望位前的云台无动作时间，单位: 秒
} __PACKED__ Tcis_SetWatchPosReq;

/** 获取守望位
 * @ref TCI_CMD_GET_WATCHPOS      =          0x0458 
 */
typedef
struct Tcis_GetWatchPosReq {
    int channel;  ///< Camera Index
    int reserved; ///< 0
} __PACKED__ Tcis_GetWatchPosReq;

/** \see Tcis_SetWatchPosReq */
typedef Tcis_SetWatchPosReq Tcis_GetWatchPosResp;

/** @anchor ptz_track_type
 * @name 轨迹的表示方式
 * @{*/
#define TRACK_BY_NO   1      ///< 轨迹由预置位号表示. 设备端保存轨迹名称
#define TRACK_BY_NO_NONAME 2 ///< 轨迹由预置位号表示，没有名字
/**@}*/

#define ACTIVE_TRACK_DEFAULT   0xffff  ///< 活动轨迹为缺省轨迹
#define ACTIVE_TRACK_AUTO_SCAN 0xfffe  ///< 活动轨迹为水平线扫
/** 云台巡航轨迹数组 */
typedef struct PtzTrackArray {
    uint16_t type;    ///< @ref ptz_track_type "轨迹表示方式"
    uint16_t n_track; ///< 轨迹数组大小
    uint16_t stay_time; ///< 在每个预置位停留的时间(s)

    /** 设置时(@ref TCI_CMD_SET_PTZ_TRACK )忽略(为0)。\n
     *  获取时(@ref TCI_CMD_GET_PTZ_TRACK ) 为当前正执行的巡航轨迹: \n 
     * - 0:  没有巡航 
     * - 1~max_track: 轨迹编号(Tcis_PtzCmd::control=TCIC_PTZ_CALL_TRACK, Tcis_PtzCmd::point=1~255) 
     * - 0xffff:   缺省轨迹(Tcis_PtzCmd::control=TCIC_PTZ_CALL_TRACK, Tcis_PtzCmd::point=0) 
     * - 0xfffe:   水平线扫(Tcis_PtzCmd::control=TCIC_PTZ_AUTO_SCAN)
     */
    uint16_t active_track; 

    union {
        /** type = TRACK_BY_NO_NONAME */
        struct track_noname {
            uint16_t track_no;  ///< 编号. >0
            uint16_t act;       ///< 动作: 0:设置；1:删除
            uint16_t n_psp;     ///< 本轨迹中的预置位数
            uint16_t pspn[0];   ///< 预置位号数组
        } __PACKED__ trck_nn[0]; ///< track_noname 轨迹数组

        /** type = TRACK_BY_NO */
        struct track {
            char name[32];      ///< 轨迹名。以'\0'结束
            uint16_t track_no;  ///< 编号。>0
            uint16_t act;       ///< 动作: 0:设置；1:删除
            uint16_t n_psp;     ///< 本轨迹中的预置位数
            uint16_t pspn[0];   ///< 预置位编号数组
        } __PACKED__ trck[0];   ///< track 轨迹数组
    } u;  ///< 轨迹数组
} __PACKED__ PtzTrackArray;


/** 巡航设置.
 * TCI_CMD_SET_PTZ_TRACK  = 0x0474 
 */
typedef struct Tcis_SetPtzTrackReq {
    int channel;   ///< Camera Index
    struct PtzTrackArray pta;  ///< 轨迹数组(一般一次设置一个位置，即数组大小为1)
} __PACKED__ Tcis_SetPtzTrackReq;

/** 获取巡航设置. 
 * TCI_CMD_GET_PTZ_TRACK  =    0x0476
 */
typedef struct Tcis_GetPtzTrackReq {
    int16_t flags;   ///< 0: 获取全部巡航轨迹; 1-查询巡航轨迹能力
    int16_t channel; ///< 视频通道号(Camera Index)(2022.12.15)
} __PACKED__ Tcis_GetPtzTrackReq;


/** 对获取巡航设置的应答. 
 * TCI_CMD_GET_PTZ_TRACK  =    0x0476
 */
typedef struct Tcis_GetPtzTrackResp {
    int16_t resp_type;     ///< 应答数据类型：=Tcis_GetPtzTrackReq::flags
    int16_t channel; ///< 视频通道号(Camera Index)(2022.12.15)
    union {
        /** 巡航能力(resp_type = 1) */
        struct cruise_cap {
            uint16_t max_tracks; ///< 支持的轨迹数。有效轨迹编号 1~max_tracks
            uint16_t type;       ///< @ref ptz_track_type "轨迹类型"
        } __PACKED__ cruise_cap; ///< 预置位能力(resp_type == 1)

        /** 巡航轨迹(resp_type = 0) */
        PtzTrackArray pta;     ///< 巡航航迹数组(resp_type == 0)
    } u; ///< 应答数据
} __PACKED__ Tcis_GetPtzTrackResp;


/**@}*/ //endof 云台
//--------------------------------------------------------------
/** 时钟表示. */
typedef struct CLOCKTIME {
	unsigned char hour; ///< Hour: 0~23
	unsigned char minute; ///< Minute: 0~59
	unsigned char second; ///< Second: 0~59
	unsigned char reserved; ///< always 0
} __PACKED__  CLOCKTIME;

/** 时间范围 */
typedef struct { CLOCKTIME from, to; } TIMERANGE; 

/** @name 时间范围2 
  @{*/
#define TR2_S_DAY     1    ///< 白天. 由设备自行判断(例如通过光敏电路)
#define TR2_S_NIGHT   2    ///< 晚上
#define TR2_S_ALLDAY  3    ///< all day
/** \struct TIMERANGE2
  时间范围2 */
typedef struct {
    unsigned char  tag;    		///< 0xff
    unsigned char  spec_time; 	///< @ref TR2_S_DAY 或 TR2_S_NIGHT 或 TR2_S_ALLDAY
    unsigned short flags;
    uint32_t       reserved;
} __PACKED__  TIMERANGE2;
/**@}*/

/** @name 布防条目 
  @{*/
#define ECEVENT_ALL  0xffffffff ///< 所有事件
/**\struct DEFENCEITEM
  布防条目
  */
typedef struct {
    /** @union Union_TR*/
    union Union_TR {
        TIMERANGE    time_range; 	///< if time_range.from.hour==255, it's @ref TIMERANGE2
        TIMERANGE2   tr2;           ///< see @ref TIMERANGE
    } u;                            ///< 时间范围

    /** 事件类型掩码. bit0:保留; bit1:移动侦测; bit2:人体检测; ...[参看 @ref ECEVENT 定义] \n
      * 如果为ECEVENT_ALL, 表示本结构应用到设备端支持的全部报警类型上
      */
    uint32_t     event_mask;
    uint32_t     event_mask2; 		///< 0. 用作值大于31的事件掩码
    unsigned int day_mask;    		///< bit-mask of week-days. bit0:Sunday; bit1-Monday; ...
} __PACKED__ DEFENCEITEM;
/**@}*/

/** @name 布防
  @{*/
/** \struct Tcis_GetDefenceReq
  获取布/撤防设置.
  @ref TCI_CMD_GET_DEFENCE_REQ           = 0x032A \n

  当设备的 @ref Feature_Cap-Defence 能力不为"bundle"时，APP在获取布/撤防设置时
  要带上此结构体,指定获取哪一个报警源的设置
 */
typedef 
struct Tcis_GetDefenceReq {
    /** 
     * 0        - 表示获取所有非单独设置的报警类型的配置。见@ref Feature_Cap-Defence 的说明
     * <0x10000 - 要获取其配置的事件类型 @ref ECEVENT 。
     */
    uint32_t event;
    uint32_t reserved;  ///< 0
} __PACKED__ Tcis_GetDefenceReq;

/** \struct Tcis_SetDefence
  设置布/撤防的参数，和获取布/撤防的应答结构体.
  @ref TCI_CMD_SET_DEFENCE_REQ             = 0x0328 \n
  TCI_CMD_SET_DEFENCE_RESP          = 0x0329 \n
  TCI_CMD_GET_DEFENCE_RESP          = 0x032B 
  */
struct Tcis_SetDefence {
    unsigned int nItems;    ///< 布/撤防计划的个数
    DEFENCEITEM items[0];   ///< 布/撤防计划，个数由nItems决定 see @ref DEFENCEITEM
} __PACKED__;
/** \see Tcis_SetDefence*/
typedef struct Tcis_SetDefence Tcis_SetDefenceReq;
/** \see Tcis_SetDefence*/
typedef struct Tcis_SetDefence Tcis_GetDefenceResp;

/** \struct Tcis_EventState
 * 报警事件状态(禁用或使能).
 *  @ref TCI_CMD_SET_EVENT_STATE =           0x031C
 */
struct Tcis_EventState {
    int event; ///< @ref ECEVENT
    int enabled; ///<1:enabled; 0:disabled
} __PACKED__;
/** \see Tcis_EventState*/
typedef struct Tcis_EventState Tcis_SetEventStateReq;
/** \see Tcis_EventState*/
typedef struct Tcis_EventState Tcis_GetEventStateResp;

/** \struct Tcis_GetEventStateReq
 *  获取报警事件状态
 *  @ref TCI_CMD_GET_EVENT_STATE =           0x031E
 */
struct Tcis_GetEventStateReq {
    int event; ///< @ref ECEVENT
    int reserved; ///< 0
} __PACKED__;
/** \see Tci_GetEventState */
typedef struct Tcis_GetEventStateReq Tcis_GetEventStateReq;
/**@}*/
//--------------------------------------------------------------
/** @name 50/60Hz选择
 *@{ */
/** @struct Tcis_SetEnvironmentReq 
 * 防闪烁参数设置请求命令的参数结构体.
 @ref TCI_CMD_SET_ENVIRONMENT_REQ        = 0x0360
 */
typedef struct
{
    unsigned int channel;       ///< Camera Index
    unsigned char mode;			///< refer to @ref ENUM_ENVIRONMENT_MODE
    unsigned char reserved[3];
} __PACKED__ Tcis_SetEnvironmentReq;


/** @struct Tcis_SetEnvironmentResp 
 * 防闪烁参数设置应答命令的参数结构体.
 @ref TCI_CMD_SET_ENVIRONMENT_RESP        = 0x0361
 */
typedef struct
{
    unsigned int channel;        ///< Camera Index
    unsigned char result;        ///< 0: success; otherwise: failed.
    unsigned char reserved[3];
} __PACKED__ Tcis_SetEnvironmentResp;


/** @struct Tcis_GetEnvironmentReq 
 * 请求获取设备当前的防闪烁参数命令的参数结构体.
 * @ref TCI_CMD_GET_ENVIRONMENT_REQ        = 0x0362,
 */
typedef struct
{
    unsigned int channel;     	///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetEnvironmentReq;

/** @struct Tcis_GetEnvironmentResp 
 * 获取当前设备防闪烁参数应答命令的参数结构体.
 * @ref TCI_CMD_GET_ENVIRONMENT_RESP        = 0x0363,
 */
typedef struct
{
    unsigned int channel;         	///< Camera Index
    unsigned char mode;            	///< refer to @ref ENUM_ENVIRONMENT_MODE
    unsigned char reserved[3];
} __PACKED__ Tcis_GetEnvironmentResp;
/**@}*/

/** @name OSD
 * @{*/
/** OSD内容 */
typedef enum { 
    OSDT_DATETIME,      ///< 日期时间
    OSDT_TEXT,          ///< 自定义文字
    OSDT_BMP1555 = 30,  ///< rgb1555
    OSDT_BMP32 = 31,    ///< argb8888
} OSDTYPE;

/** OSD 位置 */
typedef enum { 
    OSDP_LEFTTOP,      ///< 左上
    OSDP_TOPCENTER,    ///< 上部居中
    OSDP_RIGHTTOP,     ///< 右上
    OSDP_LEFTCENTER,   ///< 左中
    OSDP_MIDDLECENTER, ///< 画面正中
    OSDP_RIGHTCENTER,  ///< 右中
    OSDP_LEFTBOTTOM,   ///< 左下
    OSDP_BOTTOMCENTER, ///< 底部居中
    OSDP_RIGHTBOTTOM   ///< 右下
} OSDPOSITION;

/** @anchor osd_flags
 * @name OsdItem::flags 标志位
 * @{*/
#define OSDF_ABS_POSITION    0x0001   ///< 绝对位置
#define OSDF_DISABLED        0x0002   ///< 禁用某条osd(与 OSDF_DELETE互斥). 禁用的item仍需要传给APP
#define OSDF_DELETE          0x0004   ///< 删除某条osd(与 OSDF_DISABLED互斥)。 删除的item不要传给APP
/**@}*/

/** @anchor alignment
 * @name 对齐方式常数
 * @{*/
#define ALIGNMENT_LEFT       0x00     ///< 左对齐
#define ALIGNMENT_TOP        0x00     ///< 上对齐
#define ALIGNMENT_RIGHT      0x01     ///< 右对齐
#define ALIGNMENT_BOTTOM     0x02     ///< 下部对齐
/**@}*/

/** OSD 条目. */
typedef
struct OsdItem {
    uint16_t id;    ///< 0 ~ Tcis_GetOsdResp::nMaxOsdItems - 1. 毎个条目有唯一id
    uint16_t flags; ///< 0 或 @ref osd_flags "OsdItem 标志" 的组合
    uint32_t type;  ///< @ref OSDTYPE
    union {
        int pos;    ///< flags==0: @ref OSDPOSITION
        struct abspos {
            short x, y;         ///< osd区域在画面中的坐标. 解释受alignment的取值影响
            uint8_t alignment;  ///< <a href="#alignment">对齐方式常数</a>的组合
            uint8_t reserved[3];   ///< 0
        } abspos;   ///< flags == OSDF_ABS_POSITION
    } u;

    int len; ///< data 中的数据长度.

    /** data: 内容与 \c type 相关。长度填充到4的倍数，使下一个结构4字节边界对齐。例如\c len \c = \c 3, \c data则填充到4.
     *        举例： \c item 指向一个 OsdItem ，下一个 OsdItem 为 
     *        @code 
     *          (struct OsdItem*)(item->data + ((item->len + 3) & 0xfffc) 
     *        @endcode 
     *        或者 
     *        @code
     *          (struct OsdItem*)((long)(item->data + item->len + 3) & ~3L)
     *        @endcode
     *  - \c type=OSDT_DATETIME \n
     *      为格式字符串(参考strftime())；为空时，取设备默认格式 
     *  - \c type=OSDT_TEXT \n
     *      自定义文本. 字符编码参见 @ref Tcis_GetOsdResp::eCharEncoding
     *  - \c type=OSDT_BMP32/OSDT_BMP1555 \n
     *      带alpha属性的 bmp 文件
     */
    char data [1];
} OsdItem;


/** @struct Tcis_SetOsdReq
 * 设置OSD请求结构体. 
    TCI_CMD_SET_OSD_REQ       =         0x0374
*/
typedef struct {
    int channel;   ///< Camera Index
    int nItems;    ///< items数组大小。一次可以设置/修改/删除一条或多条OSD条目
    struct OsdItem items[1]; ///< osd条目数组
} Tcis_SetOsdReq;


/** @enum CHAR_ENCODING
 * 自定义文字字符集和编码. 所有语言都包含对ascii的支持 */
typedef enum {
    CHAR_ENCODING_ASCII,     ///< ascii only
    CHAR_ENCODING_ZH_UTF8,   ///< 中文utf-8
    CHAR_ENCODING_GB2312,    ///< 中文gb2312
    CHAR_ENCODING_GBK,       ///< 中文gbk
    CHAR_ENCODING_MAX
} CHAR_ENCODING;

/** @struct Tcis_GetOsdReq
    获取OSD 设置. 
    @ref TCI_CMD_GET_OSD_REQ         =        0x0376
*/
typedef struct {
    int channel;  ///< Camera Index
    int reserved[3]; 
} Tcis_GetOsdReq;

/** @struct Tcis_GetOsdResp
    获取OSD 应答. 
    @ref TCI_CMD_GET_OSD_RESP      =          0x0377
 */
typedef struct {
    int fSupportedTypes;  ///< 支持的OSDTYPE的位组合. 如果支持类型T, 则第T位置1. 例如支持TEXT和BMP32: (1<<OSDT_TEXT) | (1<<OSDT_BMP32)
    uint16_t eCharEncoding;    ///< 支持OSDT_TEXT时，对应的字符编码 @ref CHAR_ENCODING
    uint16_t nMaxTextLength;   ///< OSDT_TEXT 允许的最大字节数. 如果为0的话，默认为32字节
    int nMaxOsdItems;     ///< 支持最大osd条数
    int nItems;           ///< osd条目数(items大小)
    struct OsdItem items[1];     ///< osd条目数组
} __PACKED__ Tcis_GetOsdResp;
/**@}*/ //name OSD

/** @name 图像翻转
 * @{*/
/** @struct Tcis_SetVideoModeReq
 * 设置视频翻转参数请求命令参数结构体.
 * @ref TCI_CMD_SET_VIDEOMODE_REQ            = 0x0370
 */
typedef struct
{
    unsigned int channel;    ///< Camera Index
    unsigned char mode;      ///< refer to ENUM_VIDEO_MODE
    unsigned char reserved[3];
} __PACKED__ Tcis_SetVideoModeReq;

/** @struct Tcis_SetVideoModeResp
 * 设置视频翻转参数应答命令的参数结构体.
 * @ref TCI_CMD_SET_VIDEOMODE_RESP        = 0x0371
 */
typedef struct
{
    unsigned int channel;     	///< Camera Index
    unsigned char result;    	///< 0: success; otherwise: failed.
    unsigned char reserved[3];
} __PACKED__ Tcis_SetVideoModeResp;


/** @struct Tcis_GetVideoModeReq
 * 获取当前设备视频翻转参数请求命令的参数结构体.
 * @ref TCI_CMD_GET_VIDEOMODE_REQ            = 0x0372,
 */
typedef struct
{
    unsigned int channel;     ///< Camera Index
    unsigned char reserved[4];
} __PACKED__ Tcis_GetVideoModeReq;


/** @struct Tcis_GetVideoModeResp
  获取当前设备视频翻转参数应答命令的参数结构体.
  @ref TCI_CMD_GET_VIDEOMODE_RESP        = 0x0373,
  */
typedef struct
{
    unsigned int  channel;     ///< Camera Index
    unsigned char mode;        ///< refer to @ref ENUM_VIDEO_MODE
    unsigned char reserved[3];
} __PACKED__ Tcis_GetVideoModeResp;
/**@}*/

/** @name SD卡状态和格式化
 * @{*/
/** \brief SD卡状态应答结构. 
 * @ref TCI_CMD_GET_EXTERNAL_STORAGE_RESP
 */
typedef struct Tcis_SDCapResp {
    unsigned int channel;		///< 0
    /** 总容量，单位 M
      -      >0: total space size of sdcard (MBytes)								
      -  0: 无卡
      - -1: 卡状态错，需要格式化
      - -2: SD卡状态为只读(可回放但不能继续写入)
      - -3: 正在格式化
      - -4: 正在初始化
      */
    int total;

    int free;			    ///< Free space size of sdcard (MBytes)
    unsigned char reserved[8];	// reserved
} __PACKED__ Tcis_SDCapResp;


/** @struct Tcis_FormatExtStorageReq
  格式化SD卡请求命令的参数结构体.
  @ref TCI_CMD_FORMATEXTSTORAGE_REQ            = 0x0380,
  */
typedef struct
{
    unsigned int storage;     ///< Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
    unsigned char reserved[4];
} __PACKED__ Tcis_FormatExtStorageReq;


/** @struct Tcis_FormatExtStorageResp
  格式化SD卡应答命令的参数结构体.
  @ref TCI_CMD_FORMATEXTSTORAGE_REQ        = 0x0381
  */
typedef struct
{
    unsigned int  storage;     	///< Storage index
    ///< 0: success;
    ///< -1: format command is not supported.
    ///< otherwise: failed.
    char result;
    unsigned char reserved[3];
} __PACKED__ Tcis_FormatExtStorageResp;
/**@}*/

/** @name 带屏IPC
 * @{*/
/** \struct Tcis_ScreenDisplay
    @ref TCI_CMD_SET_SCREEN_DISPLAY     =     0x0382  // 设置屏幕显示
    @ref TCI_CMD_GET_SCREEN_DISPLAY     =     0x0384  // 获取屏幕显示设置
*/
typedef struct Tcis_ScreenDisplay {
	int disp_off_time; ///< 非呼叫原因(例如设置)点亮屏幕后转熄屏的时间，单位:秒.
	        ///<   - \c 0 - 表示永不熄屏
} Tcis_ScreenDisplay;
/**@}*/

/** @name 报警音
 * @{*/

/** 音频文件格式 */
typedef enum AUDIOFILEFMT { 
    AF_FMT_WAV,  ///< .wav
    AF_FMT_AMR,  ///< .amr
    AF_FMT_MP3,  ///< .mp3
    AF_FMT_M4A   ///< .m4a
} AUDIOFILEFMT;
/** 获取音频文件格式应答.
 * 音频文件格式结构体，用于协商报警音的格式. \n
 @ref  TCI_CMD_GET_ALARMTONE_CAP           = 0x041C
 * - 如果设备返回错误状态值，表示提示音不能修改，但可以通过 TCI_CMD_PLAY_ALARMTONE 播放 (2024/1/5)
 */
typedef struct Tcis_GetAlarmToneCap_Resp {
    unsigned short nSamplePerSec;           ///< 采样频率
    unsigned char  nBitsPerSample;          ///< 采样位宽
    unsigned char  nChannels;               ///< 通道数
    unsigned int   nExpectedFileFormats;    ///< 期望的文件格式数
    unsigned char  ExpectedFileFormats[8];  ///< 期望的音频格式 AUDIOFILEFMT
    unsigned int   nSupportedAudioCodecs;   ///< 支持的音频格式数
    unsigned char  SupportedAudioCodecs[8]; ///< 支持的音频格式 TCMEDIA

    /** 当前报警音标识. \n
     * - 0 - 默认
     * - 其它: 从app通过 @ref TCI_CMD_SET_ALARMTONE 指令收到的内容标识
     * @ref Tcis_SetAlarmTone_Req::id
     */
    unsigned int   idAlarmTone;
    //2021-05-21 新增域
    unsigned int   uiFileSizeLmt;            ///< 文件大小上限(单位KB)。对APP来说，当收到的应答长度>=36时才存在这个信息
} __PACKED__ Tcis_GetAlarmToneCap_Resp;

/**
  @ref TCI_CMD_SET_ALARMTONE              = 0x041E,    //设置报警音频 \n
  @ref TCI_CMD_PLAY_AUDIO                 = 0x0356     //播放语音不保存

  如果app传入不支持的音频或文件格式，ipc返回TCI_E_INVALID_PARAM
  */
typedef struct Tcis_SetAlarmTone_Req {
    unsigned int   id;               ///< 0: 默认声音; 其它:声音内容标识
    unsigned short type;             ///< 0:data为音频内容; 1:data为下载音频文件的url
    unsigned char af_fmt;            ///< @ref AUDIOFILEFMT
    unsigned char a_codec;           ///< @ref TCMEDIA
    int           data_len;          ///< length of data
    char data[0];
} __PACKED__ Tcis_SetAlarmTone_Req;
/**@}*/

/** @name 灯状态和语音提示
 * @{*/
/** 请求/返回指示灯状态.
  @ref TCI_CMD_SET_LED_STATUS          = 0x0422\n
  @ref TCI_CMD_GET_LED_STATUS          = 0x0424
  */
typedef struct Tcis_SetLedStatusReq {
    int status;         	///< 0-关闭，1-打开
    char resvered[4];   	///< 保留字段
} __PACKED__ Tcis_SetLedStatusReq;
/** \see Tcis_SetLedStatusReq*/
typedef Tcis_SetLedStatusReq Tcis_GetLedStatusResp;

/** 获取/设置提示音状态
 * @ref TCI_CMD_SET_VOICE_PROMPT_STATUS    = 0x0358 \n
 * @ref TCI_CMD_GET_VOICE_PROMPT_STATUS    = 0x035A
 */
typedef struct Tcis_VoicePromptStatus {
    int status;             ///< 0-关闭; 1-打开
    int reserved;           ///< 保留:0
} Tcis_VoicePromptStatus;

/**@}*/

/** 使用电池供电的摄像机的电池工作状态.
  @ref TCI_CMD_GET_BATTERY_STATUS      = 0x0426, \n
  */
typedef struct Tcis_GetBatteryStatusResp {
    int batteryMode;        ///< 电池工作模式0--放电，1--充电
    int batteryPower;       ///< 电池电量0-100(%),-1未知
    int batteryLow;         ///< 电量低标志.1:低电;0:有电。APP要检查收到的应答长度。>=12时才有此域
} __PACKED__ Tcis_GetBatteryStatusResp;

/** 获取4G的信号强度.
  @ref TCI_CMD_GET_WIFI_SIGNALLEVEL    = 0x0428,
  */
typedef struct Tcis_GetWifiLevelResp {
    int activeNetIntf;      //0-有线，1-WiFi, 2-4g 
    int signalLevel;        //信号强度0-100(%),-1未知
} __PACKED__ Tcis_GetWifiLevelResp;

/** @name 低功耗相关 */
/** 低功耗摄像头唤醒后的最大工作时长.
  @ref TCI_CMD_SET_MAX_AWAKE_TIME      = 0x042A,\n
  @ref TCI_CMD_GET_MAX_AWAKE_TIME      = 0x042C,
  */
typedef struct Tcis_GetMaxAwakeTimeResp {
    int max_awake_time;  	///< 设备唤醒后的最大工作时长(单位：秒). 0为一直工作
    int reserved;
} Tcis_GetMaxAwakeTimeResp;
/** \see Tcis_GetMaxAwakeTimeResp*/
typedef Tcis_GetMaxAwakeTimeResp Tcis_SetMaxAwakeTimeReq;

/** 休眠状态 
 * @ref TCI_CMD_SET_ENABLE_DORMANCY     = 0x0432 \n
 * @ref TCI_CMD_GET_ENABLE_DORMANCY     = 0x0434
 */
typedef struct Tcis_DormancyState {
    int enable;    ///< 1:允许休眠; 0:禁止休眠 
    int reserved;
} Tcis_DormancyState;

/** 多时间段变长数组 */
struct Fake_TimeRanges {
    int n_tr;             ///< 时间段数
    TIMERANGE tr[1];      ///< 时间段数组
} __PACKED__;

/** 设置低功耗设备主动唤醒时间.
    @ref TCI_CMD_SET_AWAKE_TIME      =        0x0470 
*/
/** \see struct Fake_TimeRanges */
typedef struct Fake_TimeRanges Tcis_SetAwakeTimeReq;

/** 获取低功耗设备主动唤醒时间.
    @ref TCI_CMD_GET_AWAKE_TIME      =        0x0472 
*/
typedef struct Tcis_GetAwakeTimeReq {
    uint32_t reserved;
} __PACKED__ Tcis_GetAwakeTimeReq;

/** \see struct Fake_TimeRanges */
typedef struct Fake_TimeRanges Tcis_GetAwakeTimeResp;

/** \struct TIMEPLAN.
 * 时间计划 */
typedef struct TIMEPLAN {
	TIMERANGE time_range; ///< 起止时间;如果结束时间<=开始时间;逻辑为跨天
	unsigned int day_mask; ///< 重复: bit-mask of week-day.bit0-Sunday;bit1-Monday......
	int enabled; ///< 该定时是否使能
} __PACKED__ TIMEPLAN;

/** \struct TIMEPLANS 定时计划不定长数组 */
typedef struct TIMEPLANS {
	unsigned int nItems; ///< 时间计划个数
	TIMEPLAN items[0]; ///<  时间计划数组, 个数由nItems决定
} __PACKED__ TIMEPLANS;

/** 电源策略.
 *  引入文档 https://tange-ai.feishu.cn/docx/M3DOddPWZoe8Mbxk1vpcf8avnqO
 */
typedef enum {
	PS_PERFORMANCE_FIRST, ///< 性能优先
	PS_POWERSAVING, ///< 省电模式. 开启ai, 录像不超过10"
	/** 超级省电模式(关闭本地pir唤醒).
     * @note App不发送关闭PIR的命令. 固件保留之前的PIR配置, \n
     *       但实际PIR不生效.
     */
	PS_SUPER_POWERSAVING,
	PS_USER_DEFINED ///< 用户定义录像时长
} POWERSTRATEGY;

/** @struct Tcis_PowerStrategy
 * 电池供电时电源策略.
    @ref TCI_CMD_SET_POWER_STRATEGY  =  0x048C \n
    @ref TCI_CMD_GET_POWER_STRATEGY  =  0x048E
*/
typedef struct Tcis_PowerStrategy {
	int strategy; ///< 当前工作模式 @ref POWERSTRATEGY
	int rec_len; ///< strategy=@ref PS_USER_DEFINED时的自定义录像(工作)时长. 非自定义模式时为0
	/** 定时计划. 仅当strategy不是 @ref PS_SUPER_POWERSAVING 时定时计划才有效。 \n
     *  在定时范围内, 按当前模式工作, 定时范围外按 PS_SUPER_POWERSAVING 模式工作. \n
     *  但在 strategy=PS_SUPER_POWERSAVING 时, 设备仍然要响应指令，返回或保存定时 \n
     *  计划的内容, 以方便APP对编辑操作
     */
	TIMEPLANS plans;
} __PACKED__ Tcis_PowerStrategy;

/**@}*/

/** @name 设备关闭计划
 * @{*/
/** 设备关闭计划单元. @see TIMEPLAN */
typedef struct TIMEPLAN CLOSEITEM;

/** 设备关闭计划.
  @ref TCI_CMD_SET_CLOSE_PLAN             = 0x042E \n
  @ref TCI_CMD_GET_CLOSE_PLAN             = 0x0430
  */
/** \see TIMEPLANS*/
typedef TIMEPLANS Tcis_SetClosePlanReq;

/** \see TIMEPLANS*/
typedef TIMEPLANS Tcis_GetClosePlanResp;
/**@}*/

/** @name G-sensor
 * @{*/
/** @enum GSENSORSCENE 
 * g-sensor检查场景.
 */
typedef enum GSENSORSCENE {
    GSENSOR_SCENE_DRIVING = 0,  ///< 车辆运行中(行车记录仪); 或非行车记录仪的一般使用场景
    GSENSOR_SCENE_PARKING = 1   ///< 停车状态(行车记录仪).  停用！被 TCI_CMD_SET_PARKING_DET 命令代替
} GSENSORSCENE;

/** G-Sensor设置.
  @ref TCI_CMD_SET_GSENSOR               0x432
  */
struct Tcis_GsensorSetting {
    int sensitivity;       ///< 灵敏度: 0-关闭; 1-低; 2-中; 3-高

    /** 场景 @ref GSENSORSCENE \n
     * 如果 @ref Feature_G-Sensor "G-Sensor" 能力不为 "scene", 则 scene 只能为0. \n
     * 如果设备不支持某个 scene, GET操作设备返回状态码 @ref TCI_E_INVALID_PARAM */ 
    int scene;
} __PACKED__;
/** \see Tcis_GsensorSetting */
typedef struct Tcis_GsensorSetting Tcis_SetGsensorReq;

/** \see Tcis_GsensorSetting */
typedef struct Tcis_GsensorSetting Tcis_GetGsensorResp;

/** 获取g-sensor设置.
  @ref TCI_CMD_GET_GSENSOR               0x434 \n

 */
struct Tcis_GetGsensorReq {
    /** 场景 @ref GSENSORSCENE \n
     * 如果 @ref Feature_G-Sensor "G-Sensor" 能力不为 "scene", 则 scene 只能为0. \n
     * scene非0时，APP要检查返回结构中的 scene 是不是与请求的scene一致，不一致则认为设备不支持相应场景。
     */
    int scene;
    int reserved; ///< 0
} __PACKED__;
typedef struct Tcis_GetGsensorReq Tcis_GetGsensorReq;

/** @anchor parkingdet_field_mask */
/** @name Tcis_ParkingDet::flags 停车监控域标志位 
 * 设置后表示支持相应能力，并且 Tcis_ParkingDet 结构中对应成员有效
 * @{*/
#define  PARKINGDET_F_SENS             0x0001   ///< 支持 sensitivity
#define  PARKINGDET_F_WORKTIME         0x0002   ///< 支持 work_time
/**@}*/

/** 停车监控设置. 
 *  TCI_CMD_SET_PARKING_DET     =       0x0364 \n
 *  TCI_CMD_GET_PARKING_DET     =       0x0366
 *
 *  本结构在GET时作返回，SET时为输入
 */
typedef struct Tcis_ParkingDet {
    int id;          ///< 0. 保留
    /** @ref parkingdet_field_mask "停车监控域标志位".
     *  获取时， flags 为支持的设置项的掩码。设置时，为结构体中有效成员的标志。\n
     *  例如，如果设备只支持灵敏度设置。设备在GET时，App在SET时，设置 flags=@ref PARKINGDET_F_SENS, sensitivity为灵敏度。
     */
    int flags;
    int sensitivity; ///< 灵敏度: 0-关闭; 1-低; 2-中; 3-高
    int work_time;   ///< 工作时间。单位 hour. 要与app的UI匹配(目前是8/12/24)
} Tcis_ParkingDet;


/** 获取停车监控设置. 
 *  TCI_CMD_GET_PARKING_DET      =       0x0366
 */
typedef struct Tcis_GetParkingDetReq {
    int id;  ///< 0. 保留
} Tcis_GetParkingDetReq;

/**@}*/

/** @name 音量
 * @{*/
/** 喇叭音量
  @ref TCI_CMD_SET_VOLUME 	= 0x436 \n
  @ref TCI_CMD_GET_VOLUME 	= 0x438
  */
typedef struct Tcis_SetVolume {
    int flags; 					///< 0: 音量调节范围不可知，1：音量调节范围可知
    int volume; 				///< 音量：\n 
                                ///<    - flags为0=> 1:音量加，-1:音量减
                                ///<    - flags为1=> 0-100之间的等级
} __PACKED__ Tcis_SetVolume;
/** \see Tcis_SetVolume*/
typedef Tcis_SetVolume Tcis_SetVolumeReq;
/** \see Tcis_SetVolume*/
typedef Tcis_SetVolume Tcis_GetVolumeResp;

/** MIC 灵敏度
  @ref TCI_CMD_SET_MIC_LEVEL 	=	0x43A \n
  @ref TCI_CMD_GET_MIC_LEVEL 	=	0x43C
  */
typedef struct Tcis_SetMicLevel {
    int sensitivity;					///< 灵敏度：[0-100]
    unsigned char reserved[4];
} __PACKED__ Tcis_SetMicLevel;
/** \see Tcis_SetMicLevel*/
typedef Tcis_SetMicLevel Tcis_SetMicLevelReq;
/** \see Tcis_SetMicLevel*/
typedef Tcis_SetMicLevel Tcis_GetMicLevelResp;
/**@}*/

/** @name 画中画
 * @{*/
/** @struct Tcis_PrimaryView
 * 选择和获取主画面.
 * @ref TCI_CMD_SET_PRIMARY_VIEW \n
 * @ref TCI_CMD_GET_PRIMARY_VIEW
 */
typedef struct Tcis_PrimaryView {
    int id;        ///< 固定为0.
    int channel;   ///< 主画面通道: 0|1. GET 请求时忽略
} __PACKED__ Tcis_PrimaryView;
/** \see Tcis_PrimaryView. */ 
typedef Tcis_PrimaryView Tcis_SetPrimaryViewReq;
/** \see Tcis_PrimaryView. */ 
typedef Tcis_PrimaryView Tcis_GetPrimaryViewResp;
/** \see Tcis_PrimaryView. */ 
typedef Tcis_PrimaryView Tcis_GetPrimaryViewReq;
/**@}*/

/** @name 报警灯
 * @{*/
/** 设备和获取报警灯状态.
 * @ref TCI_CMD_SET_ALARMLIGHT  = 0x442\n
 * @ref TCI_CMD_GET_ALARMLIGHT  = 0x444
*/
typedef struct Tcis_AlarmLightState {
    int channel; ///< id of light: 0
    int state;   ///< 0:关; 1:开; 2:自动; 3:按定时设置
} __PACKED__ Tcis_AlarmLightState;
/** \see Tcis_AlarmLightState*/
typedef Tcis_AlarmLightState Tcis_GetAlarmLightStateResp;
/** \see Tcis_AlarmLightState */
typedef Tcis_GetAlarmLightStateResp Tcis_SetAlarmLightStateReq;
/**@}*/

/** @name PIR
 * @{*/
/** PIR灵敏度设置.
 * @ref TCI_CMD_SET_PIR     =    0x446
 * @ref TCI_CMD_GET_PIR     =    0x448
 */
typedef struct Tcis_PirSens {
    int channel; ///< id of PIR：0
    int sens;    ///< 0:关闭; 1:低; 2:中; 3:高
} __PACKED__ Tcis_PirSens;
/** @see Tcis_PirSens */
typedef Tcis_PirSens Tcis_SetPirSensReq;
/** @see Tcis_PirSens */
typedef Tcis_PirSens Tcis_GetPirSensResp;
/**@}*/

/** @name 门铃
 * @{*/
/** 门铃呼叫应答. APP显式地接通或拒接.
 * @ref TCI_CMD_ANSWERTOCALL  =  0x450
 */
typedef struct Tcis_AnswerToCall {
    /** 呼叫应答状态. APP端支持:
     * - @ref CALLSTATE_ANSWERED
     * - @ref CALLSTATE_REJECTED 
     * - @ref CALLSTATE_HANGUP
     *
     * sdk对接听应答(CALLSTATE_ANSWERED)会通过通用应答返回状态码:
     *  - 0 成功接听
     *  - TCI_E_INPROGRESSING  已经有人在接听
     *  - TCI_E_NOT_ALLOWED    不在呼叫状态(已经挂断、超时等)
     */
    int state; //高16位用作应答源类型。为1时表示来自微信小程序

    /** for HANGUP, 0:normal hangup; 1:connection broken
     */
    int more;
} Tcis_AnswerToCall;

/** 开门请求。
 * @ref TCI_CMD_UNLOCK = 0x045A  ///< 开门 Req: @ref Tcis_UnlockReq
 */
typedef struct Tcis_UnlockReq {
    int user_id;       ///< 0
    char token[60];    ///< 加密的密钥. sdk解密后推送给应用
} Tcis_UnlockReq;

typedef enum UNLOCKRES {
    ULR_OK, ///< door is opened
    ULR_INVALID_KEY,  ///< invalid key
    ULR_HW_FAILURE,   ///< 硬件故障
    ULR_SYS_FROZEN,   ///< 开锁被冻结
    ULR_TIME_SKEWED,  ///< APP端时间与设备偏差太大
    ULR_VERIFY_FAILED, ///< 平台验证失败
} UNLOCKRES;
/** 开门应答
 * @ref TCI_CMD_UNLOCK = 0x045A  ///< 开门 Req: @ref Tcis_UnlockReq
 */
typedef struct Tcis_UnlockResp {
    int status;   ///< 开锁应答码 @ref UNLOCKRES
} Tcis_UnlockResp;

/** 门(锁)状态(暂定). 本结构在与门锁对接后会调整
 * TCI_CMD_GET_LOCK_STATE  = 0x045C  ///< 获取门(锁状态)
 */
typedef struct Tcis_LockState {
    uint32_t state;  ///< 1:开;0:关
} Tcis_LockState;
/**@}*/

/** @name 网络IP配置
 * @{*/
/** @struct IPCONFIG
 * 网络IP 配置.
 */
typedef struct IPCONFIG {
    char intf[16];      ///< 活动接口名
    int  bDhcpEnabled;  ///< 0:手动配置; 1:自动配置; 2:自动配置ip/gateway,手动dns
    char ip[16];        ///< ip地址
    char netmask[16];   ///< 子网掩码
    char gateway[16];   ///< 网关
    char dns1[16];      ///< dns服务器1
    char dns2[16];      ///< dns服务器2
    char mac[20];       ///< 设备MAC. 只读
} __PACKED__ IPCONFIG;
/**@}*/

/** @name 台灯
 * @{*/
/** @struct Tcis_GetLightReq 
 * 请求智能灯状态 \n
 * TCI_CMD_GET_LIGHT = 0x354
 */
typedef struct Tcis_GetLightReq {
    int id;       ///< 灯标识: 0
    int reserved; ///< 0
} __PACKED__ Tcis_GetLightReq;

/** @anchor light_op_mask */
/** @name 灯光设置操作内容
 * @{*/
#define SETLIGHT_F_ONOFF     0x0001  ///< 设置开关 Tcis_LightState::on
#define SETLIGHT_F_MODE      0x0002  ///< 设置控制模式 Tcis_LightState::mode
#define SETLIGHT_F_INTENSITY 0x0004  ///< 设置亮度 Tcis_LightState::intensity
#define SETLIGHT_F_DELAYSHUT 0x0008  ///< 设置延时关闭 Tcis_LightState::delay_shutdown
//#define SETLIGHT_F_COLOR     0x0008  ///< 设置颜色 Tcis_LightState::color
/**@}*/

/** 智能灯状态.
 * TCI_CMD_SET_LIGHT = 0x352 \n
 * TCI_CMD_GET_LIGHT = 0x354 \n
 *
 * \b 举例 \n
 * - 手动开灯，并且亮度设为80
 *   \code{.c}
 *   Tcis_LightState state = {
 *      .id = 0,
 *      .fMask = SETLIGHT_F_ONOFF | SETLIGHT_F_INTENSITY,
 *      .on = 1,
 *      .intensity = 50
 *   };
 *   \endcode
 */
typedef struct Tcis_LightState {
    uint16_t id;        ///< 灯标识: 0
    uint16_t fMask;     ///< 结构内容掩码。见 @ref light_op_mask. 设备返回时, 也表示其支持的配置项
    uint8_t on;         ///< 0:关闭； 1:开
    uint8_t mode;       ///< 0:manually; 1:自动控制
    uint8_t delay_shutdown; ///< 延迟关闭: 0-不延时; 1-延时
    uint8_t intensity;  ///< 强度 0~100
    //uint32_t color;   ///< 颜色0xrrggbb
} __PACKED__ Tcis_LightState;
/**@}*/

/**@name 提示间 @{*/
/** 提示音类型 */
typedef enum ENUMHINTTONE {
    EHT_XXXX        = 1,        ///< 保留。以后可以将alarmtone的设置统一到这个接口
    EHT_CRY         = 2,        ///< 哭声
    EHT_BAD_POSTURE = 3         ///< 错误坐姿
} ENUMHINTTONE;

/** @anchor hinttone_op_mask */
/** @name 设置提示音内容掩码
 * @{*/
/** 设置提示音功能开关。要设置 @ref Tcis_SetHintToneReq 的\c enabled 和 \c ht_type 成员 */
#define SETHINTTONE_F_SWITCH      0x01
/** 设置提示音数据. 要设置 @ref Tcis_SetHintToneReq 结构中除 \c enabled 之外的所有成员 */
#define SETHINTTONE_F_DATA        0x02
/*@}*/

/** 更通用的设置设备端提示音的结构。比 Tcis_SetAlarmTone_Req 多一个 ht_type 参数 \n
 * TCI_CMD_SET_HINTTONE      =         0x0480 \n
 * \n
 * app要先调用 @ref TCI_CMD_GET_ALARMTONE_CAP 获取设备端支持的音频格式.
 */
typedef struct Tcis_SetHintToneReq {
    unsigned char    fMask;          ///< 本结构中有效数据掩码. @ref hinttone_op_mask

    unsigned char    enabled;        ///< 1:使能。0:禁用

    unsigned short   ht_type;        ///< 提示对象。@ref ENUMHINTTONE
    unsigned int     id;             ///< 0: 默认声音; 其它:声音内容标识。设备端保存这个标识并在app查询时返回
    unsigned short   type;           ///< 0:data为音频内容; 1:data为下载音频文件的url
    unsigned char    af_fmt;         ///< @ref AUDIOFILEFMT
    unsigned char    a_codec;        ///< @ref TCMEDIA
    int              data_len;       ///< length of data
    char             data[0];        ///< type规定的内容: 音频数据或url
} __PACKED__ Tcis_SetHintToneReq;

/** 获取提示音的请求结构.
 * TCI_CMD_GET_HINTTONE               0x0482
 */
typedef
struct Tcis_GetHintToneReq {
    unsigned int ht_type;    ///< 提示对象。 @ref ENUMHINTTONE
} __PACKED__ Tcis_GetHintToneReq;

/** 获取提示音的应答结构.
 * TCI_CMD_GET_HINTTONE               0x0482
 */
typedef
struct Tcis_GetHintToneResp {
    short            enabled;        ///< 1:允许; 0:禁用
    unsigned short   ht_type;        ///< 提示对象。@ref ENUMHINTTONE
    unsigned int   id;               ///< TCI_CMD_SET_HINTTONE 的id参数
} __PACKED__ Tcis_GetHintToneResp;
/**@}*/

/** 运行时状态 */
typedef
enum ENUMRTSTATE {
    RT_STATE_RECORDING = 0   ///< 是否在录像. 参数：无。值: iState。 0-没有录像; 1-在录像
} ENUMRTSTATE;

/** 获取设备运行时状态
 * TCI_CMD_GET_RUNTIME_STATE = 0x0388
 */
typedef
struct Tcis_GetRuntimeStateReq {
    int state_name;    ///< 请求的状态名，@ref ENUMRTSTATE. 状态可能会有参数，见相应状态说明 
} __PACKED__ Tcis_GetRuntimeStateReq;

/** 设备端运行时状态的应答. 如果设备不支持相应的状态，\n
 * 在通用应答里返回 TCI_E_INVALID_PARAM .
 *
 * TCI_CMD_GET_RUNTIME_STATE   =   0x0388
 */
typedef
struct Tcis_RuntimeStateResp {
    int state_name;   ///< 状态名: @ref ENUMRTSTATE
    union {
        int iState;  ///< 整型状态值
    } uState;  ///< 状态值
} __PACKED__ Tcis_RuntimeStateResp;

/** @name 行车记录仪
 * @{*/
/** \struct Tcis_ParkingMonitorSwitch
 * 停车监控总开关. 用于同时使能或禁用停车监控的有所功能. \n
 *
    @ref TCI_CMD_SET_PARKING_MONITOR    =    0x0484 \n
    @ref TCI_CMD_GET_PARKING_MONITOR    =    0x0486 \n

  @note 该命令影响以下子功能: 
    - 停车监控设置 @ref TCI_CMD_GET_PARKING_DET/@ref TCI_CMD_SET_PARKING_DET
    - 缩时录像设置 @ref TCI_CMD_SET_TIMELAPSE_RECORD/@ref TCI_CMD_GET_TIMELAPSE_RECORD
 */
typedef
struct Tcis_ParkingMonitorSwitch {
    /** - \c 0: 禁用所有子功能
        - \c 1: 打开停车监控总开关，但各子功能有其自己的设置(包括是否启用); 
     */ 
    int enabled;
} __PACKED__ Tcis_ParkingMonitorSwitch;
/**@}*/

/** @name 定时任务 
 * 本处定义通用的定时任务配置机制。
 *@}*/

/** 定时任务标准动作定义 */
enum TgAction {
    TTA_OFF,  ///< 关闭/停止/...
    TTA_ON    ///< 打开/启动/...
};

/** 定时任务描述 */
typedef struct TgTimeAction {
    int       size;     ///< 本结构长度. 因为action的长度同具体动作有关。本结构是变长的

    CLOCKTIME from, to; ///< 时间范围
    uint16_t  state;    ///< 0:禁止(或单次定时器已执行); 1:有效(调度中)
    uint16_t  repeat;   ///< weekdays mask. bit0:Sunday; bit1-Monday; ...

    /** 对象相关的动作. 
     * 简单动作为一个4字节整数, 复杂动作会扩展，但长度为4的倍数. 
     * 简单动作定义见 @ref TgAction */
    uint32_t  action;
} __PACKED__ TgTimeAction;

/** \struct Tcis_TimerTask
 * 设置定时任务.
    TCI_CMD_SET_TIMER_TASK      =        0x0488 
 */
typedef
struct Tcis_TimerTask {
    uint16_t  object;   ///< 定时任务作用对象. @ref ETGTIMERTARGET
    uint16_t  id;       ///< 对象标识，用于区别同种类型的多个对象。从0开始编号
    uint32_t nItems;    ///< 任务数
    TgTimeAction Items[0];  ///< 定时任务
} __PACKED__ Tcis_TimerTask;

/** 定时任务对象 */
typedef enum ETgTimerTarget {
    TTT_ALL = 0,       ///< 不限(全部对象)
    TTT_LIGHT,         ///< 灯光定时任务
    TTT_ALARM_TONE     ///< 警戒语音
} ETGTIMERTARGET;

/** \struct Tcis_GetTimerTask
 *  获取定时任务. 
 *   TCI_CMD_GET_TIMER_TASK     =        0x048A \n
 *
 *  当app传入设备不支持 object 或 id 时，返回状态码 TCI_E_INVALID_PARAM
 */
typedef
struct Tcis_GetTimerTask {
    uint16_t object;    ///< 定时任务对象类型.  @ref ETGTIMERTARGET
    uint16_t id;        ///< 对象标识, 用于区别同种类型的多个对象. 从0开始编号
} Tcis_GetTimerTask;

/**@}*/

__END_PACKED__

#endif
