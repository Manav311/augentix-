#ifndef USE_NCNN
#ifndef INF_UTILS_LITE_H_
#define INF_UTILS_LITE_H_

#include <memory>

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/c/c_api_types.h"

#ifndef USE_MICROLITE
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#endif

#include "inf_types.h"

class LiteModelTraits {
    public:
	virtual bool PrepareInterpreter(const std::string& model_path) = 0;
	virtual bool CollectModelInfo() = 0;
	virtual void* InputTensorBuffer(size_t input_index) = 0;
	virtual void* OutputTensorBuffer(size_t output_index) = 0;
	virtual TfLiteStatus Invoke() = 0;
	virtual size_t GetArenaUsedBytes() = 0;
};

namespace utils
{
namespace lite
{
#ifndef USE_MICROLITE
int LoadModels(const char *tflite_model, std::unique_ptr<tflite::Interpreter> &interpreter,
               std::unique_ptr<tflite::FlatBufferModel> &model);
#endif

InfDataType GetDataType(TfLiteType type);

template <typename IC> InfImage GetInputImage(const std::unique_ptr<IC> &model)
{
	InfImage img{};

	if (model == nullptr)
		return img;

	const TfLiteIntArray *in_dims = model->input_tensor(0)->dims;

	img.h = in_dims->data[1];
	img.w = in_dims->data[2];
	img.c = in_dims->data[3];

	int dtype = utils::lite::GetDataType(model->input_tensor(0)->type);

	img.buf_owner = 0;
	if (dtype == Inf8U) {
		img.data = model->template typed_input_tensor<uint8_t>(0);
	} else if (dtype == Inf8S) {
		img.data = reinterpret_cast<uint8_t *>(model->template typed_input_tensor<int8_t>(0));
	} else {
		img.data = nullptr;
	}

	img.dtype = static_cast<InfDataType>(dtype | ((img.c - 1) << 3));

	return img;
}
} // lite
} // utils

#endif
#endif // USE_NCNN