#ifndef SAMPLE_UTILS_H_
#define SAMPLE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "mpi_base_types.h"

MPI_RECT_S SAMPLE_toMpiLayoutWindow(const MPI_RECT_S *pos, const MPI_SIZE_S *chn_res);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /**< SAMPLE_UTILS_H_ */