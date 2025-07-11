#ifndef EAIF_UTILS_INTERNAL_H_
#define EAIF_UTILS_INTERNAL_H_

#define CHN_UNINIT (0xff)

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_errno.h"
#include "mpi_index.h"
#include "mpi_iva.h"

#ifdef EAIF_INFERENCE_INAPP
#include "inf_classifier.h"
#include "inf_face.h"
#include "inf_detect.h"
#include "inf_image.h"
#endif // EAIF_INFERENCE_INAPP

#include "cm_iva_eaif_resp.h"

#include "eaif.h"
#include "eaif_log.h"

#define EAIF_RESP_LEN (4096 * 2)

#define EAIF_PAYLOAD_META_LEN 1024
#define EAIF_PAYLOAD_FMT_LEN 12
#define EAIF_PAYLOAD_TIME_LEN 12
#define EAIF_PAYLOAD_SHAPE_LEN 12 /* sizeof(int) * 3 */
#define EAIF_PAYLOAD_DATA_LEN (4096 * 512) /* ~2 Mb */

#define EAIF_PAYLOAD_NUM 5
#define EAIF_GETWINFRAME_TRY_MAX 2

#define EAIF_FIXED_POINT_BS (8)

typedef MPI_SIZE_S EaifFixedPointSize;

typedef enum { EAIF_MODE_REMOTE, EAIF_MODE_INAPP } EAIF_INFERENCE_MODE_E;

typedef struct {
	char name[EAIF_CHAR_LEN];
	void *ptr;
	size_t size;
} EaifRequestFormData;

typedef struct {
	union {
		EaifRequestFormData formdata[EAIF_PAYLOAD_NUM];
		struct {
			EaifRequestFormData meta; /* json str */
			EaifRequestFormData data; /* binary */
			EaifRequestFormData format; /* string */
			EaifRequestFormData time; /* string */
			EaifRequestFormData shape; /* [h-w-c] binary */
		};
	};
	//EAIF_PAYLOAD_STAT_E stat;
	unsigned char format_payload[EAIF_PAYLOAD_FMT_LEN];
	unsigned char meta_payload[EAIF_PAYLOAD_META_LEN];
	unsigned char data_payload[EAIF_PAYLOAD_DATA_LEN];
	unsigned char time_payload[EAIF_PAYLOAD_TIME_LEN];
	unsigned char shape_payload[EAIF_PAYLOAD_SHAPE_LEN];
} EaifRequestDataRemote;

typedef struct {
	union {
		// remote
		size_t size;
		// inapp
		uint32_t time;
	};
	struct {
		// remote
		char data[EAIF_RESP_LEN];
#ifdef EAIF_INFERENCE_INAPP
		InfResultList result_list;
		InfDetList det_list;
#endif
	};
} EaifRespond;

typedef struct {
	MPI_IVA_OBJ_LIST_S obj_list;
	MPI_VIDEO_FRAME_INFO_S frame_info;
    MPI_WIN target_idx;
#ifdef EAIF_INFERENCE_INAPP
    InfImage inf_image;
#endif
	int size;
} EaifRequestDataInApp;

typedef union {
	EaifRequestDataRemote *remote;
    EaifRequestDataInApp *inapp;
	void *ptr;
} EaifRequestData;

typedef struct {
	char path[EAIF_URL_CHAR_LEN];
#ifdef EAIF_INFERENCE_INAPP
	InfModelCtx ctx;
#endif
} InferenceModel;

struct eaif_info_s;
struct eaif_status_internal_s;
struct eaif_result_s;

const char *eaif_utilsGetModeChar(EAIF_INFERENCE_MODE_E mode);
EAIF_INFERENCE_MODE_E eaif_utilsDetermineMode(const char *str);
// int eaif_utilsInitRequestBuf(struct eaif_info_s *info);
int eaif_utilsCheckParam(const EAIF_PARAM_S *param);

void eaif_utilsSetUrl(EAIF_INFERENCE_MODE_E mode, const EAIF_PARAM_S *param, char *url);

int eaif_utilsFillMetaPayload(const MPI_IVA_OBJ_LIST_S *obj_list, EaifRequestDataRemote *payload);
int eaif_utilsFillTimePayload(uint32_t timestamp, EaifRequestDataRemote *payload);
int eaif_utilsFillShapePayload(const MPI_SIZE_S *res, EaifRequestDataRemote *payload, int channel);
int eaif_utilsFillFormatPayload(const EAIF_PARAM_S *param, EaifRequestDataRemote *payload);
int eaif_utilsFillDataPayload(const EAIF_PARAM_S *param, EaifRequestDataRemote *payload);

int eaif_utilsSetupRequestInapp(const MPI_IVA_OBJ_LIST_S *obj_list, const EAIF_PARAM_S *param, struct eaif_info_s *info,
                                EaifRequestDataInApp *payload);
int eaif_utilsSetupRequestRemote(const MPI_IVA_OBJ_LIST_S *obj_list, const EAIF_PARAM_S *param,
                                 struct eaif_info_s *info, EaifRequestDataRemote *payload);

int eaif_utilsSendRequestRemote(const char *url, struct eaif_info_s *info);
int eaif_utilsSendRequestInApp(struct eaif_info_s *info, InferenceModel *model);

int eaif_auxParseIvaRespJson(const EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp);

int eaif_utilsDecodeResult(EAIF_INFERENCE_MODE_E mode, const InferenceModel *model, EaifRespond *msg,
                           struct eaif_status_internal_s *status, AGTX_IVA_EAIF_RESP_S *resp);

int eaif_faceUtils(const EAIF_INF_UTILS_S *inf_utils, InferenceModel *model);

int eaif_calcScaleFactor(int dst_width, int dst_height, const MPI_SIZE_S *src, EaifFixedPointSize *scale_factor);
int eaif_assignFrameInfo(int fr_width, int fr_height, MPI_VIDEO_FRAME_INFO_S *frame_info);
int eaif_copyScaledListWithBoundary(const MPI_SIZE_S *resoln, const EaifFixedPointSize *scale_factor,
                                    const MPI_IVA_OBJ_LIST_S *src, MPI_IVA_OBJ_LIST_S *dst);
int eaif_getImageDataJpeg(MPI_ECHN chn, unsigned char *data);
int eaif_getImageDataSnapshot(MPI_WIN idx, MPI_VIDEO_FRAME_INFO_S *frame_info);

// put outside
int eaif_utilsTesturl(const char *url, struct eaif_status_internal_s *status);

int auxParseInappDetect(const InferenceModel *model, EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp);
int auxParseInappClassify(const InferenceModel *model, EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp);
int getInfFrame(const EAIF_PARAM_S *param, const struct eaif_info_s *info, EaifRequestDataInApp *payload);

#endif // EAIF_UTILS_INTERNAL_H_
