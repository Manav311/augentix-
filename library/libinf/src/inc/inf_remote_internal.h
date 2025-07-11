#ifndef INF_REMOTE_INTERNAL_H_
#define INF_REMOTE_INTERNAL_H_

#include <cstdio>
#include <string>
#include <memory>
#include <sstream>

#include "inf_remote.h"
#include "ampi.h"
#include "lite_classifier.h"
#include "lite_scrfd.h"
#include "lite_faceencode.h"

#define DEBUG(format, ...) printf("[INF DEBUG]: " format, ##__VA_ARGS__)

class RemoteCore {
    public:
	RemoteCore()
	{
		_dev = ampi_init(0);
		DEBUG("ampi_init(0) => 0x%X\n", _dev);
	}

	virtual ~RemoteCore()
	{
		if (Ready()) {
			ampi_deinit(_dev);
			DEBUG("ampi_deinit(0x%X)\n", _dev);
		}
	}

	bool Ready() const
	{
		return _dev != AMPI_FAILURE;
	}

	operator ampi_dev() const
	{
		EnsureReadyState();
		return _dev;
	}

	void* RealAddressOf(void* virtualAddr) const
	{
		EnsureReadyState();
		return reinterpret_cast<void*>(ampi_virt_to_phys(_dev, virtualAddr));
	}

    private:
	ampi_dev _dev;

	void EnsureReadyState() const
	{
		if (!Ready()) {
			throw "remote core service is NOT ready!";
		}
	}
};

class RemoteFunction {
    public:
	explicit RemoteFunction(ampi_svc token)
	        : _token(token)
	{
	}

	RemoteFunction(ampi_dev device, const std::string& function_name)
	        : _name(function_name)
	{
		_token = ampi_link_service(device, const_cast<char*>(function_name.c_str()), -1);
	}

	virtual ~RemoteFunction()
	{
		if (Valid()) {
			//DEBUG("unlink %s[0x%X]\n", _name.empty()? "function" : _name.c_str(), _token);
			DEBUG("unlink %s\n", description().c_str());
			ampi_unlink_service(_token);
		}
	}

	bool Valid() const
	{
		return _token != AMPI_FAILURE;
	}

	operator ampi_svc() const
	{
		EnsureServiceIsValid();
		return _token;
	}

	std::string description() const
	{
		std::ostringstream os;
		if (_name.empty()) {
			os << "function[" << std::hex << _token << "]";
		} else {
			os << "function[" << _name << "]";
		}

		return os.str();
	}

	//Maybe it's good to have an Invoke method to hide ampi layer communication

    private:
	void EnsureServiceIsValid() const
	{
		if (!Valid()) {
			throw "remote function is NOT valid!";
		}
	}

	ampi_svc _token;
	std::string _name;
};

class RemoteMemory {
    public:
	RemoteMemory(ampi_dev dev, size_t mem_size)
	        : _dev(dev)
	        , _size(mem_size)
	{
		_buffer = ampi_malloc(_dev, mem_size);
	}

	virtual ~RemoteMemory()
	{
		DEBUG("release %d bytes memory @%p (0x%X)\n", static_cast<uint32_t>(_size), _buffer,
		      ampi_virt_to_phys(_dev, _buffer));
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

	template <typename T> T* As() const
	{
		return reinterpret_cast<T*>(_buffer);
	}

	template <typename T> T* RealAddress() const
	{
		return reinterpret_cast<T*>(ampi_virt_to_phys(_dev, _buffer));
	}

    private:
	ampi_dev _dev;
	void* _buffer;
	size_t _size;
};

class RemoteInfModelTraits {
    public:
	virtual bool InVerboseMode() = 0;
	bool PrepareInterpreter(const std::string& model_path);
	void* InputTensorBuffer(size_t input_index);
	void* OutputTensorBuffer(size_t output_index);
	TfLiteStatus Invoke();
	size_t GetArenaUsedBytes()
	{
		return (*_model_info->As<InfRemoteModelInfo>()).arena_used_bytes;
	}
	InfImage GetInputImage();

    protected:
	std::unique_ptr<RemoteMemory> _model_data;
	std::unique_ptr<RemoteMemory> _arena;
	std::unique_ptr<RemoteMemory> _model_info;
	std::unique_ptr<RemoteMemory> _model_context;
};

class RemoteLiteClassifier : public LiteClassifierBase, protected RemoteInfModelTraits {
    public:
	explicit RemoteLiteClassifier(InfModelInfo* info)
	        : LiteClassifierBase(info)
	{
	}

    protected:
	bool CollectModelInfo() override;
	bool PrepareInterpreter(const std::string& model_path) override
	{
		return RemoteInfModelTraits::PrepareInterpreter(model_path);
	}
	void* InputTensorBuffer(size_t input_index) override
	{
		return RemoteInfModelTraits::InputTensorBuffer(input_index);
	}
	void* OutputTensorBuffer(size_t output_index) override
	{
		return RemoteInfModelTraits::OutputTensorBuffer(output_index);
	}
	TfLiteStatus Invoke() override
	{
		return RemoteInfModelTraits::Invoke();
	}
	bool InVerboseMode() override
	{
		return InfModel::InVerboseMode();
	}
	size_t GetArenaUsedBytes() override
	{
		return RemoteInfModelTraits::GetArenaUsedBytes();
	}
};

class RemoteLiteScrfd : public LiteScrfdBase, protected RemoteInfModelTraits {
    public:
	explicit RemoteLiteScrfd(InfModelInfo* info)
	        : LiteScrfdBase(info)
	{
	}

	InfImage GetInputImage() override
	{
		return RemoteInfModelTraits::GetInputImage();
	}

    protected:
	bool CollectModelInfo() override;
	bool PrepareInterpreter(const std::string& model_path) override
	{
		return RemoteInfModelTraits::PrepareInterpreter(model_path);
	}
	void* InputTensorBuffer(size_t input_index) override
	{
		return RemoteInfModelTraits::InputTensorBuffer(input_index);
	}
	void* OutputTensorBuffer(size_t output_index) override
	{
		return RemoteInfModelTraits::OutputTensorBuffer(output_index);
	}
	TfLiteStatus Invoke() override
	{
		return RemoteInfModelTraits::Invoke();
	}
	bool InVerboseMode() override
	{
		return InfModel::InVerboseMode();
	}
	size_t GetArenaUsedBytes() override
	{
		return RemoteInfModelTraits::GetArenaUsedBytes();
	}
};

class RemoteLiteFaceEncode : public LiteFaceEncodeBase, protected RemoteInfModelTraits {
    public:
	explicit RemoteLiteFaceEncode(InfModelInfo* info)
	        : LiteFaceEncodeBase(info)
	{
	}

	InfImage GetInputImage() override
	{
		return RemoteInfModelTraits::GetInputImage();
	}

    protected:
	bool CollectModelInfo() override;
	bool PrepareInterpreter(const std::string& model_path) override
	{
		return RemoteInfModelTraits::PrepareInterpreter(model_path);
	}
	void* InputTensorBuffer(size_t input_index) override
	{
		return RemoteInfModelTraits::InputTensorBuffer(input_index);
	}
	void* OutputTensorBuffer(size_t output_index) override
	{
		return RemoteInfModelTraits::OutputTensorBuffer(output_index);
	}
	TfLiteStatus Invoke() override
	{
		return RemoteInfModelTraits::Invoke();
	}
	bool InVerboseMode() override
	{
		return InfModel::InVerboseMode();
	}
	size_t GetArenaUsedBytes() override
	{
		return RemoteInfModelTraits::GetArenaUsedBytes();
	}
};

#endif //INF_REMOTE_INTERNAL_H_
