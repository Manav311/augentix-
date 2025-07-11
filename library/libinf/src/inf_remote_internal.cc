#ifdef INF_ENABLE_REMOTE_CORE

#include "inf_remote_internal.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"

RemoteCore g_remote_core;
RemoteFunction g_load_model(g_remote_core, "inf-load-model");
RemoteFunction g_model_forward(g_remote_core, "inf-model-forward");

bool RemoteInfModelTraits::PrepareInterpreter(const std::string &model_path)
{
	_arena = std::unique_ptr<RemoteMemory>(new RemoteMemory(g_remote_core, 768 * 1024));
	_model_info = std::unique_ptr<RemoteMemory>(new RemoteMemory(g_remote_core, sizeof(InfRemoteModelInfo)));
	_model_context = std::unique_ptr<RemoteMemory>(new RemoteMemory(g_remote_core, sizeof(InfRemoteModelContext)));
	int model_size;
	unsigned char* model_content = LoadModelData(model_path.c_str(), &model_size);
	_model_data = std::unique_ptr<RemoteMemory>(new RemoteMemory(g_remote_core, model_size));
	memcpy(_model_data->As<void>(), model_content, model_size);
	delete[] model_content;

	auto context = _model_context->As<InfRemoteModelContext>();
	context->verbose = InVerboseMode();

	InfLoadModelParams params{
		.model_data = _model_data->RealAddress<void>(),
		.arena = _arena->RealAddress<uint8_t>(),
		.arena_size = _arena->Size(),
		.model_info = _model_info->RealAddress<InfRemoteModelInfo>(),
		.context = _model_context->RealAddress<InfRemoteModelContext>(),
	};

	if (AMPI_FAILURE == ampi_send(g_load_model, &params, sizeof(params), -1)) {
		DEBUG("send request to remote %s FAILED!\n", g_load_model.description().c_str());
		return false;
	}

	int32_t response;
	if (AMPI_FAILURE == ampi_receive(g_load_model, &response, sizeof(response), -1)) {
		DEBUG("remote %s responding failure(%d)\n", g_load_model.description().c_str(), response);
		return false;
	}

	return response == 0;
}

void* RemoteInfModelTraits::InputTensorBuffer(size_t input_index)
{
	InfRemoteModelInfo& info = *_model_info->As<InfRemoteModelInfo>();
	return _arena->As<uint8_t>() + info.input_tensors[input_index].arena_offset;
}

void* RemoteInfModelTraits::OutputTensorBuffer(size_t output_index)
{
	InfRemoteModelInfo& info = *_model_info->As<InfRemoteModelInfo>();
	return _arena->As<uint8_t>() + info.output_tensors[output_index].arena_offset;
}

TfLiteStatus RemoteInfModelTraits::Invoke()
{
	InfRemoteModelContext& context = *_model_context->As<InfRemoteModelContext>();
	if (AMPI_FAILURE == ampi_send(g_model_forward, &context, sizeof(context), -1)) {
		DEBUG("send request to remote %s FAILED!\n", g_model_forward.description().c_str());
		throw "request model-forward FAILED!!!";
	}

	int32_t response;
	if (AMPI_FAILURE == ampi_receive(g_model_forward, &response, sizeof(response), -1)) {
		DEBUG("remote %s responding failure(%d)\n", g_model_forward.description().c_str(), response);
		throw "model-forward response failure";
	}

	return static_cast<TfLiteStatus>(response);
}

bool RemoteLiteClassifier::CollectModelInfo()
{
	InfRemoteModelInfo& info = *_model_info->As<InfRemoteModelInfo>();
	m_nums_output_tensor = info.nums_output_tensor;

	m_config->quant.zero = info.output_tensors[0].zero_point;
	m_config->quant.scale = info.output_tensors[0].scale;

	m_output_dim[0] = info.output_tensors[0].dims[0];
	m_output_dim[1] = info.output_tensors[0].dims[1];

	m_input_dim[0] = info.input_tensors[0].dims[1];
	m_input_dim[1] = info.input_tensors[0].dims[2];
	m_input_dim[2] = info.input_tensors[0].dims[3];

	m_config->dtype = utils::lite::GetDataType(info.input_tensors[0].type);
	return true;
}

bool RemoteLiteScrfd::CollectModelInfo()
{
	InfRemoteModelInfo& info = *_model_info->As<InfRemoteModelInfo>();
	m_input_dim[0] = info.input_tensors[0].dims[1];
	m_input_dim[1] = info.input_tensors[0].dims[2];
	m_input_dim[2] = info.input_tensors[0].dims[3];
	m_type = utils::lite::GetDataType(info.input_tensors[0].type);

	if (info.nums_output_tensor >= ScrfdMaxOutputSize) {
		inf_log_notice("Cannot handle (%d) of network output, should be <= (%d)!\n",
		           static_cast<int>(info.nums_output_tensor), ScrfdMaxOutputSize);
		return false;
	}

	for (size_t i = 0; i < info.nums_output_tensor; ++i) {
		InfTensorInfo& tensor = info.output_tensors[i];
		m_qinfo[i].zero = tensor.zero_point;
		m_qinfo[i].scale = tensor.scale;

		if (tensor.nums_dims != 3) {
			inf_log_notice("Scrfd Network output#%d dim Size (%d) not supported, should be (3)!\n",
			           static_cast<int>(i), static_cast<int>(tensor.nums_dims));
			return false;
		}
		int *output_dim = m_output_dim[i];
		output_dim[0] = 1;
		output_dim[1] = std::max(tensor.dims[1], tensor.dims[2]);
		output_dim[2] = std::min(tensor.dims[1], tensor.dims[2]);
	}

	/* Sort and assign dimension ind with descending order */
	/* So that feature stride is start from large to small */
	std::vector<int> ind(info.nums_output_tensor,0);

	for (size_t i = 0; i < ind.size(); ++i) {
		ind[i] = i;
	}

	std::sort(ind.begin(), ind.end(),
	          [&](const int a, const int b) { return m_output_dim[a][1] > m_output_dim[b][1]; });

	int ind_divident = (m_use_kps) ? 3 : 2;
	for (size_t i = 0; i < ind.size(); i++) {
		int output_ind = i / ind_divident;
		if (!m_output_prob_idx[output_ind] && m_output_dim[ind[i]][2] == 1) {
			m_output_prob_idx[output_ind] = ind[i];
		} else if (!m_output_reg_idx[output_ind] && m_output_dim[ind[i]][2] == 4) {
			m_output_reg_idx[output_ind] = ind[i];
		} else if (m_use_kps && !m_output_landmark_idx[output_ind] &&
		           m_output_dim[ind[i]][2] == 10) {
			m_output_landmark_idx[output_ind] = ind[i];
		}
	}
	return true;
}

InfImage RemoteInfModelTraits::GetInputImage()
{
	InfRemoteModelInfo& info = *_model_info->As<InfRemoteModelInfo>();
	InfImage img{};
	img.h = info.input_tensors[0].dims[1];
	img.w = info.input_tensors[0].dims[2];
	img.c = info.input_tensors[0].dims[3];

	InfDataType dtype = utils::lite::GetDataType(info.input_tensors[0].type);
	img.buf_owner = 0;
	if (dtype == Inf8U || dtype == Inf8S) {
		img.data = static_cast<uint8_t*>(InputTensorBuffer(0));
	} else {
		img.data = nullptr;
	}
	img.dtype = static_cast<InfDataType>(dtype | ((img.c - 1) << 3));
	return img;
}

bool RemoteLiteFaceEncode::CollectModelInfo()
{
	InfRemoteModelInfo& info = *_model_info->As<InfRemoteModelInfo>();
	if (info.nums_output_tensor != 1) {
		inf_log_err("number of output is not correct!\n");
		return false;
	}

	InfTensorInfo& output_tensor = info.output_tensors[0];
	m_config->quant.zero = output_tensor.zero_point;
	m_config->quant.scale = output_tensor.scale;

	m_encode_dim = output_tensor.dims[1];

	InfTensorInfo& input_tensor = info.input_tensors[0];
	m_input_dim[0] = input_tensor.dims[1];
	m_input_dim[1] = input_tensor.dims[2];
	m_input_dim[2] = input_tensor.dims[3];

	m_config->dtype = utils::lite::GetDataType(input_tensor.type);
	m_type = m_config->dtype;

	return true;
}

#endif
