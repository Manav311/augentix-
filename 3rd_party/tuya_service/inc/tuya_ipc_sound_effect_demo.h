/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/**
 * @file tuya_ipc_sound_effect_demo.h
 * @brief
 */

/*********************************************************************************
  *Copyright(C),2015-2020,
  *TUYA
  *www.tuya.comm
**********************************************************************************/

#ifndef TUYA_IPC_SOUND_EFFECT_DEMO_H_
#define TUYA_IPC_SOUND_EFFECT_DEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>


#define CUETONE1_NAME "cuetone1"
#define CUETONE1_PATH "/system/factory_default/one.ul"

#define CUETONE2_NAME "cuetone2"
#define CUETONE2_PATH "/system/factory_default/two.ul"


typedef struct {
	pthread_mutex_t lock;

	int sound_on;
	int play_period;
	int volume;
	unsigned int end_time;
} cuetone1_ctrl;

typedef struct {
	pthread_mutex_t lock;

	int sound_on;
	int play_period; /* unit in second */
	int volume;
	unsigned int end_time;
} cuetone2_ctrl;


/* Variables registered to audio framework */
extern cuetone1_ctrl g_cuetone1;
extern cuetone2_ctrl g_cuetone2;


/* Functions registered to audio framework */
int cuetone1_init(VOID *data);
int cuetone1_deinit(VOID *data);
int cuetone1_run(VOID *data);
int cuetone1_get_volume(VOID);

int cuetone2_init(VOID *data);
int cuetone2_deinit(VOID *data);
int cuetone2_run(VOID *data);
int cuetone2_get_volume(VOID);

/* Callback functions of Tuya APP */
VOID TUYA_APP_Ctrl_Cuetone1(BOOL_T sound_on);
VOID TUYA_APP_Ctrl_Cuetone2(BOOL_T sound_on);

#ifdef __cplusplus
}
#endif

#endif /* TUYA_IPC_SOUND_EFFECT_DEMO_H_ */
