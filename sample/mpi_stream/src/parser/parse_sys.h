#ifndef PARSE_SYS_H_
#define PARSE_SYS_H_

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

#include "sample_stream.h"

int parse_sys_param(char *tok, SAMPLE_CONF_S *conf);
void init_sys_conf(CONF_SYS_PARAM_S *conf);

#ifdef __cplusplus
}
#endif /* !__cplusplus */

#endif /* !PARSE_OSD_H_ */
