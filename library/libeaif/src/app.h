#ifndef APP_H_
#define APP_H_

#include "mpi_iva.h"

/* Base de/activate class */
int baseInitAlgo(void *ctx);
int baseReleaseBuffer(void *ctx);

/* Remote run class */
int remoteProbe(void *ctx);
//int remoteInitBuf(EaifInfo *info);
int remoteInit(void *ctx);
int remoteSetPayload(void *ctx);
int remoteSendRequest(void *ctx);
int remoteDecodeResult(void *ctx);
int remoteMergeResult(void *ctx);

/* Remote activate class */
int remoteInitModule(void *ctx);
int remoteExitModule(void *ctx);

/* Inapp run class */
int inappProbe(void *ctx);
//int inappInitBuf(EaifInfo *info);
int inappInit(void *ctx);
int inappSetPayload(void *ctx);
int inappSendRequest(void *ctx);
int inappMergeResult(void *ctx);
int inappInitFaceReco(void *ctx);

/* Inapp activate class */
int inappInitModule(void *ctx);
int inappExitModule(void *ctx);

int classifySetLocalInfo(void *ctx);
int classifyInappSendRequest(void *ctx);
int classifyInappDecodeResult(void *ctx);
int classifyMergeResult(void *ctx);
int classifyUpdate(void *ctx, const MPI_IVA_OBJ_LIST_S *obj_list);

int detectSetLocalInfo(void *ctx);
int detectInappSendRequest(void *ctx);
int detectInappDecodeResult(void *ctx);
int detectInappDecodeResultV2(void *ctx);
int detectMergeResult(void *ctx);
int detectUpdate(void *ctx, const MPI_IVA_OBJ_LIST_S *obj_list);

int faceRecoInappInitAlgo(void *ctx);
int faceRecoInappSetLocalInfo(void *ctx);
int faceRecoInappSetPayload(void *ctx);
int faceRecoInappSetPayloadV2(void *ctx);
int faceRecoInappSendRequest(void *ctx);
int faceRecoInappSendRequestV2(void *ctx);
int faceRecoInappDecodeResult(void *ctx);
int faceRecoMergeResult(void *ctx);
int faceRecoUpdate(void *ctx, const MPI_IVA_OBJ_LIST_S *obj_list);

int initRemoteCb(void *ctx);
int initInappClassifyCb(void *ctx);
int initInappDetectCb(void *ctx);
int initInappFaceRecoCb(void *ctx);
int initInappFaceRecoCbV2(void *ctx);

int initAppCb(void *ctx);
int delAppCb(void *ctx);

#endif // APP_H_