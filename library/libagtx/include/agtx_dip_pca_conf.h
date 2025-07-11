#ifndef AGTX_DIP_PCA_CONF_H_
#define AGTX_DIP_PCA_CONF_H_

#include "agtx_types.h"
struct json_object;

#define PCA_S_ENTRY_NUM 13
#define PCA_H_ENTRY_NUM 28
#define PCA_L_ENTRY_NUM 9

typedef struct {
	AGTX_INT16 h_table[PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM * PCA_L_ENTRY_NUM];
	AGTX_INT16 s_table[PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM * PCA_L_ENTRY_NUM];
	AGTX_INT16 l_table[PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM * PCA_L_ENTRY_NUM];
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_PCA_CONF_S;

#endif /* AGTX_DIP_PCA_CONF_H_ */