#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <libgen.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "eaif_trc.h"

#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#endif /* !USE_OPENCV */

#ifdef USE_TFLITE
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "lite_mtcnn.h"
#include "lite_facenet_model.h"
#include "lite_yolo_model.h"
using namespace tflite;
using tflite::utils::PrintOutput;
#endif /* !USE_TFLITE */

#ifdef USE_ARMNN
#include "armnn_mtcnn.h"
#include "armnn_facenet_model.h"
#include "armnn_yolo_model.h"
using namespace armnn;
using armnn::utils::PrintOutput;
#endif /* !USE_ARMNN */

#include "eaif_data.h"
#include "eaif_engine_demo.h"
#include "eaif_model.h"
#include "eaif_image.h"
#include "eaif_utils.h"

#ifdef USE_C4
#include "c4_classify_model.h"
#endif /* !USE_C4 */

#include <chrono>

using namespace eaif::image;
using namespace std;

void eaif::EngineDemo::SetupConfig(ModelConfig& config, const char* dataset, const VecStr *model_path)
{
	config.input_shape.w = m_w;
	config.input_shape.h = m_h;
	config.cpu_infer_type = m_cpu_infer;
	config.conf_thresh.push_back(m_conf_thresh);
	config.iou_thresh = m_iou_thresh;
	config.engine_type = (eaif::engine::EngineType)m_engine;
	config.zeros.push_back(m_zero[0]);
	config.zeros.push_back(m_zero[1]);
	config.zeros.push_back(m_zero[2]);
	config.stds.push_back(m_scale[0]);
	config.stds.push_back(m_scale[1]);
	config.stds.push_back(m_scale[2]);

	config.conf_thresh_low = 0.0f;
	config.topk = 0;

	if (dataset) {
		config.dataset = data::GetDatasetType(dataset);
		config.num_classes = data::GetNumClass(config.dataset);
		config.labels.resize(config.num_classes);
		for (int i = 0; i < config.num_classes; ++i) {
			config.labels[i] = data::GetLabel(config.dataset, i);
		}
	}

	if (m_threshold.size()) {
		config.conf_thresh.clear();
		for (const auto& thr : m_threshold)
			config.conf_thresh.push_back(thr);
	}
	config.window_min_size = m_minface;
	config.window_scale_factor = m_scalefactor;
	config.nms_internal_thresh = m_nms_internal_thr;

	if (model_path) {
		config.model_path = *model_path;
	}
	// config.model_name;
	// config.model_dir;
	// config.model_path;
	// config.inference_type;
	// config.activation_type;
}


void eaif::EngineDemo::SetDebugMode(int val)
{
	m_debug = val;
#ifdef USE_ARMNN
	armnn::utils::SetDebugMode(val);
#endif
}

void eaif::EngineDemo::SetNumThreads()
{
#ifdef USE_ARMNN
	armnn::utils::SetThreadNum(m_nthread);
#endif
}

int eaif::EngineDemo::RunCascade(const char* img_file, const char* model_dir_)
{
#ifdef USE_OPENCV
	//const char* model_dir_ = "./haarcascade_frontalface_alt2.xml";
	cv::Mat img;
	cv::Mat gray;
	cv::CascadeClassifier cascade;
	vector<cv::Rect> faces;

	Imread(img_file, img, m_w, m_h);
	cv::cvtColor(img, gray, cv::COLOR_RGB2GRAY);
	int ret = cascade.load(model_dir_);
	if (ret == 0) {
		cout << "Cannot load CascadeClassifier!" << model_dir_ << "\n" ;
		exit(0);
	}
	// Detect faces of different sizes using cascade classifier  
    TIMER(cascade.detectMultiScale( gray, faces, 1.1,  
                            3, 0|cv::CASCADE_SCALE_IMAGE, cv::Size(10, 10) ));
	for (auto face : faces) {
		printf("face pos:[%d %d %d %d]\n",
			face.x, face.y, face.x+face.width-1, face.y+face.height-1);
	}
#endif /* USE_OPENCV */
	return 0;
}

int eaif::EngineDemo::RunHog(const char* img_file)
{
#ifdef USE_OPENCV
	//const char* model_dir_ = "./haarcascade_frontalface_alt2.xml";
	cv::Mat img;
	cv::HOGDescriptor hog;
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	vector<cv::Rect> ppls;
	vector<cv::Point> ppts;
	vector<cv::Point> none;
	Imread(img_file, img, m_w, m_h);

	for (int i = 0; i < m_iter-1; ++i) {
		if (m_misc_flag != 0.0) {
			TIMER(hog.detect(img, ppts, 0, cv::Size(2,16), cv::Size(8,8), none));
		} else {
			TIMER(hog.detectMultiScale(img, ppls, 0, cv::Size(2,16), cv::Size(8,8), 1.05, 4));
		}
	}
	if (m_misc_flag != 0.0) {
		ppts.empty();
		TIMER(hog.detect(img, ppts, 0, cv::Size(2,16), cv::Size(8,8), none));
		int i = 0;
		for (auto pp : ppts) {
			printf("%d : people pos:[%d %d]\n",
				i++, pp.x, pp.y);
		}
	} else {
		ppls.empty();
		TIMER(hog.detectMultiScale(img, ppls, 0, cv::Size(2,16), cv::Size(8,8), 1.05, 4));
		int i = 0;
		for (auto pp : ppls) {
			printf("%d : people pos:[%d %d %d %d]\n",
				i++, pp.x, pp.y, pp.x+pp.width-1, pp.y+pp.height-1);
		}
	}
#endif /* USE_OPENCV */
	return 0;
}

int eaif::EngineDemo::RunYolov5(const char* model_path, const char* img_file, const char* dataset)
{

#if defined(USE_TFLITE) || defined(USE_ARMNN)
	WImage img;


	DetectModel *yolov5 = nullptr;
	if (m_engine) {
#ifdef USE_TFLITE
		LiteYolov5Model *lyolov5 = new LiteYolov5Model();
		yolov5 = dynamic_cast<DetectModel*>(lyolov5);
		yolov5->SetNumThreads(m_nthread);
#endif
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnYolov5Model *ayolov5 = new ArmnnYolov5Model();
		ayolov5->SetDebug((m_debug != 3));
		ayolov5->SetCpuType(m_cpu_infer);
		yolov5 = dynamic_cast<DetectModel*>(ayolov5);
		this->SetNumThreads();
#endif
	} else
		assert(0);
	assert(yolov5 != nullptr);
	ModelConfig confg;

	eaif_info_h("image file %s\n", img_file);
	eaif::image::Imread(img_file, img);
	if (img.channels() == 1)
		eaif::image::ImbroadcastChannel(img);
	yolov5->SetVerbose(m_verbose);

	vector<string> model_paths;
	model_paths.push_back(model_path);
	yolov5->LoadModels("", model_paths);

	if (yolov5->m_type == Eaif8U) {
		eaif_warn("Not implemented yet!");
		return -1;
	}

	ModelConfig conf;
	SetupConfig(conf, dataset);

	for (int i = 0; i < m_iter-1; ++i) {
		vector<Detection> temp;
		TIMER(yolov5->Detect(&img, temp, conf));
	}

	vector<Detection> detections;
	for (int i = 0; i < 1; ++i) {
		TIMER(yolov5->Detect(&img, detections, conf));
	}

	printf("total detections:%d\n", (int)detections.size());
	int i = 0;
	for (auto det : detections) {
		for (unsigned int c = 0; c < det.prob.size(); ++c) {
			if (det.prob[c] != 0.0f) {
				printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.4f cls:%.4f label:%s\n",
				++i, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence, det.prob[c],
				conf.labels[c].c_str());
			}
		}
	}

#endif
	return 0;
}

int eaif::EngineDemo::RunYolov4(const char* model_path, const char* img_file, const char* dataset)
{
#if defined(USE_TFLITE) || defined(USE_ARMNN)
	WImage img;

	DetectModel *yolov4 = nullptr;
	if (m_engine) {
#ifdef USE_TFLITE
		LiteYolov4Model *lyolov4 = new LiteYolov4Model();
		yolov4 = dynamic_cast<DetectModel*>(lyolov4);
		yolov4->SetNumThreads(m_nthread);
#endif
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnYolov4Model *ayolov4 = new ArmnnYolov4Model();
		ayolov4->SetDebug((m_debug != 3));
		ayolov4->SetCpuType(m_cpu_infer);
		yolov4 = dynamic_cast<DetectModel*>(ayolov4);
		this->SetNumThreads();
#endif
	} else
		assert(0);
	assert(yolov4 != nullptr);
	eaif::image::Imread(img_file, img);
	if (img.channels() == 1)
		eaif::image::ImbroadcastChannel(img);

	yolov4->SetVerbose(m_verbose);

	vector<string> model_paths;
	model_paths.push_back(model_path);
	yolov4->LoadModels("", model_paths);

	if (yolov4->m_type == Eaif8U) {
		eaif_warn("Not implemented yet!");
		return -1;
	}

	ModelConfig conf;
	SetupConfig(conf, dataset);

	for (int i = 0; i < m_iter-1; ++i) {
		vector<Detection> temp;
		TIMER(yolov4->Detect(&img, temp, conf));
	}

	vector<Detection> detections;
	TIMER(yolov4->Detect(&img, detections, conf));

	int i=0;
	printf("total detections:%d [w:%d, h:%d]\n", (int)detections.size(), img.cols, img.rows);
	for (auto det : detections) {
		for (unsigned int c = 0; c < det.prob.size(); ++c) {
			if (det.prob[c] != 0.0f) {
				printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.4f cls:%.4f label:%s\n",
				++i, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence, det.prob[c],
				conf.labels[c].c_str());
			}
		}
	}

#endif
	return 0;
}

static int GetfileId(string img_file)
{
	char file_base[256] = {};
	//char base_name_str[256];
	char *base_name;
	strcpy(file_base, img_file.c_str());
	base_name = strchr(file_base,'.');
	base_name[0] = 0;
	base_name = basename(file_base);
	return atoi(base_name);
}

#define DETECTION_DIR "detection_result"

int eaif::EngineDemo::RunYolov5Eval(const char* model_path, const char* image_list, const char* dataset)
{

#if defined(USE_TFLITE) || defined(USE_ARMNN)

	unique_ptr<DetectModel> yolov5 = nullptr;

	if (m_engine) {
#ifdef USE_TFLITE
		LiteYolov5Model *lyolov5 = new LiteYolov5Model();
		yolov5 = unique_ptr<DetectModel>(lyolov5);
		yolov5->SetNumThreads(m_nthread);
#endif
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnYolov5Model *ayolov5 = new ArmnnYolov5Model();
		ayolov5->SetDebug((m_debug != 3));
		ayolov5->SetCpuType(m_cpu_infer);
		yolov5 = unique_ptr<DetectModel>(ayolov5);
		this->SetNumThreads();
#endif
	} else
		assert(0);

	//eaif::image::ImRead(img_file, img);

	ifstream fr;
	fr.open(image_list);
	int file_id = 0;
	dataset::CreateDir(DETECTION_DIR);

	if (!fr.is_open()) {
		eaif_warn("Cannot open file %s\n", image_list);
		return -1;
	}

	yolov5->SetVerbose(m_verbose);
	vector<string> model_paths;
	model_paths.push_back(model_path);
	yolov5->LoadModels("", model_paths);

	yolov5->SetPostProcessMethod(11);
	if (yolov5->m_type == Eaif8U) {
		eaif_warn("Not implemented yet!");
		return -1;
	}

	ModelConfig conf;
	SetupConfig(conf, dataset);

	int i = 0;
	while (!fr.eof()) {
		string img_file;
		getline(fr, img_file);
		cout << "\""<< img_file << "\"\n";
		if (access(img_file.c_str(), F_OK) == -1) {
			eaif_warn("Cannot find %s, Skip!\n", img_file.c_str());
			continue;
		}
		WImage img;
		eaif::image::Imread(img_file.c_str(), img);
		if (img.channels() == 1)
			eaif::image::ImbroadcastChannel(img);

		vector<Detection> detections;
		file_id = GetfileId(img_file);
		TIMER(yolov5->Detect(&img, detections, conf));
		printf("total detections:%d for %012d(%d)\n", (int)detections.size(), file_id, i++);
		int h = img.rows;
		int w = img.cols;
		char detection_result[256] = {};
		sprintf(detection_result, "detection_result/%012d.json", file_id);
		FILE *fp = fopen(detection_result, "w");
		assert(fp!=NULL);
#if 1
		for (auto det : detections) {
			if (det.confidence != 0.0f) {
				float sx = Clamp(det.box.sx, 0.0f, (float)w);
				float sy = Clamp(det.box.sy, 0.0f, (float)h);
				float bw = Clamp(det.box.ex-det.box.sx+1, 0.0f, (float)w);
				float bh = Clamp(det.box.ey-det.box.sy+1, 0.0f, (float)h);
				int c = det.cls[0];
				//printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.4f label:%s\n",
				//++j, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence,
				//yolov5.dataset_ == data::COCO ?	data::coco::labels[det.cls[0]].c_str() : data::voc::labels[det.cls[0]].c_str());
				fprintf(fp, data::coco::result_format, file_id, data::coco::id_remap[c],
					sx, sy, bw, bh, det.confidence);
			}
		}
		fclose(fp);
#else 
		int j=0;
		for (auto det : detections) {
			if (det.confidence != 0.0f) {
				for (int c = 0; c < 80; ++c) {
					if (det.prob[c] > 0) {
					float sx = clamp(det.box.sx, 0.0f, (float)w);
					float sy = clamp(det.box.sy, 0.0f, (float)h);
					float bw = clamp(det.box.ex-det.box.sx+1, 0.0f, (float)w);
					float bh = clamp(det.box.ey-det.box.sy+1, 0.0f, (float)h);
					printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.2f label:%s\n",
					++j, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.prob[c],
					yolov4.dataset_ == data::COCO ?	data::coco::labels[c].c_str() : data::voc::labels[c].c_str());
					fprintf(fp, data::coco::result_format, file_id, data::coco::id_remap[c],
						sx, sy, bw, bh, det.prob[c]);
					}
				}
			}
		}
#endif 
		//fflush(fp);
	}
	fr.close();
	//fseek(fp, -2, SEEK_CUR);
	//fprintf(fp, "\n]\n");

#endif
	return 0;
}

int eaif::EngineDemo::RunYolov4Eval(const char* model_path, const char* image_list, const char* dataset)
{

#if defined(USE_TFLITE) || defined(USE_ARMNN)
	WImage img;

	unique_ptr<DetectModel> yolov4 = nullptr;
	if (m_engine) {
#ifdef USE_TFLITE
		LiteYolov4Model *lyolov4 = new LiteYolov4Model();
		yolov4 = unique_ptr<DetectModel>(lyolov4);
		yolov4->SetNumThreads(m_nthread);
#endif
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnYolov4Model *ayolov4 = new ArmnnYolov4Model();
		ayolov4->SetDebug((m_debug != 3));
		ayolov4->SetCpuType(m_cpu_infer);
		yolov4 = unique_ptr<DetectModel>(ayolov4);
		this->SetNumThreads();
#endif
	} else
		assert(0);
	assert(yolov4 != nullptr);

	yolov4->SetVerbose(m_verbose);

	vector<string> model_paths;
	model_paths.push_back(model_path);
	yolov4->LoadModels("", model_paths);

	if (yolov4->m_type == Eaif8U) {
		eaif_warn("Not implemented yet!");
		return -1;
	}

	//eaif::image::ImRead(img_file, img);
	ifstream fr;
	int file_id;
	fr.open(image_list);
	if (!fr.is_open()) {
		eaif_warn("Cannot open file %s\n", image_list);
		return -1;
	}
	dataset::CreateDir(DETECTION_DIR);

	ModelConfig conf;
	SetupConfig(conf, dataset);

	yolov4->SetPostProcessMethod(21);

	int i =0;
	int iter = 0;
	while (!fr.eof()) {
		string img_file;
		getline(fr, img_file);
		if (access(img_file.c_str(), F_OK) == -1) {
			eaif_warn("Cannot find %s, Skip!\n", img_file.c_str());
			continue;
		}
		if (iter++ < m_iter-1)
			continue;
		WImage img;
		cout << "\""<< img_file << "\"\n";
		eaif::image::Imread(img_file.c_str(), img);
		if (img.channels() == 1)
			eaif::image::ImbroadcastChannel(img);

		vector<Detection> detections;
		file_id = GetfileId(img_file);
		TIMER(yolov4->Detect(&img, detections, conf));
		printf("total detections:%d for %012d(%d)\n", (int)detections.size(), file_id, i++);
		int h = img.rows;
		int w = img.cols;
#if 1
		char detection_result[256] = {};
		sprintf(detection_result, "detection_result/%012d.json", file_id);
		FILE *fp = fopen(detection_result, "w");
		assert(fp!=NULL);
		for (auto det : detections) {
			if (det.confidence != 0.0f) {
				float sx = Clamp(det.box.sx, 0.0f, (float)w);
				float sy = Clamp(det.box.sy, 0.0f, (float)h);
				float bw = Clamp(det.box.ex-det.box.sx+1, 0.0f, (float)w);
				float bh = Clamp(det.box.ey-det.box.sy+1, 0.0f, (float)h);
				int c = det.cls[0];
				//printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.4f label:%s\n",
				//++j, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence,
				//yolov4.dataset_ == data::COCO ?	data::coco::labels[det.cls[0]].c_str() : data::voc::labels[det.cls[0]].c_str());
				fprintf(fp, data::coco::result_format, file_id, data::coco::id_remap[c],
					sx, sy, bw, bh, det.confidence);
			}
		}
		fclose(fp);
#else
		int j = 0;
		for (auto det : detections) {
			if (det.confidence != 0.0f) {
				for (int c = 0; c < 80; ++c) {
					if (det.prob[c] > 0) {
					float sx = clamp(det.box.sx, 0.0f, (float)w);
					float sy = clamp(det.box.sy, 0.0f, (float)h);
					float bw = clamp(det.box.ex-det.box.sx+1, 0.0f, (float)w);
					float bh = clamp(det.box.ey-det.box.sy+1, 0.0f, (float)h);
					printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.2f label:%s\n",
					++j, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.prob[c],
					yolov4.dataset_ == data::COCO ?	data::coco::labels[c].c_str() : data::voc::labels[c].c_str());
					fprintf(fp, data::coco::result_format, file_id, data::coco::id_remap[c],
						sx, sy, bw, bh, det.prob[c]);
					}
				}
			}
		}
#endif 
	}
	fr.close();
	//fseek(fp, -2, SEEK_CUR);
	//fprintf(fp, "\n]\n");

#endif
	return 0;
}

int eaif::EngineDemo::RunMtcnn(const char* img_file)
{

#if defined(USE_TFLITE) || defined(USE_ARMNN)

	unique_ptr<Mtcnn> mtcnn = nullptr;
	Mtcnn *mtcnn_ptr = nullptr;

	if (m_engine == (int)eaif::engine::TfLite) {
#ifdef USE_TFLITE
		LiteMtcnn *lmtcnn = new LiteMtcnn();
		mtcnn_ptr = dynamic_cast<Mtcnn*>(lmtcnn);
		mtcnn_ptr->SetNumThreads(m_nthread);
#endif /* !USE_TFLITE */
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnMtcnn *amtcnn = new ArmnnMtcnn();
		amtcnn->SetDebug((m_debug != 3));
		amtcnn->SetCpuType(m_cpu_infer);
		mtcnn_ptr = dynamic_cast<Mtcnn*>(amtcnn);
		this->SetNumThreads();
#endif /* !USE_ARMNN */ 
	} else
		assert(0);

	assert(mtcnn_ptr);
	mtcnn = unique_ptr<Mtcnn>(mtcnn_ptr);
	mtcnn->SetVerbose(m_verbose);

	WImage img;
	eaif::image::Imread(img_file, img);

	ModelConfig conf;
	SetupConfig(conf, nullptr, &m_model_path_vec);
	mtcnn->LoadModels(".", m_model_path_vec);

	if (mtcnn->GetModelType() == Eaif8U) {
		vector<FaceBox<int>> face_list;
		TIMER(mtcnn->FaceDetect(&img, face_list, conf));
		printf("total face:%d\n", (int)face_list.size());
		for (auto face : face_list) {
			printf("face pos:[%d %d %d %d] score:%03d landmark[%d %d %d %d %d %d %d %d %d %d]\n",
				face.x0, face.y0, face.x1, face.y1, face.score,
				face.landmark.x[0], face.landmark.x[1], face.landmark.x[2], face.landmark.x[3], face.landmark.x[4],
				face.landmark.y[0], face.landmark.y[1], face.landmark.y[2], face.landmark.y[3], face.landmark.y[4]);
		}
	} else if (mtcnn->GetModelType() == Eaif32F) {
		vector<FaceBox<float>> face_list;
		TIMER(mtcnn->FaceDetect(&img, face_list, conf));
		printf("total face:%d\n", (int)face_list.size());
		for (auto face : face_list) {
			printf("face pos:[%.2f %.2f %.2f %.2f] score:%.2f landmark[%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f]\n",
				face.x0, face.y0, face.x1, face.y1, face.score,
				face.landmark.x[0], face.landmark.x[1], face.landmark.x[2], face.landmark.x[3], face.landmark.x[4],
				face.landmark.y[0], face.landmark.y[1], face.landmark.y[2], face.landmark.y[3], face.landmark.y[4]);
		}
	} else {
		assert(0);
	}

#endif
	return 0;
}

int eaif::EngineDemo::RunMtcnnWIDER(const char* data_dir, const char *dst_dir)
{
#if defined(USE_TFLITE) || defined(USE_ARMNN)

	unique_ptr<Mtcnn> mtcnn = nullptr;
	Mtcnn *mtcnn_ptr = nullptr;

	if (m_engine == (int)eaif::engine::TfLite) {
#ifdef USE_TFLITE
		LiteMtcnn *lmtcnn = new LiteMtcnn();
		mtcnn_ptr = dynamic_cast<Mtcnn*>(lmtcnn);
		mtcnn_ptr->SetNumThreads(m_nthread);
#endif /* !USE_TFLITE */
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnMtcnn *amtcnn = new ArmnnMtcnn();
		amtcnn->SetDebug((m_debug != 3));
		amtcnn->SetCpuType(m_cpu_infer);
		mtcnn_ptr = dynamic_cast<Mtcnn*>(amtcnn);
		this->SetNumThreads();
#endif /* !USE_ARMNN */
	} else
		assert(0);

	assert(mtcnn_ptr);
	mtcnn = unique_ptr<Mtcnn>(mtcnn_ptr);

	string data_dir_in(data_dir);
	string dst_dir_in(dst_dir);
	vector<string> img_list;
	vector<string> pred_list;

	dataset::Wider data(data_dir_in, dst_dir_in);
	data.GetValList(img_list, pred_list);
	assert(img_list.size() == pred_list.size());

	ModelConfig conf;
	SetupConfig(conf, nullptr, &m_model_path_vec);
	mtcnn->LoadModels(".", m_model_path_vec);

	int start = 0;
	if (m_iter > 10000)
		start = m_iter - 10000;

	for (size_t i = start; i < img_list.size(); ++i) {
		const char* img_file_raw = data.img_list[i].c_str();
		const char* img_file = img_list[i].c_str();
		char pred_file[256] = { 0 };
		strcpy(pred_file, pred_list[i].c_str());
		const char* dirname_ = dirname(pred_file);
		dataset::CreateDir(dirname_);
		WImage img = Imread<WImage>((const char*)img_file);
		eaif_trc("Detection Progress %05d/%05d :%s(h:%d,w:%d) result:%s\n", (int)i, (int)img_list.size(), img_file, img.cols, img.rows, pred_list[i].c_str());
		FILE *fp = fopen(pred_list[i].c_str(), "w");
		fprintf(fp, "%s\n", img_file_raw);
		try {
			if (mtcnn->GetModelType() == Eaif8U) {
				vector<FaceBox<int>> face_list;
				mtcnn->FaceDetect(&img, face_list, conf);
				if (face_list.size() > 0) {
					fprintf(fp, "%d\n", (int)face_list.size());
					for (auto face : face_list) {
						fprintf(fp, "%.4f %.4f %.4f %.4f %.4f\n",
							(float)face.x0, (float)face.y0, (float)face.x1 - face.x0 + 1, (float)face.y1 - face.y0 + 1,
							(float)face.score/255.0f);
					}
				} else {
					fprintf(fp, "1\n");
					fprintf(fp, "%.4f %.4f %.4f %.4f %.4f\n", 0.0, 0.0, 0.0, 0.0, 0.99);
				}
			} else if (mtcnn->GetModelType() == Eaif32F) {
				vector<FaceBox<float>> face_list;
				mtcnn->FaceDetect(&img, face_list, conf);
				if (face_list.size() > 0) {
					fprintf(fp, "%d\n", (int)face_list.size());
					fprintf(stdout, "%d\n", (int)face_list.size());
					for (auto face : face_list) {
						fprintf(fp, "%.4f %.4f %.4f %.4f %.4f\n",
							face.x0, face.y0, face.x1 - face.x0 + 1, face.y1 - face.y0 + 1, face.score);
						fprintf(stdout, "%.4f %.4f %.4f %.4f %.4f\n",
							face.x0, face.y0, face.x1 - face.x0 + 1, face.y1 - face.y0 + 1, face.score);
					}
				} else {
					fprintf(fp, "1\n");
					fprintf(fp, "%.4f %.4f %.4f %.4f %.4f\n", 0.0, 0.0, 0.0, 0.0, 0.99);
				}
			}
		} catch (std::bad_alloc &e) {
			cout << "catch exception! " << e.what() << "\n";
			char msg[512] = {};
			sprintf(msg, "echo image index:%d name:%s >> widerfail.log\n", i, img_file);
			system(msg);
		}
		fclose(fp);
	}
#endif
	return 0;
}

int eaif::EngineDemo::RunMtcnnFDDB(const char* data_dir, const char *dst_dir)
{

#if defined(USE_TFLITE) || defined(USE_ARMNN)

	unique_ptr<Mtcnn> mtcnn = nullptr;
	Mtcnn *mtcnn_ptr = nullptr;

	if (m_engine == (int)eaif::engine::TfLite) {
#ifdef USE_TFLITE
		LiteMtcnn *lmtcnn = new LiteMtcnn();
		mtcnn_ptr = dynamic_cast<Mtcnn*>(lmtcnn);
		mtcnn_ptr->SetNumThreads(m_nthread);
#endif /* !USE_TFLITE */
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnMtcnn *amtcnn = new ArmnnMtcnn();
		amtcnn->SetDebug((m_debug != 3));
		amtcnn->SetCpuType(m_cpu_infer);
		mtcnn_ptr = dynamic_cast<Mtcnn*>(amtcnn);
		this->SetNumThreads();
#endif /* !USE_ARMNN */
	} else
		assert(0);

	assert(mtcnn_ptr);
	mtcnn = unique_ptr<Mtcnn>(mtcnn_ptr);

	WImage img;

	ModelConfig conf;
	SetupConfig(conf, nullptr, &m_model_path_vec);

	string data_dir_in(data_dir);
	string dst_dir_in(dst_dir);
	dataset::String2d fold_list;
	vector<string> pred_list;

	dataset::Fddb data(data_dir_in, dst_dir_in);
	data.GetValList(fold_list, pred_list);
	assert(fold_list.size() == pred_list.size());

	mtcnn->LoadModels(".", m_model_path_vec);

	int img_cnt = 0;
	for (size_t i = 0; i < fold_list.size(); ++i) {

		char pred_file[256] = {};
		strcpy(pred_file, pred_list[i].c_str());
		const char* dirname_ = dirname(pred_file);
		dataset::CreateDir(dirname_);
		FILE* fp = fopen(pred_list[i].c_str(), "w");

		for (size_t j = 0; j < fold_list[i].size(); ++j) {
			string& img_file = fold_list[i][j];
			const char* img_file_raw = data.img_list[i][j].c_str();
			fprintf(fp, "%s\n", img_file_raw);
			WImage img = eaif::image::Imread<WImage>(img_file.c_str());
			fprintf(stdout, "Detection Progress Id:%05d(%05d/%05d)Fold:%d/%d:(h:%d, w:%d)%s %20s\n", img_cnt++, (int)j, (int)fold_list[i].size()-1, (int)i, (int)fold_list.size()-1, img.rows, img.cols, img_file.c_str(), " ");
			if (mtcnn->GetModelType() == Eaif8U) {
				vector<FaceBox<int> > face_list;
				mtcnn->FaceDetect(&img, face_list, conf);
				if (face_list.size() > 0) {
					fprintf(fp, "%d\n", (int)face_list.size());
					for (auto face : face_list) {
						fprintf(stdout, "%.4f %.4f %.4f %.4f %.4f\n",
							(float)face.x0, (float)face.y0, (float)face.x1 - face.x0 + 1, (float)face.y1 - face.y0 + 1,
							(float)face.score/255.0f);
						fprintf(fp, "%.4f %.4f %.4f %.4f %.4f\n",
							(float)face.x0, (float)face.y0, (float)face.x1 - face.x0 + 1, (float)face.y1 - face.y0 + 1,
							(float)face.score/255.0f);
					}
				} else {
					fprintf(fp, "1\n");
					fprintf(fp, "0.0 0.0 0.0 0.0 0.99\n");
				}
			} else if (mtcnn->GetModelType() == Eaif32F) {
				vector<FaceBox<float>> face_list;
				mtcnn->FaceDetect(&img, face_list, conf);
				if (face_list.size() > 0) {
					fprintf(fp, "%d\n", (int)face_list.size());
					fprintf(stdout, "%d\n", (int)face_list.size());
					for (auto face : face_list) {
						fprintf(fp, "%.4f %.4f %.4f %.4f %.4f\n",
							face.x0, face.y0, face.x1 - face.x0 + 1, face.y1 - face.y0 + 1, face.score);
						fprintf(stdout, "%.4f %.4f %.4f %.4f %.4f\n",
							face.x0, face.y0, face.x1 - face.x0 + 1, face.y1 - face.y0 + 1, face.score);
					}
				} else {
					fprintf(fp, "1\n");
					fprintf(fp, "0.0 0.0 0.0 0.0 0.99\n");
				}
			} else {
				eaif_err("Data type not support yet!\n");
				assert(0);
			}
		}
		fclose(fp);
	}
#endif
	return 0;
}


#include <sys/stat.h>
int eaif::EngineDemo::RunFacenetModel(const char* img_file, const char* dst_dir_in)
{
#if defined(USE_TFLITE) || defined(USE_ARMNN)


	unique_ptr<FacenetModel> facenet = nullptr;
	FacenetModel *facenet_ptr = nullptr;

	if (m_engine == (int)eaif::engine::TfLite) {
#ifdef USE_TFLITE
		LiteFacenetModel *lfacenet = new LiteFacenetModel();
		facenet_ptr = dynamic_cast<FacenetModel*>(lfacenet);
		facenet_ptr->SetNumThreads(m_nthread);
#endif /* !USE_TFLITE */
	} else if (m_engine==0) {
#ifdef USE_ARMNN
		ArmnnFacenetModel *afacenet = new ArmnnFacenetModel();
		afacenet->SetDebug((m_debug != 3));
		afacenet->SetCpuType(m_cpu_infer);
		facenet_ptr = dynamic_cast<FacenetModel*>(afacenet);
		this->SetNumThreads();
#endif /* !USE_ARMNN */
	} else
		assert(0);

	const string model_path(m_model_path);

	assert(facenet_ptr);
	facenet = unique_ptr<FacenetModel>(facenet_ptr);

	vector<string> model_paths;
	model_paths.push_back(model_path);

	ModelConfig conf;
	SetupConfig(conf, nullptr);

	TIMER_FUNC("Load Model ", facenet->LoadModels("", model_paths));

	if (strcmp(img_file + strlen(img_file) - 3, "txt") == 0) {
		if (mkdir(dst_dir_in, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
			if( errno != EEXIST )
				cerr << "Cannot open dir " << dst_dir_in << endl;
		}
		FILE *fim = fopen(img_file, "r");
		char data_path[256];
		int cnt = fscanf(fim, "%s\n", data_path);
		while (!feof(fim)) {
			WImage img;
			char name[256];
			int img_no;

			cnt = fscanf(fim, "%s %d\n", name, &img_no);
			if (cnt != 2) break;
			char im_name[256];
			struct stat buffer;

			sprintf(im_name, "%s/%s/%s_%04d.png", data_path, name, name, img_no);
			cout << im_name << endl;

			eaif::image::Imread(im_name, img);
			Detection det = {{0.0f, 0.0f, (float)(img.cols - 1), (float)(img.rows - 1)}};

			for (int flip = 0; flip < 2; flip+=2) {
				char bin_file[256];
				if (flip) sprintf(bin_file, "%s/%s_%04d.png.flip.embedding", dst_dir_in, name, img_no);
				else      sprintf(bin_file, "%s/%s_%04d.png.embedding", dst_dir_in, name, img_no);

				if (stat(bin_file, &buffer) == 0) {
					cout << "Skip existed embedding: " << bin_file << endl;
					continue;
				}

				for (int i = 0; i < m_iter; ++i) {
					if (facenet->GetModelType() == Eaif8U) {
						vector<uint8_t> face_encode(facenet->encode_dim_);
						TIMER_FUNC("Inference", facenet->EncodeFace((const void *)&img, det, face_encode, conf));
						#if 0
						FILE* fp = fopen("encode.bin", "wb");
						int size = (facenet.type_ == Eaif8U) ? 1 : 4;
						fwrite(&FaceEncode[0], size, facenet.encode_dim_, fp);
						fclose(fp);
						#endif
					} else if (facenet->GetModelType() == Eaif32F) {
						vector<float> face_encode(facenet->encode_dim_);
						TIMER_FUNC("Inference", facenet->EncodeFace((const void *)&img, det, face_encode, conf));
					} else {
						eaif_err("Data type not implemented yet!\n");
						assert(0);
					}
				}
				if (m_engine == (int)eaif::engine::TfLite) {
#ifdef USE_TFLITE
					PrintOutput(((LiteFacenetModel*)facenet.get())->model_, m_range, im_name, dst_dir_in, 0);
#endif /* !USE_TFLITE */
				} else {
#ifdef USE_ARMNN
					PrintOutput(((ArmnnFacenetModel*)facenet.get())->model_, m_range, im_name, dst_dir_in, 0);
#endif /* !USE_ARMNN */
				}
			}
		}
		fclose(fim);
	} else {

		WImage img;

		eaif::image::Imread(img_file, img);
		Detection det = {{0.0f, 0.0f, (float)(img.cols-1), (float)(img.rows-1)}};

		for (int i = 0; i < m_iter; ++i) {
			if (facenet->GetModelType() == Eaif8U) {
				vector<uint8_t> face_encode(facenet->encode_dim_);
				TIMER_FUNC("Inference", facenet->EncodeFace((const void *)&img, det, face_encode, conf));
				#if 0
				FILE* fp = fopen("encode.bin", "wb");
				int size = (facenet.type_ == Eaif8U) ? 1 : 4;
				fwrite(&FaceEncode[0], size, facenet.encode_dim_, fp);
				fclose(fp);
				#endif
			} else {
				vector<float> face_encode(facenet->encode_dim_);
				TIMER_FUNC("Inference", facenet->EncodeFace((const void *)&img, det, face_encode, conf));
			}
		}
	}
	if (m_engine == (int)eaif::engine::TfLite) {
#ifdef USE_TFLITE
		PrintOutput(((LiteFacenetModel*)facenet.get())->model_, m_range);
#endif /* !USE_TFLITE */
	} else {
#ifdef USE_ARMNN
		PrintOutput(((ArmnnFacenetModel*)facenet.get())->model_, m_range);
#endif /* !USE_ARMNN */
	}

#endif /* defined(USE_TFLITE) || defined(USE_ARMNN) */
	return 0;
}

int eaif::EngineDemo::RunC4(const char* img_file, const char *model_dir_)
{

#ifdef USE_C4
	C4ClassifyModel c4;
	const char* common_pos = strchr(model_dir_, ',');
	//c4.LoadModelss("", "combined.txt.model,combined2.txt.model");
	vector<string> model_path;
	model_path.push_back(string(model_dir_, common_pos - model_dir_));;
	model_path.push_back(string(common_pos + 1));
	c4.LoadModels("", model_path);
#else // USE_C4
	eaif_warn("C4 module is disabled!\n");
#endif // USE_C4
	return 0;
}
