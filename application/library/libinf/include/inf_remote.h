#ifndef INF_REMOTE_H_
#define INF_REMOTE_H_

#include <stddef.h>

#include "tensorflow/lite/c/c_api_types.h"

#define TENSOR_DIMENSION_MAX 5
#define MAX_INPUT_TENSOR 10
#define MAX_OUTPUT_TENSOR 10

#ifdef __cplusplus
extern "C" {
#endif

typedef struct inf_tensor_info {
	TfLiteType type;
	int nums_dims;
	int dims[TENSOR_DIMENSION_MAX];
	int32_t zero_point;
	float scale;
	size_t bytes;
	size_t arena_offset;
} InfTensorInfo;

typedef struct inf_remote_model_info {
	size_t nums_input_tensor;
	size_t nums_output_tensor;
	size_t operators_size;
	size_t arena_used_bytes;
	InfTensorInfo input_tensors[MAX_INPUT_TENSOR];
	InfTensorInfo output_tensors[MAX_OUTPUT_TENSOR];
} InfRemoteModelInfo;

typedef struct inf_remote_model_context {
	const void *model;
	void *interpreter;
	bool verbose;
} InfRemoteModelContext;

typedef struct info_load_model_params {
	void *model_data;
	uint8_t *arena;
	size_t arena_size;
	InfRemoteModelInfo *model_info;
	InfRemoteModelContext *context;
} InfLoadModelParams;

#ifdef __cplusplus
}
#endif

#endif // INF_REMOTE_H_
