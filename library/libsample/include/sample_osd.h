#ifndef SAMPLE_OSD_H_
#define SAMPLE_OSD_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "mpi_index.h"
#include "mpi_osd.h"

INT32 SAMPLE_createOsdUpdateThread(void);
INT32 SAMPLE_destroyOsdUpdateThread(void);
INT32 SAMPLE_stopOsd(MPI_CHN chn_idx);
INT32 SAMPLE_createOsd(bool visible, MPI_CHN chn_idx, INT32 output_num, UINT16 width, UINT16 height);
void SAMPLE_initOsd(void);
void SAMPLE_freeOsdResources(void);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /**< SAMPLE_OSD_H_ */
