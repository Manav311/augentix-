#ifndef FACEREC0_DEMO_H_
#define FACEREC0_DEMO_H_

#include "mpi_index.h"
#include "mpi_iva.h"
#include "eaif.h"

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

typedef EAIF_PARAM_S FACERECO_PARAM_S;

int runFaceRecognition(MPI_WIN idx, FACERECO_PARAM_S *facereco_param);

#endif /* !FACEREC0_DEMO_H_ */