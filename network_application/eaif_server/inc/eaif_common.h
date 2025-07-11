#ifndef EAIF_COMMON_H_
#define EAIF_COMMON_H_

#ifdef __cplusplus
#include <vector>
#endif

#include <stdint.h>
#include <stdlib.h>
#include "eaif_trc.h"

#define IMG_DESIRED_CHANNEL 3

#define EAIF_MODEL_CONFIG_NAME "config.ini"

#define EAIF_FR_BIT_U8 8
#define EAIF_FR_BIT_U8_VAL 255
#define EAIF_FR_BIT_U8_DIVIDE(x, y) (((x) << EAIF_FR_BIT_U8) / (y))

#define EAIF_FR_BIT 13
#define EAIF_FR_BIT_DIVIDE(x, y) (((x) << EAIF_FR_BIT) / (y))
#define EAIF_FR_BIT_MUL(x, y) ((((x) >> 4) * ((y) >> 4)) >> (EAIF_FR_BIT - 8))
#define EAIF_FR_DOWN(x) ((x) >> EAIF_FR_BIT)
#define EAIF_FR_BIT_2_FLOAT(x) ((float)(x) / (1 << EAIF_FR_BIT))

#ifdef __cplusplus

template <typename T> struct Rect {
	union {
		struct {
			T x;
			T y;
			T w;
			T h;
		};
		struct {
			T sx;
			T sy;
			T ex;
			T ey;
		};
		T c[4];
	};
};

template <typename T> struct FaceData {
	int encode_dim;
	int num_faces;
	std::vector<std::string> faces;
	std::vector<std::vector<T> > encodes;

	void reset(void) {
		faces.clear();
		num_faces = 1;
		faces.push_back("unknown");
		encodes.clear();
		encodes.resize(1);
		encodes[0].resize(encode_dim, 0.0f);
	}

	int write(const std::string &fname)
	{
		FILE *fp = fopen(fname.c_str(), "wb");
		if (!fp) {
			eaif_warn("Cannot open %s\n", fname.c_str());
			return -1;
		}
		int fnum_faces = num_faces - 1;
		fwrite(&encode_dim, sizeof(int), 1, fp);
		fwrite(&fnum_faces, sizeof(int), 1, fp);

		for (size_t i = 1; i < faces.size(); i++)
			fprintf(fp, "%s\n", faces[i].c_str());

		for (size_t i = 1; i < encodes.size(); i++)
			fwrite(encodes[i].data(), encodes[i].size() * sizeof(T), 1, fp);

		fclose(fp);
		return 0;
	}

	int read(const std::string &fname)
	{
		FILE *fp = fopen(fname.c_str(), "rb");

		if (!fp) {
			eaif_warn("Cannot open %s\n", fname.c_str());
			return -1;
		}

		int rnum_faces;
		int ret = fread(&encode_dim, sizeof(int), 1, fp);
		ret = fread(&rnum_faces, sizeof(int), 1, fp);
		num_faces += rnum_faces;

		for (int i = 0; i < rnum_faces; ++i) {
			char face_name[256] = {};
			if (fscanf(fp, "%s\n", face_name) != 1) {
				eaif_warn("Cannot read face name!\n");
				fclose(fp);
				return -1;
			}
			faces.push_back(std::string(face_name));
		}

		int read_size = encode_dim * sizeof(T);
		for (int i = 0; i < rnum_faces; ++i) {
			std::vector<T> encode(encode_dim);
			ret = fread(encode.data(), read_size, 1, fp);
			if (ret != 1) {
				eaif_warn("Cannot read face encode data\n!");
				fclose(fp);
				return -1;
			}
			encodes.emplace_back(std::move(encode));
		}
		return EAIF_SUCCESS;
	}
};

template <typename T> struct FaceLandmark {
	T x[5];
	T y[5];
};

template <typename T> struct FaceBox {
	T x0;
	T y0;
	T x1;
	T y1;

	/* confidence score */
	T score;

	/*regression scale */
	T regress[4];

	/* padding stuff*/
	T px0;
	T py0;
	T px1;
	T py1;

	FaceLandmark<T> landmark;
};

template <typename T> struct ScaleWindow {
	int h;
	int w;
	T scale;
};

struct ObjectAttr {
	int idx;
	Rect<int> box;
};

using ObjectList = std::vector<ObjectAttr>;

struct Shape {
	union {
		struct {
			int w;
			int h;
		};
		int dim[2];
	};
	Shape()
	        : w(0)
	        , h(0){};
	Shape(int width, int height)
	        : w(width)
	        , h(height){};
	~Shape(){};
};

struct Classification {
	int idx;
	std::vector<int> cls; // store top-k cls if any
	std::vector<float> prob; // store conf for all classes
};

struct Detection {
	Rect<float> box;
	float confidence; // conf of presence object
	std::vector<float> prob; // store conf for all classes
	std::vector<int> cls; // store top-k cls if any
	Detection copy()
	{
		return Detection{ box, confidence, prob, cls };
	}
};

#endif

// CV_DTYPE mapping
// bit 0-2 (0-6)
// 8U/8S/16U/16S/32S/32F/64F
// bit 3-4 (0-3)
// C1/C2/C3/C4

typedef enum { // map to cv dtype
	Eaif8U = 0,
	Eaif8UC1 = 0,
	Eaif8S = 1,
	Eaif8SC1 = 1,
	Eaif32S = 4,
	Eaif32SC1 = 4,
	Eaif32F = 5,
	Eaif32FC1 = 5,
	Eaif64F = 6,
	Eaif64FC1 = 6,
	Eaif8UC3 = 16,
	Eaif8SC3 = 17,
	Eaif32FC3 = 21,
	Eaif64FC3 = 22,
	Eaif8UC4 = 24,
	Eaif8SC4 = 25,
	Eaif32SC4 = 28,
	Eaif32FC4 = 29,
	EaifUnknownType = 7,
} EaifDataType;

#define EAIF_DTYPE(x) ((x)&0b111)
#define EAIF_DTYPE_TO_C1(x) ((x)&0b111)
#define EAIF_DTYPE_TO_C3(x) (((x)&0b111) | (2 << 3))

inline int GetDSize(int e_type)
{
	int dtype = e_type & 0b111;
	if (dtype <= 1)
		return sizeof(uint8_t);
	else if (dtype <= 3)
		return sizeof(uint16_t);
	else if (dtype <= 5)
		return sizeof(uint32_t);
	else if (dtype <= 6)
		return sizeof(uint64_t);
	eaif_warn("Unknown datatype %d!\n", e_type);
	return sizeof(uint8_t);
}

inline int eaif_GetImageTypeChn(int e_type)
{
	return (((e_type >> 3) & 0x3) + 1);
}

inline int eaif_GetImageType(int e_type, uint32_t chn)
{
	if (chn == 2 || chn > 3) {
		eaif_warn("Unknown chn value %d!\n", chn);
		return Eaif32FC3;
	}
	return (e_type & 0x7) | ((chn - 1) << 3);
}

inline const char *eaif_GetDTypeString(int type)
{
	switch (type) {
	case Eaif8UC3:
	case Eaif8U: {
		return "uint8";
	}
	case Eaif8SC3:
	case Eaif8S: {
		return "int8";
	}
	case Eaif32F:
	case Eaif32FC3: {
		return "float32";
	}
	default: {
		return "unknown type!!";
	}
	}
}

#ifdef __cplusplus

class QuantInfo {
    public:
	QuantInfo(void)
	        : m_zero(0)
	        , m_scale(0)
	        , m_zero_fix(0)
	        , m_scale_fix(0){};
	QuantInfo(int zero, float scale)
	        : m_zero(zero)
	        , m_scale(scale)
	        , m_zero_fix(zero << EAIF_FR_BIT)
	        , m_scale_fix(scale * (1 << EAIF_FR_BIT)){};
	~QuantInfo(){};
	void set(int zero, float scale)
	{
		m_zero = zero;
		m_scale = scale;
		m_zero_fix = zero << EAIF_FR_BIT;
		m_scale_fix = scale * (1 << EAIF_FR_BIT);
	}
	int Convert(uint8_t in) const
	{
		return ((int)in - m_zero) * m_scale_fix;
	}
	template <typename T> float Convertf(T in) const
	{
		return ((int)in - m_zero) * m_scale;
	}
	int GetZero(void)
	{
		return m_zero;
	}
	float GetScale(void)
	{
		return m_scale;
	}
	int m_zero;
	float m_scale;
	int m_zero_fix;
	int m_scale_fix;
};

template <typename T> T Abs(T x)
{
	return (x < 0) ? -x : x;
}

template <typename T> T Clamp(T x, T low, T high)
{
	if (x < low)
		return low;
	if (x > high)
		return high;
	return x;
}

template <typename T> T Min(T a, T b)
{
	return (a > b) ? b : a;
}

template <typename T> T Max(T a, T b)
{
	return (a > b) ? a : b;
}

template <typename T> T Clip(T a, T low, T high)
{
	return (a <= low) ? low : ((a >= high) ? high : a);
}

typedef float (*VecDistFuncPtr)(const std::vector<float> &, const std::vector<float> &);

struct DistMeasure {
	VecDistFuncPtr dist_funcptr;
	int ascend;
};

float VecDistL2Norm(const std::vector<float> &v0, const std::vector<float> &v1);
float VecDistL1Norm(const std::vector<float> &v0, const std::vector<float> &v1);
float VecCosineSimilarity(const std::vector<float> &v0, const std::vector<float> &v1);

void Dump(const uint8_t *data, size_t size, int idx);
void Dump(const uint8_t *data, size_t size, const char *name);
void Dump(const std::vector<FaceBox<float> > &data, const char *name);

template <typename T, typename Y> void memoryCpyCast(const T *src, Y *dst, size_t size);

using VecStr = std::vector<std::string>;

#endif /* __cplusplus */

#endif /* !EAIF_COMMON_H_ */
