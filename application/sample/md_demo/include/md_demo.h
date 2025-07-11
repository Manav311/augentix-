#ifndef MD_DEMO_H_
#define MD_DEMO_H_

#include "mpi_index.h"
#include "vftr_md.h"
#include "vftr_shd.h"

int detectMdObject(MPI_WIN win_idx, MPI_SIZE_S *res, VFTR_MD_PARAM_S *md_attr, VFTR_SHD_PARAM_S *shd_attr,
                   VFTR_SHD_LONGTERM_LIST_S *shd_long_list);
extern VFTR_SHD_PARAM_S g_shd_param;
extern VFTR_SHD_LONGTERM_LIST_S g_shd_long_list;

#endif