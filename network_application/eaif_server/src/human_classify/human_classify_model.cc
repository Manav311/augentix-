#include "eaif_image.h"
#include "eaif_trc.h"
#include "human_classify_model.h"

using namespace std;
using namespace eaif::image;

int HumanClassifyModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	m_conf_dl.Parse(model_dir.c_str(), model_path[DLConfIndex].c_str());
	m_conf_cv.Parse(model_dir.c_str(), model_path[CVConfIndex].c_str());
	return 0;
}

int HumanClassifyModel::Classify(const void *Wimg, Classification &result, const ModelConfig& conf)
{
	const WImage *pimg = (const WImage*) Wimg;
	WImage gray_image;
	if (pimg->channels() != 1) {
		ImcvtGray(*pimg, gray_image);
		pimg = &gray_image;
	}
	bool goto_dl = false;

	m_classifier_cv->Classify(pimg, result, m_conf_cv);
	if (result.cls.size()) {
		auto& cls = result.cls[0];
		if (result.prob[cls] <= conf.conf_thresh[0] && result.prob[cls] >= conf.conf_thresh_low) {
			goto_dl = true;
		}
	}

	if (goto_dl) {
		// classify and merge
		result.cls.clear();
		result.prob.clear();
		m_classifier_dl->Classify(Wimg, result, m_conf_dl);
		if (result.cls.size()) {
			float tmp_prob = result.prob[result.cls[0]];
			result.prob.resize(1);
			result.prob[0] = tmp_prob;
		}
	}
	UpdateState();
	return 0;
}

int HumanClassifyModel::ClassifyObjList(const void *Wimg,
		const vector<ObjectAttr> &obj_list,
	    vector<Classification> &results, const ModelConfig& conf)
{
	size_t i, j, ol_len = obj_list.size();
	vector<int> result_index;
	vector<ObjectAttr> local_obj_list;
	vector<Classification> local_results;

	// inference by cv
	const WImage *pimg = (WImage*) Wimg;
	WImage gray_image;
	if (pimg->channels() != 1) {
		ImcvtGray(*pimg, gray_image);
		pimg = &gray_image;
	}

	m_classifier_cv->ClassifyObjList(pimg, obj_list, results, m_conf_cv);

	// pass the less conf object to dl
	j = 0;
	for (auto& result : results) {
		if (result.cls.size()) {
			auto& cls = result.cls[0];
			if (result.prob[cls] <= conf.conf_thresh[0] && result.prob[cls] >= conf.conf_thresh_low) {
				for (i = 0; i < ol_len; ++i) {
					if (obj_list[i].idx == result.idx) {
						local_obj_list.push_back(obj_list[i]);
						result_index.push_back(j);
						break;
					}
				}
			}
		}
		j++;
	}

	// inference by dl
	if (local_obj_list.size()) {
		//ImsaveBmp("dl.bmp", (*(const WImage*) Wimg));
		m_classifier_dl->ClassifyObjList(Wimg, local_obj_list, local_results, m_conf_dl);

		// merge result
		j = 0;
		for (auto& index : result_index) {
			auto& result = results[index];
			auto& local_result = local_results[j];
			result.cls.clear();
			result.prob.clear();
			if (local_result.cls.size()) {
				result.cls.push_back(0);
				result.prob.push_back(local_result.prob[local_result.cls[0]]);
			}
		}
	}
	UpdateState();
	return 0;
}
