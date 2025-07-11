#include <iostream>
#include <string>
#include <vector>

#include "eaif_test.h"
#include "eaif_image.h"

using namespace std;

static constexpr char img_name0[] = "../../data/obama.jpg"; // hxw (294x525)
static constexpr char img_name1[] = "../../data/00000.jpg";

/***
 * test_resizeAspectRatio_case0
 * test for pad height and pad width
 * Need visual inspection for correctness
 */
int test_resizeAspectRatio_case0(TestSuit *suit)
{
	eaif::image::Image img;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	int dst_w = 224;
	int dst_h = 224;
	eaif::image::Image dst;
	vector<float> zeros(3,0.0);
	vector<float> scales(3,0.00392156862745);
	eaif::image::ImresizeNormAspectRatio(img, dst, dst_w, dst_h, zeros.data(), scales.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testAspect0.bin", dst);

	eaif::image::Image img1;
	eaif::image::Imread(img_name1, img1);
	testAssert(img1.data);
	eaif::image::ImresizeNormAspectRatio(img1, dst, dst_w, dst_h, zeros.data(), scales.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testAspect1.bin", dst);

	testAssert(1);
	return 0;
}

/***
 * test_resizeAspectRatio_case1
 * test for exact output
 * Need visual inspection for correctness
 */
int test_resizeAspectRatio_case1(TestSuit *suit)
{
	eaif::image::Image img;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	int dst_w = 525;
	int dst_h = 294;
	eaif::image::Image dst;
	vector<float> zeros(3,0.0);
	vector<float> scales(3,0.00392156862745);
	eaif::image::ImresizeNormAspectRatio(img, dst, dst_w, dst_h, zeros.data(), scales.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testAspect0.bin", dst);

	testAssert(1);
	return 0;
}

/***
 * test_imcropResizeNorm_case0
 * test for exact output
 * Need visual inspection for correctness
 */
int test_imcropResizeNorm_case0(TestSuit *suit)
{
	eaif::image::Image img;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	//int src_w = 525;
	//int src_h = 294;
	int dst_w = 200;
	int dst_h = 200;

	// 0 - typical case
	int sx = 100, ex = 400, sy = 50, ey = 290;
	eaif::image::Image dst;
	vector<float> zeros(3,0.0);
	vector<float> scales(3,0.00392156862745);
	eaif::image::ImcropResizeNorm(img, sx, sy, ex, ey, dst, dst_w, dst_h, zeros.data(), scales.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropResizeNorm0.bin", dst);

	// 1 - crop whole image
	sx = 0; ex = 525; sy = 0; ey = 294;
	eaif::image::ImcropResizeNorm(img, sx, sy, ex, ey, dst, dst_w, dst_h, zeros.data(), scales.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropResizeNorm1.bin", dst);

	// 2 - exceed resolution
	sx = -10; ex = 530; sy = -20; ey = 315;
	eaif::image::ImcropResizeNorm(img, sx, sy, ex, ey, dst, dst_w, dst_h, zeros.data(), scales.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropResizeNorm2.bin", dst);
	return 0;
}

/***
 * test_imCropResize_case0
 * test for exact output
 * Need visual inspection for correctness
 */
int test_imcropResize_case0(TestSuit *suit)
{
	eaif::image::Image img;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	//int src_w = 525;
	//int src_h = 294;
	int dst_w = 200;
	int dst_h = 200;

	// 0 - typical case
	int sx = 100, ex = 400, sy = 50, ey = 290;
	eaif::image::Image dst;
	eaif::image::ImcropResize(img, sx, sy, ex, ey, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropResize0.bin", dst);

	// 1 - crop whole image
	sx = 0; ex = 525; sy = 0; ey = 294;
	eaif::image::ImcropResize(img, sx, sy, ex, ey, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropResize1.bin", dst);

	// 2 - exceed resolution
	sx = -10; ex = 530; sy = -20; ey = 315;
	eaif::image::ImcropResize(img, sx, sy, ex, ey, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropResize2.bin", dst);
	return 0;
}


static void square_box(int *sx, int *ex, int *sy, int *ey)
{
	int w = *ex - *sx + 1;
	int h = *ey - *sy + 1;
	int l = std::max(w, h);
	*sx = *sx+((w-l) / 2);
	*sy = *sy+((h-l) / 2);
	*ex = *sx+l-1;
	*ey = *sy+l-1;
}

static void pad_box(int x0, int y0, int x1, int y1, int *px0, int *py0, int *px1, int *py1, int w, int h)
{
	*px0=std::max(x0,1);
	*py0=std::max(y0,1);
	*px1=std::min(x1,w);
	*py1=std::min(y1,h);
}

/***
 * test_ImcropPadResize_case0
 * test for exact output
 * Need visual inspection for correctness
 */
int test_ImcropPadResize_case0(TestSuit *suit)
{
	eaif::image::Image img;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	//int src_w = 525;
	//int src_h = 294;
	int dst_w = 200;
	int dst_h = 200;
	int sx, ex, sy, ey, px0, py0, px1, py1;
	int pad_t, pad_b, pad_l, pad_r;

	// 0 - typical case
	sx = 100; ex = 400; sy = 50; ey = 290;
	// w = 300, h = 240, max = 300
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::Image dst;
	eaif::image::ImcropPadResize(img, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::Imwrite("testImcropPadResize0.jpg", dst);

	// 1 - crop whole image
	sx = 0; ex = 525; sy = 0; ey = 294;
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::ImcropPadResize(img, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::Imwrite("testImcropPadResize1.jpg", dst);

	// 2 - exceed resolution
	sx = -10; ex = 530; sy = -20; ey = 315;
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::ImcropPadResize(img, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::Imwrite("testImcropPadResize2.jpg", dst);
	return 0;
}

/***
 * test_ImcropPadResize_case0
 * test for exact output
 * Need visual inspection for correctness
 */
int test_ImcropPadResize_case1(TestSuit *suit)
{
	eaif::image::Image img, gray;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	//int src_w = 525;
	//int src_h = 294;
	int dst_w = 200;
	int dst_h = 200;
	int dst_c = 3;
	uint8_t *data = (uint8_t*)malloc(sizeof(uint8_t) * dst_w * dst_h * dst_c);
	int sx, ex, sy, ey, px0, py0, px1, py1;
	int pad_t, pad_b, pad_l, pad_r;

	// broadcast
	sx = -10; ex = 530; sy = -20; ey = 315;
	// w = 300, h = 240, max = 300
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::Image dst(dst_h, dst_w, dst_c, Eaif8UC3, data);
	eaif::image::ImcvtGray(img, gray);
	eaif::image::Imwrite("testImcropPadResize1_gray.jpg", gray);
	eaif::image::ImcropPadResize(gray, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h);
	testAssert(dst.data);
	eaif::image::Imwrite("testImcropPadResize1_bc.jpg", dst);

	return 0;
}

/***
 * test_ImcropPadResizeNorm_case0
 * test for exact output
 * Need visual inspection for correctness
 */
int test_ImcropPadResizeNorm_case0(TestSuit *suit)
{
	eaif::image::Image img;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	//int src_w = 525;
	//int src_h = 294;
	int dst_w = 200;
	int dst_h = 200;
	int px0, py0, px1, py1;
	int pad_t, pad_b, pad_l, pad_r;
	vector<float> scale(3,1/127.5);
	vector<float> zero(3,127.5);

	// 0 - typical case
	int sx = 100, ex = 400, sy = 50, ey = 290;
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::Image dst;
	eaif::image::ImcropPadResizeNorm(img, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h, zero.data(), scale.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropPadResizeNorm0.bin", dst);

	// 1 - crop whole image
	sx = 0; ex = 525; sy = 0; ey = 294;
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::ImcropPadResizeNorm(img, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h, zero.data(), scale.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropPadResizeNorm1.bin", dst);

	// 2 - exceed resolution
	sx = -10; ex = 530; sy = -20; ey = 315;
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::ImcropPadResizeNorm(img, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h, zero.data(), scale.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropPadResizeNorm2.bin", dst);
	return 0;
}

/***
 * test_ImcropPadResizeNorm_case1
 * test for exact output
 * Need visual inspection for correctness
 */
int test_ImcropPadResizeNorm_case1(TestSuit *suit)
{
	eaif::image::Image img, gray;
	eaif::image::Imread(img_name0, img);
	testAssert(img.data);
	vector<float> scale(3,1/127.5);
	vector<float> zero(3,127.5);
	//int src_w = 525;
	//int src_h = 294;
	int dst_w = 200;
	int dst_h = 200;
	int dst_c = 3;
	uint8_t *data = (uint8_t*)malloc(sizeof(float) * dst_w * dst_h * dst_c);
	int sx, ex, sy, ey, px0, py0, px1, py1;
	int pad_t, pad_b, pad_l, pad_r;

	// broadcast
	sx = -10; ex = 530; sy = -20; ey = 315;
	// w = 300, h = 240, max = 300
	square_box(&sx, &ex, &sy, &ey);
	pad_box(sx, sy, ex, ey, &px0, &py0, &px1, &py1, img.cols, img.rows);
	pad_t = std::abs(py0 - sy);
	pad_l = std::abs(px0 - sx);
	pad_b = std::abs(py1 - ey);
	pad_r = std::abs(px1 - ex);
	eaif::image::Image dst(dst_h, dst_w, dst_c, Eaif32FC3, data);
	eaif::image::ImcvtGray(img, gray);
	eaif::image::Imwrite("testImcropPadResizeNorm1_gray.jpg", gray);
	eaif::image::ImcropPadResizeNorm(gray, px0, py0, px1, py1, pad_t, pad_b, pad_l, pad_r, dst, dst_w, dst_h, zero.data(), scale.data());
	testAssert(dst.data);
	eaif::image::ImsaveBmp("testImcropPadResizeNorm1_bc.bin", dst);
	return 0;
}


int register_test()
{
	TestSuit test;
    test.vec_func = {
        //test_resizeAspectRatio_case0,
        //test_resizeAspectRatio_case1,
        //test_imcropResizeNorm_case0,
        //test_imcropResize_case0,
        test_ImcropPadResize_case0,
        test_ImcropPadResizeNorm_case0,
        test_ImcropPadResize_case1,
        test_ImcropPadResizeNorm_case1, };
    test.run();
    test.report();
    return 0;
}

int main()
{
	register_test();
	return 0;
}
