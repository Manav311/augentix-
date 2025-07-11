#ifdef USE_CROW

#include <algorithm>
#include <memory>
#include <mutex>

#include "eaif_engine.h"
#include "eaif_service.h"

class CrowApp;

class CrowService : public eaif::HttpService {
    public:
	CrowService();
	virtual ~CrowService() override;
	virtual int RegisterApp(std::string &config) override;
	virtual void Run(std::string &bindaddr, int port) override;
	virtual void Clear() override
	{
		m_engine.Clear();
	};

    private:
	std::unique_ptr<CrowApp> m_service;
	eaif::Engine m_engine;
	std::mutex lock; // inference locking
	const int concurrent_ = 1; // inference and config reading
};

#endif /* !USE_CROW */
