#include <algorithm>
#include <cmath>
#include <vector>

#include "eaif_common.h"
#include "classify_model.h"

using namespace std;

void classify::Sigmoid(float *output_addr, int num_classes)
{
	int i;
	for (i = 0; i < num_classes; ++i) {
		output_addr[i] = 1 / (1 + exp(-output_addr[i]));
	}
}

void classify::FastSigmoid(float *output_addr, int num_classes)
{
	int i;
	for (i = 0; i < num_classes; ++i) {
		output_addr[i] = 0.5 * ((output_addr[i] / (1 + abs(output_addr[i]))) + 1);
	}
}

void classify::PostProcessConf(const float *output_addr, int num_classes, float conf_thresh, Classification &result)
{
	result.prob.resize(num_classes, 0);
	int i;
	for (i = 0; i < num_classes; ++i) {
		if (output_addr[i] >= conf_thresh) {
			result.cls.push_back(i);
			result.prob[i] = output_addr[i];
		}
	}

	if (result.cls.size() <= 1)
		return;

	sort(result.cls.begin(), result.cls.end(),
	     [&](int &cls1, int &cls2) { return result.prob[cls1] > result.prob[cls2]; });
}

void classify::PostProcessTopK(const float *output_addr, int num_classes, int topk, Classification &result)
{
	if (topk == 0)
		topk = num_classes;

	result.prob.resize(num_classes, 0);
	result.cls.resize(topk, 0);

	vector<int> cls(num_classes, 0);

	for (int i = 0; i < num_classes; ++i)
		cls[i] = i;

	sort(cls.begin(), cls.end(), [&](const int &cls1, const int &cls2) { return output_addr[cls1] > output_addr[cls2]; });

	for (int i = 0; i < topk; ++i) {
		result.cls[i] = cls[i];
		result.prob[cls[i]] = output_addr[cls[i]];
	}
}

void classify::PostProcess(const float *output_addr, int num_classes, float conf_thresh, int topk, Classification &result)
{
	if (topk == 0)
		topk = num_classes;

	result.prob.resize(num_classes, 0);
	vector<int> cls(num_classes, 0);

	for (int i = 0; i < num_classes; ++i)
		cls[i] = i;

	sort(cls.begin(), cls.end(), [&](const int &cls1, const int &cls2) { return output_addr[cls1] > output_addr[cls2]; });

	for (int i = 0; i < topk; ++i) {
		int cls_value = cls[i];
		if (output_addr[cls_value] > conf_thresh) {
			result.cls.push_back(cls_value);
			result.prob[cls_value] = output_addr[cls_value];
		} else {
			break;
		}
	}
}
