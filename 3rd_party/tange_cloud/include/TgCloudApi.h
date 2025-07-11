/**
 * @file TgCloudApi.h
 * @brief 探鸽云SDK接口申明
 */
#ifndef __TgCloudApi_h__
#define __TgCloudApi_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
#include "TgCloudConst.h"
#include "TgCloudCmd.h"
#include "TgCloudCmdEx.h"
#include "ec_const.h"
#include "TgCloudUtil.h"

/** \defgroup api_structure API数据结构
 * API接口用到的数据结构
 * @{*/
/** 设备基本信息 */
typedef struct DeviceInfo {
	char device_type[16];   ///< Not used
	char vendor[16];		///<设备生产厂商 OEM
    char firmware_id[32];   ///<ota 升级用的固件标识, 平台唯一. 两款设备，如果升级包一样，就有相同的 firmware_id。
                            ///< 可以按 "公司_方案_设备类型_其它信息" 的规则自己定义一个字符串, 例如: "TANGE_HISI_IPC_001".
                            ///< 标识里不能有"."。
                            ///< 平台按字符串比较，不关心具体内容.
    char firmware_ver[10];  ///< 固件主版本,8个数字组成的字符串。like: "03020201", means 3.2.2.1. 
    char model[32];         ///< 产品型号
} TCIDEVICEINFO;

/** 云服务信息 */
typedef struct TCISERVICEINFO {
    ECSERVICETYPE serviceType; ///< 云服务类型
    time_t expiration;         ///< 过期时间(utc)
    char *objects;             ///< '\0'结尾并以一个额外 '\0'结束的允许检测的ai对象列表。例如: "body\0car\0cry\0"
} TCISERVICEINFO;

/** p2p连接句柄. A opaque pointer */
typedef void *p2phandle_t;
/** 命令回调类型 */
typedef int (*TGCMDHANDLER)(p2phandle_t handle, int cmd, const void *data, int len);

/*
typedef uint64_t feature_map_t;
void FEATURE_ZERO(feature_map_t *fm);
int FEATURE_SET(feature_map_t *fm, feature_t f);
int HAS_FEATURE(const feature_map_t *fm, feature_t f);
*/

/** P2P发送统计数据结构 */
struct TransStatUser {
    short id;           ///< 用户(连接)标识
    short vchannel;     ///< 视频通道
    short vstream;      ///< 码流
    short is_igop;      ///< 1:统计间隔为一个I帧间隔; 0:周期约为~1s
    int nBytesInBuff;   ///< 位于发送缓冲区的字节数
    int nBytesSent;     ///< 已经发送的字节数
    int nBytesTotal;    ///< 总共收到的字节数
    int nBytesThrow;    ///< 丢掉的字节数。丢包会持续到到一个I帧，这期间即使网络恢复也会丢，所以其值只能作为参考, 不能用于计算网速
    int msStatInterval; ///< 统计周期(单位: ms)
};

/** sdk 回调函数结构 */
struct TciCB {
    /** 获取设备基本信息 */
    int (*get_info)(TCIDEVICEINFO *info);

    //int (*on_get_features)(feature_map_t *features);

/** 设备能力. 参见 @ref DeviceFeatures
    \param key 能力名
    \param buf 返回能力描述的缓冲区
    \param size 缓冲区大小
    \retval 0 ok
    \retval <0 failed(not recognize)
*/
    int (*get_feature)(const char *key, char *buf, int size);

    /** 取设备当前状态或初始默认设置.
     *
     * \param key 状态名
     *  \li \c CVideoQuality 云录像清晰度. 格式: <tt>stream:channel</tt>
     *  \li \c streamQuality 实时视频默认码流. 格式: <tt>stream:channel</tt>
     *      - \c stream 码流: 0-高清; 1-标清
     *      - \c channel 视频通道号: 0, 1, ...
     *
     *  \li \c mac 设备mac地址, null结尾的12个十六进制字符
     *  \li \c versions 版本号的json数组。格式: ["main_ver", "sub_ver1", ...]. 版本号为8个数字级成的字符串如"01000208"(=1.0.2.8)
     *
     * \param buf 返回值缓冲区
     * \param size 缓冲区大小
     *
    */
    int (*get_state)(const char *key, char *buf, int size);

    /** 直连模式下用户登录帐号检查
     * \param user 用户名, 目前未使用
     * \param key 用户密码加密
     * \retval 1 验证成功
     * \retval 0 失败
     */
    int (*on_apmode_login)(const char *user, const char *key);

/** 
 * @name OTA接口
 * @{ 
 */
    /** OTA升级开始
     * \param new_version 升级包版本
     * \param size 升级包大小
     * \return 0 升级继续; -1 升级中止
     */
    int (*on_ota_download_start)(const char *new_version, unsigned int size);

    /** 升级包数据
     * 升级包下载过程中,sdk调用此回调将数据传给应用层
     * \param buff 数据
     * \param size 数据长度
     * \return 0 继续; -1 传输中止
     */
    int (*on_ota_download_data)(const uint8_t *buff, int size);

    /** 升级包下载结束
     * \param status 完成状态: 0-ok, 可以执行升级; 1-下载中断; 2-数据校验错
     * \retval -1 应用中止升级过程
     * \retval 0 如果应用可以立即执行升级，可以返回0表示升级成功
     * \retval 1 如果应用要退出进程再升级，返回1通知APP等待.
     * @note 因为返回0/1后应用会退出，不保证APP端一定能收到应答
     */
    int (*on_ota_download_finished)(int status);
/**@}*/


/** 
 * @name 二维码扫描
 * @{ 
 */
    /** 二维码扫描开始
     * 应用层在此开启取Y数据的通道
     * \return 0 ok; -1 failed
     */
    int (*qrcode_start)(void);

    /** 获取Y图片
     * \param[in,out] ppYBuff
     * \param[out] width 返回图片宽度
     * \param[out] height 返回图片高度
     * \return 0 ok; -1 停止扫描
     * @note 第一次调用时 *ppYBuff为NULL. 应用层在此分配一个缓冲区, 填入Y数据，并把缓冲区指针保存到*ppYBuff.  
     *       之后的调用 *ppYBuff为之前设置的值
     */
    int (*qrcode_get_y_data)(uint8_t **ppYBuff, int *width, int *height);

    /** 结束二维码扫描
     * \param ppYBuff *ppYBuff中为最后一次在on_get_y_data中返回的值
     * @note 应用层在此释放前面分配的内存(free(*ppYBuff)), 关闭Y通道
     */
    void (*qrcode_end)(uint8_t *pYBuff);
/**@}*/

    /** 抓取图片. 目前只支持jpeg格式
     * \param type @ref pic_type "图片类型" in TciCloudConst.h 
     * \param ppJgp *ppJpg为图片地址或用于返回图片的址.见note
     * \param size  返回时*size为图片字节数
     * \return 0 ok; @ref TCE_BUFFER_TOO_SMALL; -1 其它错误
     * @note 如果*ppJpg为NULL，应用层分配图片空间, 空间指针和图片大小分别保存在*ppJpg和*size. sdk内部负责释放空间。
     *       如果*ppJpg为非空，*ppJpg为SDK内分配的空间地址，*size为空间大小。返回时*size为实际图片大小
     */
    int (*snapshot)(int type, uint8_t **ppJpg, int *size); //??

    /** 设置wifi参数
     * \param is_switching 0:配网; 1:添加成功后修改wifi
     * \return 0 配置ok; -1 配置不对
     * \note 返回时, wifi处在station模式
     */
    int (*set_wifi)(int is_switching, const char *ssid, const char *key);

    /** 设置时区 
     * \param tzs 时区字符串. Refer to man page of tzset().
     */
    int (*set_timezone)(const char *tzs);

    /** 设置时间
     * \param time utc 时间
     */
    int (*set_time)(time_t time);

    /** sdk内部状态
     * \param status 状态. Refer to @ref status_code "Status Code" in TgCloudConst.h
     * \param pData 状态相关数据
     * \return  返回值同event的值有关. 请查看 @ref status_code 。默认返回0
     */
    int (*on_status)(int event, const void *pData, int len);


/** 
 * @name 对讲
 * @{ 
 */
    /** 开始对讲
     * \return 0 ok; -1 中断
     */
    int (*on_talkback_start)(void);

    /** 对讲数据回调
     * 格式在前面已经协商过
     * \return 0 if played
     */
    int (*talkback)(TCMEDIA at, const uint8_t *audio, int len); 

    /** 结束对讲 */
    void (*on_talkback_stop)(void);
/**@}*/

    /** @deprecated 
     * 请求I帧. 请使用 request_iframe_ex
     * \param vstream 视频流. 0-主码流; 1-辅码流
     * \return 0
     */
    int (*request_iframe)(int vstream);

    /** 当AI识别成功，通过本回调通知应用.  (本回调未使用，设为NULL)
     * \param type \ref pic_type "图片类别指示"
     * \param pJpg 正识别图片
     * \param len　图片长度
     */
    void (*ai_result)(int type, const uint8_t* pJpg, int len);

    /** 在平台端采集设备日志
     * \param action 
     *          0-开始长日志采集; 
     *          1-停止长日志采集; 
     *          2-获取日志文件
     * \param logfile NULL if action is 0 or 1, 当action为2时，应用层填入日志文件路径
     * \return  0:ok; -1:failed
     */
    int (*log)(int action, char *logfile);

    /** 请求指定图像通道的I帧
     * \param channel 图像通道
     * \param vstream 视频流. 0-主码流; 1-辅码流
     * \return 0
     */
    int (*request_iframe_ex)(int channel, int vstream);

    /** 网络传输统计回调 */
    void (*trans_stat)(const struct TransStatUser *_stat);

    /** 切换图像质量
     * @param channel 图像通道。目前为0
     * @param stream 码流 0,1,..., 在"Resolutions"能力中返回
     * @param qstr   质量描述串, 例如 HD/SD/FHD/..., 在"Resolutions"能力中返回.
     */
    void (*switch_quality)(int channel, int stream, const char *qstr);
};

/** IPv4 地址表示(网络字节顺序) */
typedef struct Ipv4Addr {
    unsigned int ip;     ///< ip, network byte-order
    unsigned short port; ///< port, network byte-order
} Ipv4Addr;

/** Simple Buffer */
typedef struct SIMPLEBUFFER {
    uint8_t *data;  ///< 数据缓冲区指针
    int size;       ///< 缓冲区大小
    int len;        ///< 数据长度
} SIMPLEBUFFER;

/** 参数文件读写函数据指针结构 */
struct paramf_ops {
    /** 分配缓冲区，并读出参数文件.
     * @param buff 缓冲区结构指针. 由实现填充结构成员。缓冲区大小为参数文件大小加上 cbExtra
     * @param cbExtra 额外分配的字节数
     * @retval 1  ok
     * @retval 0  failed
     */
    int (*alloc_and_readall)(SIMPLEBUFFER *buff, int cbExtra);

    /** 写参数文件
     * @param buff 参数缓冲区指针
     * @retval 1 ok
     * @retval 0 failed
     */
    int (*save_buff)(const SIMPLEBUFFER *buff);

    /** 释放参数缓冲区 */
    void (*free_buff)(SIMPLEBUFFER *buff);

    /** 删除参数文件(或者清空参数文件)。下一次读出时 len为0 */
    void (*remove)(void);
};

/** 用户文件下载目的地. 
 * @ref TciGetUserFile() */
union TgfDest {
    /** 数据下载到文件 */
    struct to_file {
        const char *path;  ///< IN: file path
    } file;                ///< 文件
    /** 数据下载到内存 */
    struct to_mem {
        void *mem;  ///< OUT: 指向sdk分配的内存
        int size;   ///< OUT: 数据长度
    } mem;          ///< 目标
};


/**@}*///{group api_structure}

/** \defgroup api_reference API参考
 * 通用的API参考
 * @{*/

/** 获取SDK版本号
 *  TciInit()之后调用
 *  \return SDK版本号字符串。如："rev. nn"
 */
const char *TciGetVersion(void);

/** 配置基本参数
 *  在TciStart()之前调用
 *  \param path 用于保存sdk私有数据的目录, 需要有读写权限
 *  \param uuid 设备id. 新的id参数为 "{uuid},{key}" 的形式, 烧号时由服务器分配
 *  \return 0
 */
int TciInit(const char *path, const char *uuid);

/** 预分配云存储帧缓存空间.
 *  本功能用于对上传时间敏感的事件唤醒类设备录像。
 *  可以在网络初始化前就调用本接口，之后调用 TciSendFrameEx()就会向缓冲写入帧。
 *  设备TciStart()成功后调用TciSetEventEx()，已经缓存的帧得以上传。
 *
 *  不调用本接口，TciStart()内部会在设备注册成功后执行同样功能.
 *
 * \param uCloudBuffSize 云存储缓冲区大小(单位：字节)，最小1M
 *
 * @return 内存不足时返回-1, 否则返回0
 *
 * @note TciStart()会通过回调修改系统时间，设备要保证 TciStart()前后帧的时间戳时连续的。 \n
 *      * 要在 TciInit() 之后调用
 *      * 要在调用前从RTC恢复系统时间和设置时区
 */
int TciAllocCloudBuffer(unsigned int uCloudBuffSize);

/** 启动服务
 * @pre 调用本接口前需要保证网络已经配置好
 * 本调用阻塞直到内部完成初始化或操作失败
 * \param isBound 设备本地记录的绑定状态. 复位或在TciCB::on_status()里 @ref STATUS_DELETED 时清除。收到 @ref STATUS_LOGON 时设置并保存
 * \param uCloudBuffSize 云存储缓冲区大小(单位：字节)，最小1M
 * \retval 0 启动成功
 * \retval others. Refer to @ref error_code "错误码"
 * @ref TgCloudConst.h
 */
int TciStart(int isBound, unsigned int uCloudBuffSize);

/** 启动服务. 用于 TciInit() 没有传 uuid 参数的情形
 * @param uuid  设备id. 新的id参数为 "{uuid},{key}" 的形式, 烧号时由服务器分配
 */
int TciStart2(int isBound, unsigned int uCloudBuffSize, const char *uuid);

/** 启动服务
 *  这个接口用于工厂生产测试烧号后立即启动服务。烧号前调用TciInit()时uuid为NULL, 烧号后可以在这里传入uuid。
 */
int TciStart2(int isBound, unsigned int uCloudBuffSize, const char *uuid);

/** 停止服务
 *  @pre TciStart() 返回0
 *  @return 0(成功), 或 @ref TCE_SERVER_IS_DOWN, @ref TCE_INVALID_UUID
 *  @ref TgCloudConst.h
 */
int TciStop(void);

/** 释放资源
 * @pre TciStop()
 * @return Always return 0
 */
int TciCleanup(void);

/** 注册回调 -- sdk内部事件
 * \param cb 非局部TciCB结构指针
 * \return 0
 * @note 在 TciInit() 前调用
 */
int TciSetCallback(const struct TciCB *cb);

/** 注册通用命令回调
 * 在回调里处理APP发来的命令请求
 * @note 在 TciInit() 前调用
 */
int TciSetCmdHandler(const TGCMDHANDLER cb);

/** 发送p2p命令应答
 * @param handle 连接句柄，由 TciSetCmdHandler()设置的回调被调用时会收到此句柄
 * @param cmd 命令标识
 * @param data 指向命令参数
 * @param dataSize 参数长度
 * @return 0 或 \ref error_code "错误码"
 */
int TciSendCmdResp(p2phandle_t handle, unsigned int cmd, const void *data, int dataSize);

/** 发送一个 Tcis_ErrorResp 结构作应答.
 * 这个应答用于只需要简单地通知客户端命令执行成功或失败的情形.
 * @param handle P2P句柄
 * @param cmd    命令标识
 * @param status @ref TCI_OK 或 TCI_E_xxx, 参见 @ref generic_err_code "通用错误码"
 * @return 0 或 \ref error_code "错误码"
 */
int TciSendCmdRespStatus(p2phandle_t handle, unsigned int cmd, unsigned int status);

/** 回放时发送数据帧.
 * @param handle 连接句柄
 * @param id_mt 低16位:媒体数据类型 @ref TCMEDIA ; bit16:子画面标志
 * @param frame 帧数据指针
 * @param len 帧数据长度
 * @param timestamp 帧的时间戳
 * @param uFrameFlags 视频:关键帧标志(1:关键帧;0:非关键帧); 音频: @ref audio_sample_fmt "音频采样格式"
 * @retval 0 ok
 * @retval TCE_NETWORK_BUSY @ref TCE_NETWORK_BUSY :网络拥堵，应用层要延迟一会儿(例如300ms)后重新发送
 * @retval -1 其它错误，通常意味着连接无效
 * @note  每次设备在文件内或文件间重新定位, 或者自动跳到下一个文件，要通过 TciSendPbSyncFrame 先发一个绝对时间同步帧.
 *
 *  @see TciSendPbSyncFrame
 */
int TciSendPbFrame(p2phandle_t handle, uint32_t id_mt, const uint8_t *frame, int len, uint32_t timestamp, int uFrameFlags);

/** 在回放流中插入消息. 
 * @param handle  p2p连接句柄。
 * @param type    消息类型
 * @param data1   @ref RTMSGHEAD_t::data1
 * @param data2   @ref RTMSGHEAD_t::data2
 * @param data_len 额外数据长度 @ref RTMSGHEAD_t::frame_size
 * @param data    指向额外数据
 */
int TciSendPbMessage(p2phandle_t handle, RTMTYPE type, unsigned int data1, unsigned int data2, const void *data, int data_len);

/** @def TciSendPbSyncFrame
 * 发送回放时间同步帧
 * @param handle p2p 连接句柄
 * @param utc_time 下一帧的生成时间(utc, 单位s)
 * @param is_response_to_PLAY_START  1:响应 @ref TCI_CMD_RECORD_PLAYCONTROL; 0:自动跳到下一个文件
 */
#define TciSendPbSyncFrame(handle, utc_time, is_response_to_PLAY_START) TciSendPbMessage(handle, is_response_to_PLAY_START?RTM_SYNCTIME_RESPONSE_TO_USER:RTM_SYNCTIME, 0, utc_time, NULL, 0)

/** @def TciSendPbTimelapseFlag
 * 发送缩时录像回放标志。在进入和退出时发送
 * @param handle p2p 连接句柄
 * @param flag 1:进入缩时回放; 0:退出缩时回放
 */
#define TciSendPbTimelapseFlag(handle, flag) TciSendPbMessage(handle, RTM_TIME_LAPSED, flag, 0, NULL, 0)

/** @def TciSendPbEndOfEvent
 * 在事件回放结束时发送此标志通知App. \b 必需
 */
#define TciSendPbEndOfEvent(handle) TciSendPbMessage(handle, RTM_END_OF_EVENT, 0, 0, NULL, 0)

/** 开始配置WIFI.
 * 支持AP热点或设备扫描手机上的二维码的方式配置WIFI. AP热点名称为 "AICAM_{uuid}" 的形式. \n
 * 该调用会一直阻塞直到获得了正确的wifi配置. 
 * \param mode 配网方式. @ref GWM_QRCODE, @ref GWM_AP 的组合。
 */
int TciConfigWifi(int mode);

/** 在sdk外部扫二维码. 调用此接口验证
 */
int TciCheckRegString(const char *s);

/** 仅停止二维码扫描 */
int TciStopConfigWifi(void);

/** 保存wifi注册信息.
 * 用于wifi模块从AP模式切换到STA模式必须重启的情形。 \n
 * @return 要保存的数据的地址。 *len 为要保存的数据的长度
 *
 * @note 在 TciConfigWifi()返回后调用 TciGetRegisterConfiguration() 操作并且保存返回的数据。\n
 * 重启后不调用 TciConfigWifi(), 而是用保存的内容调用 TciSetRegisterConfiguration()。
 * 然后执行 TciStart() 完成注册过程。
 */
const void *TciGetRegisterConfiguration(int *len);

/** 恢复wifi注册信息.
 * \return 非0表示成功
 */
SA_BOOL TciSetRegisterConfiguration(const void *data, int len);

/** 发送0通道实时音视频帧. SDK内部会将数据分发到云端和APP.
 * @param stream 视频流. 0: 高清; 1: 标清
 * @param mt 媒体类型
 * @param pFrame 指向数据帧。视频要带4个前导字节 00 00 00 01
 * @param length 数据帧长度
 * @param ts timestamp. 单位为ms
 * @param uFrameFlags 视频:关键帧标志(1:关键帧;0:非关键帧); 音频: @ref audio_sample_fmt "音频采样格式"
 */
int TciSendFrame(int stream, TCMEDIA mt, const uint8_t *pFrame, int length, uint32_t ts, int uFrameFlags);


/** 发送实时音视频帧. SDK内部会将数据分发到云端和APP.
 * @param channel Channel of video: 0|1|...。 如果是 "n-composed"类型的设备，channel 0为默认预览通道
 * @param stream 视频流. 0: 高清; 1: 标清
 * @param mt 媒体类型
 * @param pFrame 指向数据帧。视频要带4个前导字节 00 00 00 01
 * @param length 数据帧长度
 * @param ts timestamp. 单位为ms
 * @param uFrameFlags 视频:关键帧标志(1:关键帧;0:非关键帧); 音频: @ref audio_sample_fmt "音频采样格式"
 */
int TciSendFrameEx(int channel, int stream, TCMEDIA mt, const uint8_t *pFrame, int length, uint32_t ts, int uFrameFlags);

/** 告诉SDK下一个视频关键帧的实际发生时间(UTC).
 * @param channel/stream  云存的通道和码流
 * @param t 下一个关键帧的发生时间
 * @note 仅用于(低功耗设备)在应用启动前缓存数据的情形. 普通情况下以调用 TciSendFrameEx()时的时间作为帧的发生时间. \n
 *       这个时间用于确定云存储的开始时间和回放定位. \n
 *       固件在发送视频关键帧前调用这个接口
 */
int TciSetKeyVideoTime(int channel, int stream, time_t t);

/** 发送实时流消息. 
 * @param channel 通道。为-1时在全部通道上发送
 * @param stream  码流。为-1时在全部码流上发送
 * @param type    消息类型. 参见 @ref RTMTYPE
 * @param data1   消息相关数据
 * @param data2   消息相关数据
 * @param data_len 额外数据长度 @ref RTMSGHEAD_t::frame_size
 * @param data    指向额外数据
 */
int TciSendLiveMessage(int channel, int stream, RTMTYPE type, unsigned int data1, unsigned int data2, const void *data, int data_len);

/** 发送预置位到位通知 */
#define TciSendLiveMsg_ReachPsp(channel, psp_num) TciSendLiveMessage(channel, -1, RTM_REACH_PSP, channel, psp_num, NULL, 0)

/** 多目摄像机镜头切换时发送的标志帧, 要求在新镜头的第一帧前发送 */
#define TciSendLiveMsg_LensSwitch(channel, stream) TciSendLiveMessage(channel, stream, RTM_LENS_SWITCH, channel, stream, NULL, 0)

/** @deprecated 发送镜头切换通知, 旧的名字 */
#define TciSendLensSwitchFlag(channel, stream) TciSendLiveMsg_LensSwitch(channel, stream)

/** 发送通知.
 * @param handle 指定连接。如果为NULL，则发广播.
 * @param type 消息类型
 * @param type    消息类型. 参见 @ref RTMTYPE
 * @param data1   消息相关数据
 * @param data2   消息相关数据
 * @param data    指向额外数据
 * @param data_len 额外数据长度
 */
int TciSendRtMsg(p2phandle_t handle, RTMTYPE type, unsigned int data1, unsigned data2, const void *data, int data_len);

/** 上报事件。本版本允许附带事件特定参数.
 * @param evtp 事件参数指针
 * @return 0:事件已经排队等待发送; non-zeor:错误码
 * @note 本操作是异步的，返回0后要求evtp->jpg_pic仍然有效。 \n
 *       如果evtp->evtp_flags 最低位非0, 上传结束后sdk会释放空间. \n
 *       evtp->evtp_flags最低位为0，用于jpg_pic为全局空间的情形. \n
 *       返回非0时，调用者要自己释放图片空间.
 */
int TciSetEventEx(EVENTPARAM *evtp);

/** 停止事件, 仅少数事件才有意义.
 * @param evt ECEVENT_DOORBELL 为取消呼叫.
 * @note 取消呼叫时, sdk内部自动取呼叫的时间
 */
int TciStopEvent(ECEVENT evt);

/** @deprecated 开始事件的云录像
 * @param event 事件类型
 * @return 0
 * @note 如果不调用 TciSetRecordStop(), sdk内部在30"后自动结束录像。 \n
 *       如果事件一直持续, 应用层要在30"内再次调用本接口。
 *
 * @note 本接口不再需要。sdk内部会自动开始和停止云录像
 */
int TciSetRecordStart(ECEVENT event);

/** @deprecated 云事件录像停止
 * @param event 事件类型
 * @return 0
 *
 * @note 本接口不再需要。sdk内部会自动开始和停止云录像
 */
int TciSetRecordStop(ECEVENT event);

/** 绑定应用私有数据指针到p2p handle
 * @param[in] handle P2P句柄
 * @param[in] pUser 应用私有数据指针
 */
void TciSetSessionUserData(p2phandle_t handle, void *pUser);

/** 返回由TciSetSessionUserData()设置的应用私有数据指针
 * @param[in] handle P2P句柄
 * @retval 返回P2P句柄绑定的应用私有数据，没有则返回NULL
 */
void *TciGetSessionUserData(p2phandle_t handle);

/** 控制日志输出. 应该在TciInit()前调用 
 * @param b_output_to_console 是否输出到控制台
 * @param log_path log文件路径。应该是一个全局或静态的字符串指针. 为NULL时不创建日志文件
 * @param max_log_size 日志文件最大尺寸(单位:byte)
 */
void TciSetLogOption(int b_output_to_console, const char *log_path, int max_log_size);

/** 设置SDK日志输出等级.
 * @param level  3:Information(default); 5:verbose; 6:debug
 */
void TciSetLogLevel(int level);

/** @deprecated
 * 上报4G设备信息. 请使用 TciReport4GInfoEx()
 * @param imei
 * @param iccid
 * @param state G4STATE_xxx
 * @return 0:成功; <0:错误码
 */
int TciReport4GInfo(const char *imei, const char *iccid, ECG4STATE state);

/** sim 卡类型 */
typedef enum {
	SIMT_PHY, ///< 物理卡
	SIMT_ESIM, ///< ESIM
	SIMT_VSIM ///< vsim卡
} SIMCARDTYPE;

/** 4G 设备信息 */
typedef struct TG4GINFO {
	char imei[24]; ///< 4g模块标识
	char iccid[24]; ///< sim卡号
	SIMCARDTYPE sim_type; ///< sim卡类型
} TG4GINFO;

/** 上报4G设备信息，代替 TciReport4GInfo()
 * @param info 指向4G信息结构
 * @param state 4G联网状态
 * @return 0:成功; <0:错误码
 */
int TciReport4GInfoEx(const TG4GINFO *info, ECG4STATE state);

/** @name 休眠准备
 * @{*/
/** 准备休眠
 * 本api建立到唤醒服务器的tcp连接，并且返回连接数.
 * 调用者对返回的socket调用getpeername()/getsockname()取得与唤醒服务器的连接信息，
 * 并使用此信息配置wifi模组，然后对主芯片下电。在模组里每50"向唤醒
 * 服务器发送64字节的心跳包，包内容任意。
 *
 * 注意: 设备如果要进入休眠，应用层不能关闭返回的socket.
 *
 * @param socks 接收连接socket
 * @param size  socks数组大小
 * @retval >=0 number of sockets connected to wakeup-servers
 * @retval <0 failed
 */
int TciPrepareHiberation(int *socks, int size);

/** 获取唤醒服务器ip地址
 * @param servers 接收服务器地址。最多返回3个地址
 * @return 服务器个数
 */
int TciGetWakeupServers(Ipv4Addr servers[3]);


/** 生成唤醒服务器登录命令.
 * 本接口用于到唤醒服务器的tcp连接不由主机创建的情形.
 *
 * @param randKey 服务器生成的随机串
 * @param len_of_authstring 登录命令长度
 * @return 登录字节串指针。如果失败返回 NULL
 *
 * @note 应用要按以下步骤建立到唤醒服务器的有效连接:
 *   1. 应用调用 TciGetWakeupServers() 获取唤醒服务器的地址
 *   2. 通信模块建立到唤醒服务器的连接, 接收由服务器返回的randkey
 *   3. 应用使用此randkey调用本接口
 *   4. 通信模块向服务器发送本接口生成的认证字符串
 *   6. 主控下电. 通信模块每50"向服务器发送64字节的心跳包，包内容任意。
 */
const uint8_t *TciPrepareAuthString(const char *randKey, int *len_of_authstring);
/**@}*/

/** @name 云存储自动补录 
 * @{*/
/** 设置云存储后备存储目录和缓存策略.
 * 当设置后备存储目录后，SDK会利用此目录暂存上传失败的录像和图片,
 * 并在合适的时机重传. \n
 * 本功能要在 TciStart() 前调用
 * @param sd_path 用于后备存储的目录
 * @param buffer_hint 内存缓存策略.
 * @retval 0
 * @note 缓存策略影响用于云存储的内存大小。参看 @ref ECBUFFERHINT 的说明
 */
int TciSetBackStore(const char *sd_path, ECBUFFERHINT buffer_hint);

/** 允许或禁止后备上传.
 * @param en 1:enable; 0:disable
 * @note 默认由sdk根据网络情况决定上传时间。如果应用层要临时禁止上传，可调用此接口
 */
void TciBackStoreEnableUpload(int en);

/** 这个接口用于sd卡格式化前释放backstore, 和格式化后重新开启后备存储
 * @param en 0-释放; 非0-重新打开
 */
void TciBackStoreEnable(int en);
/**@}*/

/** 上报GPS信息
 * @param time 当前时间
 * @param longitude 当前经度值(单位:度). >0 东经，<0 西经
 * @param latitude 当前纬度值(单位:度).  >0 北纬，<0 南纬
 * @param speed 速度 km/h
 * @param angle 对正北方向的夹角: 0~359
 * @param signal_strength 0:未知(忽略); 1:弱; 2:中; 3:强
 * @return 0:ok; non-zero: error code
*/
int TciReportGpsInfo(const unsigned int time, const double longitude, const double latitude, 
        double speed, int angle , int signal_strength);

/** 开启p2p功能，进入厂测状态.
 * @param key_path  key 文件路径
 * @retval 0  ok
 * @retval -1 failed
 */
int TciStartInTestMode(const char *key_path);

/** 上报电池状态
 * @param qoe 电池电量百分比(0~100); -1:未知
 * @param qoe_low
 *      - 1: 电量低;
 *      - 0: 电量正常;
 *      - -1:未知
 * @param charging 1-正在充电; 0-放电状态
 * @return 0-成功; 非0-错误码
 */
int TciReportBatteryStatus(int qoe, int qoe_low, int charging);

/** 设置云存储录像通道的码流
 * @param channel 视频通道
 * @param stream 码流: 0-主; 1-辅
 * @note 本接口拟用于画中画设备. \n
 *      当收到 @ref TCI_CMD_SET_PRIMARY_VIEW 命令时，设备可能希望云录像通道与实时画面一致，就可以调用此接口.  \n
 *      非此情形，用户可以通过App发送 @ref TCI_CMD_SET_CLOUD_VIDEO_QUALITY_REQ 来显式设置云录像通道和码流。这一命令在sdk内部处理, 不会回调给应用层.
 */
void TciSetCloudStream(int channel, int stream);


/** @name 用户定义云上传
 * @{*/
/** 开始一个用户定义上传.
 * 自定义上传主要用于sdk本身预录时间不足或不能录像，要从SD卡录像上传的情形. \n
 * 必须在 TciStart() 成功后调用
 *
 * @param evt 事件类型. @ref ECEVENT (作为参数传入时要强传为 const char *型), 或长度小于16的字符串。 \n
 *            目前仅支持行车记录仪的事件(@ref ECEVENT_DR_BEGIN ~ @ref ECEVENT_DR_END) \n
 *            或者 TciCB::on_status(@ref STATUS_SDER, SDER *, ...) 回调的平台定义事件( SDER::event )
 * @param tRecordStart 录像起始时间
 * @param jpg_pic 图片数据(如果有的话)指针(SDK不会take图片缓冲区的所有权)
 * @param pic_len 图片数据长度
 * @return 如果允许上传，返回一个用户上传句柄。否则返回NULL
 * @note 用户自定义上传只能是事件上传，长度限制在30秒内. \n
 *       自定义上传的是事后发觉的事件，这个时候录像已经保存到sd卡了。\n
 *       自定义上传的时间范围不能与TciSetEvent 的时间重叠，也不能与另一个自定义事件的时间范围重叠
 */
void *TciUduBegin2(const char *evt, time_t tRecordStart, const char *jpg_pic, int pic_len);
#define TciUduBegin(evt, tRecordStart) TciUduBegin2(evt, tRecordStart, NULL, 0)

/** 自定义上传过程，写入帧.
 * @param hUdu TciUduBegin() 返回的上传句柄
 * @param mt 媒体类型. 参见 TciSendPbFrame() 的 id_mt 参数
 * @param ts 时间戳
 * @param pFrame 帧数据指针
 * @param size 帧数据大小
 * @param uFrameFlags 视频:关键帧标志(1:关键帧;0:非关键帧); 音频: @ref audio_sample_fmt "音频采样格式"
 * @return 0:ok; 非0:录像长度超限，用户要调用 TciUduEnd()
 */
int TciUduPutFrame(void *hUdu, TCMEDIA mt, uint32_t ts, const uint8_t *pFrame, int size, int uFrameFlags);

/** 结束自定义上传
 * @param hUdu TciUduBegin() 返回的上传句柄
 */
void TciUduEnd(void *hUdu);
/**@}*///name 用户定义云上传

/** 上报错误 */
int TciReportError(const char *name, const char *detail);

/** 用于选择调试环境
 * @param env 1:测试环境; 0:正式环境
 * @note 在 TciStart() 之前调用
 */
void TciSelectEnvironment(int env);

typedef void (*TCIONLIVEFRAMECB)(int channel, int stream, TCMEDIA mt, const uint8_t *pFrame, uint32_t size, uint32_t ts, int uFrameFlags);
void TciRegisterLiveFrameCB(TCIONLIVEFRAMECB cb);
void TciUnregisterLiveFrameCB(TCIONLIVEFRAMECB cb);

/** 设置参数文件操作指针.
 * SDK会保存工作参数到flash. 小系统设备在flash上没有标准文件I/O支持时，需要提供载入和写参数的操作。本操作要在 TciInit() 之前调用 \n
 * linux和有文件系统的liteos不需要设置
 * @param ops 参数文件操作结构指针
 */
void TciSetParamFileOps(struct paramf_ops *ops);

/** 设置最大p2p连接数.
 * @param num 要设置的最大连接数
 * @return 最大连接数有效值
 * @note 只能在 TciStart()和TciConfigWifi()前调用
 */
int TciSetMaxP2pClientsNum(int num);

/** 低功耗设备的工作模式 */
typedef enum {
    PM_ALLON,        ///< 常电模式。非低功耗设备的默认模式
    PM_SLEEPABLE,    ///< 系统可休眠(会收到STATUS_IDLE状态回调。这是低功耗设备的默认模式)
    PM_NETDOWN       ///< 主控正常工作，网络模块进入休眠模式
} PMODE;

/** 设置低功耗设备的工作模式.
 *
 * @note 需要在TciStart()之后调用. 调用逻辑一定参看文档 @ref a21_ManuallyPowerManagement
 */
void TciSetPowerMode(PMODE mode);

/** 从NETDOWN模式下恢复网络后，调用本api使p2p快速上线 */
void TciFastRecoverP2p(void);

/** 上报锁操作日志
 * \param log 日志文本，格式见注释
 *
 * @note The \c log buffer contains "\n" seperated lines. each line contains time, optional user id, and event. like:
\code
[2021.01.03 02:00:00] [001] open the door
[2021.01.03 05:30:20] password error
\endcode
The second [...], if presents, contains user id 
*/
int TciUploadOpLog(char *log);

/** 门锁事件上报。图片和录像都在内部处理了.
 * 参数见外部文档
 */
int TciSetLockEvent(int cls, int msg, int usrtyp, int usrid);
int TciSetDoorbellEvent();

char *TciGetDeviceInfoString(void);

/** 下载升级包
 * @param subdev NULL或子设备id. 用于选择相应的固件
 * @return 0:ok; <0:error code
 */
int TciUpgradeOnTheAir(const char *subdev);

/** 设备升级包信息 */
typedef struct DEVICEOTAINFO {
	int size; ///< 升级包大小
	char ver[16]; ///< 升级包版本
	char md5[36]; ///< md5 检验字符串
	char url[256]; ///< 下载地址
} DEVICEOTAINFO;

/** 查询升级包.
 * @param uuid
 * @param ota  返回升级包信息
 * @note 先要调用 TciStart()
 */
int TciQueryForOta(const char *uuid, DEVICEOTAINFO *ota);

/** 查询第三方升级包.
 * @param fw_id  第三方设备的 firmware id
 * @param fw_ver 第三方设备的当前版本号。版本号在平台上是以8位数字("01010101")的方式记录.  \n
 *               如果第三方固件的版本有不同表示方式，需要在上传固件时自行维护一个映射关系.
 * @param ota    返回升级包信息
 * @return  0:查询成功, 是否有可用升级包看 *ota 内容; <0:错误码
 * @note 先要调用 TciStart()
 */
int TciQueryForOta2(const char *fw_id, const char *fw_ver, DEVICEOTAINFO *ota);

/** 报告系统运行时状态
 e8 @param state  状态名. @ref ENUMRTSTATE
 * @param pData  state 相关参数。具体见
 */
void TciSetRtState(ENUMRTSTATE state, void *pData);

/** 通用sdk工作选项设置接口.
 * @param opt <a href="#sdk_opt_consts">选项名</a>
 * @param pVal 传入选项值
 * @return >=0:ok; <0:错误
 */
int TciSetSysOption(int opt, const void *pVal);

/** 通用sdk工作选项获取接口.
 * @param opt <a href="#sdk_opt_consts">选项名</a>
 * @param pVal 返回选项的值
 * @return >=0:ok; <0:错误
 */
int TciGetSysOption(int opt, void *pVal);

/** @anchor sdk_opt_consts */
/** @name SDK选项常数 
 * @{*/

/** 有人观看时禁止上传
 * 类型: int*
 * - 1 disable
 * - 0 Allow parallel operation
 */
#define TCOPT_DISABLE_UPLOAD_WHEN_VIEWING   1

/** 云存储缓冲文件数.
 * 类型: int*: 1~6
 */
#define TCOPT_BUFFER_QUEUE_SIZE             2

/** 传输内容 */
typedef enum ETRANSSERVICE {
    TRANS_SERVICE_LIVE_AV   = 0,
    TRANS_SERVICE_AUDIO_PIC = 1,
    TRANS_SERVICE_PIC_ONLY  = 2,
    TRANS_SERVICE_MAX
} ETRANSSERVICE;
/** 传输内容
 * 类型: int*: 
 *  - 0  live video/audio
 *  - 1  picture+audio
 *  - 2  picture only. 
 *  @ref ETRANSSERVICE.
 */
#define TCOPT_TRANS_MEDIA_TYPE              3

/** tcp连接方式
 * 类型 int*:
 *  - 0 default
 *  - 1 并发
 */
#define TCOPT_TCP_CONNECTOR                 4

/** backstore 存储天数
 * 类型 int*:
 *  - 0 同云服务保存天数
 *  - >0 自定义的存储天数.不超过服务规定的天数
 */
#define TCOPT_BACKSTORE_SAVEDAYS            5

/** 最大云存data文件。超出的帧会丢弃.
 * 类型: int*
 */
#define TCOPT_MAX_DATAFILE_SIZE             6

/** 仅上报AI事件.
 * 类型: int*
 */
#define TCOPT_ONLY_REPORT_AI_EVENT 7

/** 用户没有应答不能拉流. 用于楼宇对讲
 *  类型: int *
 */
#define TCOPT_NO_STREAMING_WITHOUT_ACCEPTING_CALL 8
/**@}*/

/** 设置低功耗设备唤醒原因, 设备进入休眠时会上报给服务器.
 * @param t_wakeup 唤醒时间
 * @param r <0:异常唤醒; 0:远程唤醒; >0:唤醒事件类型(ECEVENT)
 * @param user_reason 当r=ECEVENT_USER_DEFINED, user_reason为用户定义的唤醒理由字符串,长度不超过15个字符. \n
 *        其它情形下传NULL
 * @param sig_lvl wifi或4G信号强度: 0~100. 传负数时由用户自己解释
 * @param sdc_rec SD卡录像长度
 * @note  要在 STATUS_IDLE 返回0的时候调用，因为随此上报的还有别的信息。
 */
void TciSetWakeupReason(time_t t_wakeup, int r, const char *user_reason, int sig_lvl, int sdc_rec);

/** 设备端准备强制休眠.
 * 本操作会先通知实时用户(app端收到通知后要停止视频)，然后停止云上传、退出p2p、断开长连接等. \n
 * 返回后设备可以立即下电。
 * @param bDiscardFilesInQueue 是否放弃上传队列中未处理的文件.  \n
 *        如果为 SA_FALSE, 会尝试上传队列中的所有文件. \n
 *        建议在开启了后备存储并且存储路径有效时 SA_TRUE，这样在网络不好，队列中文件多时可以加速返回. \n
 *        其它情况下的设置由应用自行载定.
 */
int TciForceSleeping(SA_BOOL bDiscardFilesInQueue);

/** 发送用户自定义数据到第三方平台.
 * 需要平台间对接
 */
int TciSendUserData(const unsigned char *data, int len);

/** 下载用户文件.
 * @param name  文件上传到服务器端时确定的文件标识
 * @param bToFile 1:下载到文件; 0:下载到内存
 * @param dest 下载到文件时，设置 dest->file.path 为本地文件路径; 下载到内存时，内容在dest->mem中返回
 */
int TciGetUserFile(const char *name, SA_BOOL bToFile, union TgfDest *dest);

/** @deprecated 设置p2p信息.
 * @param sss 从平台获取的p2p信息字符串
 * @return 0:ok; -1:内容错误
 * @note 在 TciStart()前调用
 */
int TciSetP2pInfo(const char *sss);

/** 处理从蓝牙收到的注册数据
 * @param data 命令0x8006的数据
 * @note 参看《蓝牙的应用层协议.md》文档
 * */
int TciProcessRegInfo(const void *data);

/** @}*///group api_reference

/** \defgroup register_in_mcu MCU配网api
 * @ingroup api_reference
 * 低功耗设备在MCU端进行配网
 *
 * @{*/
/** 获取设备属性字符串。调用者把该字符串传给MCU，用于在MCU端激活设备.
 *  调用者要 free() 返回的字符串.
 */
char *TciGetActivationAttrs(void);

/** 返回要同步到MCU的数据.
  \param pLen 用于返回数据长度.
  \return 要同步到MCU的数据缓冲区地址。调用者要负责释放(free())
 */
unsigned char *TciGetPlatConfig(int *pLen);

/** 同步从MCU来的数据.
  \param pData 从MCU收到的同步数据
  \param len   数据长度
  \return 0:成功; <0:失败
 */
int TciSyncPlatConfig(const unsigned char *pData, int len);
/**@}*/// group register_in_mcu


/** \defgroup misc_tool_logfile 日志上传工具
 * \ingroup api_reference
 * 以查询的方式上传日志，主要用于低功耗设备
 * @{*/
/** 查询日志上传请求
 * @param uuid
 * @param model_id may be NULL
 * @param ppUrl 如果有请求，返回上传地址
 * @return 0:ok; none-zero: error code
 * @note 如果有请求，用返回的地址调用 TgPostMultiparts()
 */
int TgQueryUploadReq(const char* uuid, const char *model_id, char **ppUrl);

/**  传日志文件
 * @param url TgQueryUploadReq() 返回的上传地址
 * @param uuid
 * @param path 本地文件名
 * @param timeout 超时值(ms)
 * @return 0:ok; otherwise error
 */
int TgPostLogFile(const char *url, const char *uuid, const char *path, int timeout);

/** 上传内存中的日志
 * @param url TgQueryUploadReq() 返回的上传地址
 * @param uuid
 * @param mem 内存地址
 * @param size 数据长度
 * @param name 保存为文件名name
 * @param timeout 超时值(ms)
 * @return 0:ok; otherwise error
 */
int TgPostLogMem(const char *url, const char *uuid, const void *mem, int size, const char *name, int timeout);
/**@}*/

/** \defgroup misc 其它功能
 * \ingroup api_reference
 * @{*/
/** 设备上线后IP发生变化时通知SDK, 用于立即更新上连接 */
void TcnIpChanged();

typedef struct _IceConfig {
	char *ip;
	int port;
	char *username;
	char *password;
	int is_turn;
} ICECONFIG;

/** 开启本地工作模式.
 * 该模式用于仅在局域网工作的设备. 有需要先要同我们联系.
 * @param url
 * @param cfg webrtc 服务配置数组
 * @param size 数组大小
 * @return 0:成功; <0:错误码
 * @note 函数内部会复制cfg信息, 返回后用户可以释放cfg引用或间接引用的空间
 */
int TciLocalStart(const char *url, const ICECONFIG *cfg, int size);

/** @anchor sdk_state_flags */
/** @name SDK状态标志
 * @{*/
#define TGSS_F_CS_INSERVICE 0x00000001 ///< 云文件正在上传或者新文件正在打包
#define TGSS_F_P2P_CONNECTED 0x00000002 ///< 有用户连接
#define TGSS_F_P2P_STREAMING 0x00000004 ///< 用户正在拉流
#define TGSS_F_P2P_INSETTING 0x00000008 ///< 用户处在设置界面
#define TGSS_F_BS_UPLOADING 0x00000010 ///< 后备存储正在上传
#define TGSS_F_CALL_PENDING 0x00000020 ///< 呼叫正在等待处理
#define TGSS_F_IN_CONVERSATION 0x00000040 ///< 呼叫通话中
/**@}*/

/** 获取SDK内部状态. 
 * @return 0 或 @ref sdk_state_flags "SDK状态标志" 的组合
 * @note 本函数只是用来告诉用户当前用什么事件在处理. \n
 *       不能用来代替 STATUS_IDLE 状态回调. 
 */
unsigned int TciGetSdkState(void);

/** 清除快启缓存的数据 */
void TciClearEarlyCache();

/** \defgroup 呼入 */
/** 接听.
 * @param info 来自 STATUS_INCALL */
int TciAcceptInCall(const struct InCallInfo *info);

/** 拒接.
 * @param info 来自 STATUS_INCALL */
int TciRejectInCall(const struct InCallInfo *info);

/** 挂断当前通话 */
int TciHangup(void);
/**@}*/

/**@}*/ //end of misc group

#ifdef __cplusplus
}
#endif

#endif
