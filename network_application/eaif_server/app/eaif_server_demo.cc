#include <iostream>
#include <cstring>
#include <getopt.h>
#include <cassert>

#include "eaif_model.h"
#include "eaif_engine_demo.h"
#include "eaif_trc.h"

void print_ops()
{
	printf("\t ===== help ====\n"
#if defined(USE_ARMNN) && defined(USE_TFLITE)
		"\t --engine -N <armnn(default)/tflite> inference engine\n"
#endif
		"\t (mode inf/fn/mtcnn/fn/cas/hog/mtcnnWider/mtcnnFddb/c4)\n" 
		"\t --inf <model file>	:: inference model direcly\n"
		"\t --yolov4/5 <model file>\n"
		"\t     <conf thresh> <iou thresh> <datasetname> <eval[int]>\n"
		"\t --mtcnnWider/--mtcnnFddb <pnet file> <rnet file> <onet file>\n"
		"\t     <pnet-thr:0.6> <rnet-thr:0.7> <onet-thr:0.9> <output-nms-thr:0.6> <scale-factor:0.709> <minface:40>\n"
		"\t	--input <input file> \n"
		"\t	--dst <image file> output dir path for evaluation\n"
		"\t --imh resize image height\n"
		"\t --imw resize image width\n"
		"\t --ver verbose value(0/1)\n"
		"\t --range <start index> <end index> print ele value from start to end index\n"
		"\t --iter <iter time> loop for read write inference\n"
		"\t --zero <zero> norm zero input\n"
		"\t --scale <scale> norm scale input\n"
		"\t --cpuinfer <\"cpuacc\"/\"cpuref\"> for armnn inference)\n"
		"\t --thread <num thread>\n"
		"\t --debug (0-5 on for debug mode armnn only trace:debug:info:warning:error:fatal)\n\n");
}

int main(int argc, char **argv)
{
	eaif::EngineDemo engine;
#ifdef USE_ARMNN
	engine.m_engine = 0;
#else
	engine.m_engine = 1;
#endif
	engine.m_range[0] = 0;
	engine.m_range[1] = 1;
	engine.m_verbose = 0;

	char* model_file = {};
	char* data_dir = {};
	char* dst_dir = {};
	char dataset[16] = "coco\0";
	int trace_level = 3;

	int height = 0, width = 0;
	std::cout << __LINE__ << " input args :" << argc << "\n";

	int c = 0;
	typedef union {
		uint32_t type;
		struct {
		uint32_t inf:1;
		uint32_t fn:1;
		uint32_t cas:1;
		uint32_t hog:1;
		uint32_t mtcnn:1;
		uint32_t mtcnnWider:1;
		uint32_t mtcnnFddb:1;
		uint32_t yolov5:1;
		uint32_t yolov4:1;
		uint32_t c4:1;
		};
	} Run;
	Run runtype = { 0 };

	static struct option long_options[] =
	{
		// inference type
		{"engine", required_argument, 0,'N'},
		{"inf",	required_argument,		 0, 'f'},
		{"fn",	no_argument, 0, 'e'},
		{"mtcnn",	required_argument, 0, 'm'},
		{"cas", no_argument, 0, 'c'},
		{"hog", no_argument, 0, 'H'},
		{"yolov5", no_argument, 0, 'y'},
		{"yolov4", no_argument, 0, 'Y'},
		{"c4", no_argument, 0, 'C'},
		// evaluation data
		{"mtcnnWider",	required_argument,		 0, 'a'},
		{"mtcnnFddb",	required_argument,		 0, 'b'},
		// image information
		{"input", required_argument, 0, 'i'},
		{"dst", required_argument, 0, 'd'},
		{"imh", required_argument, 0, 'h'},
		{"imw", required_argument, 0, 'w'},
		// verbose
		{"ver", no_argument, 0, 'v'},
		// debug
		{"range", required_argument, 0, 'r'},
		// testing
		{"iter", required_argument, 0, 't'},
		// quantize model only
		{"zero", required_argument, 0, 'z'},
		{"scale", required_argument, 0, 's'},
		{"cpuinfer", required_argument, 0, 'p'},
		{"debug", required_argument, 0, 'D'},
		{"eval", required_argument, 0, 'E'},
		{"thread", required_argument, 0, 'T'},
		{0, 0, 0, 0}
	};


	while (1)
	{
		/* getopt_long stores the option index here. */
		int option_index = 0;
		char *tok;
		int i;

		c = getopt_long (argc, argv, "N:f:em:ca:b:i:h:w:vr:t:z:s:d:y:Y:p:D:T",
					 long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
		break;

		switch (c)
		{
		case 'N':
		    if (strcmp(optarg, "armnn")==0)
#ifdef USE_ARMNN
			   engine.m_engine = (int)eaif::engine::Armnn;
#else
			{
				eaif_info_h("ARMNN is not enabled!");
				assert(0);
			}
#endif
		   else 
#ifdef USE_TFLITE
			   engine.m_engine = (int)eaif::engine::TfLite; // set to tflite
#else
			{
				eaif_info_h("TFLITE is not enabled!");
				assert(0);
			}
#endif
			break;
		case 'f':
			assert(runtype.type == 0);
			runtype.inf = 1;
			model_file = optarg;
			break;
		case 'e':
			assert(runtype.type == 0);
			runtype.fn = 1;
			break;
		case 'y':
			assert(runtype.type == 0);
			runtype.yolov5 = 1;
			model_file = argv[optind];
			if (argv[optind+1][0] == '-')
				break;
			engine.m_conf_thresh = atof(argv[optind+1]);
			engine.m_iou_thresh = atof(argv[optind+2]);
			strcpy(dataset, argv[optind+3]);
			engine.m_misc_flag = atof(argv[optind+4]);
			break;
		case 'Y':
			assert(runtype.type == 0);
			runtype.yolov4 = 1;
			model_file = argv[optind];
			if (argv[optind+1][0] == '-')
				break;
			engine.m_conf_thresh = atof(argv[optind+1]);
			engine.m_iou_thresh = atof(argv[optind+2]);
			strcpy(dataset, argv[optind+3]);
			engine.m_misc_flag = atof(argv[optind+4]);
			break;
		case 'm': {
			assert(runtype.type == 0);
			runtype.mtcnn = 1;
			if (argv[optind][0] == '-')
				break;
			int index = 0;
			engine.m_model_path_vec.push_back(optarg);
			engine.m_model_path_vec.push_back(argv[optind+index++]);
			engine.m_model_path_vec.push_back(argv[optind+index++]);
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_iou_thresh = atof(argv[optind+index++]);
			engine.m_scalefactor = atof(argv[optind+index++]);
			engine.m_minface = atoi(argv[optind+index++]);
			eaif_info_h("Setup thr:%.3f %.3f %.3f sc:%.3f minface:%d model_path:%s %s %s\n",
			engine.m_threshold[0], engine.m_threshold[1], engine.m_threshold[2],
			engine.m_scalefactor, engine.m_minface,
			engine.m_model_path_vec[0].c_str(),
			engine.m_model_path_vec[1].c_str(),
			engine.m_model_path_vec[2].c_str());
			break;
		}
		case 'c':
			assert(runtype.type == 0);
			runtype.cas = 1;
			break;
		case 'H':
			assert(runtype.type == 0);
			runtype.hog = 1;
			if (argv[optind][0] == '-')
				break;
			engine.m_misc_flag = atof(argv[optind]);
			break;
		case 'a': {
			assert(runtype.type == 0);
			runtype.mtcnnWider = 1;
			int index = 0;
			engine.m_model_path_vec.push_back(optarg);
			engine.m_model_path_vec.push_back(argv[optind+index++]);
			engine.m_model_path_vec.push_back(argv[optind+index++]);
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_iou_thresh = atof(argv[optind+index++]);
			engine.m_scalefactor = atof(argv[optind+index++]);
			engine.m_minface = atoi(argv[optind+index++]);
			eaif_info_h("Setup thr:%.3f %.3f %.3f sc:%.3f minface:%d model_path:%s %s %s\n",
			engine.m_threshold[0], engine.m_threshold[1], engine.m_threshold[2],
			engine.m_scalefactor, engine.m_minface,
			engine.m_model_path_vec[0].c_str(),
			engine.m_model_path_vec[1].c_str(),
			engine.m_model_path_vec[2].c_str());
			break;
		}
		case 'b': {
			assert(runtype.type == 0);
			runtype.mtcnnFddb = 1;
			int index = 0;
			engine.m_model_path_vec.push_back(optarg);
			engine.m_model_path_vec.push_back(argv[optind+index++]);
			engine.m_model_path_vec.push_back(argv[optind+index++]);
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_threshold.push_back(atof(argv[optind+index++]));
			engine.m_iou_thresh = atof(argv[optind+index++]);
			engine.m_scalefactor = atof(argv[optind+index++]);
			engine.m_minface = atoi(argv[optind+index++]);
			eaif_info_h("Setup thr:%.3f %.3f %.3f sc:%.3f minface:%d model_path:%s %s %s\n",
			engine.m_threshold[0], engine.m_threshold[1], engine.m_threshold[2],
			engine.m_scalefactor, engine.m_minface,
			engine.m_model_path_vec[0].c_str(),
			engine.m_model_path_vec[1].c_str(),
			engine.m_model_path_vec[2].c_str());
			break;
		}
		case 'C':
			assert(runtype.type == 0);
			runtype.c4 = 1;
			break;
		case 'i':
			data_dir = optarg;
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'v':
			engine.m_verbose = 1;// = atoi(optarg);
			break;
		case 'T':
			engine.m_nthread = atoi(optarg);
			break;
		case 'r':
			engine.m_range[0] = std::max(atoi(optarg), 0);
			printf("argv[%d]: %s\n", optind, argv[optind]);
			if (argv[optind][0] == '-') {
				engine.m_range[1] = engine.m_range[0]++;
				break;
			}
			engine.m_range[1] = std::max(atoi(argv[optind]), engine.m_range[0]+1);
			break;
		case 't':
			engine.m_iter = std::max(atoi(optarg), 1);
			break;
		case 'z':
			tok = strtok(optarg, ",");
			for (i = 0; i < 3 && tok != NULL; i++) {
				engine.m_zero[i] = atof(tok);
				tok = strtok(NULL, ",");
			}
			for (int j = i; j < 3; j++) {
				engine.m_zero[j] = engine.m_zero[j - 1]; \
			}
			break;
		case 's':
			tok = strtok(optarg, ",");
			for (i = 0; i < 3 && tok != NULL; i++) {
				engine.m_scale[i] = atof(tok);
				tok = strtok(NULL, ",");
			}
			for (int j = i; j < 3; j++) {
				engine.m_scale[j] = engine.m_scale[j - 1];
			}
			break;
		case 'd':
			dst_dir = optarg;
			break;
		case 'p':
			if (strcmp(optarg, "cpuacc") == 0)
				engine.m_cpu_infer = 0;
			else if (strcmp(optarg, "cpuref") == 0)
				engine.m_cpu_infer = 1;
			else
				assert(0);
			break;
		case 'D':
			trace_level = atoi(optarg);
			assert(trace_level <= 5 && trace_level >= 0);
			engine.SetDebugMode(trace_level);
			break;
		default:
			printf("wrong option specification\n");
			print_ops();
			exit(-1);
			break;
		}
	}
	assert(runtype.type > 0);
	assert(data_dir != NULL);
	engine.SetDim(height, width);
	if (runtype.mtcnnWider) {
		assert(dst_dir != NULL);
		engine.RunMtcnnWIDER(data_dir, dst_dir);
	} else if (runtype.mtcnnFddb) {
		assert(dst_dir != NULL);
		engine.RunMtcnnFDDB(data_dir, dst_dir);
	} else if (runtype.inf) {
		assert(model_file != NULL);
		engine.RunMinimum(model_file, data_dir);
	} else if (runtype.cas) {
		assert(model_file == NULL);
		std::string model = "haarcascade_frontalface_alt2.xml";
		engine.RunCascade(data_dir, model.c_str());
	} else if (runtype.hog) {
		 	engine.RunHog(data_dir);
	} else if (runtype.fn) {
		assert(dst_dir != NULL);
		engine.RunFacenetModel(data_dir, dst_dir);
	} else if (runtype.mtcnn) {
		engine.RunMtcnn(data_dir);
	} else if (runtype.yolov5) {
		assert(model_file != NULL);
		if (engine.m_misc_flag)
			engine.RunYolov5Eval(model_file, data_dir, dataset);
		else 
			engine.RunYolov5(model_file, data_dir, dataset);
	} else if (runtype.yolov4) {
		assert(model_file != NULL);
		if (engine.m_misc_flag)
			engine.RunYolov4Eval(model_file, data_dir, dataset);
		else
			engine.RunYolov4(model_file, data_dir, dataset);
	} else if(runtype.c4) {
		engine.RunC4(model_file, data_dir);
	}
	return 0;
}


