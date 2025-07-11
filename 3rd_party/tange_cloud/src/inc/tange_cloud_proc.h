#ifndef TGC_PROC_H_
#define TGC_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif

int TGC_initProc(char *uuid);
int TGC_deinitProc(void);
int TGC_runProc(void);
void TGC_stopProc(void);

#ifdef __cplusplus
}
#endif

#endif /* TGC_PROC_H_ */