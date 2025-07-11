#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "libosd.h"

#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

int help()
{
	printf("[usage]:\r\n");
	printf("-h help()\r\n");
	printf("-i input RGBA8888 file path	\r\n");
	printf("-W width\r\n");
	printf("-H height\r\n");
	return 0;
}

void normalizeAYUV3544(int *pa, int *py, int *pu, int *pv)
{
	int a, y, u, v;
	a = *pa;
	y = *py;
	u = *pu;
	v = *pv;

	y = ((y * 31 * 100) / 255) / 100;
	u = ((u * 15 * 100) / 255) / 100;
	v = ((v * 15 * 100) / 255) / 100;
	a = ((a * 7 * 100) / 255) / 100;

	a = a > 7 ? 7 : a;
	y = y > 31 ? 31 : y;
	u = u > 15 ? 15 : u;
	v = v > 15 ? 15 : v;

	*pa = a;
	*py = y;
	*pu = u;
	*pv = v;
}

int main(int argc, char **argv)
{
	libosd_log_info("start");

	int c = 0;
	char inputPath[128];
	uint32_t width, height;
	width = 100;
	height = 100;
	int ayuv_img_size = (width * height) * 2;
	while ((c = getopt(argc, argv, "hi:W:H:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'i':
			snprintf(&inputPath[0], 128, "%s", argv[optind - 1]);
			break;
		case 'W':
			width = atoi(argv[optind - 1]);
			break;
		case 'H':
			height = atoi(argv[optind - 1]);
			break;
		default:
			help();
			exit(1);
		}
	}

	FILE *fp;
	FILE *fp_ayuv;
	fp = fopen(&inputPath[0], "rb");
	if (fp == NULL) {
		libosd_log_err("failed to open rgba file");
	}

	fp_ayuv = fopen("./save.ayuv", "wb");
	if (fp == NULL) {
		libosd_log_err("failed to open save.ayuv file");
		fclose(fp);
		return -1;
	}
	ayuv_img_size = (width * height) * 2;
	libosd_log_debug("ayuv header: (w,h,size) = (%d, %d, %d)", width, height, ayuv_img_size);

	fwrite(&width, sizeof(uint32_t), 1, fp_ayuv);
	fseek(fp_ayuv, sizeof(uint32_t), SEEK_SET);
	fwrite(&height, sizeof(uint32_t), 1, fp_ayuv);
	fseek(fp_ayuv, sizeof(uint32_t) * 2, SEEK_SET);
	fwrite(&ayuv_img_size, sizeof(uint32_t), 1, fp_ayuv);
	fseek(fp_ayuv, sizeof(uint32_t) * 3, SEEK_SET);

	char rgbabuf[width * 4];
	char ayuvBuf[width * height * 2];
	int i, j, r, g, b, a8, a3, y, u, v, ayuvIdx;
	ayuvIdx = 0;

	for (i = 0; i < height; i++) {
		fread(&rgbabuf[0], width * 4, 1, fp);
		for (j = 0; j < width * 4; j += 4) {
			r = rgbabuf[j + 0];
			g = rgbabuf[j + 1];
			b = rgbabuf[j + 2];
			a8 = rgbabuf[j + 3];

			y = 0.299 * r + 0.587 * g + 0.114 * b;
			u = -0.147 * r - 0.289 * g + 0.436 * b + 128;
			v = 0.615 * r - 0.515 * g - 0.100 * b + 128;
			a3 = a8 * 100 / 255;

			normalizeAYUV3544(&a3, &y, &u, &v);

			ayuvBuf[ayuvIdx] = MERGE44(u, v);
			ayuvIdx += 1;
			ayuvBuf[ayuvIdx] = MERGE35(a3, y);
			ayuvIdx += 1;
		}
		fseek(fp, width * 4 * i, SEEK_SET);
	}

	fwrite(ayuvBuf, width * height * 2, 1, fp_ayuv);
	fclose(fp_ayuv);

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
