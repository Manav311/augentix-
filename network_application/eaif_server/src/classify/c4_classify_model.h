#ifdef USE_C4
#ifndef C4_CLASSIFY_MODEL_H_
#define C4_CLASSIFY_MODEL_H_

#include "eaif_common.h"
#include "eaif_model.h"
#include "eaif_utils.h"

#include "c4_tool.h"

class C4ClassifyModel : public eaif::Model {
    public:
	C4ClassifyModel()
	        : scanner(HUMAN_height, HUMAN_width, HUMAN_xdiv, HUMAN_ydiv, 256, 0.7)
	{
		scanner.hist.Create(1, scanner.baseflength * (scanner.xdiv - C4_EXT) * (scanner.ydiv - C4_EXT));
	}
	virtual ~C4ClassifyModel(){};
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Classify(const void *Wimg, Classification &result, const ModelConfig &conf) override;
	virtual int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                            std::vector<Classification> &results, const ModelConfig &conf) override;
	virtual int Detect(const void *Wimg, std::vector<Detection> &detections, const ModelConfig &conf) override
	{
		return 0;
	}
	virtual void SetDebug(int debug)
	{
	}
	void SetVerbose(int verbose)
	{
		m_verbose = verbose;
	}

	static const int HUMAN_height = 108;
	static const int HUMAN_width = 36;
	static const int HUMAN_xdiv = 9;
	static const int HUMAN_ydiv = 4;
	C4::DetectionScanner scanner;
	C4::IntImage<REAL> original;

    protected:
	int m_verbose;
};

#endif /* !C4_CLASSIFY_MODEL_H_ */
#endif // !USE_C4
