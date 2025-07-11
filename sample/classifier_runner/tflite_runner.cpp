#include <memory>
#include <cstdio>
#include <list>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model_builder.h"
#include "tensorflow/lite/kernels/register.h"

#include "utils.h"

constexpr const char *g_version = "0.1";


std::unique_ptr<tflite::FlatBufferModel> loadModelFromFile(const char *filename)
{
	return tflite::FlatBufferModel::BuildFromFile(filename);
}

bool createInterpreterWithModel(std::unique_ptr<tflite::Interpreter> &interpreter, const tflite::FlatBufferModel &model)
{
	tflite::ops::builtin::BuiltinOpResolver resolver;
	tflite::InterpreterBuilder builder(model, resolver);
	TfLiteStatus status = builder(&interpreter);
	return status == kTfLiteOk;
}



int main(int argc, const char *argv[])
{
	printf("===== %s v%s =====\n\n", argv[0], g_version);
	if (argc < 3) {
		printUsage(argv[0]);
		return 1;
	}

	const char *modelFilePath = argv[1];
	const std::unique_ptr<tflite::FlatBufferModel> model = loadModelFromFile(modelFilePath);
	if (!model) {
		printf("[ERROR] FAILED to load model from file: %s\n", modelFilePath);
		return 2;
	}

	std::unique_ptr<tflite::Interpreter> interpreter;
	if (!createInterpreterWithModel(interpreter, *model)) {
		printf("[ERROR] FAILED to create interpreter!!!\n");
		return 3;
	}

	TfLiteStatus status = interpreter->AllocateTensors();
	if (status != kTfLiteOk) {
		printf("[ERROR] AllocateTensors() FAILED!!!\n");
		return 4;
	}

	TfLiteTensor *inputTensor = interpreter->input_tensor(0);
	printf("Input tensor dims(%d): \n", inputTensor->dims->size);
	for (int i = 0; i < inputTensor->dims->size; ++i) {
		printf("%d\n", inputTensor->dims->data[i]);
	}

	const char *reason;
	if (!isInputTypeSupported(inputTensor, &reason)) {
		printf("[ERROR] %s!!!\n", reason);
		return 5;
	}

	std::list<const char*> inputFiles;
	for (int i = 2; i < argc; ++i) {
		inputFiles.emplace_back(argv[i]);
	}

	std::for_each(inputFiles.begin(), inputFiles.end(), [&](const char *filename) {
		printf("[INFO] loading %s ...\n", filename);
		size_t contentSize;
		void *inputData = loadFileContent(filename, inputTensor->bytes, &contentSize);
		if (!populateInputTensor(inputTensor, inputData, contentSize)) {
			printf("[ERROR] CANNOT populate input tensor!!!\n");
			return;
		}
		free(inputData);

		TfLiteStatus infStatus = interpreter->Invoke();
		if (infStatus != kTfLiteOk) {
			printf("[ERROR] do inference FAILED!!!\n");
			return;
		}

		float score;
		TfLiteTensor *outputTensor = interpreter->output_tensor(0);
		TfLiteType outputType = outputTensor->type;
		if (outputType == kTfLiteUInt8) {
			int n = interpreter->typed_output_tensor<uint8_t>(0)[0];
			printf("raw: %4d   ", n);
			score = outputTensor->params.scale * (n - outputTensor->params.zero_point);
		} else if (outputType == kTfLiteInt8) {
			int n = interpreter->typed_output_tensor<int8_t>(0)[0];
			printf("raw: %4d   ", n);
			score = outputTensor->params.scale * (n - outputTensor->params.zero_point);
		} else if (outputType == kTfLiteFloat32) {
			score = interpreter->typed_output_tensor<float>(0)[0];
		} else {
			printf("[WARNING] MEANINGLESS score!\n");
			score = 0;
		}
		float conf = 1 / (1 + std::exp(-score));
		printf("score: %.6f\tconf: %.6f\n", score, conf);
	});

	return 0;
}
