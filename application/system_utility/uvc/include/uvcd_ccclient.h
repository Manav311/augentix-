#ifndef UVCD_CCCLIENT_H_
#define UVCD_CCCLIENT_H_

#include "cc_common.h"

#define JSON_STR_LEN CC_JSON_STR_BUF_SIZE
#define JSON_EOF_CHAR '}'

int ccSet(char *jstr);
int ccGet(char *jstr);
int openCC(void);
void closeCC(void);

#define PREPARE_VIDEO(buf, width, height, fps, venc_type, bit_rate, cmd_type)                                                                                                                        \
	sprintf(buf,                                                                                                                                                                                 \
	        "\"video_strm_list\": [ { \"strm_en\": 1, \"width\": %d, \"height\": %d, \"output_fps\": %d, \"venc_type\": %d, \"cbr_bit_rate\": %u, \"vbr_max_bit_rate\": %u } ], \"cmd_id\": %d", \
	        width, height, fps, venc_type, bit_rate, bit_rate, cmd_type)

#endif /* UVCD__CCCLIENT_H_ */
