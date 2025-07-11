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

int testRemoteSetPayload(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;
	MPI_IVA_OBJ_LIST_S *obj_list = &info->local_list;

	if (g_eaif_verbose)
		TI_TIC(start);

	EaifRequestDataRemote *payload = info->payload.remote;
	if (payload == NULL) {
		eaif_log_warn("EAIF payload buffer is not init!");
		return -1;
	}

	MPI_IVA_OBJ_LIST_S scaled_list = {};
	MPI_SIZE_S resoln = {};
	int channel = 3;
	const MPI_IVA_OBJ_LIST_S *ol_ptr = obj_list;
	const MPI_SIZE_S *resoln_ptr = &info->src_resoln;

	if (param->data_fmt == EAIF_DATA_MPI_Y) {
		eaif_copyScaledObjList(&info->scale_factor, obj_list, &scaled_list);
		ol_ptr = &scaled_list;
		resoln.width = param->snapshot_width;
		resoln.height = param->snapshot_height;
		resoln_ptr = &resoln;
		channel = 1;
	}

	/* Setup meta payload metadata*/
	eaif_utilsFillMetaPayload(ol_ptr, payload);

	/* Setup time payload metadata*/
	eaif_utilsFillTimePayload(obj_list->timestamp, payload);

	/* Setup shape payload */
	eaif_utilsFillShapePayload(resoln_ptr, payload, channel);

	/* Setup format payload */
	eaif_utilsFillFormatPayload(param, payload);

	testRemoteSetDataPayload(param, payload);

	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

	return 0;
}