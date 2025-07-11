#ifndef AGTX_SYS_INFO_H_
#define AGTX_SYS_INFO_H_


#include "agtx_types.h"


#define MAX_AGTX_SYS_INFO_S_DEV_NAME_SIZE 33


typedef struct {
	AGTX_UINT8 dev_name[MAX_AGTX_SYS_INFO_S_DEV_NAME_SIZE];
} AGTX_SYS_INFO_S;


#endif /* AGTX_SYS_INFO_H_ */
