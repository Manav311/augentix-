#ifndef TUYA_UNC_INTF_H
#define TUYA_UNC_INTF_H

/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tuya_unc_intf.h - Tuya-unicorn interface
 * Copyright (C) 2019 ShihChieh Lin, Augentix Inc. <shihchieh.lin@augentix.com>
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 */

#include "tuya_test_frame.h"

UncTestFrame *cmd_ttou(TuyaTestFrame *tframe);
TuyaTestFrame *cmd_utot(UncTestFrame *uframe);

#endif
