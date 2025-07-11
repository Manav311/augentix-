#include <malloc.h>
#include <string.h>
#include "utils.h"

void printUsage(const char *progName)
{
	printf("Usage:\n");
	printf("%s model_file_path [input_file ...]\n", progName);
}

bool isInputTypeSupported(const TfLiteTensor *tensor, const char **reason)
{
	printf("[DEBUG] tensor type = %d\n", tensor->type);
	if (tensor->type == kTfLiteInt8 || tensor->type == kTfLiteUInt8) {
		return true;
	}

	if (reason) {
		*reason = "currently ONLY support int8/uint8 input tensor";
	}
	return false;
}

bool populateInputTensor(const TfLiteTensor *tensor, void *data, size_t dataSize)
{
	return populateInput(tensor->type, tensor->data.data, tensor->bytes, data, dataSize);
}

bool populateInput(TfLiteType inputType, void *inputBuffer, size_t bufferSize, void *data, size_t dataSize)
{
	if (bufferSize != dataSize) {
		printf("[DEBUG] data size %d is NOT equal to tensor expected size %d.\n", dataSize, bufferSize);
		return false;
	}

	if (inputType == kTfLiteUInt8) {
		memcpy(inputBuffer, data, dataSize);
		return true;
	} else if (inputType == kTfLiteInt8) {
		uint8_t *p = data;
		uint8_t *q = inputBuffer;
		for (size_t i = 0; i < dataSize; ++i) {
			*q = *p - 128;
			++p;
			++q;
		}
		return true;
	} else {
		return false;
	}
}

void *loadFileContent(const char *filename, size_t maxBytes, size_t *loadedBytes)
{
	uint8_t *buffer = malloc(maxBytes);
	//	FILE *fp = fopen(filename, "rb");
	//	*loadedBytes = fread(buffer, 1, maxBytes, fp);
	//	fclose(fp);
	loadFileContentTo(buffer, filename, maxBytes, loadedBytes);
	return buffer;
}

void loadFileContentTo(void *at, const char *filename, size_t maxBytes, size_t *loadedBytes)
{
	FILE *fp = fopen(filename, "rb");
	*loadedBytes = fread(at, 1, maxBytes, fp);
	fclose(fp);
}

#ifndef TF_LITE_STATIC_MEMORY
void probeTensor(const TfLiteTensor *tensor, const char *prefix)
{
	printf("%stype=%d\n", prefix, tensor->type);
	printf("%sdata.data=%p\n", prefix, tensor->data.data);
	printf("%sdims=%p\n", prefix, tensor->dims);
	printf("%sparams{scale=%f, zero_point=%d}\n", prefix, tensor->params.scale, tensor->params.zero_point);
	printf("%sallocation type=%d\n", prefix, tensor->allocation_type);
	printf("%sbytes=%d\n", prefix, tensor->bytes);
	printf("%sallocation=%p\n", prefix, tensor->allocation);
	printf("%sname=%p\n", prefix, tensor->name);
	printf("%sdelegate=%p\n", prefix, tensor->delegate);
	printf("%sbuffer handle=%d\n", prefix, tensor->buffer_handle);
	printf("%sdata stale?=%d\n", prefix, tensor->data_is_stale);
	printf("%sis variable?=%d\n", prefix, tensor->is_variable);
	printf("%squantization{type=%d, params=%p}\n", prefix, tensor->quantization.type, tensor->quantization.params);
	printf("%sdims signature=%p\n", prefix, tensor->dims_signature);
	//	printf("%s\n", prefix);
}
#else
void probeTensor(const TfLiteTensor *tensor, const char *prefix)
{
	printf("%squantization{type=%d, params=%p}\n", prefix, tensor->quantization.type, tensor->quantization.params);
	printf("%sparams{scale=%f, zero_point=%d}\n", prefix, tensor->params.scale, tensor->params.zero_point);
	printf("%sdata.data=%p\n", prefix, tensor->data.data);
	printf("%sdims=%p\n", prefix, tensor->dims);
	printf("%sbytes=%d\n", prefix, tensor->bytes);
	printf("%stype=%d\n", prefix, tensor->type);
	printf("%sallocation type=%d\n", prefix, tensor->allocation_type);
	printf("%sis variable?=%d\n", prefix, tensor->is_variable);
}
#endif
