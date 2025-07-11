#ifndef EAIF_SERVICE_
#define EAIF_SERVICE_

#include <cstring>
#include <string>
#include <memory>

#include "eaif_common.h"

template <typename T> struct MsgData {
	MsgData()
	        : load(nullptr)
	        , size(0){};
	MsgData(MsgData<T> &imsg)
	        : load(imsg.load)
	        , size(imsg.size){};
	const T *load;
	int size;
	void Copy(T *a)
	{
		memcpy(a, load, size);
	}
};

template <> void MsgData<char>::Copy(char *a);

class RequestMessage {
    public:
	RequestMessage(MsgData<char> &iformat, MsgData<uint32_t> &itime, MsgData<char> &imeta, MsgData<int> &ishape,
	               MsgData<uint8_t> &idata)
	        : m_format(iformat)
	        , m_time(itime)
	        , m_meta(imeta)
	        , m_shape(ishape)
	        , m_data(idata){};

	template <typename Treq> RequestMessage(const Treq &req);

	MsgData<char> m_format;
	MsgData<uint32_t> m_time;
	MsgData<char> m_meta;
	MsgData<int> m_shape; // h, w, c
	MsgData<uint8_t> m_data;
};

using ReqMsgPtr = std::shared_ptr<RequestMessage>;

inline int SprintfView(char *msg, MsgData<char> &str)
{
	memcpy(msg, str.load, str.size);
	return str.size;
}

#define SERVICE_RET_SIZE (4096)

namespace mpi
{
namespace utils
{
class MpiCtx;
}
}

namespace eaif
{
class HttpService {
    public:
	HttpService()
	        : port(40080)
	        , bindaddr("127.0.0.1")
	{
		InitMpi();
	};
	virtual ~HttpService()
	{
		ExitMpi();
	};
	virtual int RegisterApp(std::string &config) = 0;
	virtual void Run(std::string &bindaddr, int port) = 0;
	virtual void Clear() = 0;

    protected:
	template <typename Tresult, typename Tmodelinfo>
	void EasyCompDecode(const Tmodelinfo &info, uint32_t time_val, const Tresult &result);
	void EasyCompRet(const std::string &title, const std::string &reason);
	void EasyCompRet(const std::string &title, const std::string &reason, char *buffer,
	                 int size = SERVICE_RET_SIZE);
	void EasyCompStrVec(const std::string &title, const std::string &content, const std::vector<std::string> &vec,
	                    char *buffer, int size = SERVICE_RET_SIZE);
	std::string EasyStrCompRet(const std::string &title, const std::string &reason);
	template <typename Timage>
	int EasyLoadImage(const RequestMessage &req, Timage &img, int channel = IMG_DESIRED_CHANNEL);

	void InitMpi(void);
	void ExitMpi(void); // user must exit mpi in destructor
	int ReleaseMpiFrameIfAny(void);

	int port;
	std::string bindaddr;
	char buf[SERVICE_RET_SIZE];
	mpi::utils::MpiCtx *mpi_ctx;
};

class HttpServiceFactory {
    public:
	static std::shared_ptr<HttpService> GetServiceInstance(void);
};

} // namespace eaif

template <typename Treq> RequestMessage::RequestMessage(const Treq &msg)
{
	for (uint32_t i = 0; i < msg.parts.size(); ++i) {
		for (auto header : msg.parts[i].headers) {
			for (auto ele : header.params) {
				auto &body = msg.parts[i].body;
				if (ele.first.compare("name") == 0) {
					if (ele.second.compare("format") == 0) {
						m_format.load = body.c_str();
						m_format.size = body.size();
					} else if (ele.second.compare("shape") == 0) {
						m_shape.load = reinterpret_cast<const int *>(body.c_str());
						m_shape.size = body.size();
					} else if (ele.second.compare("time") == 0) {
						m_time.load = reinterpret_cast<const uint32_t *>(body.c_str());
						m_time.size = body.size();
					} else if (ele.second.compare("meta") == 0) {
						m_meta.load = body.c_str();
						m_meta.size = body.size();
					} else if ((ele.second.compare("data") == 0) || (ele.second.compare("image") == 0)){
						m_data.load = reinterpret_cast<const uint8_t *>(body.c_str());
						m_data.size = body.size();
					} else {
					};
				}
			}
		}
	}
}

#endif /* !EAIF_SERVICE_ */
