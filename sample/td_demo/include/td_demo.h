#ifndef TD_DEMO_H_
#define TD_DEMO_H_

#include "mpi_index.h"
#include "vftr_td.h"

int detectTdObject(MPI_WIN win_idx, MPI_CHN_LAYOUT_S *chn_layout, VFTR_TD_PARAM_S *td_param);

typedef struct {
	int is_reset;
	int is_write;
	UINT8 mv_hist_cfg_idx;
	UINT8 var_cfg_idx;
	UINT8 y_avg_cfg_idx;
	VFTR_TD_MPI_INPUT_S mpi_input;
	MPI_PATH path;
} TD_CTX_S;

#endif
