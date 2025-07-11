#ifndef EAIF_DATA_H_
#define EAIF_DATA_H_

#include <string>

namespace data
{
enum set { None = -1, Coco, Voc, CrowdHuman, Num };

struct coco {
	static const char name[32];
	static const int num_class;
	static const std::string labels[80];
	static const int id_remap[80];
	static const char result_format[512];
	static const char result_format_start[];
	static const char result_format_end[];
	static const char result_format_delimit[];
};

struct voc {
	static const char name[32];
	static const int num_class;
	static const std::string labels[20];
};

struct crowdhuman {
	static const char name[32];
	static const int num_class;
	static const std::string labels[2];
	static const int id_remap[2];
	static const char result_format[512];
	static const char result_format_start[];
	static const char result_format_end[];
	static const char result_format_delimit[];
};

int GetNumClass(int dataset);
int GetDatasetType(const char *dataset);
const char *GetLabel(int dataset, int index);
} // namespace data

#endif