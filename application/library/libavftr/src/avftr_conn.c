#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <stdlib.h>
#include <fcntl.h> /* For O_* constants */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "avftr_log.h"
#include "avftr_conn.h"
#include "avftr.h"

typedef enum {
	AVFTR_START,
	AVFTR_CLOSING,
	AVFTR_STOP,
} AVFTR_SERV_STATE_E;

// Server
int avftrResShmFD; //Shared memory for result FD
AVFTR_CTX_S *avftr_res_shm;
AVFTR_VIDEO_CTX_S *vftr_res_shm;
AVFTR_AUDIO_CTX_S *aftr_res_shm;
AVFTR_SERV_STATE_E g_avftr_serv_stat = AVFTR_STOP;

/**
 * @brief Clean old file descriptors.
 * @param[in]  fileName    file name.
 * @see none
 * @retval  none.
 */
static void cleanOldFd(const char *fileName)
{
	char cmd[256];

	if (!access(fileName, F_OK)) {
		avftr_log_err("Deleting old unix file descriptor.");

		if (!remove(fileName)) {
			sprintf(cmd, "rm -f %s", AVFTR_SKT_PATH);
			system(cmd); //force delete
		}
	}
}

/**
 * @brief Initialize AVFTR server.
 * @param[in]  none
 * @see VIDEO_FTR_exitServer()
 * @retval  0 success.
 * @retval -1 failure.
 */
int AVFTR_initServer(void)
{
	if (g_avftr_serv_stat != AVFTR_STOP) {
		avftr_log_err("Server is running or was closed incorrectly. Server state = %d", g_avftr_serv_stat);
		goto errexit;
	}

	g_avftr_serv_stat = AVFTR_START;

	/* Clean shm first */
	cleanOldFd(AVFTR_RES_PATH);

	/* Open fixed size shared memory for transmitting inference result */
	avftrResShmFD = shm_open(AVFTR_RES_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (avftrResShmFD == -1) {
		avftr_log_err("Open shared memory for IPC failed. %s", strerror(errno));
		goto erropen;
	}

	ftruncate(avftrResShmFD, AVFTR_RES_SIZE);
	avftr_res_shm = (AVFTR_CTX_S *)mmap(0, AVFTR_RES_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, avftrResShmFD, 0);
	if (avftr_res_shm == MAP_FAILED) {
		avftr_log_err("Mmap memory failed. %s", strerror(errno));
		goto errmmap;
	}
	vftr_res_shm = &avftr_res_shm->vftr;
	aftr_res_shm = &avftr_res_shm->aftr;

	return 0;

errmmap:
	close(avftrResShmFD);
	avftrResShmFD = -1;

erropen:
	g_avftr_serv_stat = AVFTR_STOP;

errexit:
	return -1;
}

/**
 * @brief Exit avftr get result server.
 * @retval  0 success.
 * @retval -1 failure.
 * @see AVFTR_initServer()
 */
int AVFTR_exitServer(void)
{
	if (g_avftr_serv_stat != AVFTR_START) {
		avftr_log_err("Server state is not RUNNING is %d", g_avftr_serv_stat);
	}

	g_avftr_serv_stat = AVFTR_CLOSING;
	if (avftr_res_shm != NULL) {
		munmap(avftr_res_shm, AVFTR_RES_SIZE);
		avftr_res_shm = NULL;
		vftr_res_shm = NULL;
		aftr_res_shm = NULL;
	}

	if (avftrResShmFD != -1) {
		close(avftrResShmFD);
		avftrResShmFD = -1;
	}

	g_avftr_serv_stat = AVFTR_STOP;

	return 0;
}

/**
 * @brief Initialize avftr get result client.
 * @param[out]  shm_fd    pointer of result shared memory file descriptor
 * @param[out]  skt_fd    pointer of client socket file descriptor
 * @param[out]  res_shm   pointer of result shared memory address
 * @retval  0 success.
 * @retval -1 failure.
 * @see AVFTR_exitClient()
 */
int AVFTR_initClient(int *shm_fd, int *skt_fd __attribute__((unused)), AVFTR_CTX_S **res_shm)
{
	/* Open shared memory for result */
	*shm_fd = shm_open(AVFTR_RES_PATH, O_RDONLY, S_IRUSR | S_IWUSR);
	if (*shm_fd < 0) {
		avftr_log_err("Client open shared memory failed. err: %d (%s)", errno, strerror(errno));
		goto errexit;
	}

	*res_shm = (AVFTR_CTX_S *)mmap(0, AVFTR_RES_SIZE, PROT_READ, MAP_SHARED, *shm_fd, 0);
	if (*res_shm == MAP_FAILED) {
		avftr_log_err("Map memory failed. err: %d (%s)", errno, strerror(errno));
		goto errmmap;
	}

	return 0;

errmmap:
	close(*shm_fd);
	*shm_fd = -1;

errexit:
	return -1;
}

/**
 * @brief Exit audio/video feature get result client.
 * @param[in]  shm_fd    pointer of result shared memory file descriptor
 * @param[in]  skt_fd    pointer of client socket file descriptor
 * @param[in]  res_shm   pointer of result shared memory address
 * @see VIDEO_FTR_initClient()
 * @retval  0 success.
 * @retval -1 failure.
 */
int AVFTR_exitClient(int *shm_fd, int *skt_fd __attribute__((unused)), AVFTR_CTX_S **res_shm)
{
	if (*res_shm != NULL) {
		munmap(*res_shm, AVFTR_RES_SIZE);
		*res_shm = NULL;
	}

	close(*shm_fd);
	*shm_fd = -1;

	return 0;
}
