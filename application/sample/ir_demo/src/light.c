#include "light.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>

#include "mpi_dip_alg.h"

#define LIGHT_SENSOR_UNIT (1024)

int calcMpiSceneLuma(const int dev, const int path)
{
	MPI_EXPOSURE_INFO_S exp_info;
	UINT64 ev;
	UINT32 scene_luma = 0;
	int ret = 0;
	MPI_PATH idx;
	idx.dev = dev;
	idx.path = path;

	ret = MPI_queryExposureInfo(idx, &exp_info);

	if (ret) {
		return ret;
	}

	if (exp_info.luma_avg > 19275) {
		//Overexposure can't trust.
		return -EINVAL;
	}
	ev = exp_info.inttime * exp_info.sensor_gain;
	ev = (ev + 16) >> 5;
	scene_luma = (exp_info.luma_avg << 16) / ev;

	scene_luma = MAX(scene_luma, 0);

	return scene_luma;
}

int calcMpiLumaIir(const int prev_luma, const int curr_luma, const int current_weight)
{
	int delta_luma = 0;
	int ratio_step = 8;
	int weight_ratio = 256; // 256 = 1x
	int new_weight = current_weight;
	int iir_luma = curr_luma;

	//            /|
	//           / |
	//          /  |weight_ratio
	//         /   |(curr_luma > prev_luma： 1 ~ 1/9x)
	//        /    |(curr_luma < prev_luma： 1 ~ 7x)
	//       /_____|
	//         delat_luma(0~8192)

	if (curr_luma > prev_luma) {
		delta_luma = CLAMP(curr_luma - prev_luma, 0, 8192);
		//4096 = 7 * 256
		weight_ratio = (((delta_luma * 1792) + (1 << 14)) >> 15) + 256;
		new_weight = ((current_weight << ratio_step) + (weight_ratio >> 1)) / weight_ratio;
		new_weight = CLAMP(new_weight, 1, 255);
	} else if (curr_luma < prev_luma) {
		delta_luma = CLAMP(prev_luma - curr_luma, 0, 8192);
		//1536 = 6 * 256
		weight_ratio = (((delta_luma * 1536) + (1 << 14)) >> 15) + 256;
		new_weight = ((current_weight * weight_ratio) + (1 << (ratio_step - 1))) >> ratio_step;
		new_weight = CLAMP(new_weight, 1, 255);
	} else {
		//curr_luma = prev_luma
		new_weight = current_weight;
	}

	//(new_weight * curr_luma + (256 - new_weight) * prev_luma) / 256
	iir_luma = ((curr_luma * new_weight) + ((MPI_IIR_UNIT - new_weight) * prev_luma) + (1 << (MPI_IIR_PRC - 1))) >>
	           MPI_IIR_PRC;

	return iir_luma;
}

int binarySearch(const int value, const int *arry, const int i0, const int i1)
{
	if ((i1 - i0) <= 1) {
		return i0;
	}

	int i = (i0 + i1) >> 1;

	if (value > arry[i]) {
		return binarySearch(value, arry, i0, i);
	}

	return binarySearch(value, arry, i, i1);
}

int arrPixIntpl(const int val, const int curve_num, const int *curve_bin, const int *curve_val)
{
	if ((val > curve_bin[0]) || (val < curve_bin[curve_num - 1])) {
		syslog(LOG_ERR, "index out of range !\n");
		return -1;
	}

	int j = binarySearch(val, curve_bin, 0, curve_num - 1);

	int norm = curve_bin[j] - curve_bin[j + 1];
	int alpha = val - curve_bin[j + 1];

	if (norm <= 0) {
		syslog(LOG_ERR, "invalid numerator !\n");
		return norm;
	}

	return alphaPixBlend(curve_val[j], curve_val[j + 1], alpha, norm);
}

int alphaPixBlend(const int pix0, const int pix1, const int alpha, const int norm)
{
	int64_t tmp = (int64_t)alpha * (int64_t)(pix0 - pix1);
	return (tmp > 0) ? pix1 + (tmp + (norm >> 1)) / norm : pix1 + (tmp - (norm >> 1)) / norm;
}

int checkRange(char *str, int value, int low_val, int high_val)
{
	if ((value < low_val) || (value > high_val)) {
		syslog(LOG_INFO, "[EVENT][WARNING] %s = %u, It's out off range [%u, %u]\n", str, value, low_val,
		       high_val);
		return MPI_FAILURE;
	}
	return MPI_SUCCESS;
}
