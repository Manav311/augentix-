#ifndef UNICORN_STREAM_H_
#define UNICORN_STREAM_H_

#define MAX_FOCUS_ROI_NUM (16)

#include <mpi_index.h>
#include <mpi_dev.h>

typedef enum {
	ACTION_NULL = 0,
	ACTION_GET_ROI = 1,
	ACTION_SET_ROI = 2,
	ACTION_QUERY_STATUS = 3,
} ACTION_E;

typedef struct {
	int choice;
	MPI_WIN win_idx;
	UINT8 cfg_idx;
	MPI_ISP_VAR_CFG_S var_cfg;
	int roi_cnt;
} APP_CFG_S;

struct json_object;

int runMpiSys(char *type, int clientfd);
int initMpiSys(void);
int exitMpiSys(void);
int runFocusMpiSys(char *type, int clientfd);
int runPanoramaMpiSys(char *jstr, int clientfd);
int runMultiRoiFocusMpiSys(char *jstr, int clientfd);
int setMpiSetting(char *jstr, int len, int cmd_id, int clientfd);
int getMpiSetting(char *params, char **jstr, int cmd_id, char **table);

#endif
