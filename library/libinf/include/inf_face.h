#ifndef INF_FACE_H_
#define INF_FACE_H_

#include "inf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#include "inf_model.h"

#define INF_FACEDET_MTCNN_NAME "mtcnn"
#define INF_FACEDET_SCRFD_NAME "scrfd"

int Inf_InvokeFaceDet(InfModelCtx *ctx, const InfImage *img, InfDetList *result);
int Inf_InvokeFaceDetObjList(InfModelCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result);

int Inf_RegisterFaceRoi(InfModelCtx *ctx, const InfImage *img, const MPI_RECT_POINT_S *roi, const char *face_name);
int Inf_RegisterFaceRoiDet(InfModelCtx *ctx, const InfImage *img, const MPI_RECT_POINT_S *roi, const char *face_name);
int Inf_DeleteFace(InfModelCtx *ctx, const char *face_name);

int Inf_SaveFaceData(InfModelCtx *ctx, const char *face_database);
int Inf_LoadFaceData(InfModelCtx *ctx, const char *face_database);
int Inf_DeleteFaceData(InfModelCtx *ctx, const char *face);
int Inf_ResetFaceData(InfModelCtx *ctx);

int Inf_InvokeFaceRecoRoi(InfModelCtx *ctx, const InfImage *img, const MPI_RECT_POINT_S *rect,
                             InfDetList *result);
int Inf_InvokeFaceRecoList(InfModelCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result);

int Inf_InvokeFaceRecoStageOne(InfModelCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result);
int Inf_InvokeFaceRecoStageTwo(InfModelCtx *ctx, InfDetList *result);

// for ut only
int Inf_InvokeFaceEncode(InfModelCtx *ctx, const InfImage *img, const MPI_RECT_POINT_S *roi, int *encode_dim, float **face_encode);

#ifdef __cplusplus
}
#endif

#endif
