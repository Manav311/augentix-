#ifndef _VIDEO_PROC_H_
#define _VIDEO_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif

int video_process_initial(void *arg);
int video_process_deinitial(void);
int startVideoRun(void);
int stopVideoRun(void);
void *getVideoContext(void);
void *thread_VideoFrameData(void *arg);

#ifdef __cplusplus
}
#endif

#endif
