#ifndef TEST_APP_H_
#define TEST_APP_H_

// for fake interface
#include "mpi_dev.h"

// for src
#include "app.h"
#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"

// for utils
#include "utils.h"

typedef struct {
	int size;
	MPI_IVA_OBJ_LIST_S *obj_list;
} ObjListArray;

typedef struct {
	float video_fps;
	int width;
	int height;
	int channel;
	int num_frames;
	char eaif_param_file[512];
	char eaif_objList_file[512];
	char video_input_file[512];
} EaifTestConfig;

typedef struct {
	CvVideo vid;
	CvImage image;
	int is_running;
	FILE *fw;
} EaifCvCtx;


int testRemoteSetDataPayload(const EAIF_PARAM_S *param, EaifRequestDataRemote *payload);
int testGetInfImage(void *ctx);

int testRemoteSetPayload(void *ctx);
int testInappSetPayload(void *ctx);
int testFaceRecoInappSetPayload(void *ctx);

#endif // TEST_APP_H_