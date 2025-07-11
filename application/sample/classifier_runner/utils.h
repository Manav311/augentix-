#ifndef CLASSIFIER_RUNNER_UTILS_H
#define CLASSIFIER_RUNNER_UTILS_H

#include "tensorflow/lite/c/common.h"

#ifdef __cplusplus
extern "C" {
#endif

void printUsage(const char *progName);
bool isInputTypeSupported(const TfLiteTensor *tensor, const char **reason);
bool populateInputTensor(const TfLiteTensor *tensor, void *data, size_t dataSize);
bool populateInput(TfLiteType inputType, void *inputBuffer, size_t bufferSize, void *data, size_t dataSize);
void *loadFileContent(const char *filename, size_t maxBytes, size_t *loadedBytes);
void loadFileContentTo(void *at, const char *filename, size_t maxBytes, size_t *loadedBytes);

void probeTensor(const TfLiteTensor *tensor, const char *prefix);
#ifdef __cplusplus
}
#endif

#endif //CLASSIFIER_RUNNER_UTILS_H
