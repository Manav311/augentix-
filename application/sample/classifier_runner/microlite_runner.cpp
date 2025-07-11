#include <list>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "utils.h"
//#include "math/ca7_math.h"

constexpr const char *g_version = "0.1";

constexpr int kTensorArenaSize = 4 * 1024 * 1024;
alignas(8) uint8_t tensor_arena[kTensorArenaSize];

constexpr size_t kModelStoreSize = 2 * 1024 * 1024;
alignas(8) uint8_t model_store[kModelStoreSize];

#define REGISTER_STATIC_OP_RESOLVER(micro_op_resolver)               \
	static int op_registered = 0;                                \
	static tflite::MicroMutableOpResolver<19> micro_op_resolver; \
	if (!op_registered) {                                        \
		micro_op_resolver.AddConcatenation();                \
		micro_op_resolver.AddConv2D();                       \
		micro_op_resolver.AddDepthwiseConv2D();              \
		micro_op_resolver.AddFullyConnected();               \
		micro_op_resolver.AddMaxPool2D();                    \
		micro_op_resolver.AddMean();                         \
		micro_op_resolver.AddPad();                          \
		micro_op_resolver.AddQuantize();                     \
		micro_op_resolver.AddRelu();                         \
		micro_op_resolver.AddReshape();                      \
		micro_op_resolver.AddStridedSlice();                 \
		micro_op_resolver.AddTranspose();                    \
		/*------------*/                                     \
		micro_op_resolver.AddAveragePool2D();                \
		micro_op_resolver.AddSoftmax();                      \
                micro_op_resolver.AddAdd();                          \
                micro_op_resolver.AddLogistic();                     \
                micro_op_resolver.AddPack();                         \
                micro_op_resolver.AddResizeNearestNeighbor();        \
                micro_op_resolver.AddShape();        	             \
		op_registered = 1;                                   \
	}


tflite::MicroInterpreter *createInterpreter(void *modelData)
{
	static tflite::MicroErrorReporter error_reporter;
	TF_LITE_REPORT_ERROR(&error_reporter, "[DEBUG] testing error reporter.");
	static const tflite::Model *model = tflite::GetModel(modelData);
	if (model->version() != TFLITE_SCHEMA_VERSION) {
		TF_LITE_REPORT_ERROR(&error_reporter,
		                     "Model provided is schema version %d not equal "
		                     "to supported version %d.",
		                     model->version(), TFLITE_SCHEMA_VERSION);
		return nullptr;
	}

	static tflite::AllOpsResolver resolver;
//	REGISTER_STATIC_OP_RESOLVER(resolver);
	static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize, &error_reporter);

	return &interpreter;
}

int main(int argc, const char *argv[])
{
	printf("===== %s v%s =====\n\n", argv[0], g_version);
	if (argc < 2) {
		printUsage(argv[0]);
		return 1;
	}

	const char *modelFilePath = argv[1];
	std::string modelFile{modelFilePath};
//	const size_t modelBufferSize = 2 * 1024 * 1024;
	size_t modelSize;
	/*void *modelData = loadFileContent(modelFilePath, modelBufferSize, &modelSize);
	if (!modelData) {
		printf("[ERROR] FAILED to load model from file: %s\n", modelFilePath);
		return 2;
	}*/
	loadFileContentTo(model_store, modelFilePath, kModelStoreSize, &modelSize);

	printf("[DEBUG] model size = %d bytes.\n", modelSize);
	if (modelSize == kModelStoreSize) {
		printf("[WARNING] model file MAYBE larger than buffer size(%d bytes)!\n", kModelStoreSize);
	}

	tflite::MicroInterpreter *interpreter = createInterpreter(model_store);
	if (!interpreter) {
		printf("[ERROR] FAILED to create interpreter!!!\n");
		return 3;
	}

	if (modelFile.find("pnet", 0) != std::string::npos) {
		printf("[DEBUG] resizing input tensor.\n");
		TfLiteTensor *input_var = interpreter->input_tensor(0);
		printf("Is input a variable tensor? %d\n", input_var->is_variable);
		TfLiteIntArray *dims = input_var->dims;
		dims->data[1] = dims->data[2] = 12;
		input_var->bytes = dims->data[0] * dims->data[1] * dims->data[2] * dims->data[3];
		printf("[DEBUG] allocate tensors.\n");
		TfLiteStatus status = interpreter->AllocateTensors();
		if (status != kTfLiteOk) {
			printf("[ERROR] AllocateTensors() FAILED!!!\n");
			return 4;
		}
	} else {
		printf("[DEBUG] allocate tensors.\n");
		TfLiteStatus status = interpreter->AllocateTensors();
		if (status != kTfLiteOk) {
			printf("[ERROR] AllocateTensors() FAILED!!!\n");
			return 4;
		}
	}

	TfLiteTensor *inputTensor = interpreter->input(0);
	printf("Input tensor dims(%d): \n", inputTensor->dims->size);
	for (int i = 0; i < inputTensor->dims->size; ++i) {
		printf("%d\n", inputTensor->dims->data[i]);
	}

	printf("[INFO] %d bytes arena @[%p, %p) used.\n", interpreter->arena_used_bytes(), tensor_arena,
	       tensor_arena + kTensorArenaSize);



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
		TfLiteTensor *outputTensor = interpreter->output(0);
		/*printf("[DEBUG] probing output tensor.\n");
		probeTensor(outputTensor, "OT: ");*/

		TfLiteType outputType = outputTensor->type;
		if (outputType == kTfLiteUInt8) {
			int n = interpreter->typed_output_tensor<uint8_t>(0)[0];
			printf("raw: %4d   ", n);
			score = outputTensor->params.scale * (n - outputTensor->params.zero_point);
		} else if (outputType == kTfLiteInt8) {
			int n = interpreter->typed_output_tensor<int8_t>(0)[0];
			printf("raw: %4d   ", n);
			score = outputTensor->params.scale * (n - outputTensor->params.zero_point);
//		} else if (outputType == kTfLiteFloat32) {
//			score = interpreter->typed_output_tensor<float>(0)[0];
		} else {
			printf("[WARNING] MEANINGLESS score!\n");
			score = 0;
		}
		float conf = 1 / (1 + std::exp(-score));
		printf("score: %.6f\tconf: %.6f\n", score, conf);
	});
	return 0;
}
