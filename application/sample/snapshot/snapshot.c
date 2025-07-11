#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "mpi_index.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_enc.h"

int g_mult_thrd = 0;
int g_snapshot_width = 0;
int g_snapshot_height = 0;
volatile sig_atomic_t g_exit = 0;

typedef struct thrd_envelope {
	pthread_t *tid; /* thread ID */
	int thrd_cnt;
	MPI_ECHN echn_idx;
	INT32 quality_idx;
	char *fname;
} ThrdEnvelope;

static void sigintHandler(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else {
		perror("Unexpected signal!\n");
		exit(1);
	}

	g_exit = 1;
}

static void help(const char *bin)
{
	printf("Usage:\n");
	printf("\t%s <image_format> [snapshot_options]\n", bin);
	printf("\n");
	printf("Support Image Format:\n");
	printf("\t%s jpeg <num_times> <IDX> <FILE>\n", bin);
	printf("\t%s jpeg2 <num_times> <IDX> <QUALITY_IDX> <FILE>\n", bin);
	printf("\t%s nv12 <num_times> <DEV> <CHN> <WIN> <WIDTH> <HEIGHT> <FILE>\n", bin);
	printf("\t%s rgb <num_times> <DEV> <CHN> <WIN> <WIDTH> <HEIGHT> <FILE>\n", bin);
	printf("\n");
	printf("Other Options:\n");
	printf("\t%s -h --help Print HELP message", bin);
	printf("\n");
}

static INT32 takeRgbSnapshot(MPI_WIN win_idx, const char *fname)
{
	MPI_VIDEO_FRAME_INFO_S info = { 0 };
	info.width = g_snapshot_width;
	info.height = g_snapshot_height;
	INT32 err = 0;
	FILE *fp;

	printf("Taking snapshot of video window (d:c:w=%d:%d:%d) to %s\n", win_idx.dev, win_idx.chn, win_idx.win,
	       fname);

	/* Set to RGB type */
	info.type = MPI_SNAPSHOT_RGB;

	err = MPI_DEV_getWinFrame(win_idx, &info, 1200);
	if (err != MPI_SUCCESS) {
		printf("Failed to take snapshot. err: %d\n", err);
		goto end;
	}

	/* Open file */
	fp = fopen(fname, "wb");
	if (fp == NULL) {
		printf("Failed to open file %s for snapshot. err: %d\n", fname, -errno);
		goto release_memory;
	}

	/* Output PPM format */
	fprintf(fp, "P6\n%d %d\n255\n", info.width, info.height);

	/* Write snapshot data */
	fwrite(info.uaddr, info.size, 1, fp);

	/* Close file */
	fclose(fp);

release_memory:

	MPI_DEV_releaseWinFrame(win_idx, &info);

end:

	return err;
}

static INT32 takeYonlySnapshot(MPI_WIN win_idx, const char *fname)
{
	MPI_VIDEO_FRAME_INFO_S info = { 0 };
	info.width = g_snapshot_width;
	info.height = g_snapshot_height;
	INT32 err = 0;
	FILE *fp;

	printf("Taking Y-only snapshot of video window (d:c:w=%d:%d:%d) to %s\n", win_idx.dev, win_idx.chn, win_idx.win,
	       fname);

	err = MPI_DEV_getWinFrame(win_idx, &info, 1200);
	if (err != MPI_SUCCESS) {
		printf("Failed to take snapshot. err: %d\n", err);
		goto end;
	}

	fp = fopen(fname, "wb");
	if (fp == NULL) {
		printf("Failed to open file %s for snapshot. err: %d\n", fname, -errno);
		goto release_memory;
	}

	fprintf(fp, "P5\n%d %d\n255\n", info.width, info.height);
	fwrite(info.uaddr, info.size, 1, fp);
	fclose(fp);

release_memory:

	MPI_DEV_releaseWinFrame(win_idx, &info);

end:

	return err;
}

static INT32 takeNv12Snapshot(MPI_WIN win_idx, const char *fname)
{
	MPI_VIDEO_FRAME_INFO_S info = { 0 };
	INT32 err = 0;
	FILE *fp;

	info.width = g_snapshot_width;
	info.height = g_snapshot_height;
	info.type = MPI_SNAPSHOT_NV12;

	printf("Taking NV12 snapshot of video window (d:c:w=%d:%d:%d) to %s\n", win_idx.dev, win_idx.chn, win_idx.win,
	       fname);

	err = MPI_DEV_getWinFrame(win_idx, &info, 1200);
	if (err != MPI_SUCCESS) {
		printf("Failed to take snapshot. err: %d\n", err);
		goto end;
	}

	fp = fopen(fname, "wb");
	if (fp == NULL) {
		printf("Failed to open file %s for snapshot. err: %d\n", fname, -errno);
		goto release_memory;
	}

	/* For Nv12 snapshot, "uaddr" represents the starting address of the Y channel's data,
	 * and "uaddr_1" represents the starting address of the C channel's data.
	 * Please note that Y and C channels' data are not necessarily physically 
	 * contiguous, so they have to be read seperately.
	 * Since our data is in NV12 format, it implies that the C channel is only half as large
	 * as the Y channel, and 1 UV-pair value will be shared by 4 Y values. */
	fwrite(info.uaddr, (info.width * info.height), 1, fp);
	fwrite(info.uaddr_1, (info.width * info.height / 2), 1, fp);
	fclose(fp);

release_memory:

	MPI_DEV_releaseWinFrame(win_idx, &info);

end:

	return err;
}

static INT32 takeJpegSnapshot(MPI_ECHN echn_idx, const char *fname)
{
	MPI_STREAM_PARAMS_S param;
	FILE *fp;
	INT32 err;

	err = MPI_initBitStreamSystem();
	if (err != MPI_SUCCESS) {
		printf("MPI_initBitStreamSystem(). failed. err = %d\n", err);
		goto end;
	}

	printf("Taking snapshot of encoder channel (c=%d) to %s\n", echn_idx.chn, fname);

	err = MPI_ENC_getChnFrame(echn_idx, &param, -1);
	if (err != MPI_SUCCESS) {
		printf("Failed to take snapshot.\n");
		goto release_mpp_resource;
	}

	fp = fopen(fname, "wb");
	if (fp == NULL) {
		printf("Failed to open file %s for snapshot\n", fname);
		goto release_memory;
	}

	printf("seg_cnt = %d\n", param.seg_cnt);
	for (int i = 0; (uint32_t)i < param.seg_cnt; i++) {
		printf("seg[%d] uaddr = 0x%08X, size = 0x%08X\n", i, (unsigned int)param.seg[i].uaddr,
		       (unsigned int)param.seg[i].size);
		fwrite(param.seg[i].uaddr, param.seg[i].size, 1, fp);
	}

	fclose(fp);

release_memory:

	err = MPI_ENC_releaseChnFrame(echn_idx, &param);
	if (err != MPI_SUCCESS) {
		printf("MPI_ENC_releaseChnFrame(). err = %d\n", err);
	}

release_mpp_resource:

	err = MPI_exitBitStreamSystem();
	if (err != MPI_SUCCESS) {
		printf("MPI_exitBitStreamSystem(). err = %d\n", err);
	}

end:

	return err;
}

void *myThreadFunc(void *arg)
{
	ThrdEnvelope *env_ptr = (ThrdEnvelope *)arg;
	int *tid = (int *)env_ptr->tid;
	// char *fname = env_ptr->fname;
	char fname[256];
	MPI_ECHN echn_idx = env_ptr->echn_idx;
	int quality_idx = env_ptr->quality_idx;
	MPI_STREAM_PARAMS_V2_S param;
	FILE *fp;
	INT32 i, err;
	INT32 retry_cnt = 3;

	printf("[%s] Thread ID: %d, Thread Count: %d\n", __func__, *tid, env_ptr->thrd_cnt);

	for (i = 0; i < retry_cnt; i++) {
		err = MPI_ENC_getChnFrameV2(echn_idx, quality_idx, &param, 1200);
		/* Request from again if timout occurred */
		if (err == -ETIMEDOUT) {
			continue;
		} else {
			break;
		}
	}

	if (err != MPI_SUCCESS) {
		printf("Failed to take snapshot.\n");
		return arg;
	}

	sprintf(fname, "/mnt/nfs/jpeg2_%d_%d.jpg", quality_idx, env_ptr->thrd_cnt);
	fp = fopen(fname, "wb");
	if (fp == NULL) {
		printf("Failed to open file %s for snapshot\n", fname);
		goto release_memory;
	}

	printf("seg_cnt = %d\n", param.seg_cnt);
	for (i = 0; (uint32_t)i < param.seg_cnt; i++) {
		printf("seg[%d] uaddr = 0x%08X, size = 0x%08X, thread count: %d\n", i, (unsigned int)param.seg[i].uaddr,
		       (unsigned int)param.seg[i].size, env_ptr->thrd_cnt);
		fwrite(param.seg[i].uaddr, param.seg[i].size, 1, fp);
	}

	fclose(fp);

release_memory:

	err = MPI_ENC_releaseChnFrameV2(echn_idx, &param);
	if (err != MPI_SUCCESS) {
		printf("MPI_ENC_releaseChnFrame(). err = %d\n", err);
	}

	return arg;
}

static INT32 takeJpegSnapshotV2(MPI_ECHN echn_idx, char *fname, int quality_idx)
{
	MPI_STREAM_PARAMS_V2_S param;
	FILE *fp;
	INT32 err;
	INT32 i;
	INT32 retry_cnt = 3;

	param.timestamp.tv_sec = 1200; // wait time for snapshot in ms

	err = MPI_initBitStreamSystem();
	if (err != MPI_SUCCESS) {
		printf("MPI_initBitStreamSystem(). failed. err = %d\n", err);
		goto end;
	}

	printf("Taking snapshot of encoder channel (c=%d) to %s\n", echn_idx.chn, fname);

	if (g_mult_thrd) {
		/* Simulate multiple simultaneous snapshot requests by running 10 threads together */
		INT32 *thrd_ptr;
		ThrdEnvelope env[g_mult_thrd];
		pthread_t tid[g_mult_thrd];

		for (i = 0; i < g_mult_thrd; i++) {
			env[i].echn_idx = echn_idx;
			env[i].quality_idx = quality_idx;
			env[i].tid = &tid[i];
			env[i].thrd_cnt = i;
			pthread_create(&tid[i], NULL, myThreadFunc, (void *)&env[i]);
		}

		for (i = 0; i < g_mult_thrd; i++) {
			pthread_join(tid[i], (void **)&thrd_ptr);
			printf("[%s] Value received: %i\n", __func__, *thrd_ptr);
		}

	} else {
		for (i = 0; i < retry_cnt; i++) {
			err = MPI_ENC_getChnFrameV2(echn_idx, quality_idx, &param, 1200);
			/* Request from again if timout occurred */
			if (err == -ETIMEDOUT) {
				continue;
			} else {
				break;
			}
		}
		if (err != MPI_SUCCESS) {
			printf("Failed to take snapshot.\n");
			goto release_mpp_resource;
		}

		fp = fopen(fname, "wb");
		if (fp == NULL) {
			printf("Failed to open file %s for snapshot\n", fname);
			goto release_memory;
		}

		printf("seg_cnt = %d\n", param.seg_cnt);
		for (int i = 0; (uint32_t)i < param.seg_cnt; i++) {
			printf("seg[%d] uaddr = 0x%08X, size = 0x%08X\n", i, (unsigned int)param.seg[i].uaddr,
			       (unsigned int)param.seg[i].size);
			fwrite(param.seg[i].uaddr, param.seg[i].size, 1, fp);
		}

		fclose(fp);
	}

release_memory:

	err = MPI_ENC_releaseChnFrameV2(echn_idx, &param);
	if (err != MPI_SUCCESS) {
		printf("MPI_ENC_releaseChnFrame(). err = %d\n", err);
	}

release_mpp_resource:

	err = MPI_exitBitStreamSystem();
	if (err != MPI_SUCCESS) {
		printf("MPI_exitBitStreamSystem(). err = %d\n", err);
	}

end:
	return err;
}

int main(int argc, char **argv)
{
	char fname[256];
	MPI_ECHN echn_idx;
	MPI_WIN win_idx;
	bool takeJpeg = false;
	bool takeJpeg2 = false;
	bool takeNv12 = false;
	bool takeYonly = false;
	bool takeRgb = false;
	int repeat = 1;
	int err;
	sigset_t block_interrupt;

	sigemptyset(&block_interrupt);
	sigaddset(&block_interrupt, SIGINT);
	signal(SIGINT, sigintHandler);

	if (argc == 1 || (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
		help(argv[0]);
		return EXIT_SUCCESS;
	} else if ((strcmp(argv[1], "y") == 0 || strcmp(argv[1], "nv12") == 0 || strcmp(argv[1], "rgb") == 0) &&
	           argc == 9) {
		if (strcmp(argv[1], "nv12") == 0) {
			takeNv12 = 1;
		} else if (strcmp(argv[1], "rgb") == 0) {
			takeRgb = 1;
		} else {
			takeYonly = 1;
		}

		win_idx = MPI_VIDEO_WIN(atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
		g_snapshot_width = atoi(argv[6]);
		g_snapshot_height = atoi(argv[7]);
		strcpy(fname, argv[8]);
	} else if (strcmp(argv[1], "jpeg") == 0 && argc == 5) {
		takeJpeg = 1;
		echn_idx = MPI_ENC_CHN(atoi(argv[3]));
		strcpy(fname, argv[4]);
	} else if (strcmp(argv[1], "jpeg2") == 0 && (argc == 6 || argc == 7)) {
		g_mult_thrd = (argc == 7) ? atoi(argv[6]) : 0;
		takeJpeg2 = 1;
		echn_idx = MPI_ENC_CHN(atoi(argv[3]));
		strcpy(fname, argv[5]);
	} else {
		help(argv[0]);
		return EXIT_FAILURE;
	}

	err = MPI_SYS_init();
	if (err != MPI_SUCCESS) {
		printf("MPI_SYS_init() failed. err: %d\n", err);
		return EXIT_FAILURE;
	}

	repeat = atoi(argv[2]); /* Get repeat value */

	for (int i = 0; i < repeat && g_exit == 0; i++) {
		sigprocmask(SIG_BLOCK, &block_interrupt, NULL);
		if (takeYonly) {
			takeYonlySnapshot(win_idx, fname);
		}

		if (takeNv12) {
			takeNv12Snapshot(win_idx, fname);
		}

		if (takeJpeg) {
			takeJpegSnapshot(echn_idx, fname);
		}

		if (takeJpeg2) {
			takeJpegSnapshotV2(echn_idx, fname, atoi(argv[4]));
		}

		if (takeRgb) {
			takeRgbSnapshot(win_idx, fname);
		}
		sigprocmask(SIG_UNBLOCK, &block_interrupt, NULL);

		if (i < (repeat - 1))
			usleep(1000 * 1000);
	}

	err = MPI_SYS_exit();
	if (err != MPI_SUCCESS) {
		printf("MPI_SYS_exit() failed. err: %d\n", err);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
