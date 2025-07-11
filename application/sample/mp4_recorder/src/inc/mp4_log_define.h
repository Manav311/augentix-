#ifndef MP4_RECORDER_LOG_DEFINE_H_
#define MP4_RECORDER_LOG_DEFINE_H_

#include <stdio.h>

#ifndef __DEBUG__
#define __DEBUG__ 0
#endif

#define mp4_log_err(fmt, args...) printf("[ERROR] " fmt, ##args)
#define mp4_log_warn(fmt, args...) printf("[WARNING] " fmt, ##args)
#define mp4_log_notice(fmt, args...) printf("[NOTICE] " fmt, ##args)
#define mp4_log_info(fmt, args...) printf("[INFO] " fmt, ##args)

#if __DEBUG__
#define mp4_log_debug(fmt, args...) printf("[DEBUG] " fmt, ##args)
#else
#define mp4_log_debug(fmt, args...)
#endif
#endif //MP4_RECORDER_LOG_DEFINE_H_
