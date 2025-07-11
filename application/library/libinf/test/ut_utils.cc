#include <vector>

#include "inf_ut.h"
#include "inf_utils.h"
#include "inf_face_internal.h"

#define MAX(a,b) ((a) > (b)) ? (a) : (b)


/*
void VecL2Norm(const std::vector<float>& src, std::vector<float>& dst);
*/
int ut_VecL2Norm(InfTest *test)
{
	std::vector<float> src_addr{3,5,-2,-4,0,25};
	std::vector<float> output_addr(6, 0);
	float gt[] = {0.11512943,  0.19188239, -0.07675296, -0.15350591,  0., 0.95941195};

	VecL2Norm(src_addr, output_addr);
	for (size_t i = 0; i < src_addr.size(); i++) {
		testAssertFloatEqMsg(output_addr[i], gt[i], "Wrong result for VecL2Norm-1");
	}

	std::vector<float> src_addr1{7,7.2,7.21,8,6,-2,-25};
	std::vector<float> output_addr1(7, 0);
	float gt1[] = {0.23572577,  0.24246079,  0.24279754,  0.26940088,  0.20205066,
       -0.06735022, -0.84187774};

	VecL2Norm(src_addr1, output_addr1);
	for (size_t i = 0; i < output_addr1.size(); i++) {
		testAssertFloatEqMsg(output_addr1[i], gt1[i], "Wrong result for VecL2Norm-2");
	}

	std::vector<float> src_addr2{0,0,0,0,0,0,0};
	std::vector<float> output_addr2(7, 0);
	float gt2[] = {0,0,0,0,0,0,0};

	VecL2Norm(src_addr2, output_addr2);
	for (size_t i = 0; i < output_addr1.size(); i++) {
		testAssertFloatEqMsg(output_addr2[i], gt2[i], "Wrong result for VecL2Norm-3");
	}

	return 0;
}

/*
float NormVecDistL2Norm(const std::vector<float>& v0, const std::vector<float>& v1);
*/
int ut_NormVecDistL2Norm(InfTest *test)
{
	std::vector<float> v0{0.1,0.2,0.3,0.4,0.5};
	std::vector<float> v1{-0.2,0.1,0.4,0.7,0.9};
	float gt = 0.36683;
	float res = NormVecDistL2Norm(v0, v1);
	testAssertFloatEqMsg(res, gt, "Wrong result for ut_NormVecDistL2Norm-1");

	std::vector<float> v3{0.3,0.1,-0.3,-0.2,0.5};
	std::vector<float> v4{-0.3,-0.2,0.4,0.7,-0.9};
	float gt2 = 1.969076189;
	res = NormVecDistL2Norm(v3, v4);
	testAssertFloatEqMsg(res, gt2, "Wrong result for ut_NormVecDistL2Norm-2");
	return 0;
}

/*
float VecCosineSimilarity(const std::vector<float>& v0, const std::vector<float>& v1);
*/
int ut_VecCosineSimilarity(InfTest *test)
{
	std::vector<float> v0{0.1,0.2,0.3,0.4,0.5};
	std::vector<float> v1{-0.2,0.1,0.4,0.7,0.9};
	float gt = 0.9327153;
	float res = VecCosineSimilarity(v0, v1);
	testAssertFloatEqMsg(res, gt, "Wrong result for cosine similarity");

	std::vector<float> v3{0.3,0.1,-0.3,-0.2,0.5};
	std::vector<float> v4{-0.3,-0.2,0.4,0.7,-0.9};
	float gt2 = MAX(-0.9386305203160419,0);
	res = VecCosineSimilarity(v3, v4);
	testAssertFloatEqMsg(res, gt2, "Wrong result for cosine similarity-2");
	return 0;
}

/*
void Sigmoid(float *output_addr, int num_classes)
*/
int ut_Sigmoid(InfTest *test)
{
	float output_addr[] = {3,5,-2,-4,0,25};
	float gt[] = {0.95257413, 0.99330715, 0.11920292, 0.01798621, 0.5, 1};

	int classes = 6;

	Sigmoid(output_addr, classes);

	for (int i = 0; i < classes; i++) {
		testAssertFloatEqMsg(output_addr[i], gt[i], "Wrong result for sigmoid-1");
	}

	float output_addr1[] = {7,7.2,7.21,8,6,-2,-25};
	float gt1[] = {9.99088949e-01, 9.99253971e-01, 9.99261389e-01, 9.99664650e-01, 9.97527377e-01, 1.19202922e-01, 1.38879439e-11};
	classes = 7;

	Sigmoid(output_addr1, classes);
	for (int i = 0; i < classes; i++) {
		testAssertFloatEqMsg(output_addr1[i], gt1[i], "Wrong result for sigmoid-2");
	}

	return 0;
}

/*
void Softmax(float *output_addr, int num_classes);
*/
int ut_Softmax(InfTest *test)
{
	float output_addr[] = {3,5,-2,-4,0,25};
	float gt[] = {2.789468e-10, 2.061153e-9, 1.87952881e-12, 2.5436656e-13, 1.388794e-11, 9.9999998e-01};
	int classes = 6;

	Softmax(output_addr, classes);

	for (int i = 0; i < classes; i++) {
		testAssertFloatEqMsg(output_addr[i], gt[i], "Wrong result for softmax-1");
	}

	float output_addr1[] = {7,7.2,7.21,8,6,-2,-25};
	float gt1[] = {1.52873280e-01, 1.86719846e-01, 1.88596412e-01, 4.15552659e-01,
       5.62389369e-02, 1.88660615e-05, 1.93601253e-15};
	classes = 7;

	Softmax(output_addr1, classes);
	for (int i = 0; i < classes; i++) {
		testAssertFloatEqMsg(output_addr1[i], gt1[i], "Wrong result for softmax-2");
	}

	return 0;
}

/*
void PadAndRescale(int pad, const Shape& img, const Shape& dst, const MPI_RECT_POINT_S& roi,
							MPI_RECT_POINT_S& dst_roi);
*/
int ut_PadAndRescale(InfTest *test)
{
	int pad = 32;
	Shape img_shape{1280, 720};
	Shape network_input{320, 192};
	MPI_RECT_POINT_S grouped{320,320,320+320,320+320}; // square
	MPI_RECT_POINT_S dst_roi{};
	MPI_RECT_POINT_S& g = grouped;
	MPI_RECT_POINT_S& d = dst_roi;

	PadAndRescale(pad, img_shape, network_input, grouped, dst_roi);

	printf("ut_PadAndRescale()-case1 [%d %d %d %d] net ratio: %.3f => [%d %d %d %d] dst ratio:%.3f\n",
		g.sx, g.sy, g.ex, g.ey, (float)320/192,d.sx, d.sy, d.ex, d.ey,
		(float)(d.ex-d.sx+1)/(d.ey-d.sy+1));


	pad = 32;
	img_shape.w = 540;
	img_shape.h = 360;
	// Shape network_input{320, 192};
	// MPI_RECT_POINT_S grouped{320,320,320+320,320+320}; // square
	// MPI_RECT_POINT_S dst_roi{};
	// MPI_RECT_POINT_S& g = grouped;
	// MPI_RECT_POINT_S& d = dst_roi;

	PadAndRescale(pad, img_shape, network_input, grouped, dst_roi);

	printf("ut_PadAndRescale()-case2 [%d %d %d %d] net ratio: %.3f => [%d %d %d %d] dst ratio:%.3f\n",
		g.sx, g.sy, g.ex, g.ey, (float)320/192,d.sx, d.sy, d.ex, d.ey,
		(float)(d.ex-d.sx+1)/(d.ey-d.sy+1));

	pad = 32;
	img_shape.w = 1920;
	img_shape.h = 1080;
	// network_input.w = {320, 192};
	grouped=(MPI_RECT_POINT_S){480,0,1440,1080};
	// MPI_RECT_POINT_S& g = grouped;
	// MPI_RECT_POINT_S& d = dst_roi;

	PadAndRescale(pad, img_shape, network_input, grouped, dst_roi);

	printf("ut_PadAndRescale()-case3 [%d %d %d %d] net ratio: %.3f => [%d %d %d %d] dst ratio:%.3f\n",
		g.sx, g.sy, g.ex, g.ey, (float)320/192,d.sx, d.sy, d.ex, d.ey,
		(float)(d.ex-d.sx+1)/(d.ey-d.sy+1));


	return 0;
}

/*
void NmsBoxes(std::vector<FaceBox> &input, float threshold, int type, std::vector<FaceBox> &output);
*/
int ut_NmsBoxes(InfTest *test)
{
	// a - sx = 280, sy = 206, ex = 411, ey = 378
	// b - sx = 287, sy = 191, ex = 428, ey = 383
	std::vector<FaceBox> input, output;
	FaceBox a{280, 206, 411, 378}, b{287, 191, 428, 383};
	input.push_back(a);
	input.push_back(b);
	float threshold = 0.2;
	NmsBoxes(input, threshold, NMS_UNION, output);

	printf("ut_NmsBoxes()-a [%.2f %.2f %.2f %.2f] b: [%.2f %.2f %.2f %.2f] -> output.Size(%d)\n",
		a.x0, a.y0, a.x1, a.y1, b.x0, b.y0, b.x1, b.y1, (int)output.size());
	return 0;
}

/*
void RescaleFaceBox(int net_h, int net_w, MPI_RECT_POINT_S& roi);
*/
int ut_RescaleFaceBox(InfTest *test)
{
	// int net_h, int net_w, MPI_RECT_POINT_S& roi
	// w:h = 96/96 > 60/100
	int net_h = 96, net_w = 96;
	MPI_RECT_POINT_S roi = {280-30, 280-50, 280+29, 280+49};
	MPI_RECT_POINT_S gt = {280-50, 280-50, 280+49, 280+49};
	RescaleFaceBox(net_h, net_w, roi);
	testAssertMsg(memcmp(&roi, &gt, sizeof(MPI_RECT_POINT_S))==0,
		"0-input[%d %d %d %d] is not eq to gt[%d %d %d %d]",
		roi.sx, roi.sy, roi.ex, roi.ey,
		gt.sx, gt.sy, gt.ex, gt.ey
		);

	// w:h = 56/112 < 60/100
	net_h = 112, net_w = 56;
	roi = (MPI_RECT_POINT_S){280-30, 280-50, 280+29, 280+49};
	gt = (MPI_RECT_POINT_S){280-30, 280-60, 280+29, 280+59};
	RescaleFaceBox(net_h, net_w, roi);
	testAssertMsg(memcmp(&roi, &gt, sizeof(MPI_RECT_POINT_S))==0,
		"1-input[%d %d %d %d] is not eq to gt[%d %d %d %d]",
		roi.sx, roi.sy, roi.ex, roi.ey,
		gt.sx, gt.sy, gt.ex, gt.ey
		);

	// w:h = 56/112 == 60/100
	net_h = 112, net_w = 56;
	roi = (MPI_RECT_POINT_S){280-30, 280-60, 280+29, 280+59};
	gt = (MPI_RECT_POINT_S){280-30, 280-60, 280+29, 280+59};
	RescaleFaceBox(net_h, net_w, roi);
	testAssertMsg(memcmp(&roi, &gt, sizeof(MPI_RECT_POINT_S))==0,
		"2-input[%d %d %d %d] is not eq to gt[%d %d %d %d]",
		roi.sx, roi.sy, roi.ex, roi.ey,
		gt.sx, gt.sy, gt.ex, gt.ey
		);


	return 0;
}

int main()
{
	REGISTER_TEST(ut_VecL2Norm);
	REGISTER_TEST(ut_NormVecDistL2Norm);
	REGISTER_TEST(ut_VecCosineSimilarity);
	REGISTER_TEST(ut_Sigmoid);
	REGISTER_TEST(ut_Softmax);
	REGISTER_TEST(ut_PadAndRescale);
	REGISTER_TEST(ut_NmsBoxes);
	REGISTER_TEST(ut_RescaleFaceBox);
	TEST_RUN();

    return 0;
}
