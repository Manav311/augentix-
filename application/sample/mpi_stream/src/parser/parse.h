#ifndef PARSE_H_
#define PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "sample_stream.h"

/**
 * @struct APP_CONF_S
 * @brief contain all configuration fron case_config and arguments
 */
typedef struct {
	SAMPLE_CONF_S sample_conf; /**< all config from case_config */
	INT32 reservation_level; /**< for sample_publisher */
	INT32 recycle_level; /**< for sample_publisher*/
	LightSrc *head[MPI_MAX_INPUT_PATH_NUM]; /**< whether support multiple iq or not, day lightSrc always created. */
	LightSrcDetection
	        *detection[MPI_MAX_INPUT_PATH_NUM]; /**< lightSrc detection method and params of each input path*/
	char window_path[PATH_MAX]; /**< window.ini path for sub-stream IQ */
} APP_CONF_S;

int parseCmdArgs(int argc, char *argv[], APP_CONF_S *conf);
void initConf(APP_CONF_S *conf);
void printHelp(char *str);
void printConf(SAMPLE_CONF_S *conf);
void deinitConf(APP_CONF_S *conf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !PARSE_H_ */
