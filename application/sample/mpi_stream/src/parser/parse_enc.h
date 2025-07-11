#ifndef PARSE_ENC_H_
#define PARSE_ENC_H_

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

#include "mpi_enc.h"
#include "sample_stream.h"

int parse_enc_chn_param(char *tok, SAMPLE_CONF_S *conf);
void print_venc_attr(MPI_VENC_ATTR_S *p);
void init_enc_chn_conf(CONF_ECHN_PARAM_S *p);

#ifdef __cplusplus
}
#endif /* !__cplusplus */

#endif /* !PARSE_ENC_H_ */
