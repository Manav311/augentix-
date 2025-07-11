/** \file ec_const.h
 *  \brief 常数和错误码定义
 */

#ifndef __ec_const_h__
#define __ec_const_h__

/** \ingroup api_const
 * @{*/

/** @name Media Type
 * @{
 */
/** \enum TCMEDIA 
 * 媒体类型枚举常数
 */
typedef enum TCMEDIA {
    TCMEDIA_INVALID = 0,            ///< 无效类型
    TCMEDIA_VIDEO_H264    = 1,      ///< H.264   "h264"
    TCMEDIA_AUDIO_G711A   = 2,      ///< G.711A  "g711a" "alaw"
    TCMEDIA_AUDIO_ALAW	  = TCMEDIA_AUDIO_G711A,  ///< G.711A  "g711a" "alaw"

	//----------------------------------
    TCMEDIA_VIDEO_IJPG    = 10,     ///< Private Inter-frame jpeg codec
    TCMEDIA_VIDEO_MPEG    = 11,     ///< Mpeg    "mpeg"
    TCMEDIA_VIDEO_JPEG   =  12,     ///< jpeg    "jpeg"
    TCMEDIA_VIDEO_MJPEG   = TCMEDIA_VIDEO_JPEG,     ///< Mjpeg "jpeg"
    TCMEDIA_VIDEO_H265    = 13,     ///< H.265   "h265"
    TCMEDIA_VIDEO_MAX,

    TCMEDIA_AUDIO_ULAW	   = 51,    ///< G.711U  "g711u" "ulaw"
    TCMEDIA_AUDIO_G711U    = TCMEDIA_AUDIO_ULAW, ///< G.711U
    TCMEDIA_AUDIO_PCM      = 52,    ///< Raw PCM  "pcm"
    TCMEDIA_AUDIO_ADPCM    = 53,    ///< ADPCM    "adpcm"
    TCMEDIA_AUDIO_ADPCM_IMA = TCMEDIA_AUDIO_ADPCM,  ///< ADPCM  "adpcm"
    TCMEDIA_AUDIO_ADPCM_DVI4 = 54,  ///< DVI4  "adpcm-dvi4"
    TCMEDIA_AUDIO_G726_16  = 55,    ///< G.726-16
    TCMEDIA_AUDIO_G726_24  = 56,    ///< G.726-24
    TCMEDIA_AUDIO_G726_32  = 57,    ///< G.726-32
    TCMEDIA_AUDIO_G726_40  = 58,    ///< G.726-40
    TCMEDIA_AUDIO_AAC      = 59,    ///< AAC  "aac"
    TCMEDIA_AUDIO_MP3      = 60,    ///< MP3  "mp3"
    TCMEDIA_AUDIO_AMR      = 61,    ///< AMR  "amr"
    TCMEDIA_AUDIO_MAX
} TCMEDIA;

/** 是否视频媒体类型 */
#define TCMEDIA_IS_VIDEO(mt) ((mt) && (((mt)<TCMEDIA_VIDEO_MAX && (mt)>TCMEDIA_AUDIO_G711A) || (mt)==TCMEDIA_VIDEO_H264))
/** 是否音频媒体类型 */
#define TCMEDIA_IS_AUDIO(mt) ((mt) && ((mt)==TCMEDIA_AUDIO_G711A || ((mt)>=TCMEDIA_AUDIO_ULAW && (mt)<TCMEDIA_AUDIO_MAX)))

/**@}*/ //name: Media Type

/** @anchor audio_sample_fmt 
 * @name 音频采样格式
 *
 * int TciSendFrame(int stream, TCMEDIA mt, const uint8_t *pFrame, int length, uint32_t ts, int uFrameFlags); \n
 * int TciSendFrameEx(int channel, int stream, TCMEDIA mt, const uint8_t *pFrame, int length, uint32_t ts, int uFrameFlags); \n
 * int TciSendPbFrame(p2phandle_t handle, uint32_t id_mt, const uint8_t *frame, int len, uint32_t timestamp, int uFrameFlags);
 *
 * 上面接口的最后一个参数 \c uFrameFlags ，当媒体类型为音频时，为音频采样格式.
 *
 * 该格式由采样频率、采样位宽和声道数据组成：
 *
 * \code
 *  uFrameFlags = (samplerate << 2) | (datebits << 1) | channel
 * \endcode
 *
 * 当 \c uFrameFlags 为0时，SDK会将其改为默认配置.
 *
 * 本系统默认的音频采样格式为 8000/16位/单声道, 对应 
 * \code
 * uFrameFlags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 < 1) | AUDIO_CHANNEL_MONO = 2
 * \endcode
 *
 * @{ */

/** 音频采样频率常数 */
typedef enum ENUM_AUDIO_SAMPLERATE {
	AUDIO_SAMPLE_8K			= 0x00, ///< 8000
	AUDIO_SAMPLE_11K		= 0x01, ///< 11000
	AUDIO_SAMPLE_16K		= 0x02, ///< 16000
	AUDIO_SAMPLE_22K		= 0x03, ///< 22000
	AUDIO_SAMPLE_24K		= 0x04, ///< 24000
	AUDIO_SAMPLE_32K		= 0x05, ///< 32000
	AUDIO_SAMPLE_44K		= 0x06, ///< 44000
	AUDIO_SAMPLE_48K		= 0x07  ///< 48000
} ENUM_AUDIO_SAMPLERATE;

/** 音频采样位宽常数 */
typedef enum ENUM_AUDIO_DATABITS {
	AUDIO_DATABITS_X		= 0, ///< 16 bits. 为了兼容，0也表示16位采样。8位采样用别的方式表示
	AUDIO_DATABITS_16		= 1  ///< 16 bits
} ENUM_AUDIO_DATABITS;

/** 音频通道数 */
typedef enum ENUM_AUDIO_CHANNEL {
	AUDIO_CHANNEL_MONO		= 0,  ///< 单声道
	AUDIO_CHANNEL_STERO		= 1   ///< 双声道
} ENUM_AUDIO_CHANNEL;

/**@}*/

/** 上报事件类型.
 * 事件可能需要携带额外参数。参数通过 EVENTPARAM::evt_data 传递，内容与具体事件相关
 */
typedef enum ECEVENT {
	ECEVENT_NONE = 0, ///< [] none
	ECEVENT_MOTION_DETECTED, ///< [motion] is detected (=1)
	ECEVENT_HUMAN_BODY, ///< [body] human body is detected (=2)
	ECEVENT_SOUND, ///< [sound] (=3)
	ECEVENT_PIR, ///< [pir](=4)

	ECEVENT_SMOKE, ///< [smoke] (=5)
	ECEVENT_TEMPERATURE_L, ///< [tempL] temperature low(=6). 参数: MKEVTDATA_Temperatur()
	ECEVENT_TEMPERATURE_H, ///< [tempH] temperature high(=7). 参数: MKEVTDATA_Temperatur()
	ECEVENT_HUMIDITY_L, ///< [humidL] humidity low(=8). 参数: MKEVTDATA_Humidity()
	ECEVENT_HUMIDITY_H, ///< [humidH] humidity high(=9). 参数: MKEVTDATA_Humidity()
	ECEVENT_GENERIC_SENSOR, ///< [generic] 通用传感器类消息 (=10)

	ECEVENT_DR_BEGIN, ///< 行车记录仪事件范围开始(=11)
	ECEVENT_G_SENSOR =
	        ECEVENT_DR_BEGIN, ///< [g-sensor] G-Sensor(碰撞事件)(=11). 参数: NULL or EVTDATA_SERIOUS_COLLISION
	ECEVENT_COLLISION = ECEVENT_G_SENSOR, ///< = @ref ECEVENT_G_SENSOR(=11)
	ECEVENT_SETOFF, ///< [set-off] set off car (=12)
	ECEVENT_PARK, ///< [park] car parked(=13)
	ECEVENT_SPEED_UP, ///< [speed-up] speed burstly up(=14)
	ECEVENT_SPEED_DOWN, ///< [speed-down] speed burstly down(=15)
	ECEVENT_DR_END = ECEVENT_SPEED_DOWN, ///< 行车记录仪事件范围结束(=15)

	ECEVENT_CALL, ///< [call] (=16)
	ECEVENT_DOORBELL = ECEVENT_CALL, ///< 保留旧的命名 = ECEVENT_CALL
	ECEVENT_PASSBY, ///< [passby] 有人路过(=17)
	ECEVENT_STAY, ///< [stay] 有人停留(=18)

	//ECEVENT_OBJECT,                ///< object recognization
	//ECEVENT_CAR = ECEVENT_OBJECT,                   ///<

	ECEVENT_LOCK, ///< [lock] 门锁消息(大类)(=19). 细分消息在data部分

	ECEVENT_CRY, ///< [cry] 检测到哭声(=20)
	ECEVENT_ENTER, ///< [enter] 进入区域(=21)
	//参数: MKEVTDAT_SitPoseSens()。这个在sdk内部处理
	ECEVENT_SITPOSE, ///< [bad_posture] sitting pose. 坐姿检测.(=22)

	ECEVENT_LEAVE, ///< [leave] 离开区域 "leave". 由sdk生成?(=23)
	ECEVENT_TUMBLE, ///< [tumble] 摔倒(=24)

	ECEVENT_SNAPSHOT, ///< [snapshot] 手动抓拍(=25)
	ECEVENT_CALL2, ///< [call.2]呼叫按键2(=26)

	ECEVENT_MAX,
	ECEVENT_USER_DEFINED = 255 ///< 自定义事件。使用方式见文档
} ECEVENT;

#define IsDoorBellEvent(e) (e == ECEVENT_CALL || e == ECEVENT_CALL2)

/** Doorbell call state */
typedef enum ECALLSTATE {
	CALLSTATE_MISSED, ///< 未接. sdk内部定时
	CALLSTATE_ANSWERED, ///< 已接。@ref TCI_CMD_ANSWERTOCALL
	CALLSTATE_REJECTED, ///< 拒接。@ref TCI_CMD_ANSWERTOCALL

	CALLSTATE_CANCELLED, ///< 设备端取消呼叫(上报 ECEVENT_CALL 事件 status=0)

	CALLSTATE_HANGUP, ///< 用户结束通话挂断接听. sdk在连接断开时也会产生。@ref TCI_CMD_ANSWERTOCALL

	CALLSTATE_BUSY ///< 用户占线(呼叫微信小程序用户时)
} ECALLSTATE;

/** 云服务类型 */
typedef enum ECSERVICETYPE {
	ECGS_TYPE_STORAGE = 1, ///< 云存储服务
	ECGS_TYPE_AI = 2, ///< AI服务
	ECGS_TYPE_WXVOIP = 3 ///< 微信 VoIP
} ECSERVICETYPE;

/** 4G状态 */
typedef enum ECG4STATE {
    G4STATE_IDLE,     ///< communicate by lan
    G4STATE_WORKING,  ///< communicate by 4g
    G4STATE_FAILURE   ///< 4g module has failure
} ECG4STATE;

/** 云上传文件队列长度.
 * - 队列长度大于0时，SDK内部生成的云录像会被放到上传队列依次上传。 \n
 *   正常情况下，在下一个文件生成并投入队列前，当前文件已经上传完成并从队列中移出. \n
 *   但当网络有问题时，则正在上传或待上传的文件仍在队列中，队列满时，新的文件不能  \n
 *   再投入队列中。 \n
 *   如果设了后存储，当队列满或上传失败，文件会写到后备空间, 然后择机重传, 否则丢弃。
 * - 队列长度为0时，文件直接写到后备空间。
 * - 较大的队列长度在网络不好、没有后备存储时对改善事件云录像丢包有利. 但应用要保证 \n
 *   系统有足够的内存可用 -- 每个文件长5", 应用按最大可能的码流来估算用于上传队列的 \n
 *   内存空间
 */
typedef enum ECBUFFERHINT {
	BUFFERHINT_NONE = 0, ///< 无缓冲. 数据先写卡再从卡上传, 仅用于行车记录仪的报警录像
	BUFFERHINT_SMALLEST = BUFFERHINT_NONE, ///< BUFFERHINT_NONE 的旧名
	BUFFERHINT_1FILE = 1, ///< 队列长度为1.
	BUFFERHINT_SMALL = BUFFERHINT_1FILE, ///< BUFFERHINT_1BUFFER 的旧名
	BUFFERHINT_2FILES = 2, ///< 队列长度为2
	BUFFERHINT_DEFAULT = BUFFERHINT_2FILES, ///< 默认队列长度为2
	BUFFERHINT_3FILES = 3, ///< 队列长度3
	BUFFERHINT_4FILES = 4, ///< 队列长度4
	BUFFERHINT_5FILES = 5, ///< 队列长度5
	BUFFERHINT_6FILES = 6 ///< 队列长度6
} ECBUFFERHINT;

/**@}*/ //group: api_const

/** \ingroup api_structure
 * @{*/
/** @name 特定事件相关参数
 * 参见 @ref EVENTPARAM::evt_data 
 * @{*/
#define EVTDATA_SERIOUS_COLLISION    (void*)0x01  ///< 仅针对 ECEVENT_COLLISION 事件, 严重碰撞
#define EVTDATA_PARKING_COLLISION    (void*)0x02  ///< 停车时碰撞
/**@}*/

typedef struct EVENTPARAM_v0 {
	int cbSize;
	ECEVENT event;
	long int tHappen;
	int status;
	char *jpg_pic;
	unsigned int pic_len;
	int evtp_flags;
	void *evt_data;
	void *pic_extra;
} EVENTPARAM_v0;

typedef struct EVENTPARAM_v1 {
	int cbSize;
	ECEVENT event;
	long int tHappen;
	int status;
	char *jpg_pic;
	unsigned int pic_len;
	int evtp_flags;
	void *evt_data;
	void *pic_extra;
	const char *x_event_name;
} EVENTPARAM_v1;

/** 事件上报参数. 设置时, 调用 memset(&ep, 0, sizeof(ep) 将所有未用到成员清0 */
typedef struct EVENTPARAM {
    int cbSize;            ///< 本结构大小, =sizeof(EVENTPARAM)。调用者要设置此成员。用于以后结构变化

    ECEVENT     event;    ///< 事件类型. 参数(evt_data)见事件的说明
    long int    tHappen;  ///< 事件发生时间
    int status;           ///< 1:事件开始; 0:事件结束(暂不支持)

    char *jpg_pic;  ///< 图片指针。没有图片时为NULL
    unsigned int pic_len; ///< 图片长度

/** @anchor eventparam_flags */
/** @name 事件参数标志位 */
/**@{*/
#define EPF_RELEASE_PIC_IN_SDK         0x01  ///< 由sdk释放图片内存
#define EPF_ONLY_AIEVENT_IF_SVCFORAI   0x02  ///< used internally by sdk
#define EPF_DONT_RECORD                0x04  ///< 事件不录像, 例如大多数门锁事件。 used internally by sdk
#define EPF_SNAPSHOT_ON_NEED           0x08  ///< 需要时由sdk请求图片。有此标志时，设置 jpg_pic=NULL, pic_len=0
#define EPF_POST_EVENT_REPORT 0x10 ///< 事件补报(已经在mcu里上报过), 只记录不推送 <-- 目前只上报图片
#define EPF_RECORD_ONLY 0x20 ///< 仅触发录像，不上报事件和上传图片
	/**@}*/
	/** 事件处理的标志. 0 或 @ref eventparam_flags "事件参数标志位" 的组合 */
	int evtp_flags;

	/** 特定事件相关参数.
     *  其值是个预定义常数或json字符串或cJSON对象. 内容会同事件一起上传到平台并记录。 \n
     *  json字符串可以使用 MKEVTDATA_xxxx 辅助函数来生成. 返回值及其处理同 snprintf()。\n
     *  cJSON对象使用 MKEVTDAT_xxx 来生成. \n
     *  MKEVTDATA_xxx/MKEVTDAT_xxx 定义在 tgutil.h 中. \n
     *  默认要设置为NULL */
	void *evt_data;

	void *pic_extra; ///< 图片的额外参数, 用于图片处理，不与事件一起记录. 目前用于ai

	/** [2024/3/25添加]
     * 如果event为 ECEVENT_USER_DEFINED, 这里为自定义事件名. 否则要设为 NULL
     * 自定义名称以 "x:" 开头，需要在平台先注册.
     */
	const char *x_event_name;

	/** [2024/6/7] */
	ECEVENT evtToReplace; ///< 要替换的事件的类型
	long int tPrevEvent; ///< 要替换的事件的时间戳
} EVENTPARAM;

/** \struct AiResult
 * AI服务器检测结果 */
struct AiResult {
    int  id;      ///< 序号。一张图片如果有多个ai结果，id从0开始递增 
    char *name;   ///< 识别结果。NULL则没有识别到对象
    void *jpg_pic;  ///< Ai结果对应的图片
    int  pic_len;   ///< 图片长度

    /** 调用时为0. 如果应用不希望sdk释放图片空间，可以将这个值设为1. \n
     * 回调返回后应用可以继续使用图片，但要记得以后自己释放 */
    int dont_release;
};
/**@}*/


/* Error code */
#define ECP_E_OK                0      ///< No error
//300~599                       error code defined in http protocol

/** @name 本地错误
 * @{*/
#define ECP_E_COMMUNICATION     -10000  ///< error in network communication
#define ECP_E_NOT_INITIALIZED   -10001  ///< McFetchNewOssToken is not called
#define ECP_E_INVALID_PARAMETER -10002  ///< Invalid parameter
#define ECP_E_OUT_OF_MEMORY     -10003  ///< Out of memory
#define ECP_E_UNEXPECTED_RESPONSE    -10004  ///< unexpected response
#define ECP_E_TOO_FREQUENT      -10005  ///< Too frequent calls
#define ECP_E_NOTALLOWED        -10006  ///< Not Allowed
/**@}*/

/** @name 服务器端错误码
 * @{
 */
#define ECP_E_OHTER             -20000  ///< Other errors
#define ECP_E_USERID_NOT_FOUND  -20001  ///< userid 未找到
#define ECP_E_UUID_NOT_FOUND    -20002  ///< uuid 不存在
#define ECP_E_UUID_ALREADY_ACTIVATED -20003
#define ECP_E_UUID_NOT_ACTIVATED -20004
#define ECP_E_UUID_IS_UNBOUND   -20005
#define ECP_E_CANNOT_GET_TOKEN  -20006

#define ECP_E_OSS_TIMESKEWED    -20007
#define ECP_E_OSS_INVALIDACCESSKEY -20008
#define ECP_E_OSS_ACCESSDENIED  -20009
/**@}*/

#endif

