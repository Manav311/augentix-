#ifdef USE_C4

#include <algorithm>
#include <cstring>

#include "eaif_image.h"
#include "eaif_trc.h"

#include "classify_model.h"
#include "c4_tool.h"
#include "c4_classify_model.h"

using namespace std;
using namespace eaif::image;

static struct timespec start;

int C4ClassifyModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose) {
		TIC(start);
	}

	if (model_path.size() != 2)
		return EAIF_FAILURE;

	string model_one = model_dir + "/" + model_path[0];
	string model_two = model_dir + "/" + model_path[1];

	C4::LoadCascade(scanner, model_one, model_two);
	scanner.setExtendSize(config.extend_size.h, config.extend_size.w);

	if (m_verbose) {
		TOC("Load model", start);
	}

	return 0;
}

int C4ClassifyModel::Classify(const void *Wimg, Classification &result, const ModelConfig& conf)
{
	WImage *pimg = (WImage *)Wimg;
	WImage wimg, resize_img;
	eaif_check(pimg->type() == Eaif8UC3 || pimg->type() == Eaif8U);

	if (m_verbose) {
		TIC(start);
	}
	if (m_verbose) {
		TOC("Preprocess WImage", start);
	}
	if (pimg->channels() != 1) {
		ImcvtGray(*pimg, wimg);
		pimg = &wimg;
	}

	Imresize(*pimg, resize_img, HUMAN_width + 5 + C4_EXTEND_SIZE, HUMAN_height + 5 + C4_EXTEND_SIZE);
	original.Load((uint8_t *)resize_img.data, wimg.rows, wimg.cols);

	if (m_verbose) {
		TIC(start);
	}

	float score = scanner.detect(original);

	if (m_verbose) {
		TOC("Inference Call", start);
	}

	if (m_verbose) {
		TIC(start);
	}	
	
	classify::PostProcessConf(&score, conf.num_classes, conf.conf_thresh[0], result);

	if (m_verbose) {
		TOC("PostProcessing", start);
	}

	UpdateState();

	return 0;
}

int C4ClassifyModel::ClassifyObjList(const void *Wimg, const vector<ObjectAttr> &obj_list,
                                           vector<Classification> &results, const ModelConfig& conf)
{

	const WImage *pimg = (const WImage *)Wimg;
	WImage wimg, resized_img;

	if (pimg->channels() != 1) {
		ImcvtGray(*pimg, wimg);
		pimg = &wimg;
	}

	for (size_t i = 0; i < obj_list.size(); ++i) {

		auto &box = obj_list[i].box;

		if (m_verbose) {
			TIC(start);
		}
		ImcropResize(*pimg, box.sx, box.sy, box.ex, box.ey,
			resized_img, HUMAN_width + 5 + C4_EXTEND_SIZE, HUMAN_height + 5 + C4_EXTEND_SIZE);
		if (m_verbose) {
			TOC("Preprocess WImage for obj", start);
		}
		if (m_verbose) {
			TIC(start);
		}
		original.Load((uint8_t *)resized_img.data, resized_img.rows, resized_img.cols);
		float score = min((REAL)100., max((REAL)0., (scanner.detect(original) + 5) * 10)) / 100;
		if (m_verbose) {
			TOC("Inference Call", start);
		}
		Classification single_result;
		classify::PostProcessConf(&score, conf.num_classes, conf.conf_thresh[0], single_result);
		single_result.idx = obj_list[i].idx;
		results.push_back(single_result);
	}

	UpdateState(obj_list.size());
	return 0;
}

#endif // USE_C4
