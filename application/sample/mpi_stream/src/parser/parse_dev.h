#ifndef PARSE_DEV_H_
#define PARSE_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

#include "sample_stream.h"

int parse_dev_param(char *tok, SAMPLE_CONF_S *conf);
void print_chn_layout(MPI_CHN_LAYOUT_S *p);
void print_window_attr(int k, MPI_WIN_ATTR_S *p);
void print_stitch_attr(MPI_STITCH_ATTR_S *p);
void print_ldc_attr(MPI_LDC_ATTR_S *p);
void print_panorama_attr(MPI_PANORAMA_ATTR_S *p);
void print_panning_attr(MPI_PANNING_ATTR_S *p);
void print_surround_attr(MPI_SURROUND_ATTR_S *p);
void init_path_conf(CONF_PATH_PARAM_S *p);
void init_win_conf(MPI_WIN_ATTR_S *p);
void init_chn_conf(CONF_CHN_PARAM_S *p);

#ifdef __cplusplus
}
#endif /* !__cplusplus */

#endif /* !PARSE_DEV_H_ */
