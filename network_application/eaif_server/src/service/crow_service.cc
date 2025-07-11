#ifdef USE_CROW

#include <mutex>
#include <vector>

#include <stdio.h>

#include "crow.h"

#include "eaif_image.h"
#include "eaif_model.h"
#include "eaif_engine.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include "crow_service.h"

using namespace std;
using namespace eaif::image;

class CrowApp
{
	public:
	crow::SimpleApp app;
	int capture = 0;
	int index = 0;
	string capture_path = "/mnt/sdcard/eaif_capture";
	string img_name = "capture.jpg";
};

template RequestMessage::RequestMessage(const crow::multipart::message&);
template RequestMessage::RequestMessage(const crow::multipart::message_view&);

inline int IsValidContentType(const crow::request& req, const string& type)
{
	return (req.headers.find("Content-Type")->second.find(type)==0);
}

static void CaptureClassifyResult(CrowApp *service, const ModelInfo &info,
	const WImage &img, const vector<ObjectAttr> &obj_list,
	const vector<Classification> &classifications)
{
	if (obj_list.size()!=classifications.size()) {
		eaif_warn("Input obj size vs classification size is different!\n");
		return;
	}

	for (size_t i = 0; i < obj_list.size(); i++) {
		char img_path[512] = {};
		auto& obj = obj_list[i];
		auto& result = classifications[i];
		string cls_name;
		if (result.cls.size() == 0) {
			cls_name = "negative";
		} else {
			cls_name = info.labels->at(result.cls[0]);
		}
		auto crop = Imcrop(img, obj.box.sx, obj.box.sy, obj.box.ex, obj.box.ey);

		int size = sprintf(img_path, "%s/%s", service->capture_path.c_str(), cls_name.c_str());
		mkdir(img_path, 0777);
		size += sprintf(&img_path[size], "/capture%05d.jpg", service->index++);
		Imwrite(img_path, crop);
	}
}

CrowService::CrowService()
{
	m_service = unique_ptr<CrowApp>(new CrowApp);
}

CrowService::~CrowService() { ExitMpi(); };

int CrowService::RegisterApp(string& config)
{
	int ret = m_engine.Setup(config);
	if (ret != EAIF_SUCCESS)
		return ret;

	crow::mustache::set_base(".");

	auto& crow_app = m_service->app;

	CROW_ROUTE(crow_app, "/")
	.methods("GET"_method)
	([](const crow::request& req){
		CROW_LOG_INFO << "ping!";
		return crow::response{"{\"success\":1}\n"};
	});

	CROW_ROUTE(crow_app, "/home")
	([&]{
		string index_path = m_engine.GetTemplatePath() + "/index.html";
		return crow::mustache::load(index_path).render();
	});

	CROW_ROUTE(crow_app, "/static/<string>")
	([&](const string& path){
		string templates_path = m_engine.GetTemplatePath() + "/" + path;
		cout << "info " << templates_path  << "\n";
		return crow::mustache::load(templates_path).render();
	});


	CROW_ROUTE(crow_app, "/avail")
	.methods("GET"_method)
	([&](const crow::request& req){
		char valid_models[1024];
		int size = 0;
		vector<string> classifier_names;
		vector<string> detector_names;
		vector<string> facereco_names;

		m_engine.QueryModelNames(eaif::engine::Classify, classifier_names);
		m_engine.QueryModelNames(eaif::engine::Detect, detector_names);
		m_engine.QueryModelNames(eaif::engine::FaceReco, facereco_names);

		size += sprintf(valid_models, "{\"success\":1, \"classifiers\":[");
		for (auto& model_str : classifier_names) {
			size += sprintf(&valid_models[size], "\"%s\",", model_str.c_str());
		}
		if (classifier_names.size()) size--;

		size += sprintf(&valid_models[size], "],\"detectors\":[");
		for (auto& model_str : detector_names) {
			size += sprintf(&valid_models[size], "\"%s\",", model_str.c_str());
		};
		if (detector_names.size()) size--;

		size += sprintf(&valid_models[size], "],\"facereco\":[");
		for (auto& model_str : facereco_names) {
			size += sprintf(&valid_models[size], "\"%s\",", model_str.c_str());
		};
		if (facereco_names.size()) size--;

		size += sprintf(&valid_models[size], "]}\n");

		return crow::response{string(valid_models)};
	});

	CROW_ROUTE(crow_app, "/model_states")
	.methods("GET"_method)
	([&](const crow::request& req){
		char model_states_str[1024] = {};
		int size = 0;

		vector<StatePair> states;
		m_engine.GetModelStates(states);

		size += sprintf(model_states_str + size, "{\"success\":1,\"model_states\":[");
		for (auto& stat : states) {
			size += sprintf(model_states_str + size,
				"{\"%s\":{\"infer_cnt\":%d}},",
				stat.first.c_str(), stat.second.infer_count);
		}
		if (states.size()) size--;
		size += sprintf(model_states_str + size, "]}\n");

		return crow::response{string(model_states_str)};
	});

	CROW_ROUTE(crow_app, "/clear_model_states")
	.methods("GET"_method)
	([&](const crow::request& req){
		lock_guard<std::mutex> guard(lock);
		m_engine.ClearModelStates();
		auto str = EasyStrCompRet("success", "clean up model states");
		return crow::response(str);
	});

	CROW_ROUTE(crow_app, "/clear")
		.methods("GET"_method)
	([&](const crow::request& req){
		lock_guard<std::mutex> guard(lock);
		Clear();
		auto str = EasyStrCompRet("success", "clean up model");
		return crow::response(str);
	});

	CROW_ROUTE(crow_app, "/update")
		.methods("GET"_method)
	([&](const crow::request& req){
		lock_guard<std::mutex> guard(lock);
		m_engine.Update();
		auto str = EasyStrCompRet("success", "updated model!");
		return crow::response(str);
	});

	CROW_ROUTE(crow_app, "/reload")
		.methods("GET"_method)
	([&](const crow::request& req){
		lock_guard<std::mutex> guard(lock);
		Clear();
		m_engine.Update();
		auto str = EasyStrCompRet("success", "reloaded models!");
		return crow::response(str);
	});


	CROW_ROUTE(crow_app, "/capture")
		.methods("GET"_method)
	([&](const crow::request& req){
		lock_guard<std::mutex> guard(lock);
		string str;
		char *name = req.url_params.get("name");
		char *type = req.url_params.get("type");
		char *path = req.url_params.get("path");

		if (name) m_service->img_name = string(name);
		
		if (type)
			m_service->capture = 1; // snapshot for once
		else {
			m_service->capture = 2; // keep recording for classification
			if (path) {
				m_service->capture_path = string(path);
			}
			mkdir(m_service->capture_path.c_str(), 0777);
			eaif_info_h("Start capture to %s\n", m_service->capture_path.c_str());
		}

		str = EasyStrCompRet("success", "Start capturing!");

		return crow::response(str);
	});

	CROW_ROUTE(crow_app, "/predict/<string>")
		.methods("POST"_method)
	([&](const crow::request& req, const std::string& model_str){

		if (!IsValidContentType(req, "multipart/form-data")) {
			auto str = EasyStrCompRet("fail", "Invalid content-type!");
			return crow::response(str);
		}

		lock_guard<std::mutex> guard(lock);

		char *c_iter = req.url_params.get("t");
		int iter = (c_iter) ? atoi(c_iter) : 1;

		//crow::multipart::message msg(req);
		crow::multipart::message_view msg_v(req);

		ModelInfo info;
		int ret = m_engine.QueryModelInfo(model_str, info);

		if (ret == EAIF_FAILURE) {
			auto str = EasyStrCompRet("fail", "Cannot find requested model");
			return crow::response(str);
		}
		
		//RequestMessage req_msg(msg);
		RequestMessage req_msg_v(msg_v);
		uint32_t mtime = *req_msg_v.m_time.load;

		WImage img;

		COND_TIMER_FUNC(
			m_engine.GetVerbose(), "Loading image",
			ret = EasyLoadImage(req_msg_v, img, info.channels)
			);

		if (ret == EAIF_FAILURE) {
			ReleaseMpiFrameIfAny();
			return crow::response(buf);
		}

		if (m_service->capture == 1) {
			m_service->capture = 0;
			eaif_info_h("Writing to %s\n", m_service->img_name.c_str());
			Imwrite(m_service->img_name.c_str(), img);
		}

		switch (info.inference_type) {

			case eaif::engine::FaceReco: {
			} case eaif::engine::Detect: {

				vector<Detection> detections;

				for (int i = 0; i < iter; ++i) {
					detections.clear();
					m_engine.Detect(info.index, (const void*) &img, detections);
				}

				EasyCompDecode(info, mtime, detections);

				break;
			} case eaif::engine::Classify: {

				vector<Classification> classifications;
				vector<ObjectAttr> obj_list;

				int ret = DecodeJsonStr2Object(
					req_msg_v.m_meta.load,
					req_msg_v.m_meta.size,
					obj_list);

				if (!req_msg_v.m_meta.size && ret) {
					EasyCompRet("fail", "Invalid json format!");
					break;
				}

				if (obj_list.size() == 0) {

					eaif_info_h("run classify\n");

					classifications.resize(1);

					for (int i = 0; i < iter; i++)
						m_engine.Classify(info.index, (const void*) &img, classifications[0]);

				} else {

					eaif_info_h("run classify obj list\n");

					eaif_info_h("%s\n", string(req_msg_v.m_meta.load, req_msg_v.m_meta.size).c_str());

					for (int i = 0; i < iter; i++) {

						classifications.clear();

						m_engine.ClassifyObjList(info.index, (const void*) &img,
								obj_list,
								classifications);
					}
				}

				EasyCompDecode(info, mtime, classifications);

				if (m_service->capture == 2) {
					CaptureClassifyResult(m_service.get(), info, img, obj_list, classifications);
				}

				break;

			} default: {

				EasyCompRet("fail", "Unknown error caught!");

				break;
			}
		}

		if (m_engine.GetVerbose()) cerr << string(buf) << "\n";

		ReleaseMpiFrameIfAny();
		return crow::response(buf);
	});

	CROW_ROUTE(crow_app, "/facereco/<string>") // for face utility function
		.methods("GET"_method)
	([&](const crow::request& req, const string& utility_str) {

		lock_guard<std::mutex> guard(lock);		

		string ret_str;
		ModelInfo info;
		int ret = m_engine.QueryModelInfo("facereco", info);

		if (ret == EAIF_FAILURE) {
			ret_str = EasyStrCompRet("fail", "Cannot find facereco model");
			return crow::response(ret_str);
		}

		if (!strncmp(utility_str.c_str(), "save", 4)) {

			ret = m_engine.SaveFaceData(info.index);
			if (ret != EAIF_SUCCESS)
				ret_str = EasyStrCompRet("fail", "Cannot save Face Data!");
			else
				ret_str = EasyStrCompRet("success", "Saved Face Data!");

		} else if (!strncmp(utility_str.c_str(), "load", 4)) {

			ret = m_engine.LoadFaceData(info.index);
			if (ret != EAIF_SUCCESS)
				ret_str = EasyStrCompRet("fail", "Cannot load Face Data!");
			else
				ret_str = EasyStrCompRet("success", "Loaded Face Data!");

		} else if (!strncmp(utility_str.c_str(), "query", 4)) {

			vector<string> face_list;
			ret = m_engine.QueryFaceInfo(info.index, face_list);
			if (ret != EAIF_SUCCESS)
				ret_str = EasyStrCompRet("fail", "Cannot load face info!");
			else {
				EasyCompStrVec("success", "Faces", face_list, buf);
				return crow::response(buf);
			}

		} else
			ret_str = EasyStrCompRet("fail", "Unknown request method for facereco");

		return crow::response(ret_str);
	});

	CROW_ROUTE(crow_app, "/facereco/<string>") // for testing
		.methods("POST"_method)
	([&](const crow::request& req, const string &req_str){

		if (!IsValidContentType(req, "multipart/form-data")) {
			auto str = EasyStrCompRet("fail", "Invalid content-type!");
			return crow::response(str);
		}

		lock_guard<std::mutex> guard(lock);

		crow::multipart::message_view msg_v(req);

		ModelInfo info;
		int ret = m_engine.QueryModelInfo("facereco", info);

		if (ret == EAIF_FAILURE) {
			auto str = EasyStrCompRet("fail", "Cannot find requested model");
			return crow::response(str);
		}
		
		//RequestMessage req_msg(msg);
		string ret_str;
		RequestMessage req_msg_v(msg_v);

		WImage img;

		COND_TIMER_FUNC(
			m_engine.GetVerbose(), // cond
			"Load Image", // str
			ret = EasyLoadImage(req_msg_v, img, info.channels) // exec
			);
		if (ret == EAIF_FAILURE) {
			ReleaseMpiFrameIfAny();
			return crow::response(buf);
		}

		if (!strncmp(req_str.c_str(), "register", 8)) {
			char *face_char = req.url_params.get("face");
			char *full_face_img_char = req.url_params.get("full");

			string face = (face_char) ? string(face_char) : "unkown";
			int is_full_image = (full_face_img_char) ? 1 : 0;

			ret = m_engine.RegisterFace(info.index, (const void*) &img, face, is_full_image);

			if (ret == EAIF_FAILURE)
				ret_str = EasyStrCompRet("fail", "Cannot Register Face for " + face + " !");
			else 
				ret_str = EasyStrCompRet("success", "Registered Face for " + face + " !");
		} else
			ret_str = EasyStrCompRet("fail", "Method not supported!");

		ReleaseMpiFrameIfAny();
		return crow::response(ret_str);


	});

	CROW_ROUTE(crow_app, "/empty_view/<string>") // for testing
		.methods("POST"_method)
	([&](const crow::request& req, const std::string& model_str){

		if (!IsValidContentType(req, "multipart/form-data")) {
			auto str = EasyStrCompRet("fail", "Invalid content-type!");
			return crow::response(str);
		}

		crow::multipart::message_view msg_v(req);

		RequestMessage req_msg_v(msg_v);

		if (model_str.compare("yolov5") == 0) {

			int size = snprintf(buf, SERVICE_RET_SIZE,"{\"success\":1,\"time\":%u,"
				"\"predictions\":[{\"idx\":1,\"labels\":\"human\",\"prob\":0.9123}],"
				"\"format\":\"", *req_msg_v.m_time.load);
			size += SprintfView(&buf[size], req_msg_v.m_format);
			size += snprintf(&buf[size], SERVICE_RET_SIZE-size, "\",\"shape\":[%d,%d,%d],\"meta\":",
				req_msg_v.m_shape.load[0], req_msg_v.m_shape.load[1], req_msg_v.m_shape.load[2]);

			size += SprintfView(&buf[size], req_msg_v.m_meta);
			size += snprintf(&buf[size], SERVICE_RET_SIZE-size, "}\n");
		} else {
			EasyCompRet("fail", "Unknown error caught!");
		}
		return crow::response(buf);
	});

	CROW_ROUTE(crow_app, "/empty/<string>") // for testing
		.methods("POST"_method)
	([&](const crow::request& req, const std::string& model_str){

		if (!IsValidContentType(req, "multipart/form-data")) {
			EasyCompRet("fail", "Invalid content-type!");
			return crow::response(buf);
		}

		//crow::multipart::message msg(req);
		crow::multipart::message msg(req);

		//RequestMessage req_msg(msg);
		RequestMessage req_msg(msg);

		if (model_str.compare("yolov5") == 0) {

			snprintf(buf,
			SERVICE_RET_SIZE,
			"{\"success\":1,"
			"\"time\":%u,"
			"\"predictions\":[{\"idx\":1,\"labels\":[\"%s\"],\"prob\":[%.4f],\"label_num\":1}],"
			"\"format\":\"%s\",\"shape\":[%d,%d,%d],\"meta\":%s}\n",
			*req_msg.m_time.load, "human", 0.998f,
			req_msg.m_format.load,
			req_msg.m_shape.load[0], req_msg.m_shape.load[1], req_msg.m_shape.load[2],
			req_msg.m_meta.load
			);

		} else {
			EasyCompRet("fail", "Unknown error caught");
		}
		return crow::response(buf);
	});
	return EAIF_SUCCESS;
}

void CrowService::Run(string& bindaddr, int port)
{
	m_service->app.bindaddr(bindaddr.c_str())
	     .port(port)
	     .concurrency(concurrent_)
	     .run();
	return;
}

#endif // USE_CROW