#ifndef EF_DEMO_H_
#define EF_DEMO_H_

#include "mpi_index.h"
#include "vftr_ef.h"
#include "vftr_shd.h"

int detectEfObject(MPI_WIN win_idx, MPI_SIZE_S *res, VFTR_EF_PARAM_S *ef_param, VFTR_SHD_PARAM_S *shd_attr,
                   VFTR_SHD_LONGTERM_LIST_S *shd_long_list);
extern VFTR_SHD_PARAM_S g_shd_param;
extern VFTR_SHD_LONGTERM_LIST_S g_shd_long_list;

#endif
