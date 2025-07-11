#include <cstdio>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <cstring>

#include "utils.h"
#include "ampi.h"
#include "inf_remote.h"

constexpr const char *g_version = "0.1";

class RemoteCore
{
    public:
	RemoteCore()
	{
		_dev = ampi_init(0);
	}

	virtual ~RemoteCore()
	{
		ampi_deinit(_dev);
	}

	operator ampi_dev() const
	{
		return _dev;
	}

	void *RealAddressOf(void *virtualAddr)
	{
		return reinterpret_cast<void *>(ampi_virt_to_phys(_dev, virtualAddr));
	}

    private:
	ampi_dev _dev;
};

class RemoteFunction
{
    public:
	explicit RemoteFunction(ampi_svc token)
	        : _token(token)
	{
	}
	virtual ~RemoteFunction()
	{
		ampi_unlink_service(_token);
	}

	operator ampi_svc() const
	{
		return _token;
	}

//	Maybe it's good to have an Invoke method to hide ampi layer communication

    private:
	ampi_svc _token;
};

class RemoteMemory
{
    public:
	RemoteMemory(ampi_dev dev, size_t memSize)
	        : _dev(dev), _size(memSize)
	{
		_buffer = ampi_malloc(_dev, memSize);
	}
	virtual ~RemoteMemory()
	{
		ampi_free(_dev, _buffer);
	}

	operator void *() const
	{
		return _buffer;
	}

	size_t Size() const
	{
		return _size;
	}

	template<typename T>
	T *As() const
	{
		return reinterpret_cast<T *>(_buffer);
	}

	template<typename T>
	T *RealAddress() const
	{
		return reinterpret_cast<T *>(ampi_virt_to_phys(_dev, _buffer));
	}

    private:
	ampi_dev _dev;
	void *_buffer;
	size_t _size;
};

int main(int argc, const char *argv[])
{
	printf("===== %s v%s =====\n\n", argv[0], g_version);
	int target_output = 0;
	if (argc < 2) {
		printUsage(argv[0]);
		return 1;
	}

//	if (argc > 2) {
//		target_output = atoi(argv[2]);
//	}
	const char *modelFilePath = argv[1];
	RemoteCore remote_service;

	size_t modelSize;
	size_t modelStoreSize = 1024 * 700;
	RemoteMemory modelStore(remote_service, modelStoreSize);
	loadFileContentTo(modelStore, modelFilePath, modelStoreSize, &modelSize);
	printf("[INFO] model file loaded: %d bytes.\n", modelSize);

	size_t arenaSize = 1024 * 1024;
	RemoteMemory arena(remote_service, arenaSize);

	RemoteMemory infoMem(remote_service, sizeof(InfModelInfo));
	RemoteMemory contextMem(remote_service, sizeof(InfHdEngineContext));

	const char *loadModelEP = "load-hd-model";
	InfLoadHdModelParams params = {
		.model_data = modelStore.realAddress<void>(),
		.arena = arena.realAddress<uint8_t>(),
		.arena_size = arenaSize,
		.model_info = infoMem.realAddress<InfModelInfo>(),
		.context = contextMem.realAddress<InfHdEngineContext>(),
		.output_selector = target_output,
	};
	RemoteFunction loadModelToken(ampi_link_service(
	        remote_service, const_cast<char *>(loadModelEP), -1));

	int32_t response;
	printf("Prepare interpreter ...\n");
	ampi_send(loadModelToken, &params, sizeof(params), -1);
	if (ampi_receive(loadModelToken, &response, sizeof(response), -1) < 0) {
		printf("[ERROR] %s NOT responsive!!!\n", loadModelEP);
		return 2;
	}

	if (response != AMPI_SUCCESS) {
		printf("[INFO] %s FAILED!!!\n", loadModelEP);
		return 0;
	}

	auto info = infoMem.as<InfModelInfo>();
	printf("[INFO] Interpreter is ready.\n");
	printf("===============================\n");
	printf("input type: %d\n", info->input_type);
	printf("input dims: ");
	for (int i = 0; i < info->input_n_dim; ++i) {
		printf("%s%d", i == 0 ? "" : " x ", info->input_dims[i]);
	}
	printf("\n");
	printf("input bytes: %d\n", info->input_bytes);
	printf("input quantization:\n");
	printf("  scale: %f\n", info->input_quantization.scale);
	printf("  zero point: %d\n", info->input_quantization.zero_point);
	
	printf("output type: %d\n", info->output_type);
	printf("output dims: ");
	for (int i = 0; i < info->output_n_dim; ++i) {
		printf("%s%d", i == 0 ? "" : " x ", info->output_dims[i]);
	}
	printf("\n");
	printf("output bytes: %d\n", info->output_bytes);
	printf("output quantization:\n");
	printf("  scale: %f\n", info->output_quantization.scale);
	printf("  zero point: %d\n", info->output_quantization.zero_point);

	printf("operators size: %d\n", info->operators_size);
	printf("used arena bytes: %d\n", info->arena_used_bytes);

	if (argc <= 2) return 0;

	std::list<const char*> inputFiles;
	for (int i = 2; i < argc; ++i) {
		inputFiles.emplace_back(argv[i]);
	}

	const char *forwardEP = "hd-forward";
	RemoteFunction forwardToken(ampi_link_service(
	        remote_service, const_cast<char *>(forwardEP), -1));
	RemoteMemory imageMem(remote_service, info->input_bytes);
	RemoteMemory infResultMem(remote_service, info->output_bytes);
	InfHdForwardParams forwardParams = {
		.context = contextMem.realAddress<InfHdEngineContext>(),
		.input_data = imageMem.realAddress<void>(),
		.input_size = imageMem.size(),
		.output_buffer = infResultMem.realAddress<void>(),
		.buffer_size = infResultMem.size(),
	};

	std::for_each(inputFiles.begin(), inputFiles.end(), [&](const char *filename) {
		printf("[INFO] loading %s ...\n", filename);
		size_t contentSize;
		void *inputData = loadFileContent(filename, forwardParams.input_size, &contentSize);
		/*if (!populateInput(info->input_type, imageMem.as<void>(), imageMem.size(),
		                   inputData, contentSize)) {
			printf("[ERROR] CANNOT populate input tensor!!!\n");
			return;
		}*/
//		memcpy(imageMem.as<void>(), inputData, std::min(imageMem.size(), contentSize));
		if (!populateInput(info->input_type, arena.as<uint8_t>() + info->input_tensor_offset, imageMem.size(),
		                   inputData, contentSize)) {
			printf("[ERROR] CANNOT populate input tensor!!!\n");
			return;
		}

		free(inputData);

		if (ampi_send(forwardToken, &forwardParams, sizeof(forwardParams), -1)) {
			printf("[ERROR] invoke %s FAILED!!!\n", forwardEP);
			return;
		}

		if (ampi_receive(forwardToken, &response, sizeof(response), -1) < 0) {
			printf("[ERROR] %s NOT responsive!!!\n", forwardEP);
			return;
		}

		if (response == 0) {
			printf("[ERROR] NO response data, maybe unsupported data type!!!\n");
			return;
		} else if (response < 0) {
			printf("[ERROR] remote endpoint return error code: %d\n", -response);
			return;
		}

		float score;
		if (info->output_type == kTfLiteUInt8) {
			int n = infResultMem.as<uint8_t>()[0];
			printf("raw: %4d   ", n);
			score = info->output_quantization.scale * (n - info->output_quantization.zero_point);
		} else if (info->output_type == kTfLiteInt8) {
			int n = infResultMem.as<int8_t>()[0];
			printf("raw: %4d   ", n);
			score = info->output_quantization.scale * (n - info->output_quantization.zero_point);
		} else {
			printf("[WARNING] MEANINGLESS score!\n");
			score = 0;
		}
		float conf = 1 / (1 + std::exp(-score));
		printf("score: %.6f\tconf: %.6f\n", score, conf);
	});
	return 0;
}
