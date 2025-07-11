/** \file TgCloudUtil.h
 *
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct cJSON;

/** 温度单位 */
typedef enum {
	TEMP_C, ///< Celcius
	TEMP_F ///< Farenheit
} TEMPTYPE;
/** 生成 ECEVENT_TEMPERATURE_H/ECEVENT_TEMPERATURE_L 事件的额外参数 
 * @param type 温度单位
 * @param temper 温度
 * @param obuf 输出的json格式字符串缓冲区
 * @param size 输入时, size 为缓冲区大小
 * @return json字符串长度+1。如果大于等于size, 意味obuf缓冲区不足，要重新分配大小至少为返回值的空间并再次调用
 * @see MKEVTDAT_Temperatur
 */
int MKEVTDATA_Temperatur(TEMPTYPE type, float temper, char *obuf, int size);

/** 返回json对象表示的温度事件参数.
 * @see MKEVTDATA_Temperatur
 */
struct cJSON *MKEVTDAT_Temperatur(TEMPTYPE type, float temper);

/** 生成 ECEVENT_HUMIDITY_H/ECEVENT_HUMIDITY_L 事件的额外参数
 * @param humid 湿度: 0~100
 * @param obuf 输出的json格式字符串缓冲区
 * @param size 输入时, size 为缓冲区大小
 * @return json字符串长度+1。如果大于等于size, 意味obuf缓冲区不足，要重新分配大小至少为返回值的空间并再次调用
 * @see MKEVTDAT_Humidity
 */
int MKEVTDATA_Humidity(int humid, char *obuf, int size);

/** 返回json对象表示的湿度参数
 * @param humid 湿度: 0~100
 */
struct cJSON *MKEVTDAT_Humidity(int humid);

/** ECEVENT_SITPOSE 事件参数
 *  @param sens 坐姿检测灵敏度参数。0(最灵敏)|1|2(最准确)
 */
struct cJSON *MKEVTDAT_SitPoseSens(int sens);

/** 生成喂食事件的数据 */
int MKEVTDATA_Feeding(int nServings, char *obuf, int size);

/** 上报喂食事件
 * @param isManually 手动还是自动喂食
 * @param nServing   喂食份数
 * @param pic        图片
 * @param pic_len    图片长度
 * @return <0: 错误码
 */
int TcuSendFeedingEvent(int isManually, int nServing, void *pic, int pic_len);

#ifdef __cplusplus
} /* extern "C" */
#endif
