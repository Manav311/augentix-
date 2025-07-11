/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: header file for Wi-Fi Station component
 * Author: Hisilicon
 * Create: 2020-09-09
 */

#ifndef __SAMPLE_COMM_H__
#define __SAMPLE_COMM_H__
#include "hi_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CMD_MAX_LEN 1500
#define SOCK_BUF_MAX 1500
#define SOCK_PORT 8822

#define sample_log_print(fmt, args...) printf("[LPW_CTRL] " fmt "\n", ##args);

#define sample_unused(x) ((x) = (x))
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
