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
 * @file tuya_service.h
 * @brief
 */

#ifndef TUYA_SERVICE_H_
#define TUYA_SERVICE_H_

/*
 * Caution !!!
 * In order to overrule TRUE/FALSE definition of TUYA IPC SDK, include "mpi_types.h" in the first place.
 */
#include "mpi_base_types.h"
#include "tuya_ipc_api.h"

static void tuyaNetStatChangeCb(BYTE_T stat);
OPERATE_RET initTuyaSdk(WIFI_INIT_MODE_E init_mode, const char *token);
int loadTuyaId(void);
void showHelp(char *app_name);
int main(int argc, char *argv[]);

#endif /* TUYA_SERVICE_H_ */
