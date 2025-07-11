#ifndef SAMPLE_VENC_EXTEND_H_
#define SAMPLE_VENC_EXTEND_H_

#include "sample_dip.h"

typedef struct venc_extend_info {
	MPI_ECHN chn;
	MPI_PATH path;
	char venc_extend[SRC_TYPE_NUM][PATH_MAX];
	struct venc_extend_info *next;
} VencExtendInfo;

int SAMPLE_initVencExtendInfo(VencExtendInfo *head);
int SAMPLE_setVencExtend(MPI_PATH path_idx, LightSrcType mode);
void SAMPLE_deinitVencExtendInfo();

#endif