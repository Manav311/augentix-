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
 * @file tuya_ipc_motion_detection.h
 * @brief
 */

#ifndef TUYA_IPC_MOTION_DETECTION_H_
#define TUYA_IPC_MOTION_DETECTION_H_
#include <stdio.h>

void *thread_md_proc(void *arg);
OPERATE_RET TUYA_APP_Enable_AI_Detect();
VOID TUYA_APP_Update_Md_Parameter();
BOOL_T TUYA_APP_Check_Event_Triggerd();

extern unsigned int eventDetectCntr;

#endif /* TUYA_IPC_MOTION_DETECTION_H_ */
