#ifndef AVFTR_CONN_H_
#define AVFTR_CONN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "avftr.h"

#define AVFTR_SKT_PATH "/tmp/avftr_skt_path" /**< Path to audio/video feature socket */
#define AVFTR_RES_PATH "/tmp_avftr_res_path" /**< Path to audio/video feature result shared memory */

#define AVFTR_RES_SIZE \
	(((sizeof(AVFTR_CTX_S) + 4095) / 4096) * 4096) /**< Shared memory size of audio/video feature result */

typedef struct {
	AVFTR_VIDEO_CTX_S vftr;
	AVFTR_AUDIO_CTX_S aftr;
} AVFTR_CTX_S;

int AVFTR_initServer(void);
int AVFTR_exitServer(void);
int AVFTR_initClient(int *shm_fd, int *skt_fd, AVFTR_CTX_S **res_shm);
int AVFTR_exitClient(int *shm_fd, int *skt_fd, AVFTR_CTX_S **res_shm);

#ifdef __cplusplus
}
#endif

#endif /* !AVFTR_CONN_H_ */
