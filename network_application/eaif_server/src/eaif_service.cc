#include <chrono>
#include <string>
#include <vector>

#include <stdio.h>

#include "eaif_common.h"
#include "eaif_data.h"
#include "eaif_engine.h"
#include "eaif_image.h"
#include "eaif_service.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#ifdef USE_CROW
#include "crow_service.h"
#endif /* !USE_CROW */

using namespace std;
using namespace eaif::image;

static inline void UpdateSize(int& buffer_size, int size)
{
	buffer_size = max(0, SERVICE_RET_SIZE-size);
}

template <> void MsgData<char>::Copy(char *a)
{
	memcpy(a, load, size);
	a[size] = 0;
}

#define EasyCompRetEx(x, fmt, args...) \
	{ \
		char msg[256] = {}; \
		snprintf(msg, 256, fmt, ##args); \
		EasyCompRet(x, string(msg)); \
	}

void eaif::HttpService::InitMpi(void)
{
	mpi_ctx = new mpi::utils::MpiCtx();
}

void eaif::HttpService::ExitMpi(void)
{
	delete mpi_ctx;
	mpi_ctx = nullptr;
}

void eaif::HttpService::EasyCompStrVec(const std::string &title, const std::string &content, const std::vector<std::string> &vec, char *buffer, int size)
{
	int buffer_size = size;
	int ssize = snprintf(buffer, buffer_size, "{\"%s\":1,\"%s\":[", title.c_str(), content.c_str());
	UpdateSize(buffer_size,ssize);
	for (auto &str : vec) {
		ssize += snprintf(&buffer[ssize], buffer_size, "\"%s\",", str.c_str());
		UpdateSize(buffer_size, ssize);
	}
	if (vec.size()) ssize--;
	ssize += snprintf(&buffer[ssize], buffer_size, "]}\n");
	UpdateSize(buffer_size,ssize);
	if (buffer_size < 0) {
		EasyCompRet("fail", "Vector string exceed buffer size", buffer, SERVICE_RET_SIZE);
		return;
	}
	return;
}

void eaif::HttpService::EasyCompRet(const std::string& title, const std::string& reason)
{
	EasyCompRet(title, reason, buf, SERVICE_RET_SIZE);
}

void eaif::HttpService::EasyCompRet(const std::string& title, const std::string& reason, char* buffer, int size)
{
	snprintf(buffer, size, "{\"%s\":1, \"reason\":\"%s\"}\n",
		title.c_str(), reason.c_str());
}

std::string eaif::HttpService::EasyStrCompRet(const std::string& title, const std::string& reason)
{
	return "{\"" + title + "\":1, \"reason\":\"" + reason + "\"}\n";
}

static inline int IsInFilterCls(int cls, const vector<int>* filter_cls)
{
	for (size_t i = 0; i < filter_cls->size(); ++i) {
		if (cls == filter_cls->at(i))
			return 1;
	}
    return 0;
}

static inline int IsInFilterOutCls(int cls, const vector<int>* filter_out_cls)
{
	for (size_t i = 0; i < filter_out_cls->size(); ++i) {
		if (cls == filter_out_cls->at(i))
			return 1;
	}
    return 0;
}

static inline int IsPutIntoVec(int cls, const vector<int>* filter_cls, const vector<int>* filter_out_cls)
{
	int filter_cls_size = filter_cls->size();
	int filter_out_cls_size = filter_out_cls->size();
	if (!filter_cls_size) {
		if (!filter_out_cls_size) {
			return 1;
		} else {
			if (IsInFilterOutCls(cls, filter_out_cls))
				return 0;
			return 1;
		}
	}
	return IsInFilterCls(cls, filter_cls);
}

namespace eaif
{
template<typename Tresult, typename Tmodelinfo>
void HttpService::EasyCompDecode(const Tmodelinfo& info, uint32_t time_val, const Tresult& results) { cerr << "not implemented !\n"; };

template<>
void HttpService::EasyCompDecode(const ModelInfo& info, uint32_t time_val, const vector<Detection>& results)
{
	int i = 0;
	int buffer_size = SERVICE_RET_SIZE;
	vector<int> results_idx;
	for (auto& det : results) {
		if (IsPutIntoVec(det.cls[0], info.filter_cls, info.filter_out_cls))
			results_idx.push_back(i);
		i++;
	}

	//eaif_warn("%d %d\n", results_idx.size(), results_idx[0]);
	int size = snprintf(buf, buffer_size, "{\"success\":1,\"time\":%u,\"pred_num\":%u,\"predictions\":[",
		time_val, static_cast<unsigned int>(results_idx.size()));
	UpdateSize(buffer_size,size);
	i = 0;
	for (auto& idx : results_idx) {
		auto& det = results[idx];
		size += snprintf(&buf[size], buffer_size,
			"{\"idx\":%d,\"rect\":[%.0f,%.0f,%.0f,%.0f],\"prob\":[\"%.4f\"],\"label\":[\"%s\"],\"label_num\":1},",
		++i, det.box.sx, det.box.sy, det.box.ex, det.box.ey,
		det.confidence,	info.labels->at(det.cls[0]).c_str());
		UpdateSize(buffer_size,size);
	}
	if (results_idx.size()) size--;
	size += snprintf(&buf[size], buffer_size, "]}\n");
	UpdateSize(buffer_size,size);
}

template<>
void HttpService::EasyCompDecode(const ModelInfo& info, uint32_t time_val, const vector<Classification>& results)
{
	int buffer_size = SERVICE_RET_SIZE;
	int size = snprintf(buf, buffer_size, "{\"success\":1,\"time\":%u,\"pred_num\":%u,\"predictions\":[",
		time_val, static_cast<unsigned int>(results.size()));
	UpdateSize(buffer_size,size);

	for (auto& res : results) {
		int j = 0;
		vector<int> cls_idx;
		for (auto& cls : res.cls) {
			if (IsPutIntoVec(cls, info.filter_cls, info.filter_out_cls))
				cls_idx.push_back(j);
			j++;
		}
		size += snprintf(&buf[size], buffer_size,
			"{\"idx\":%d,\"label_num\":%u,\"prob\":[", res.idx, static_cast<uint32_t>(cls_idx.size()));
		UpdateSize(buffer_size,size);
		for (auto& idx : cls_idx) {
			auto& cls = res.cls[idx];
			size += snprintf(&buf[size], buffer_size,
			    "\"%.4f\",", res.prob[cls]);
			UpdateSize(buffer_size,size);
		}
		if (cls_idx.size()) size--;
		size += snprintf(&buf[size], buffer_size,
		    "],\"label\":[");
		for (auto& idx : cls_idx) {
		    auto& cls = res.cls[idx];
			size += snprintf(&buf[size], buffer_size,
			      "\"%s\",", info.labels->at(cls).c_str());
			UpdateSize(buffer_size,size);
		}
		if (cls_idx.size()) size--;
		size += snprintf(&buf[size], buffer_size,
		        "]},");
		UpdateSize(buffer_size,size);
	}
	if (results.size()) size--;
	size += snprintf(&buf[size], buffer_size, "]}\n");
}

template<typename Timage>
int HttpService::EasyLoadImage(const RequestMessage& req_msg, Timage& img, int channel) {

	int ret = EAIF_SUCCESS;

	if (req_msg.m_format.size == 3) {
		if (memcmp(req_msg.m_format.load, "jpg", req_msg.m_format.size) == 0) {
			eaif::image::Imdecode(
				const_cast<uint8_t*>(req_msg.m_data.load),
				req_msg.m_data.size,
				static_cast<int>(Eaif8UC3),
				img, channel);
			if (!img.data) {
				EasyCompRet("fail", "Cannot load image jpeg data!");
				ret = EAIF_FAILURE;
			}
		} else if (memcmp(req_msg.m_format.load, "rgb", req_msg.m_format.size) == 0) {
			const int *shape = req_msg.m_shape.load;
			if (shape[2] != 3) {
				EasyCompRet("fail", "Input rgb image channel should be 3!");
				ret = EAIF_FAILURE;
			} else if ((shape[0] * shape[1] * channel) != req_msg.m_data.size) {
				EasyCompRetEx("fail", "Input r image size (%u) does not match to shape (%dx%dx3)",
					req_msg.m_data.size,
					shape[0],
					shape[1]);
				ret = EAIF_FAILURE;
			} else {
				img = Timage(shape[0], shape[1], Eaif8UC3, const_cast<uint8_t*>(req_msg.m_data.load));
				if (!img.data) {
					EasyCompRet("fail", "Cannot load rgb image data!");
					ret = EAIF_FAILURE;
				}
			}
		} else {
			EasyCompRet("fail", "Input format is not valid!");
			ret = EAIF_FAILURE;
		}
	} else if (req_msg.m_format.size == 7) {
		if (memcmp(req_msg.m_format.load, "mpi_jpg", req_msg.m_format.size) == 0) {
			uint8_t *data = nullptr;
			int target_echn = *(int*)req_msg.m_data.load;
			int size = mpi::utils::GetServiceJpeg(&data, target_echn);
			if (!data) {
				EasyCompRet("fail", "Cannot load mpi jpeg data!");
				ret = EAIF_FAILURE;
			} else {
				eaif::image::Imdecode(data, size, static_cast<int>(Eaif8UC3), img, channel);
				if (!img.data) {
					EasyCompRet("fail", "Cannot load image data!");
					ret = EAIF_FAILURE;
				}
				delete data;
				data = nullptr;
			}
		} else if (memcmp(req_msg.m_format.load, "mpi_y__", req_msg.m_format.size) == 0) {
			// ignore input channel here
			uint8_t *data = 0;
			int target_window = *(int*)req_msg.m_data.load;
			int *shape = (int*)req_msg.m_shape.load;
			ret = mpi_ctx->GetServiceY(target_window, shape[0], shape[1], &data);
			if (!data) {
				EasyCompRet("fail", "Cannot load mpi snapshot Y data!");
				ret = EAIF_FAILURE;
			}
			img = Timage(shape[0], shape[1], Eaif8UC1, data);
		} else if (memcmp(req_msg.m_format.load, "mpi_rgb", req_msg.m_format.size) == 0) {
			// ignore input channel here
			uint8_t *data = 0;
			int target_window = *(int*)req_msg.m_data.load;
			int *shape = (int*)req_msg.m_shape.load;
			ret = mpi_ctx->GetServiceRgb(target_window, shape[0], shape[1], &data);
			if (!data) {
				EasyCompRet("fail", "Cannot load mpi snapshot RGB data!");
				ret = EAIF_FAILURE;
			}
			img = Timage(shape[0], shape[1], Eaif8UC3, data);
		} else {
			EasyCompRet("fail", "Input format is not valid!");
			ret = EAIF_FAILURE;
		}
	} else {
		EasyCompRet("fail", "Input format is not valid!");
		ret = EAIF_FAILURE;
	}

	return ret;
}

int HttpService::ReleaseMpiFrameIfAny(void)
{
	return mpi_ctx->ReleaseFrameIfAny();
}

template int HttpService::EasyLoadImage(const RequestMessage&, WImage&, int);

} // namespace eaif

std::shared_ptr<eaif::HttpService> eaif::HttpServiceFactory::GetServiceInstance(void)
{
#ifdef USE_CROW
	CrowService *crow_app = new CrowService();
	HttpService *service = dynamic_cast<HttpService*>(crow_app);
	return std::shared_ptr<HttpService>(service);

#else // !USE_CROW
	return nullptr;
#endif
}
