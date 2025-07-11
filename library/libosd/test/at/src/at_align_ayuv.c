#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>

#include "mpi_sys.h"

#include "libosd.h"

typedef enum { AYUV, FONTAYUV, COMPACTFONT } FORMAT;

extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
extern int alignUVVal(char *ayuv_buf, int width, int height);
#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

typedef struct {
	UINT32 index_offset;
	UINT32 index_size;
	UINT32 ascii_index;
	UINT32 ascii_width;
	UINT32 cht_index;
	UINT32 cht_width;
	UINT32 other_index;
	UINT32 other_width;
	UINT32 data_offset;
	UINT32 data_size;
} AyuvInfo_S;

typedef struct {
	UINT32 image_offset;
	UINT32 image_size;
	UINT32 image_width;
	UINT32 image_height;
} ASCII_INDEX;

int getAYUVFONTINFO(char *inputPath, FORMAT f)
{
	if (f != FONTAYUV) {
		libosd_log_err("not support");
		return -EINVAL;
	}

	FILE *fp;
	fp = fopen(inputPath, "rb");
	if (fp == NULL) {
		libosd_log_err("Failed to open .ayuv");
		return -EINVAL;
	}
	AyuvInfo_S fontInfo;
	fread(&fontInfo, sizeof(fontInfo), 1, fp);
	libosd_log_info("index_offset : %d", fontInfo.index_offset);
	libosd_log_info("index_size : %d", fontInfo.index_size);
	libosd_log_info("ascii_index : %d", fontInfo.ascii_index);
	libosd_log_info("ascii_width : %d", fontInfo.ascii_width);
	libosd_log_info("cht_index : %d", fontInfo.cht_index);
	libosd_log_info("cht_width : %d", fontInfo.cht_width);
	libosd_log_info("other_index : %d", fontInfo.other_index);
	libosd_log_info("other_width : %d", fontInfo.other_width);
	libosd_log_info("data_offset : %d", fontInfo.data_offset);
	libosd_log_info("data_size : %d", fontInfo.data_size);

	char font_data[fontInfo.data_size];
	fseek(fp, fontInfo.data_offset, SEEK_SET);
	fread(&font_data[0], fontInfo.data_size, 1, fp);

	char font_index[fontInfo.index_size];
	fseek(fp, fontInfo.index_offset, SEEK_SET);
	fread(&font_index[0], fontInfo.index_size, 1, fp);

	FILE *fp_new;
	fp_new = fopen("/mnt/nfs/ethnfs/font.ayuv", "wb");
	if (fp_new == NULL) {
		libosd_log_err("Failed to open new");
		fclose(fp);
	}
	fwrite(&fontInfo, sizeof(fontInfo), 1, fp_new);
	libosd_log_err("new write: %d", sizeof(fontInfo));
	fwrite(&font_index[0], fontInfo.index_size, 1, fp_new);
	libosd_log_err("new write: %d", fontInfo.index_size);

	ASCII_INDEX *idx = (ASCII_INDEX *)&font_index[0];
	unsigned char input_ascii = 0;
	UINT32 real_addr = 0;
	uint8_t ayuv[2];
	int new_idx = 0;

	char fileName[32];
	for (int i = 32; i < 127; i++) {
		input_ascii = i;
		input_ascii = input_ascii - 32;
		sprintf(&fileName[0], "/mnt/nfs/ethnfs/%d.ayuv", input_ascii);
		real_addr = (UINT32)&font_data[idx[input_ascii].image_offset];
		libosd_log_err("gen %d %d %d txt, offset: %d", idx[input_ascii].image_width,
		               idx[input_ascii].image_height, input_ascii, idx[input_ascii].image_offset);
		char tmp_ayuv[idx[input_ascii].image_width * idx[input_ascii].image_height * 2];
		int tmp_idx = 0;
		for (int j = 0; j < idx[input_ascii].image_height; j++) {
			for (int k = 0; k < idx[input_ascii].image_width; k++) {
				ayuv[0] = font_data[idx[input_ascii].image_offset +
				                    (j * idx[input_ascii].image_width + k) * 2];
				ayuv[1] = font_data[idx[input_ascii].image_offset +
				                    (j * idx[input_ascii].image_width + k) * 2 + 1];
				{
					ayuv[0] = 0x88;
				}
				tmp_ayuv[tmp_idx] = ayuv[0];
				tmp_idx += 1;
				tmp_ayuv[tmp_idx] = ayuv[1];
				tmp_idx += 1;
			}
		}
		saveAYUV(&fileName[0], idx[input_ascii].image_width, idx[input_ascii].image_height, (char *)real_addr,
		         idx[input_ascii].image_width * idx[input_ascii].image_height * 2);

		fwrite(&tmp_ayuv[0], idx[input_ascii].image_width * idx[input_ascii].image_height * 2, 1, fp_new);
		libosd_log_err("new write: %d, total: %d",
		               idx[input_ascii].image_width * idx[input_ascii].image_height * 2, new_idx);
		new_idx += idx[input_ascii].image_width * idx[input_ascii].image_height * 2;
	}

	for (int i = 95; i < 104; i++) {
		input_ascii = i;
		sprintf(&fileName[0], "/mnt/nfs/ethnfs/%d.ayuv", input_ascii);
		real_addr = (UINT32)&font_data[idx[input_ascii].image_offset];
		libosd_log_err("gen %d %d %d txt, offset: %d", idx[input_ascii].image_width,
		               idx[input_ascii].image_height, input_ascii, idx[input_ascii].image_offset);
		char tmp_ayuv[idx[input_ascii].image_width * idx[input_ascii].image_height * 2];
		int tmp_idx = 0;
		for (int j = 0; j < idx[input_ascii].image_height; j++) {
			for (int k = 0; k < idx[input_ascii].image_width; k++) {
				ayuv[0] = font_data[idx[input_ascii].image_offset +
				                    (j * idx[input_ascii].image_width + k) * 2];
				ayuv[1] = font_data[idx[input_ascii].image_offset +
				                    (j * idx[input_ascii].image_width + k) * 2 + 1];
				{
					ayuv[0] = 0x88;
				}
				tmp_ayuv[tmp_idx] = ayuv[0];
				tmp_idx += 1;
				tmp_ayuv[tmp_idx] = ayuv[1];
				tmp_idx += 1;
			}
		}
		saveAYUV(&fileName[0], idx[input_ascii].image_width, idx[input_ascii].image_height, (char *)real_addr,
		         idx[input_ascii].image_width * idx[input_ascii].image_height * 2);
		fwrite(&tmp_ayuv[0], idx[input_ascii].image_width * idx[input_ascii].image_height * 2, 1, fp_new);
		libosd_log_err("new write: %d, total: %d",
		               idx[input_ascii].image_width * idx[input_ascii].image_height * 2, new_idx);
		new_idx += idx[input_ascii].image_width * idx[input_ascii].image_height * 2;
	}
	libosd_log_info("new total = %d", new_idx + 1704);

	fclose(fp);
	fclose(fp_new);

	return 0;
}
int alignAYUVimage(char *inputPath, FORMAT f)
{
	FILE *fp;
	fp = fopen(inputPath, "rb");
	if (fp == NULL) {
		libosd_log_err("Failed to open .ayuv");
		return -EINVAL;
	}

	uint32_t img_width, img_height;
	fread(&img_width, sizeof(uint32_t), 1, fp);
	fseek(fp, sizeof(uint32_t), SEEK_SET);
	fread(&img_height, sizeof(uint32_t), 1, fp);
	fseek(fp, sizeof(uint32_t) * 3, SEEK_SET);

	char ayuv_buf[img_width * img_height * 2];
	fread(&ayuv_buf[0], img_width * img_height * 2, 1, fp);
	fclose(fp);

	alignUVVal(&ayuv_buf[0], img_width, img_height);
	uint8_t ayuv[2];
	for (int i = 0; i < img_height; i++) {
		for (int j = 0; j < img_width; j++) {
			ayuv[0] = ayuv_buf[(i * img_width + j) * 2];
			ayuv[1] = ayuv_buf[(i * img_width + j) * 2 + 1];
			if (ayuv[0] == 0x77) {
				ayuv[0] = 0x88;
			}
			ayuv_buf[(i * img_width + j) * 2] = ayuv[0];
		}
	}

	saveAYUV("/mnt/nfs/ethnfs/save-logo.ayuv", img_width, img_height, &ayuv_buf[0], img_width * img_height * 2);
	libosd_log_err("save to save-logo.ayuv");

	return 0;
}

#define DISPLAYABLE_CHARACTER_NUMBERS 104 /**< Number of displayable ASCII character. */
const int week[7][3] = { { 95, 96, 103 }, { 95, 96, 97 },  { 95, 96, 98 }, { 95, 96, 99 },
	                 { 95, 96, 100 }, { 95, 96, 101 }, { 95, 96, 102 } };
int alignCompactAYUV(char *inputPath, FORMAT f)
{
	int ret = 0;

	FILE *fp;
	fp = fopen(inputPath, "rb");
	if (fp == NULL) {
		libosd_log_err("Failed to open .ayuv");
		return -EINVAL;
	}

	int index_offset;
	int index_size;
	int data_offset;
	int data_size;

	ASCII_INDEX osdindex[DISPLAYABLE_CHARACTER_NUMBERS];

	fread(&index_offset, sizeof(uint32_t), 1, fp);
	fread(&index_size, sizeof(uint32_t), 1, fp);
	fread(&data_offset, sizeof(uint32_t), 1, fp);
	fread(&data_size, sizeof(uint32_t), 1, fp);

	fseek(fp, index_offset, SEEK_SET);
	fread(osdindex, index_size, 1, fp);
	char *p_font_ayuv = NULL;
	p_font_ayuv = malloc(data_size);
	if (p_font_ayuv == NULL) {
		libosd_log_err("Cannot allocate memory for %s. err: %d", inputPath, -ENOMEM);
		ret = -ENOMEM;
		goto closefd;
	}

	fseek(fp, data_offset, SEEK_SET);
	fread(p_font_ayuv, data_size, 1, fp);
closefd:
	fclose(fp);

	libosd_log_info("%s index_offset: %d, index_size %d, data_offset %d, data_size: %d", inputPath, index_offset,
	                index_size, data_offset, data_size);

	int total_size = 0;
	int total_idx = data_offset;
	//char fileName[64];
	for (int i = 0; i < DISPLAYABLE_CHARACTER_NUMBERS; i++) {
		libosd_log_info("[%d] image_offset:%d, size: %d, w: %d, h: %d", i, osdindex[i].image_offset,
		                osdindex[i].image_size, osdindex[i].image_width, osdindex[i].image_height);
		if (total_idx != osdindex[i].image_offset + data_offset) {
			libosd_log_err("total_idx != osdindex[%d].image_offset", i);
		}
		total_size += osdindex[i].image_size;
		total_idx += osdindex[i].image_size;
#if 0
		sprintf(&fileName[0], "/mnt/nfs/ethnfs/%d.ayuv", i);
		saveAYUV(&fileName[0], osdindex[i].image_width, osdindex[i].image_height, p_font_ayuv + osdindex[i].image_offset,
		        osdindex[i].image_width * osdindex[i].image_height * 2);
#endif
		char ay = 0x00;
		char uv = 0x00;
		for (int j = 0; j < osdindex[i].image_height; j++) {
			for (int k = 0; k < osdindex[i].image_width; k++) {
				uv = p_font_ayuv[osdindex[i].image_offset + (j * osdindex[i].image_width + k) * 2];
				ay = p_font_ayuv[osdindex[i].image_offset + (j * osdindex[i].image_width + k) * 2 + 1];
				uv = 0x88;
				p_font_ayuv[osdindex[i].image_offset + (j * osdindex[i].image_width + k) * 2] = uv;
				p_font_ayuv[osdindex[i].image_offset + (j * osdindex[i].image_width + k) * 2 + 1] = ay;
			}
		}
#if 0
		sprintf(&fileName[0], "/mnt/nfs/ethnfs/%d-align.ayuv", i);
		saveAYUV(&fileName[0], osdindex[i].image_width, osdindex[i].image_height, p_font_ayuv + osdindex[i].image_offset,
		        osdindex[i].image_width * osdindex[i].image_height * 2);
#endif
	}

	if (total_size != (data_size)) {
		libosd_log_err("total_size != (data_offset + data_size)");
	}

	FILE *fp_new;
	fp_new = fopen("/mnt/nfs/ethnfs/compact_chinese_noalpha-1.ayuv", "wb");
	if (fp_new == NULL) {
		libosd_log_err("Failed to open new");
		fclose(fp_new);
	}

	fwrite(&index_offset, sizeof(uint32_t), 1, fp_new);
	fwrite(&index_size, sizeof(uint32_t), 1, fp_new);
	fwrite(&data_offset, sizeof(uint32_t), 1, fp_new);
	fwrite(&data_size, sizeof(uint32_t), 1, fp_new);
	fwrite(osdindex, index_size, 1, fp_new);
	fwrite(p_font_ayuv, data_size, 1, fp_new);

	fclose(fp_new);
	free(p_font_ayuv);
	return ret;
}

void help()
{
	printf("usage:\r\n");
	printf("-h help()\r\n");
	printf("-f format AYUV or FONTAYUV , COMPACTFONT\r\n");
	printf("-i input path \r\n");
	return;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	int c = 0;
	char inputPath[128];
	FORMAT format = AYUV;
	while ((c = getopt(argc, argv, "hi:f:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'i':
			snprintf(&inputPath[0], 128, "%s", argv[optind - 1]);
			break;
		case 'f':
			if (atoi(argv[optind - 1]) == 0) {
				format = AYUV;
			} else if (atoi(argv[optind - 1]) == 1) {
				format = FONTAYUV;
			} else if (atoi(argv[optind - 1]) == 2) {
				format = COMPACTFONT;
			} else {
				libosd_log_err("failed to find input format");
				exit(1);
			}
			break;
		default:
			help();
			exit(1);
		}
	}
	if (format == FONTAYUV) {
		getAYUVFONTINFO(inputPath, format);
	} else if (format == AYUV) {
		alignAYUVimage(inputPath, format);
	} else if (format == COMPACTFONT) {
		alignCompactAYUV(inputPath, format);
	}

	return 0;
}