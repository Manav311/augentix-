#ifndef TD_DEMO_H_
#define TD_DEMO_H_

#include "mpi_index.h"
#include "vftr_dk.h"

int detectDkObject(MPI_WIN win_idx, MPI_SIZE_S *res, VFTR_DK_PARAM_S *dk_param);

#endif