#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
// for fake interface
#include "mpi_dev.h"
#include "mpi_iva.h"
// for src
#include "app.h"
#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"

#ifdef EAIF_INFERENCE_INAPP
#include "inf_types.h"
#endif

// for utils
#include "test_app.h"
#include "utils.h"
extern int g_eaif_verbose;
static struct timespec start;

int testFaceRecoInappSetPayload(void *ctx)
{
	if (g_eaif_verbose)
		TI_TIC(start);

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;
	EaifRequestDataInApp *payload = info->payload.inapp;

	if (info->inf_fr_stage == 1) { /* stage 2 by pass image query */
		payload->size = 1;
		return 0;
	}

	eaif_log_debug("Enter");

	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	if (payload == NULL) {
		eaif_log_warn("EAIF payload inapp buffer is not init!");
		return -1;
	}
	// if fr and stage1 else pass
	eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, local_list, &payload->obj_list);

	testGetInfImage(ctx);

#endif // EAIF_INFERENCE_INAPP;
	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

	return ret;
}