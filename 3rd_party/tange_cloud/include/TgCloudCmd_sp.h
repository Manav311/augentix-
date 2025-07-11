/** \file TgCloudCmd_sp.h
 *
 * 本文件包含特殊类型设备的命令和数据结构
 */
#pragma once

#include "TgCloudCmd.h"
#ifdef __cplusplus
extern "C" {
#endif

/**@ingroup p2pcmds @{*/

/** @anchor pet_feeder */
/** @name 喂食器
  - DeviceType = PetFeeder
 @{*/
#define TCMD_FEEDER_GET_CONFIG 0x0500 ///< 查询配置. req: none; resp: Tcis_FeederConfig
#define TCMD_FEEDER_SET_TIMERS 0x0502 ///< 设置喂食定时.  req: Tcis_FeederTimers; resp: generic
#define TCMD_FEEDER_GET_TIMERS 0x0504 ///< 获取喂食定时器设置. req: none; resp: Tcis_FeederTimers
#define TCMD_FEEDER_FEED_FOOD 0x0506 ///< 手工喂食. req: Tcis_Feed; resp: generic

/** 喂食器配置.
 * TCMD_FEEDER_GET_CONFIG     = 0x0500 \n
 * 这个结构可能扩展. App 端要检查收到的数据包的长度
 */
typedef struct Tcis_FeederConfig {
	int max_timers; ///< 支持的定时配置数
	int max_servings; ///< 最大供食份数
} __PACKED__ Tcis_FeederConfig;

typedef struct FEEDERTIMER {
	CLOCKTIME clock; ///< 喂食时间
	uint8_t state; ///< 0:禁止(或单次定时器已执行); 1:有效(调度中)
	uint8_t repeat; ///< weekdays mask. bit0:Sunday; bit1-Monday; ...
	uint16_t serving; ///< 食物份数
} __PACKED__ FEEDERTIMER;

/** 喂食定时设置.
 * TCMD_FEEDER_SET_TIMERS     = 0x0500
 * TCMD_FEEDER_GET_TIMERS     = 0x0502
 */
typedef struct Tcis_FeederTimers {
	int nTimers;
	FEEDERTIMER tiems[1];
} __PACKED__ Tcis_FeederTimers;

/** 手动喂食.
 * TCMD_FEEDER_FEED_FOOD      = 0x0506
 */
typedef struct Tcis_Feed {
	int nServing; ///< 投喂份数
	int reserved; ///< 0
} __PACKED__ Tcis_Feed;

/**@}*/ //喂食器

/**@}*/ //ingroup p2pcmds
#ifdef __cplusplus
} /* extern "C" */
#endif
