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
 * @file tuya_ipc_audio_demo.h
 * @brief
 */

/*********************************************************************************
  *Copyright(C),2015-2020, 
  *TUYA 
  *www.tuya.comm
**********************************************************************************/

#ifndef TUYA_IPC_AUDIO_DEMO_H_
#define TUYA_IPC_AUDIO_DEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_ring_buffer.h"


VOID IPC_APP_Notify_Siren(int alarm);

VOID TUYA_APP_Enable_Speaker_CB(BOOL_T enabled);

VOID TUYA_APP_Rev_Audio_CB(IN CONST MEDIA_FRAME_S *p_audio_frame, TUYA_AUDIO_SAMPLE_E audio_sample,
                           TUYA_AUDIO_DATABITS_E audio_databits, TUYA_AUDIO_CHANNEL_E audio_channel);

BOOL_T TUYA_APP_Get_Siren_Status(VOID);

VOID TUYA_APP_Force_Siren_On(BOOL_T on);

VOID TUYA_APP_Update_Siren_Parameter(VOID);

VOID TUYA_APP_Update_Voice_Parameter(VOID);

OPERATE_RET TUYA_APP_Init_Audio(VOID);

VOID TUYA_APP_Deinit_Audio(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TUYA_IPC_AUDIO_DEMO_H_ */
