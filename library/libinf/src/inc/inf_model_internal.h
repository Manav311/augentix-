#ifndef INF_MODEL_INTERNAL_H_
#define INF_MODEL_INTERNAL_H_

// #include <ctime>
#include <cstdlib>

#include "inf_types.h"
#include "inf_log.h"

class InfModel
{
public:
	InfModel() = default;
	virtual ~InfModel() = default;
	virtual void SetModelThreads(int nthread)
	{
		m_num_thread = nthread;
	}

	virtual void SetDebug(int debug)
	{
		m_debug = debug;
	}
	virtual int Classify(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfResultList *result)
	{
		return 0;
	}
	virtual int Detect(const InfImage *img, InfDetList *result)
	{
		return 0;
	}
	virtual int Detect(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfDetList *result)
	{
		return 0;
	}
	virtual InfImage GetInputImage()
	{
		InfImage img{};
		return img;
	}

	void SetNumThreads(int nthread)
	{
		m_num_thread = nthread;
	}
	void SetVerbose(int verbose)
	{
		m_verbose = verbose;
	}

	virtual bool InVerboseMode()
	{
		return m_verbose;
	}

	InfModelInfo *m_config = nullptr;
	InfDataType m_type;
	int m_verbose = 0;
	int m_debug = 0;
	int m_num_thread = 1;
	struct timespec start;
	char m_snapshot_prefix[256] = {};
	int m_snapshot_cnt = 0;

protected:
	void SetupVerboseModeFromEnvironment()
	{
		char* verbose = getenv("INF_VERBOSE");
		if (verbose) {
			m_verbose = atoi(verbose);
			inf_log_notice("INF_VERBOSE is detected: %d.", m_verbose);
		}
	}
};

class InfModelFactory
{
public:
	static InfModel *Create(const char *config);

private:
	static InfModel *CreateClassify(InfModelInfo *info);
	static InfModel *CreateFaceDet(InfModelInfo *info);
	static InfModel *CreateFaceEncode(InfModelInfo *info);
	static InfModel *CreateFaceReco(InfModelInfo *info);
};

#endif /* INF_MODEL_INTERNAL_H_ */
