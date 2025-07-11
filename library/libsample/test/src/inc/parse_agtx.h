#ifndef PARSE_AGTX_H_
#define PARSE_AGTX_H_

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

#include <stdlib.h>
#include "mpi_dev.h"

#include "sample_stream.h"

int parseLayout(const SAMPLE_CONF_S *conf, const int chn_idx, const char *path);
int parseCodecAttr(const SAMPLE_CONF_S *sample_conf, const int enc_idx, const char *path);
int parseRateControlAttr(const int enc_idx, const char *path);
int parseViewTypeAttr(const SAMPLE_CONF_S *sample_conf, const MPI_WIN_VIEW_TYPE_E type, const char *path);

#ifdef __cplusplus
}
#endif /* !__cplusplus */

#endif /* !PARSE_DEV_H_ */