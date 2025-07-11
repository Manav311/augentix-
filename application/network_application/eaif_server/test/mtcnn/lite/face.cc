#include <algorithm>
#include <iostream>
#include <string>

#include <string.h>
#include <assert.h>

#include "eaif_common.h"
#include "eaif_test.h"

#include "eaif_image.h"
#include "eaif_model.h"
#include "facereco_model.h"

#define FACERECO_MODEL_PATH "../../../models/facereco"

#define NUM_THREAD 1
#define DEBUG 1
#define VERBOSE 1

using namespace std;

string g_image_path = "../../../data/obama.jpg";
int g_no_ppl = 1;
int g_pre_resize = -1;
int g_gray_scale = 0;

int test_mtcnn_memory(TestSuit *suit)
{
    ModelConfig config;
    string model_path = FACERECO_MODEL_PATH;
    vector<Detection> dets;

    int ret = config.Parse(model_path.c_str());
    if (ret) {
        return -1;
    }
    config.face_infer_type = eaif::engine::Mtcnn;
    config.image_pre_resize = g_pre_resize;
    FacerecoModel *facereco_model = new FacerecoModel();
    facereco_model->config = config;
    facereco_model->SetNumThreads(NUM_THREAD);
    facereco_model->SetDebug(DEBUG);
    facereco_model->SetVerbose(VERBOSE);
    ret = facereco_model->LoadModels(config.model_dir, config.model_path);
    if (ret == EAIF_FAILURE) {
        delete facereco_model;
        facereco_model = nullptr;
        return 0;
    }

    { 
        eaif::image::Image img;
        if (!g_gray_scale) {
            eaif::image::Imread(g_image_path.c_str(), img);
        } else {
            eaif::image::Image raw;
            eaif::image::Imread(g_image_path.c_str(), raw);
            eaif::image::ImcvtGray(raw, img);
        }
        printf("image size: %dx%dx%d from %s\n", img.cols, img.rows, img.channels(), g_image_path.c_str());
        for (int i = 0; i < 2; i++) {
            dets.clear();
            facereco_model->Detect((const void*)&img, dets, config);
        }
    }
    printf("total %u faces detected vs expected %d\n", (uint32_t)dets.size(), g_no_ppl);
    testAssert(dets.size() == (uint32_t)g_no_ppl);

    delete facereco_model;
    facereco_model = nullptr;
    return 0;
}

int test_facenet_memory(TestSuit *suit)
{
    ModelConfig config;
    string model_path = FACERECO_MODEL_PATH;
    vector<string> face_names;

    config.face_infer_type = eaif::engine::Facenet;
    int ret = config.Parse(model_path.c_str());
    if (ret) {
        return -1;
    }
    FacerecoModel *facereco_model = new FacerecoModel();
    facereco_model->config = config;
    facereco_model->SetNumThreads(NUM_THREAD);
    facereco_model->SetDebug(DEBUG);
    facereco_model->SetVerbose(VERBOSE);
    ret = facereco_model->LoadModels(config.model_dir, config.model_path);
    if (ret == EAIF_FAILURE) {
        delete facereco_model;
        facereco_model = nullptr;
        return 0;
    }

    {
        eaif::image::Image img;
        eaif::image::Imread(g_image_path.c_str(), img);
        printf("image size: %dx%dx%d from %s\n", img.cols, img.rows, img.channels(), g_image_path.c_str());
        Detection det{{0,0,(float)img.cols-1,(float)img.rows-1}};
        for (int i = 0; i < 2; i++) {
            facereco_model->Register((void*)&img, "test" + to_string(i), det, config);
        }
        facereco_model->QueryFaceInfo(face_names);
    }
    testAssert(face_names[1] == "test1");

    delete facereco_model;
    facereco_model = nullptr;
    return 0;
}

int register_test()
{
    TestSuit test;

    test.vec_func = {
        test_mtcnn_memory
    //    ,test_facenet_memory
        };
    test.run();
    test.report();
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 3) {
        g_image_path = argv[1];
        g_no_ppl = atoi(argv[2]);
    }
    if (argc == 4) {
        g_pre_resize = atoi(argv[3]);
    }
    if (argc == 5) {
        g_gray_scale = atoi(argv[4]);
    }
    register_test();
    return 0;
}
