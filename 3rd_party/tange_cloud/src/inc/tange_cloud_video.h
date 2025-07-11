#ifndef TGC_VIDEO_H_
#define TGC_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

int TGC_initVideoSystem(void);
int TGC_deinitVideoSystem(void);
void TGC_stopVideoRun(void);
void *thread_VideoFrameData(void *arg);
int TGC_switchVideoSystem(int stream, const char *qstr);

#ifdef __cplusplus
}
#endif

#endif /* TGC_VIDEO_H_ */