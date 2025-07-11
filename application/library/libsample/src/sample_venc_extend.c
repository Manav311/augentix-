/*include*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "sample_venc_extend.h"
#include "sample_dip.h"
#include "mpi_limits.h"
#include "mpi_enc.h"
#include "utlist.h"

static VencExtendInfo *g_info = NULL;
static LightSrcType g_lastType[MPI_MAX_INPUT_PATH_NUM] = { SRC_TYPE_DAY };

int SAMPLE_initVencExtendInfo(VencExtendInfo *head)
{
	if (head == NULL) {
		fprintf(stderr, "NULL pointer\n");
		return -EINVAL;
	}
	g_info = head;
	VencExtendInfo *item, *tmp;

	LL_FOREACH_SAFE(g_info, item, tmp)
	{
		UINT8 lastType = g_lastType[item->path.path];

		if (!item->venc_extend[lastType] || access(item->venc_extend[lastType], R_OK) != 0) {
			fprintf(stderr, "path%u, chn%d, venc_extend file : %s is NULL or illegal file\n",
			        item->path.path, item->chn.chn, item->venc_extend[lastType]);
		} else if (MPI_ENC_setVencExtendFile(item->chn, item->venc_extend[lastType]) == MPI_SUCCESS) {
			fprintf(stdout, "path%u, chn%u,set %s success\n", item->path.path, item->chn.chn,
			        item->venc_extend[lastType]);
		} else {
			fprintf(stderr, "path%u, chn%d, set venc_extend file : %s failed\n", item->path.path,
			        item->chn.chn, item->venc_extend[lastType]);
		}
	}
	return 0;
}

void SAMPLE_deinitVencExtendInfo()
{
	g_info = NULL;
}

int SAMPLE_setVencExtend(MPI_PATH path_idx, LightSrcType type)
{
	if (type >= SRC_TYPE_NUM || path_idx.path >= MPI_MAX_INPUT_PATH_NUM) {
		fprintf(stderr, "Path number or Light Source type are invalid\n");
		return -EINVAL;
	}
	if (g_info == NULL) {
		return -ENODEV;
	}

	g_lastType[path_idx.path] = type;

	VencExtendInfo *item, *tmp;
	LL_FOREACH_SAFE(g_info, item, tmp)
	{
		if (item->path.path == path_idx.path) {
			if (MPI_ENC_setVencExtendFile(item->chn, item->venc_extend[type]) == MPI_SUCCESS) {
				fprintf(stdout, "path%u, chn%u,set %s success\n", item->path.path, item->chn.chn,
				        item->venc_extend[type]);
			} else {
				fprintf(stdout, "path%u, chn%u,set %s failed\n", item->path.path, item->chn.chn,
				        item->venc_extend[type]);
			}
		}
	}
	return 0;
}