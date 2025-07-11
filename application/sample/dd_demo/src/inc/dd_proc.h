#ifndef _DD_PROC_H_
#define _DD_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif

int initProc(void *input_url);
int deinitProc(void);
int runProc(void);
int startProc(void);
void stopProc(void);

#ifdef __cplusplus
}
#endif

#endif