#include "eaif_common.h"
#include "eaif_data.h"
#include "eaif_engine.h"
#include "eaif_trc.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

using namespace std;

//TBD use thread safe strtok_r if needed

int ParseConfigParam(char* str, ModelConfig* conf, int *dataset_hit)
{
	int hit = 1;
	char *tok = nullptr;
	if (!strcmp(str, "model_name")) {
		tok = strtok(nullptr, "=\n");
		conf->model_name = tok;
	} else if (!strcmp(str, "inference_type")) {
		tok = strtok(nullptr, "=\n");
		conf->inference_type = eaif::engine::GetInferenceType(tok);
	} else if (!strcmp(str, "engine_type")) {
		tok = strtok(nullptr, "=\n");
		conf->engine_type = eaif::engine::GetEngineType(tok);
	} else if (!strcmp(str, "activation_type")) {
		tok = strtok(nullptr, "=\n");
		conf->activation_type = eaif::engine::GetActivationType(tok);
	} else if (!strcmp(str, "width")) {
		tok = strtok(nullptr, "=\n");
		conf->input_shape.w = atoi(tok);
	} else if (!strcmp(str, "height")) {
		tok = strtok(nullptr, "=\n");
		conf->input_shape.h = atoi(tok);
	} else if (!strcmp(str, "channels")) {
		tok = strtok(nullptr, "=\n");
		conf->channels = atoi(tok);
	} else if (!strcmp(str, "resize_keep_ratio")) {
		tok = strtok(nullptr, "=\n");
		conf->resize_keep_ratio = atoi(tok);
		if (conf->resize_keep_ratio) {
			eaif_warn("Resize keep ratio is not supported currently.\n");
		}
	} else if (!strcmp(str, "topk")) {
		tok = strtok(nullptr, "=\n");
		conf->topk = atoi(tok);
	} else if (!strcmp(str, "model_path")) {
		tok = strtok(nullptr, "=\n");
		if (tok)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok) {
			conf->model_path.emplace_back(string(tok));
			tok = strtok(nullptr, ",");
			i++;
		};
	} else if (!strcmp(str, "extend_size")) {
		tok = strtok(nullptr, "=\n");
		if (tok)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok && i < 2) {
			conf->extend_size.dim[i] = atoi(tok);
			tok = strtok(nullptr, ",");
			i++;
		};
	} else if (!strcmp(str, "dataset")) {
		assert(*dataset_hit == 0);
		tok = strtok(nullptr, "=\n");
		conf->dataset = data::GetDatasetType(tok);
		conf->num_classes = data::GetNumClass(conf->dataset);
		conf->labels.resize(conf->num_classes);
		for (int i = 0; i < conf->num_classes; ++i) {
			conf->labels[i] = data::GetLabel(conf->dataset, i);
		}
		*dataset_hit = 1;
	} else if (!strcmp(str, "num_classes")) {
		if (*dataset_hit == 1) {
			eaif_warn("Skip num_classes as dataset is specified!\n");
			return hit;
		}
		tok = strtok(nullptr, "=\n");
		int num = atoi(tok);
		if (*dataset_hit == 2 && conf->num_classes != num) {
			eaif_err("Number of class is not align the size of labels!\n");
			assert(0);
		}
		conf->num_classes = num;
		*dataset_hit = 1;
	} else if (!strcmp(str, "labels")) {
		assert(*dataset_hit == 0 || *dataset_hit == 1);
		tok = strtok(nullptr, "=\n");
		char label[64];
		if (tok != nullptr)
			tok = strtok(tok, ",");
		else
			return 0;
		while (tok != nullptr) {
			strcpy(label, tok);
			conf->labels.push_back(string(label));
			tok = strtok(nullptr, ",");
		}
		conf->num_classes = conf->labels.size();
		*dataset_hit = 2;
	} else if (!strcmp(str, "zeros")) {
		tok = strtok(nullptr, "=\n");
		if (tok != nullptr)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok != nullptr && i < 3) {
			conf->zeros.push_back(atof(tok));
			tok = strtok(nullptr, ",");
			i++;
		};
	} else if (!strcmp(str, "stds")) {
		tok = strtok(nullptr, "=\n");
		if (tok != nullptr)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok != nullptr && i < 3) {
			conf->stds.push_back(atof(tok));
			tok = strtok(nullptr, ",");
			i++;
		};
	} else if (!strcmp(str, "conf_thresh")) {
        int tmp;
        tok = strtok(nullptr, "=\n");
        if (tok != nullptr)
            tok = strtok(tok, ",");
        else
            return 0;
        while (tok != nullptr) {
            tmp = atoi(tok);
            if (tmp == -1)
                return hit;
            conf->conf_thresh.push_back(atof(tok));
            tok = strtok(nullptr, ",");
        }
	} else if (!strcmp(str, "conf_thresh_low")) {
		tok = strtok(nullptr, "=\n");
		conf->conf_thresh_low = atof(tok);
	} else if (!strcmp(str, "iou_thresh")) {
		tok = strtok(nullptr, "=\n");
		conf->iou_thresh = atof(tok);
	} else if (!strcmp(str, "image_pre_resize")) {
		tok = strtok(nullptr, "=\n");
		conf->image_pre_resize = atoi(tok);
	} else if (!strcmp(str, "nms_internal_thresh")) {
        int tmp;
        tok = strtok(nullptr, "=\n");
        if (tok != nullptr)
            tok = strtok(tok, ",");
        else
            return 0;
        while (tok != nullptr) {
            tmp = atoi(tok);
            if (tmp == -1)
                return hit;
            conf->nms_internal_thresh.push_back(atof(tok));
            tok = strtok(nullptr, ",");
        }
	} else if (!strcmp(str, "window_min_size")) {
		tok = strtok(nullptr, "=\n");
		conf->window_min_size = atoi(tok);
	} else if (!strcmp(str, "window_scale_factor")) {
		tok = strtok(nullptr, "=\n");
		conf->window_scale_factor = atof(tok);
	} else if (!strcmp(str, "align_margin")) {
		tok = strtok(nullptr, "=\n");
		conf->align_margin = atoi(tok);
	} else if (!strcmp(str, "cpu_type")) {
		tok = strtok(nullptr, "=\n");
		conf->cpu_infer_type = atoi(tok);
	} else if (!strcmp(str, "face_infer_type")) {
		tok = strtok(nullptr, "=\n");
		conf->face_infer_type = eaif::engine::GetFaceInferType(tok);
	} else if (!strcmp(str, "filter_cls")) {
        int tmp;
        tok = strtok(nullptr, "=\n");
        if (tok != nullptr)
            tok = strtok(tok, ",");
        else
            return 0;
        while (tok != nullptr) {
            tmp = atoi(tok);
            if (tmp == -1)
                return hit;
            conf->filter_cls.push_back(atoi(tok));
            tok = strtok(nullptr, ",");
        }
	} else if (!strcmp(str, "filter_out_cls")) {
        int tmp;
        tok = strtok(nullptr, "=\n");
        if (tok != nullptr)
            tok = strtok(tok, ",");
        else
            return 0;
        while (tok != nullptr) {
            tmp = atoi(tok);
            if (tmp==-1)
                return hit;
            conf->filter_cls.push_back(tmp);
            tok = strtok(nullptr, ",");
        }
	} else {
		hit = 0;
	}
	return hit;
}

int ParseParam(char* str, ModelConfig* config)
{
	int hit = 0;
	char *tok = strtok(str, "=");
	int dataset_hit = 0;

	while (tok != nullptr) {
		hit = 0;
		if (!strncmp(tok, "#", strlen("#"))) {
			hit = 1;
			break;
		} else if (!strncmp(tok, "\n", strlen("\n"))) {
			hit = 1;
			break;
		}
		hit = ParseConfigParam(tok, config, &dataset_hit);
		if (hit == 1)
			goto next;
		else if (hit == 0) {
			printf("Unknown parameter: %s\n", tok);
			hit = 1;
			break;
		} else if (hit < 0) {
			if (!strncmp(tok, "\n", strlen("\n"))) {
				hit = 1;
				break;
			} else {
				printf("Unknown parameter: %s\n", tok);
				break;
			}
		}
	next:
		tok = strtok(NULL, "=");
	}
	return hit;
}

#define PLOG(x) cout << #x << " " << x << "\n";

void PrintConfig(ModelConfig& config)
{
	PLOG(config.model_name);
	PLOG(config.model_dir);
	PLOG(config.model_path[0]);
	PLOG(config.model_conf);
	PLOG(config.input_shape.w);
	PLOG(config.input_shape.h);
	PLOG(config.channels);
	PLOG(config.num_classes);
	PLOG(config.dataset);
	PLOG(config.zeros[0]);
	PLOG(config.stds[0]);
	if (config.conf_thresh.size())
		PLOG(config.conf_thresh[0]);
	PLOG(config.conf_thresh_low);
	PLOG(config.iou_thresh);
	PLOG(config.cpu_infer_type);
	PLOG(config.inference_type);
	PLOG(config.engine_type);
	if (config.filter_cls.size())
		PLOG(config.filter_cls[0]);
	if (config.filter_out_cls.size())
		PLOG(config.filter_cls[0]);
	if (config.nms_internal_thresh.size())
		PLOG(config.nms_internal_thresh[0]);
	PLOG(config.window_min_size);
	PLOG(config.window_scale_factor);
	PLOG(config.align_margin);
}

int ModelConfig::Parse(const char* input_model_path, const char* config_name)
{
	model_dir = input_model_path;
	model_conf = string(input_model_path) + "/" + string(config_name);

	char str[256] = {};

	FILE *fp = fopen(model_conf.c_str(), "r");
	if (fp == nullptr) {
		eaif_warn("Cannot open file from %s\n", model_conf.c_str());
		return EAIF_FAILURE;
	}

	while (fgets(str, sizeof(str), fp) != NULL) {
		int ret = ParseParam(str, this);
		/* Stop parsing when a unknown parameter found */
		if (!ret) {
			printf("Parsing parameter file failed.\n");
			break;
		}
	}
	fclose(fp);
	//PrintConfig(*this);
	return EAIF_SUCCESS;
};

#ifdef USE_JSON
#include "json.h"

void ParseObject(ObjectAttr& obj, struct json_object* json_obj)
{
	struct json_object *tmp_obj;
	struct json_object *tmp_obj_attr;
	struct json_object *tmp_obj_int;
	int i;
	if (json_object_object_get_ex(json_obj, "obj", &tmp_obj)) {
		if (json_object_object_get_ex(tmp_obj, "id", &tmp_obj_attr)) {
			obj.idx = json_object_get_int(tmp_obj_attr);
		}
		if (json_object_object_get_ex(tmp_obj, "rect", &tmp_obj_attr)) {
			int array_len = json_object_array_length(tmp_obj_attr);
			if (array_len < 4) {
				eaif_warn("Invalid input rect on json str!");
				for (i = 0; i < 4; i++) {
					obj.box.c[i] = 0;
				}
			}
			for (i = 0; i < 4; i++) {
				tmp_obj_int = json_object_array_get_idx(tmp_obj_attr, i);
				obj.box.c[i] = json_object_get_int(tmp_obj_int);
			}
		}
	}
}

void ParseObjectList(std::vector<ObjectAttr>& obj_list, struct json_object* json_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(json_obj, "od", &tmp_obj)) {

		int array_len = json_object_array_length(tmp_obj);
		obj_list.resize(array_len);
		for (i = 0; i < array_len; i++) {
			ParseObject(obj_list[i], json_object_array_get_idx(tmp_obj, i));
		}
	}
}

#endif

int DecodeJsonStr2Object(const char* str, size_t size,
	                    std::vector<ObjectAttr>& obj_list)
{
#ifdef USE_JSON
	/*
	{\"od\":[
	"{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"vel\":[%d,%d],\"cat\":\"\",\"shaking\":0}},"
	]}
	*/
	int retval = 0;
	enum json_tokener_error jerr;
	struct json_object *json_obj = NULL;
	struct json_tokener *tok = json_tokener_new();
	json_obj = json_tokener_parse_ex(tok, str, size);

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_success) {
		ParseObjectList(obj_list, json_obj);
		if (json_obj != NULL) {
			json_object_put(json_obj); //Decrement the ref count and free if it reaches zero
		} else {
			eaif_warn("empty json object parsed\n");
		}
	} else {
		eaif_warn(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		retval = -1;
	}
	json_tokener_free(tok);
	return retval;
#else
	eaif_err(" JSON lib is not enabled!");
	assert(0);
	return 0;
#endif
}

