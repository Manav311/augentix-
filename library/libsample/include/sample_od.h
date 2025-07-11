#ifndef SAMPLE_OD_H_
#define SAMPLE_OD_H_
#ifdef CB_BASED_OD

#include "ml.h"

INT32 SAMPLE_OD_registerCallback(MPI_WIN idx, MPI_IVA_OD_CALLBACK_S *cb);

#endif //CB_BASED_OD
#endif //SAMPLE_OD_H_
