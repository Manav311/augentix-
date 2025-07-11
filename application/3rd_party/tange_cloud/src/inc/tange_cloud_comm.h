#ifndef TGC_COMM_H_
#define TGC_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "TgCloudApi.h"
#include "agtx_lpw.h"

#define MAX_WIFI_RETRY_TIMES 5

#if defined(TANGE_USE_AGT_MPI_STREAM)
#define KEEP_ALIVE_FILE "/tmp/augentix/keep_alive"
#endif

int TGC_initComm(char *uuid);
void TGC_deinitComm(void);
int TGC_startComm(void);
int get_mode(void);
void set_mode(int mode);

// P2P Streaming
void TGC_sendFrame(int channel, int stream, TCMEDIA tm, const uint8_t *pFrame, unsigned int length, uint32_t ts,
                   unsigned int flages);
void TGC_sendPbFrame(p2phandle_t handle, TCMEDIA mt, const uint8_t *frame, unsigned int length, uint32_t ts,
                     unsigned int flages);

unsigned int GetTimeStampMs(void);

lpw_handle openLpwHandler(void);
void closeLpwHandler(lpw_handle hd);
void resetIdleTime(void);
int shouldSleep(void);

#ifdef __cplusplus
}
#endif

#endif /* TGC_COMM_H_ */
