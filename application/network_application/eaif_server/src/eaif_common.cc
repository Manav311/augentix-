#include <vector>

#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "eaif_common.h"

using namespace std;

float VecDistL2Norm(const vector<float>& v0, const vector<float>& v1)
{
	float sum = 0.0f;
	float ele = 0.0f;
	for (size_t i = 0; i < v0.size(); ++i) {
		ele = v0[i] - v1[i];
		sum += ele * ele;
	}
	return sqrt(sum);
}

float VecDistL1Norm(const vector<float>& v0, const vector<float>& v1)
{
	float sum = 0.0f;
	for (size_t i = 0; i < v0.size(); ++i)
		sum += Abs(v0[i] - v1[i]);
	return sum;
}

float VecCosineSimilarity(const vector<float>& v0, const vector<float>& v1)
{
	float deno_v0 = 0.0f, deno_v1 = 0.0f, numerator = 0.0f;
	for (size_t i = 0; i < v0.size(); ++i) {
		numerator += v0[i] * v1[i];
		deno_v0 += v0[i] * v0[i];
		deno_v1 += v1[i] * v1[i];
	}
	if (deno_v0 == 0.0f || deno_v1 == 0.0f)
		return 0.0f;
	return Abs(numerator / (sqrt(deno_v0) * sqrt(deno_v1)));
}

void Dump(const uint8_t* data, size_t size, int idx)
{
	char file[64] = { 0 };
	sprintf(file, "dump-%d.bin", idx);
	FILE *fp = fopen(file, "wb");
	fwrite(data, 1, size, fp);
	fclose(fp);
}

void Dump(const uint8_t* data, size_t size, const char *name)
{
	FILE *fp = fopen(name, "wb");
	fwrite(data, 1, size, fp);
	fclose(fp);
}

void Dump(const vector<FaceBox<float>> &data, const char *name)
{
	FILE *fp = fopen(name, "wb");
	for (auto& face : data) {
		fwrite(&face.x0, 1, sizeof(float), fp);
		fwrite(&face.y0, 1, sizeof(float), fp);
		fwrite(&face.x1, 1, sizeof(float), fp);
		fwrite(&face.y1, 1, sizeof(float), fp);
		fwrite(&face.score, 1, sizeof(float), fp);
		fwrite(face.regress, 4, sizeof(float), fp);
	}
	fclose(fp);
}


template<typename T, typename Y>
void MemoryCpyCast(const T* src, Y* dst, size_t size)
{
	for (int i = 0; i < size; ++i) {
		dst[i] = (Y) src[i];
	}
}
