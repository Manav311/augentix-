#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cassert>

using namespace std;

const string cModel_name = "model";
const string cSolver_type = "solver_type";
const string cNr_class = "nr_class";
const string cLabel = "label";
const string cNr_feature = "nr_feature";
const string cBias = "bias";
const string cW = "w";
const string cLinear = "L2R_L2LOSS_SVC";
const string cDualSVC = "L2R_L2LOSS_SVC_DUAL";


const string cNr_vector = "nr_vector";

const int reserve = 4096 * 512;

#define UNDEFINED_FLT (-99999999)

class c4_model_parser {
	public:
	string solver_type;
	int nr_class;
	vector<int> labels;
	int nr_feature;
	int bias;
	vector<float> data;

	int nr_vector;
	string model_name;
	int bias_size;

	void parse(const string &file);
	void save(const string &file) const;
	void print_model_info(void) const;
};

string getStrType(const string& buf, const string& cmp_type)
{
	if (!strncmp(buf.c_str(), cmp_type.c_str(), cmp_type.size())) {
		return string((strchr(buf.c_str(), ' ') + 1));
	}
	assert(0);
	return {};
}

int getIntType(const string& buf, const string& cmp_type)
{
	if (!strncmp(buf.c_str(), cmp_type.c_str(), cmp_type.size())) {
		return atoi((strchr(buf.c_str(), ' ') + 1));
	}
	assert(0);
	return {};
}

void getIntArrType(const string& buf, const string& cmp_type, vector<int> &value)
{
	char *msg = new char[buf.size() + 1];
	strcpy(msg, buf.c_str());

	if (!strncmp(msg, cmp_type.c_str(), cmp_type.size())) {
		char* tok = strtok(msg, " \n");
		assert(tok != NULL);
		tok = strtok(NULL, " \n");
		while (tok) {
			value.push_back(atoi(tok));
			tok = strtok(NULL, " \n");
		}
	}
	delete msg;
}

void c4_model_parser::print_model_info(void) const
{
	cout << cModel_name << " " << model_name << "\n";
	cout << cSolver_type << " " << solver_type << "\n";
	cout << cNr_class << " " << nr_class << "\n";
	cout << cLabel;
	for (auto& l : labels)
		cout << " " << l;
	cout << "\n";
	cout << cNr_feature << " " << nr_feature << "\n";
	cout << cBias << " " << bias << "\n";
	cout << cNr_vector << " " << nr_vector << "\n";
}

void c4_model_parser::parse(const string &file)
{
	std::ifstream in(file);
	if (in.good() == false) {
		std::cout << "SVM model " << file << " can not be loaded." << std::endl;
		exit(-1);
	}

	model_name = file;

	std::string buffer;
	std::getline(in, buffer); // first line
	solver_type = getStrType(buffer, cSolver_type);

	std::getline(in, buffer); // second line
	nr_class = getIntType(buffer, cNr_class);

	std::getline(in, buffer); // third line
	getIntArrType(buffer, cLabel, labels);

	std::getline(in, buffer); // four
	nr_feature = getIntType(buffer, cNr_feature);

	std::getline(in, buffer); // five
	bias = getIntType(buffer, cBias);

	in >> buffer;
	assert(buffer == "w");
	std::getline(in, buffer); //end of line 6

	data.reserve(reserve);
	int i = 1;
	while (!in.eof()) {
		float d = UNDEFINED_FLT;
		in >> d;
		if (d != UNDEFINED_FLT)
			data.push_back(d);
		//printf("%d\n", i++);
	};

	in.close();

	if (bias)
		bias_size = (solver_type == cLinear) ? 1 : 2;

	assert(data.size() % (nr_feature + bias) == 0);

	nr_vector = data.size() / nr_feature;
}

void c4_model_parser::save(const string &file) const
{
	FILE *fp = fopen(file.c_str(), "wb");

	assert(fp);

	char buf[512] = {};
	int size = sprintf(buf, "%s %s\n"
		         "%s %d\n"
		         "%s",
		         cSolver_type.c_str(), solver_type.c_str(),
		         cNr_class.c_str(), nr_class,
		         cLabel.c_str());
	for (auto& cls : labels) {
		size += sprintf(buf + size, " %d", cls);
	}
	size += sprintf(buf + size,
		"\n"
		"%s %d\n"
		"%s %d\n"
		"%s %d\n"
		"w\n",
		cNr_feature.c_str(), nr_feature,
		cBias.c_str(), bias_size,
		cNr_vector.c_str(), nr_vector);
	fprintf(fp, "%s", buf);
	fwrite(data.data(), data.size() * sizeof(float), 1, fp);
	fclose(fp);
}

string help(const string& msg)
{
	return msg + 
	       "\n\n"
	       "\t\tusage:\n"
	       "\t\t\tc4_model_parser <input_model_file> <output_model_file>\n";
}
int main(int argc, char** argv)
{
	if (argc != 3) {
		cout << help("Input arguments number is not correct!");
		exit(0);
	}

	c4_model_parser parser;

	parser.parse(string(argv[1]));
	parser.print_model_info();
	parser.save(string(argv[2]));

	cout << "finished!\n";
}
