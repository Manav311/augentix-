#include "rtsp_shm.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "avftr_conn.h"
#include "rtsp_app_cb.h"

int RSHM_init(const char *args, MPI_WIN win)
{
#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
	// init avftr serv
	int ret = AVFTR_initServer();
	if (ret) {
		fprintf(stderr, "[ERROR] init avftr server fail\n");
		return -EINVAL;
	}

	if (strcmp(args, "hd") == 0) {
		ret = initHdCb(win);
		if (ret) {
			fprintf(stderr, "[ERROR] init HD callback fail\n");
			return -EINVAL;
		}
	} else if (strcmp(args, "md") == 0) {
		ret = initMdCb(win);
		if (ret) {
			fprintf(stderr, "[ERROR] init MD callback fail\n");
			return -EINVAL;
		}
	} else {
		fprintf(stderr, "error: undefined callback type !\n");
		return -EINVAL;
	}
#endif
	return 0;
}

int RSHM_updateStatus(MPI_WIN idx, void *buf, uint32_t count, uint32_t flag, struct timespec ts)
{
	if (g_app_cb) {
		return g_app_cb(idx, buf, count, flag, ts);
	}

	return 0;
}

void RSHM_exit()
{
#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
	AVFTR_exitServer();
#endif
}

const char *RSHM_getAppCbString(void)
{
	static char cb_list_string[256];
	sprintf(cb_list_string, "'hd', 'md'");
	return cb_list_string;
}
