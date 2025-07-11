#include <algorithm>
#include <iostream>
#include <string>

#include <string.h>
#include <assert.h>

#include "eaif_common.h"
#include "eaif_test.h"

#include "facereco_common.h"
#include "mtcnn.h"
#include "lite_mtcnn.h"

using namespace std;

constexpr float scalef = 0.3;
constexpr float thresholdf = 0.5;
constexpr double scaled = 0.3;
constexpr double thresholdd = 0.5;

const string py_dir = "python/";
const string pfile_img = py_dir + "pnet_sc1_img_y1_231_308_3.bin";
const string pfile_heatmap = py_dir + "pnet_sc1_conf110_149.bin";
const string pfile_coord = py_dir + "pnet_sc1_reg110_149_4.bin";
const string pfile_box = py_dir + "pnet_sc1_boxes154_9.bin";
const string pfile_nms = py_dir + "pnet_sc1_nms0_5-out78.bin";

const string cxx_dir = "cxx/";
const string cfile_img = cxx_dir + "pnet_sc1_img_img.bin";
const string cfile_heatmap = cxx_dir + "pnet_sc1_conf_110_149_2.bin";
const string cfile_coord = cxx_dir + "pnet_sc1_reg_110_149_4.bin";
const string cfile_box = cxx_dir + "pnet_sc1_box_154_9.bin";
const string cfile_nms = cxx_dir + "pnet_sc1_nms0_5_79_9.bin";

#define del6(a,b,c,d,e,f) \
{\
    delete[] a; delete[] d; \
    delete[] b; delete[] e; \
    delete[] c; delete[] f; \
}
#define del8(a,b,c,d,e,f,g,h) \
{\
    delete[] a; delete[] d; delete[] g;\
    delete[] b; delete[] e; delete[] h;\
    delete[] c; delete[] f; \
}

template<typename T, typename F>
int cmp_face_box(const FaceBox<T>& a, const FaceBox<F>& b)
{
    return fsame(a.x0,b.x0) &&
        fsame(a.y0,b.y0) &&
        fsame(a.x1,b.x1) &&
        fsame(a.y1,b.y1) &&
        fsame(a.score,b.score) &&
        fsame(a.regress[0], b.regress[0]) &&
        fsame(a.regress[1], b.regress[1]) &&
        fsame(a.regress[2], b.regress[2]) &&
        fsame(a.regress[3], b.regress[3]);
}

template<typename T>
void assign_facebox(const float* facebuffer, int num, vector<FaceBox<T>>& vf)
{
    vf.resize(num);
    for (int i = 0; i < num; i++) {
        auto& f = vf[i];
        int idx = i * 9;
        f.x0 = facebuffer[idx++];
        f.y0 = facebuffer[idx++];
        f.x1 = facebuffer[idx++];
        f.y1 = facebuffer[idx++];
        f.score = facebuffer[idx++];
        for (int j = 0; j < 4; j++)
            f.regress[j] = facebuffer[idx++];
    }
}

template<typename T>
void assign_facebox2(const float* buf, int num, const vector<FaceBox<T>>& fi, vector<FaceBox<T>>& fo)
{
    fo.resize(num);
    for (int i = 0; i < num; i++) {
        float val = buf[i];
        int idx = val;
        const auto& f = fi[idx];
        auto& o = fo[idx];
        o.x0 = f.x0;
        o.y0 = f.y0;
        o.x1 = f.x1;
        o.y1 = f.y1;
        o.score = f.score;
        for (int j = 0; j < 4; j++)
            o.regress[j] = f.regress[j];
    }
}

int test_generate_bounding_box(TestSuit *suit)
{
    int cw = 149, ch = 110;
    int pboxn = 154;
    int cboxn = 154;
    int ret __attribute__((unused));

    // conf map
    FILE *pfconf = fopen(pfile_heatmap.c_str(), "rb");
    assert(pfconf);
    float *ptmp = new float[ch * cw * 2];
    float *pconf = new float[ch * cw * 2];
    ret = fread(ptmp, ch * cw * sizeof(float), 1, pfconf);
    for (int i = 0; i < ch * cw; i++)
        pconf[i*2+1] = ptmp[i];
    fclose(pfconf);
    delete[] ptmp;

    FILE *cfconf = fopen(cfile_heatmap.c_str(), "rb");
    assert(cfconf);
    float *cconf = new float[ch * cw * 2];
    ret = fread(cconf, ch * cw * sizeof(float), 2, cfconf);
    fclose(cfconf);

    int same;
    //int same = memcmp(pconf, cconf, sizeof(float) * ch * cw);
    //if (same != 0) {
     //   test_log("input of %s %s are different!\n", pfile_heatmap.c_str(),
     //           cfile_heatmap.c_str());
      //  return 1;
    //}

    // reg map
    FILE *pfreg = fopen(pfile_coord.c_str(), "rb");
    assert(pfreg);
    float *preg = new float[ch * cw * 4];
    ret = fread(preg, ch * cw * sizeof(float) * 4, 1, pfreg);
    fclose(pfreg);

    FILE *cfreg = fopen(cfile_coord.c_str(), "rb");
    assert(cfreg);
    float *creg = new float[ch * cw * 4];
    ret = fread(creg, ch * cw * sizeof(float) * 4, 1, cfreg);
    fclose(cfreg);

    same = memcmp(preg, creg, sizeof(float) * ch * cw);
    if (same != 0) {
        testLog("input of %s %s are different!\n", pfile_coord.c_str(),
                cfile_coord.c_str());
        return 1;
    }

    // box result gt
    FILE *pfbox = fopen(pfile_box.c_str(), "rb");
    assert(pfbox);
    float *pbox = new float[pboxn * 9];
    ret = fread(pbox, pboxn * sizeof(float) * 9, 1, pfbox);
    fclose(pfbox);

    FILE *cfbox = fopen(cfile_box.c_str(), "rb");
    assert(cfbox);
    float *cbox = new float[cboxn * 9];
    ret = fread(cbox, cboxn * sizeof(float) * 9, 1, cfbox);
    fclose(cfbox);

    vector<FaceBox<float>> pvbox;
    assign_facebox(pbox, pboxn, pvbox);

    vector<FaceBox<float>> cvbox;
    assign_facebox(cbox, cboxn, cvbox);

    // box result generate
    vector<FaceBox<float>> cfacelist;
    GenerateBoundingBox(cconf, 0, creg, scalef, thresholdf, ch, cw, cfacelist, 0);

    vector<FaceBox<float>> pfacelist;
    GenerateBoundingBox(pconf, 0, preg, scalef, thresholdf, ch, cw, pfacelist, 0);

    vector<bool> a(cfacelist.size());
    if (cfacelist.size() != pfacelist.size())
        testLog("output of bounding from same input are different py:%d vs cxx:%d!!!\n",
            (int)pfacelist.size(), (int)cfacelist.size());

    for (size_t i = 0; i < cfacelist.size(); i++) {
        auto& cface = cfacelist[i];
        auto& pface = pfacelist[i];
        a[i] = cmp_face_box(cface, pface);
    }

    for (auto i : a)
        testAssert(i);

    for (size_t i = 0; i < cfacelist.size(); i++) {
        auto& cface = cfacelist[i];
        auto& pface = cvbox[i];
        a[i] = cmp_face_box(cface, pface);
    }

    //for (auto i : a)
    //    assert(i);
    del6(creg, cconf, cbox, preg, pconf, pbox);
    return 0;
}

template<typename T>
bool cmp_face_sort(const FaceBox<T>& a, const FaceBox<T>& b)
{
    return a.score > b.score;
}

int test_generate_bounding_box_1(TestSuit *suit)
{
    int cw = 149, ch = 110;
    int pboxn = 154;
    int cboxn = 154;
    int ret __attribute__((unused));

    // conf map
    FILE *pfconf = fopen(pfile_heatmap.c_str(), "rb");
    assert(pfconf);
    float *ptmp = new float[ch * cw * 2];
    float *pconf = new float[ch * cw * 2];
    ret = fread(ptmp, ch * cw * sizeof(float), 1, pfconf);
    for (int i = 0; i < ch * cw; i++)
        pconf[i*2+1] = ptmp[i];
    fclose(pfconf);
    delete[] ptmp;

    FILE *cfconf = fopen(cfile_heatmap.c_str(), "rb");
    assert(cfconf);
    float *cconf = new float[ch * cw * 2];
    ret = fread(cconf, ch * cw * sizeof(float), 2, cfconf);
    fclose(cfconf);

    int same;
    //int same = memcmp(pconf, cconf, sizeof(float) * ch * cw);
    //if (same != 0) {
     //   test_log("input of %s %s are different!\n", pfile_heatmap.c_str(),
     //           cfile_heatmap.c_str());
      //  return 1;
    //}

    // reg map
    FILE *pfreg = fopen(pfile_coord.c_str(), "rb");
    assert(pfreg);
    float *preg = new float[ch * cw * 4];
    ret = fread(preg, ch * cw * sizeof(float) * 4, 1, pfreg);
    fclose(pfreg);

    FILE *cfreg = fopen(cfile_coord.c_str(), "rb");
    assert(cfreg);
    float *creg = new float[ch * cw * 4];
    ret = fread(creg, ch * cw * sizeof(float) * 4, 1, cfreg);
    fclose(cfreg);

    same = memcmp(preg, creg, sizeof(float) * ch * cw);
    if (same != 0) {
        testLog("input of %s %s are different!\n", pfile_coord.c_str(),
                cfile_coord.c_str());
        return 1;
    }

    // box result gt
    FILE *pfbox = fopen(pfile_box.c_str(), "rb");
    assert(pfbox);
    float *pbox = new float[pboxn * 9];
    ret = fread(pbox, pboxn * sizeof(float) * 9, 1, pfbox);
    fclose(pfbox);

    FILE *cfbox = fopen(cfile_box.c_str(), "rb");
    assert(cfbox);
    float *cbox = new float[cboxn * 9];
    ret = fread(cbox, cboxn * sizeof(float) * 9, 1, cfbox);
    fclose(cfbox);

    vector<FaceBox<float>> pvbox;
    assign_facebox(pbox, pboxn, pvbox);

    vector<FaceBox<float>> cvbox;
    assign_facebox(cbox, cboxn, cvbox);

    // box result generate
    vector<FaceBox<float>> cfacelistf;
    GenerateBoundingBox(cconf, 0, creg, scalef, thresholdf, ch, cw, cfacelistf, 0);

    vector<FaceBox<float>> pfacelistf;
    GenerateBoundingBox(pconf, 0, preg, scalef, thresholdf, ch, cw, pfacelistf, 0);

    vector<FaceBox<double>> cfacelistd;
    GenerateBoundingBox(cconf, 0, creg, scaled, thresholdd, ch, cw, cfacelistd, 0);

    vector<FaceBox<double>> pfacelistd;
    GenerateBoundingBox(pconf, 0, preg, scaled, thresholdd, ch, cw, pfacelistd, 0);

    vector<bool> a(cfacelistf.size());

    // compare double generate vs python flow
    sort(pvbox.begin(), pvbox.end(), cmp_face_sort<float>);
    sort(cfacelistf.begin(), cfacelistf.end(), cmp_face_sort<float>);
    sort(cfacelistd.begin(), cfacelistd.end(), cmp_face_sort<double>);

    for (size_t i = 0; i < cfacelistd.size(); i++) {
        auto& fface = cfacelistd[i];
        auto& dface = pvbox[i];
        a[i] = cmp_face_box(dface, fface);
    }

    for (auto i : a)
        testAssert(i);

    sort(pfacelistf.begin(), pfacelistf.end(), cmp_face_sort<float>);
    sort(cfacelistf.begin(), cfacelistf.end(), cmp_face_sort<float>);

    a.resize(pfacelistf.size());
    for (size_t i = 0; i < pfacelistf.size(); i++) {
        auto& fface = pfacelistf[i];
        auto& dface = cfacelistf[i];
        a[i] = cmp_face_box(dface, fface);
    }

    for (auto i : a)
        testAssert(i);

    //for (size_t i = 0; i < cfacelistf.size(); i++) {
    //    auto& fface = cfacelistf[i];
    //    auto& dface = cfacelistd[i];
    //    a[i] = cmp_face_box(dface, fface);
    //}

    //for (auto i : a)
    //    assert(i);
    del6(creg, cconf, cbox, preg, pconf, pbox);

    return 0;
}

int test_nms(TestSuit *suit)
{
    int cw = 149, ch = 110;
    int pboxn = 154;
    int cboxn = 154;
    int pnmsn = 78;
    int cnmsn = 79;
    int ret __attribute__((unused));

    // conf map
    FILE *pfconf = fopen(pfile_heatmap.c_str(), "rb");
    assert(pfconf);
    vector<float> ptmp(ch * cw * 2);
    vector<float> pconf(ch * cw * 2);
    ret = fread((uint8_t*)ptmp.data(), ch * cw * sizeof(float), 1, pfconf);
    for (int i = 0; i < ch * cw; i++)
        pconf[i*2+1] = ptmp[i];
    fclose(pfconf);

    FILE *cfconf = fopen(cfile_heatmap.c_str(), "rb");
    testAssert(cfconf);
    vector<float> cconf(ch * cw * 2);
    ret = fread((uint8_t*)cconf.data(), ch * cw * sizeof(float), 2, cfconf);
    fclose(cfconf);

    int same;
    //int same = memcmp(pconf, cconf, sizeof(float) * ch * cw);
    //if (same != 0) {
     //   test_log("input of %s %s are different!\n", pfile_heatmap.c_str(),
     //           cfile_heatmap.c_str());
      //  return 1;
    //}

    // reg map
    FILE *pfreg = fopen(pfile_coord.c_str(), "rb");
    testAssert(pfreg);
    vector<float> preg(ch * cw * 4);
    ret = fread((uint8_t*)preg.data(), ch * cw * sizeof(float) * 4, 1, pfreg);
    fclose(pfreg);

    FILE *cfreg = fopen(cfile_coord.c_str(), "rb");
    testAssert(cfreg);
    vector<float> creg(ch * cw * 4);
    ret = fread((uint8_t*)creg.data(), ch * cw * sizeof(float) * 4, 1, cfreg);
    fclose(cfreg);

    same = memcmp(preg.data(), creg.data(), sizeof(float) * ch * cw);
    if (same != 0) {
        testLog("input of %s %s are different!\n", pfile_coord.c_str(),
                cfile_coord.c_str());
        return 1;
    }

    // box result gt
    FILE *pfbox = fopen(pfile_box.c_str(), "rb");
    testAssert(pfbox);
    vector<float> pbox(pboxn * 9);
    ret = fread((uint8_t*)pbox.data(), pboxn * sizeof(float) * 9, 1, pfbox);
    fclose(pfbox);

    FILE *cfbox = fopen(cfile_box.c_str(), "rb");
    testAssert(cfbox);
    vector<float> cbox(cboxn * 9);
    ret = fread((uint8_t*)cbox.data(), cboxn * sizeof(float) * 9, 1, cfbox);
    fclose(cfbox);

    vector<FaceBox<float>> pvbox;
    assign_facebox(pbox.data(), pboxn, pvbox);

    vector<FaceBox<float>> cvbox;
    assign_facebox(cbox.data(), cboxn, cvbox);

    // nms result
    FILE *pfnms = fopen(pfile_nms.c_str(), "rb");
    testAssert(pfnms);
    vector<float> pnms(pnmsn);
    ret = fread((uint8_t*)pnms.data(), pnmsn * sizeof(float), 1, pfnms);
    fclose(pfnms);

    FILE *cfnms = fopen(cfile_nms.c_str(), "rb");
    testAssert(cfnms);
    vector<float> cnms(cnmsn * 9);
    ret = fread((uint8_t*)cnms.data(), cnmsn * sizeof(float) * 9, 1, cfnms);
    fclose(cfnms);

    vector<FaceBox<float>> pvnms;
    for (int i = 0; i < pnmsn; ++i) {
        int val = pnms[i];
        FaceBox<float> &f = pvbox[val];
        FaceBox<float> box;
        box.x0 = f.x0;
        box.x1 = f.x1;
        box.y0 = f.y0;
        box.y1 = f.y1;
        box.score = f.score;
        box.regress[0] = f.regress[0];
        box.regress[1] = f.regress[1];
        box.regress[2] = f.regress[2];
        box.regress[3] = f.regress[3];
        pvnms.push_back(box);
    }
    vector<FaceBox<float>> cvnms(cnmsn);
    assign_facebox(cnms.data(), cnmsn, cvnms);


    // box result generate
    // floating point 
    vector<FaceBox<float>> cfacelistf;
    GenerateBoundingBox((const float*)cconf.data(), 0, (const float*)creg.data(), scalef, thresholdf, ch, cw, cfacelistf, 0);

    vector<FaceBox<float>> pfacelistf;
    GenerateBoundingBox((const float*)pconf.data(), 0, (const float*)preg.data(), scalef, thresholdf, ch, cw, pfacelistf, 0);

    // double precision
    vector<FaceBox<double>> cfacelistd;
    GenerateBoundingBox((const float*)cconf.data(), 0, (const float*)creg.data(), scaled, thresholdd, ch, cw, cfacelistd, 0);

    vector<FaceBox<double>> pfacelistd;
    GenerateBoundingBox((const float*)pconf.data(), 0, (const float*)preg.data(), scaled, thresholdd, ch, cw, pfacelistd, 0);

    vector<bool> a(cfacelistf.size());

    // compare double generate vs python flow
    // dump output
    sort(pvbox.begin(), pvbox.end(), cmp_face_sort<float>);
    sort(cvbox.begin(), cvbox.end(), cmp_face_sort<float>);

    // generate output
    sort(pfacelistf.begin(), pfacelistf.end(), cmp_face_sort<float>);
    sort(cfacelistf.begin(), cfacelistf.end(), cmp_face_sort<float>);

    // generated double precision
    sort(pfacelistd.begin(), pfacelistd.end(), cmp_face_sort<double>);
    sort(cfacelistd.begin(), cfacelistd.end(), cmp_face_sort<double>);

    // nms
    float nms_thf = 0.5;
    double nms_thd = 0.5;

    vector<FaceBox<float>> pnmsf;
    vector<FaceBox<float>> pnmsff;
    vector<FaceBox<float>> cnmsf;
    vector<FaceBox<double>> pnmsd;
    vector<FaceBox<double>> cnmsd;

    NmsBoxes(pvbox, nms_thf, NMS_UNION, pnmsf);
    NmsBoxes(pfacelistf, nms_thf, NMS_UNION, pnmsff);
    NmsBoxes(cfacelistf, nms_thf, NMS_UNION, cnmsf);
    NmsBoxes(pfacelistd, nms_thd, NMS_UNION, pnmsd);
    NmsBoxes(cfacelistd, nms_thd, NMS_UNION, cnmsd);

    sort(pvnms.begin(), pvnms.end(), cmp_face_sort<float>);
    sort(pnmsf.begin(), pnmsf.end(), cmp_face_sort<float>);
    sort(pnmsff.begin(), pnmsff.end(), cmp_face_sort<float>);
    sort(cvnms.begin(), cvnms.end(), cmp_face_sort<float>);
    sort(cnmsf.begin(), cnmsf.end(), cmp_face_sort<float>);
    sort(pnmsd.begin(), pnmsd.end(), cmp_face_sort<double>);
    sort(cnmsd.begin(), cnmsd.end(), cmp_face_sort<double>);

    a.resize(pvbox.size());
    // python box float vs p double gen box
    for (size_t i = 0; i < pvbox.size(); i++) {
        auto& fface = pvbox[i];
        auto& dface = pfacelistd[i];
        a[i] = cmp_face_box(dface, fface);
    }
    for (auto i : a)
        testAssert(i);


    a.resize(pnmsf.size());
    // python box float nms vs p double gen box double nms
    for (size_t i = 0; i < pnmsf.size(); i++) {
        auto& fface = pnmsf[i];
        auto& dface = pnmsd[i];
        a[i] = cmp_face_box(dface, fface);
    }
    for (auto i : a)
        testAssert(i);

    a.resize(pnmsff.size());
    // python gen float nms float nms vs p cxx float dump nms
    for (size_t i = 0; i < pnmsff.size(); i++) {
        auto& fface = pnmsff[i];
        auto& dface = cnmsf[i];
        a[i] = cmp_face_box(dface, fface);
    }
    for (auto i : a)
        testAssert(i);

    a.resize(cnmsd.size());
    // python nms vs p cxx gen double dump nms
    for (size_t i = 0; i < cnmsd.size(); i++) {
        auto& fface = cnmsd[i];
        auto& dface = pvnms[i];
        a[i] = cmp_face_box(dface, fface);
    }
    for (auto i : a)
        testAssert(i);

    return 0;
}

int register_test()
{
    TestSuit test;

    test.vec_func = { test_generate_bounding_box, test_generate_bounding_box_1, test_nms };
    test.run();
    test.report();
    return 0;
}

int main()
{
    register_test();
    return 0;
}
