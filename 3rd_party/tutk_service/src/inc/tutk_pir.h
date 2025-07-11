#ifndef TUTK_PIR_H_
#define TUTK_PIR_H

#ifdef __cplusplus
extern "C" {
#endif

int getPIRStatus();
int TUTK_startPIRnotificationThread(void *deviceCtx);

#ifdef __cplusplus
}
#endif

#endif