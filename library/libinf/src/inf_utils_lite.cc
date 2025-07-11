#ifndef USE_NCNN
#include <memory>

#ifndef USE_MICROLITE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#pragma GCC diagnostic pop
#endif

#include "inf_utils_lite.h"
#include "inf_log.h"

InfDataType utils::lite::GetDataType(TfLiteType type)
{
	switch (type) {
	case kTfLiteUInt8:
		return Inf8U;
	case kTfLiteInt8:
		return Inf8S;
	case kTfLiteFloat32:
		return Inf32F;
	default:
		inf_log_err("TFLite inference is not implemented for dtype %d yet!", type);
		return InfUnknownType;
	}
}

#ifndef USE_MICROLITE
int utils::lite::LoadModels(const char* tflite_model,
	std::unique_ptr<tflite::Interpreter>& interpreter,
	std::unique_ptr<tflite::FlatBufferModel>& model)
{
	model =	tflite::FlatBufferModel::BuildFromFile(tflite_model);
	if (model == nullptr) {
		interpreter.reset();
		return -1;
	}

	// Build the interpreter
	tflite::ops::builtin::BuiltinOpResolver resolver;
	tflite::InterpreterBuilder builder(*model, resolver);
	builder(&interpreter);
	if (interpreter == nullptr) {
		return -1;
	}
	return 0;
}
#endif
#endif // USE_NCNN