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

int testInappSetPayload(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;

	if (g_eaif_verbose)
		TI_TIC(start);

	EaifRequestDataInApp *payload = info->payload.inapp;
	if (payload == NULL) {
		eaif_log_warn("EAIF payload buffer is not init!");
		return -1;
	}

	payload->size = 0;
	payload->target_idx = param->target_idx;

	const bool face_det_roi = info->algo->face_det_roi;
	const bool face_preserve_prev = info->algo->face_preserve_prev;

	if (param->api == EAIF_API_FACEDET && face_preserve_prev && param->inf_with_obj_list)
		info->prev_obj_list = *local_list;

	if (param->api == EAIF_API_FACERECO) {
		// Full image recognition
		eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, local_list, &payload->obj_list);
	} else if (param->api == EAIF_API_FACEDET && face_det_roi && !param->inf_with_obj_list) {
		MPI_IVA_OBJ_LIST_S ol = {};
		ol.obj_num = 1;
		ol.obj[0] = (MPI_IVA_OBJ_ATTR_S){
			.id = 0,
			.life = 160,
			.rect = (MPI_RECT_POINT_S){ (info->src_resoln.width * 9) / 100,
			                            (info->src_resoln.height * 16) / 100,
			                            (info->src_resoln.width * 91) / 100,
			                            (info->src_resoln.height * 84) / 100 },
		};
		eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, &ol, &payload->obj_list);
	} else {
		eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, local_list, &payload->obj_list);
	}

	/* Setup image payload */
	testGetInfImage(ctx);

	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

	return 0;
}