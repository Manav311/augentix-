#include <cstring>
#include <cassert>
#include "eaif_data.h"
#include "eaif_trc.h"

const char data::coco::name[] = "coco";
const int data::coco::num_class = 80;
const std::string data::coco::labels[] = 
	{
	"person", "bicycle", "car", "motorcycle", "airplane",
	"bus", "train", "truck", "boat",
	"traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
	"bird", "cat", "dog", "horse", "sheep",
	"cow", "elephant", "bear", "zebra", "giraffe",
	"backpack", "umbrella", "handbag", "tie", "suitcase",
	"frisbee", "skis", "snowboard", "sports ball", "kite",
	"baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket",
	"bottle", "wine glass", "cup", "fork", "knife",
	"spoon", "bowl", "banana", "apple", "sandwich",
	"orange", "broccoli", "carrot", "hot dog", "pizza",
	"donut", "cake", "chair", "couch", "potted plant",
	"bed", "dining table", "toilet", "tv", "laptop",
	"mouse", "remote", "keyboard", "cell phone", "microwave",
	"oven", "toaster", "sink", "refrigerator", "book",
	"clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
	};

const int data::coco::id_remap[] = 
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20,
 21, 22, 23, 24, 25, 27, 28, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
 41, 42, 43, 44, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
 61, 62, 63, 64, 65, 67, 70, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 90};
const char data::coco::result_format[] =           //sx   ex    width  height
"{\"image_id\":%d,\"category_id\":%d,\"bbox\":[%.2f,%.2f,%.2f,%.2f],\"score\":%.5f},\n";
const char data::coco::result_format_start[] = "[\n";
const char data::coco::result_format_end[] = "\n]\n";
const char data::coco::result_format_delimit[] = ",";


const char data::voc::name[] = "voc";
const int data::voc::num_class = 20;
const std::string data::voc::labels[] = 
	 {
		"aeroplane", "bicycle", "bird", "boat", "bottle",
		"bus", "car", "cat", "chair", "cow",
		"diningtable", "dog", "horse", "motorbike", "person",
		"pottedplant", "sheep", "sofa", "train", "tvmonitor"
	};

const char data::crowdhuman::name[] = "crowdhuman";
const int data::crowdhuman::num_class = 2;
const std::string data::crowdhuman::labels[] = 
	 {
		"person", "head"
	};

int data::GetNumClass(int dataset)
{
	switch(dataset)
	{
		case data::Coco:
			return data::coco::num_class;
		case data::Voc:
			return data::voc::num_class;
		case data::CrowdHuman:
			return data::crowdhuman::num_class;
		default:
			assert(0);
	}
}

int data::GetDatasetType(const char* dataset)
{
	if (strcmp(dataset, "coco") == 0)
		return data::Coco;
	else if (strcmp(dataset, "voc") == 0)
		return data::Voc;
	else if (strcmp(dataset, "crowdhuman") == 0)
		return data::CrowdHuman;
	else {
		std::cerr << "Cannot find dataset:" << std::string(dataset) << "\n";
		return -1;
	}
}

const char* data::GetLabel(int dataset, int index)
{
	switch(dataset)
	{
		case data::Coco:
			assert(index < data::coco::num_class);
			return data::coco::labels[index].c_str();
		case data::Voc:
			assert(index < data::voc::num_class);
			return data::voc::labels[index].c_str();
		case data::CrowdHuman:
			assert(index < data::crowdhuman::num_class);
			return data::crowdhuman::labels[index].c_str();
		default:
			assert(0);
	}
}
