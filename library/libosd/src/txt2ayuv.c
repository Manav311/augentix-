#include "txt2ayuv.h"

#ifdef SDL_ENABLE
int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol)
{
	if (bcolor == WHITE) {
		pbackcol->r = 0xff;
		pbackcol->g = 0xff;
		pbackcol->b = 0xff;
	} else if (bcolor == BLACK) {
		pbackcol->r = 0x00;
		pbackcol->g = 0x00;
		pbackcol->b = 0x00;
	} else if (bcolor == TRANSPARENT) {
		libosd_log_debug("trans background");
		if ((*pfcolor == 0) & ((*(pfcolor + 1) == 0) & (*(pfcolor + 2) == 0))) {
			pbackcol->r = 0xff;
			pbackcol->g = 0xff;
			pbackcol->b = 0xff;
		} else {
			pbackcol->r = 0x00;
			pbackcol->g = 0x00;
			pbackcol->b = 0x00;
		}
	}
	return 0;
}
#else
#endif
int rounding(int a, int b)
{
	int tmp = 0;
	tmp = (a + (b >> 2)) / b;
	return tmp;
}

int alignUVVal(char *ayuv_buf, int width, int height)
{
	libosd_log_debug("get (%d, %d)", width, height);

	int u;
	int v;

	/*4 * 4 normal case*/
	for (int i = 0; i < height - 1; i += 2) {
		for (int j = 0; j < width - 1; j += 2) {
			u = 0;
			u += GETUPPER4BIT(ayuv_buf[(i * width + j) * 2]);
			u += GETUPPER4BIT(ayuv_buf[(i * width + j + 1) * 2]);
			u += GETUPPER4BIT(ayuv_buf[((i + 1) * width + j) * 2]);
			u += GETUPPER4BIT(ayuv_buf[((i + 1) * width + j + 1) * 2]);

			u = (u) >> 2;

			v = 0;
			v += GETLOWER4BIT(ayuv_buf[(i * width + j) * 2]);
			v += GETLOWER4BIT(ayuv_buf[(i * width + j + 1) * 2]);
			v += GETLOWER4BIT(ayuv_buf[((i + 1) * width + j) * 2]);
			v += GETLOWER4BIT(ayuv_buf[((i + 1) * width + j + 1) * 2]);

			v = (v) >> 2;

			ayuv_buf[(i * width + j) * 2] = MERGE44(u, v);
			ayuv_buf[(i * width + j + 1) * 2] = MERGE44(u, v);
			ayuv_buf[((i + 1) * width + j) * 2] = MERGE44(u, v);
			ayuv_buf[((i + 1) * width + j + 1) * 2] = MERGE44(u, v);

			libosd_log_debug("[%d, %d]align (%d, %d, %d, %d)", i, j, (i * width + j) * 2,
			                 (i * width + j + 1) * 2, ((i + 1) * width + j) * 2,
			                 ((i + 1) * width + j + 1) * 2);
		}
	}

	if ((width % 2) == 1) {
		for (int i = 0; i < height - 1; i += 2) {
			if (i > height) {
				libosd_log_err("end at : %d", i);
				break;
			}
			u = GETUPPER4BIT(ayuv_buf[((i * width) + (width - 1)) * 2]);
			u += GETUPPER4BIT(ayuv_buf[((i + 1) * width + (width - 1)) * 2]);
			u = (u) >> 1;
			v = GETLOWER4BIT(ayuv_buf[((i * width) + (width - 1)) * 2]);
			v += GETLOWER4BIT(ayuv_buf[((i + 1) * width + (width - 1)) * 2]);
			v = (v) >> 1;
			ayuv_buf[((i * width) + (width - 1)) * 2] = MERGE44(u, v);
			ayuv_buf[((i + 1) * width + (width - 1)) * 2] = MERGE44(u, v);
			libosd_log_debug("[%d]end width align (%d, %d)", i, ((i * width) + (width - 1)) * 2,
			                 ((i + 1) * width + (width - 1)) * 2);
		}
	}

	if ((height % 2) == 1) {
		for (int j = 0; j < width - 1; j += 2) {
			u = GETUPPER4BIT(ayuv_buf[((height - 1) * width + j) * 2]);
			u += GETUPPER4BIT(ayuv_buf[((height - 1) * width + j + 1) * 2]);
			u = (u) >> 1;

			v = GETLOWER4BIT(ayuv_buf[((height - 1) * width + j) * 2]);
			v += GETLOWER4BIT(ayuv_buf[((height - 1) * width + j + 1) * 2]);
			v = (v) >> 1;
			ayuv_buf[((height - 1) * width + j) * 2] = MERGE44(u, v);
			ayuv_buf[((height - 1) * width + j + 1) * 2] = MERGE44(u, v);
			libosd_log_debug("[%d]end height align (%d, %d)", j, ((height - 1) * width + j) * 2,
			                 ((height - 1) * width + j + 1) * 2);
		}
	}

	return 0;
}

int RGBTrans2YUV(int *pr, int *pg, int *pb, int *py, int *pu, int *pv)
{
	int y, u, v;
	int r, g, b;
	r = *pr;
	g = *pg;
	b = *pb;

	y = (0.183 * 1024) * r + (0.614 * 1024) * g + (0.062 * 1024) * b;
	y = y > -1 ? (y + 512) : (y - 512);
	y = y >> 10;
	y += 16;

	u = (-0.101 * 1024) * r - (0.338 * 1024) * g + (0.439 * 1024) * b;
	u = u > -1 ? (u + 512) : (u - 512);
	u = u >> 10;
	u += 128;

	v = (0.439 * 1024) * r - (0.399 * 1024) * g - (0.04 * 1024) * b;
	v = v > -1 ? (v + 512) : (v - 512);
	v = v >> 10;
	v += 128;

	*py = y;
	*pu = u;
	*pv = v;

	return 0;
}

int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv)
{
	int a, y, u, v;
	a = *pa;
	y = *py;
	u = *pu;
	v = *pv;

	y = (((y * 32) + 128) >> 8);
	u = (((u * 16) + 128) >> 8);
	v = (((v * 16) + 128) >> 8);
	a = (((a * 8) + 128) >> 8);

	a = a > 7 ? 7 : a;
	y = y > 31 ? 31 : y;
	u = u > 15 ? 15 : u;
	v = v > 15 ? 15 : v;

	*pa = a;
	*py = y;
	*pu = u;
	*pv = v;

	return 0;
}

int checkKeyingBackground(char *pfcolor, int *pr, int *pg, int *pb, int *pa)
{
	if ((*pfcolor == 0) & ((*(pfcolor + 1) == 0) & (*(pfcolor + 2) == 0))) {
		if ((*pr >= 0xf8) & ((*pg >= 0xfc) & (*pb >= 0xf8))) {
			*pa = 0x00;
		}
	} else {
		if ((*pr == 0) & ((*pg == 0) & (*pb == 0))) {
			*pa = 0x00;
		}
	}
	return 0;
}

int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize)
{
	/*save ayuv to .imgayuv*/
	FILE *fp_ayuv;
	fp_ayuv = fopen(filePath, "wb+");
	if (fp_ayuv == NULL) {
		libosd_log_err("Failed to open ayuv fp");
		return -EINVAL;
	}
	fwrite(&w, sizeof(uint32_t), 1, fp_ayuv);
	fseek(fp_ayuv, sizeof(uint32_t), SEEK_SET);
	fwrite(&h, sizeof(uint32_t), 1, fp_ayuv);
	fseek(fp_ayuv, sizeof(uint32_t) * 2, SEEK_SET);

	int ayuv_img_size = (w * h) * 2;
	fwrite(&ayuv_img_size, sizeof(uint32_t), 1, fp_ayuv);
	fseek(fp_ayuv, sizeof(uint32_t) * 3, SEEK_SET);
	fwrite(payuvBuf, bufSize, 1, fp_ayuv);
	fclose(fp_ayuv);

	return 0;
}

int getBmpSize(const char *path, int *width, int *height, int *inline_offset)
{
	if (path == NULL) {
		libosd_log_err("path can't be NULL");
		return -EINVAL;
	}

	libosd_log_debug("open: %s", path);
	FILE *fp;
	fp = fopen(path, "rb");
	if (fp == NULL) {
		libosd_log_err("bmp file empty");
		return -EINVAL;
	}
#ifdef OSD_DEBUG
	/*get data offset*/
	char fileheader[BITMAP_FILE_HEADER_LEN];
	BITMAP_FILE_HEADER fileHeader = { 0 };
	fread(&fileheader[0], BITMAP_FILE_HEADER_LEN, 1, fp);
	fileHeader.fileSize = (fileheader[5] << 24) | (fileheader[4] << 16) | (fileheader[3] << 8) | fileheader[2];
	fileHeader.data_offset = (fileheader[13] << 24) | (fileheader[12] << 16) | (fileheader[11] << 8) |
	                         fileheader[10];
	libosd_log_debug("bmp file size: %u, data offset: %u", fileHeader.fileSize, fileHeader.data_offset);
#endif
	fseek(fp, BITMAP_FILE_HEADER_LEN, SEEK_SET);

	char infoheader[BITMAP_INFO_HEADER_LEN];
	BITMAP_INFO_HEADER infoHeader;
	fread(&infoheader[0], BITMAP_INFO_HEADER_LEN, 1, fp);
	infoHeader.header_size = (infoheader[3] << 24) | (infoheader[2] << 16) | (infoheader[1] << 8) | infoheader[0];
	infoHeader.width = (infoheader[7] << 24) | (infoheader[6] << 16) | (infoheader[5] << 8) | infoheader[4];
	infoHeader.height = (infoheader[11] << 24) | (infoheader[10] << 16) | (infoheader[9] << 8) | infoheader[8];
	infoHeader.bit_per_pixel = (infoheader[15] << 8) | infoheader[14];
	infoHeader.compression = (infoheader[19] << 24) | (infoheader[18] << 16) | (infoheader[17] << 8) |
	                         infoheader[16];
	libosd_log_debug("bmp info[%d] (%d, %d):bit : %d, compression type: %u", infoHeader.header_size,
	                 infoHeader.width, infoHeader.height, infoHeader.bit_per_pixel, infoHeader.compression);
	if (infoHeader.compression == 0) {
		libosd_log_debug("no need palette");
	}

	int iline4Offset = 0;
	if ((infoHeader.width * 3) % 4 != 0) {
		iline4Offset = 4 - (infoHeader.width * 3) % 4;
	}
	libosd_log_debug("inline offset: %d", iline4Offset);

	*inline_offset = iline4Offset;
	*width = infoHeader.width;
	*height = infoHeader.height;

	fclose(fp);

	return 0;
}

int readBmp2AYUV(const char *path, char *pfcolor, BACKGROUND_COLOR bcolor, int width, int height, int inline_offset,
                 char *ptr)
{
	if (path == NULL) {
		libosd_log_err("path can't be NULL");
		return -EINVAL;
	}

	libosd_log_info("open: %s", path);
	FILE *fp;
	fp = fopen(path, "rb");
	if (fp == NULL) {
		libosd_log_err("bmp file empty");
		return -EINVAL;
	}

	char rgbBuf[width * 3];
	int r, g, b, a, y, u, v, i, j;
	int ayuv_udx = 0;
	for (i = height - 1; i >= 0; i--) {
		fseek(fp, 54 + (width * 3 + inline_offset) * i, SEEK_SET);
		fread(&rgbBuf[0], width * 3, 1, fp);
		for (j = 0; j < width * 3; j += 3) {
			b = rgbBuf[j + 0];
			g = rgbBuf[j + 1];
			r = rgbBuf[j + 2];
			a = 0xff;
			if (bcolor == TRANSPARENT) {
				checkKeyingBackground(pfcolor, &r, &g, &b, &a);
			}
			y = 0.299 * r + 0.587 * g + 0.114 * b;
			u = -0.147 * r - 0.289 * g + 0.436 * b + 128;
			v = 0.615 * r - 0.515 * g - 0.100 * b + 128;

			normalizeAYUV3544(&a, &y, &u, &v);

			ptr[ayuv_udx] = MERGE44(u, v);
			ayuv_udx += 1;
			ptr[ayuv_udx] = MERGE35(a, y);
			ayuv_udx += 1;
		}
	}

	fclose(fp);
	return 0;
}

int readbmpInfo(const char *path, char *pfcolor, BACKGROUND_COLOR bcolor)
{
	if (path == NULL) {
		libosd_log_err("path can't be NULL");
		return -EINVAL;
	}

	libosd_log_debug("open: %s", path);
	FILE *fp;
	fp = fopen(path, "rb");
	if (fp == NULL) {
		libosd_log_err("bmp file empty");
		return -EINVAL;
	}

	/*get data offset*/
	char fileheader[BITMAP_FILE_HEADER_LEN];
	BITMAP_FILE_HEADER fileHeader = { 0 };
	fread(&fileheader[0], BITMAP_FILE_HEADER_LEN, 1, fp);
	fileHeader.fileSize = (fileheader[5] << 24) | (fileheader[4] << 16) | (fileheader[3] << 8) | fileheader[2];
	fileHeader.data_offset = (fileheader[13] << 24) | (fileheader[12] << 16) | (fileheader[11] << 8) |
	                         fileheader[10];
	libosd_log_debug("bmp file size: %u, data offset: %u", fileHeader.fileSize, fileHeader.data_offset);

	fseek(fp, 0, BITMAP_FILE_HEADER_LEN);

	char infoheader[BITMAP_INFO_HEADER_LEN];
	BITMAP_INFO_HEADER infoHeader;
	fread(&infoheader[0], BITMAP_INFO_HEADER_LEN, 1, fp);
	infoHeader.header_size = (infoheader[3] << 24) | (infoheader[2] << 16) | (infoheader[1] << 8) | infoheader[0];
	infoHeader.width = (infoheader[7] << 24) | (infoheader[6] << 16) | (infoheader[5] << 8) | infoheader[4];
	infoHeader.height = (infoheader[11] << 24) | (infoheader[10] << 16) | (infoheader[9] << 8) | infoheader[8];
	infoHeader.bit_per_pixel = (infoheader[15] << 8) | infoheader[14];
	infoHeader.compression = (infoheader[19] << 24) | (infoheader[18] << 16) | (infoheader[17] << 8) |
	                         infoheader[16];
	libosd_log_debug("bmp info[%d] (%d, %d):bit : %d, compression type: %u", infoHeader.header_size,
	                 infoHeader.width, infoHeader.height, infoHeader.bit_per_pixel, infoHeader.compression);
	if (infoHeader.compression == 0) {
		libosd_log_debug("no need palette");
	}

	int iline4Offset = 0;
	if ((infoHeader.width * 3) % 4 != 0) {
		iline4Offset = 4 - (infoHeader.width * 3) % 4;
	}
	libosd_log_debug("inline offset: %d", iline4Offset);

	fseek(fp, fileHeader.data_offset, SEEK_SET);
	char rgbBuf[infoHeader.width * 3];
	char ayuvBuf[infoHeader.width * infoHeader.height * 2];
	int r, g, b, a, y, u, v, i, j;
	int ayuvIdx = 0;
	fseek(fp, infoHeader.width * 3 + iline4Offset, SEEK_END);

	libosd_log_debug("pfcolor %0x %0x %0x", pfcolor[0], pfcolor[1], pfcolor[2]);
	for (i = infoHeader.height - 1; i >= 0; i--) {
		fseek(fp, fileHeader.data_offset + (infoHeader.width * 3 + iline4Offset) * i, SEEK_SET);
		fread(&rgbBuf[0], infoHeader.width * 3, 1, fp);
		for (j = 0; (uint32_t)j < infoHeader.width * 3; j += 3) {
			b = rgbBuf[j + 0];
			g = rgbBuf[j + 1];
			r = rgbBuf[j + 2];
			a = 0x07;
			if (bcolor == TRANSPARENT) {
				checkKeyingBackground(pfcolor, &r, &g, &b, &a);
			}
			y = 0.299 * r + 0.587 * g + 0.114 * b;
			u = -0.147 * r - 0.289 * g + 0.436 * b + 128;
			v = 0.615 * r - 0.515 * g - 0.100 * b + 128;

			normalizeAYUV3544(&a, &y, &u, &v);

			ayuvBuf[ayuvIdx] = MERGE44(u, v);
			ayuvIdx += 1;
			ayuvBuf[ayuvIdx] = MERGE35(a, y);
			ayuvIdx += 1;
		}
	}
	char ayuvName[64];
	snprintf(&ayuvName[0], 64, "%s.ayuv", path);
	saveAYUV(&ayuvName[0], infoHeader.width, infoHeader.height, &ayuvBuf[0], sizeof(ayuvBuf));

	fclose(fp);
	return 0;
}
#ifdef SDL_ENABLE

int generateTxtBmp(char *fontpath, char *pfcolor, BACKGROUND_COLOR bcolor, char *txt, int size)
{
	TTF_Font *font = NULL;
	SDL_Surface *text = NULL, *tmp = NULL;

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdl2ttf");
		SDL_Quit();
	}

	char ctimestamp[128];

	snprintf(&ctimestamp[0], 128, "%s", txt);

	font = TTF_OpenFont(fontpath, size); /*need dynamic change size*/

	if (font == NULL) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		return -EINVAL;
	}

	SDL_Color forecol = { pfcolor[0], pfcolor[1], pfcolor[2] };
	SDL_Color backcol = { 0x00, 0x00, 0x00 };

	setBackgroundColor(pfcolor, bcolor, &backcol);

	libosd_log_debug("fcolor(%d, %d, %d), bcolor(%d, %d, %d)", forecol.r, forecol.g, forecol.b, backcol.r,
	                 backcol.g, backcol.b);
	text = TTF_RenderUTF8_Shaded(font, ctimestamp, forecol, backcol);
	if (text == NULL) {
		libosd_log_err("Invaild UTF8 word");
	}

	SDL_PixelFormat *fmt;
	fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
	if (fmt == NULL) {
		libosd_log_err("failed to alloc a SDL_PixelFormat");
	}
	memset(fmt, 0, sizeof(SDL_PixelFormat));

	fmt->BitsPerPixel = 16;
	fmt->BytesPerPixel = 2;

	tmp = SDL_ConvertSurface(text, fmt, 0);
	if (tmp == NULL) {
		libosd_log_err("failed convert interface, %s", SDL_GetError());
	} else {
		SDL_SaveBMP(tmp, "/tmp/save.bmp");
	}

	/*save a black text bmp*/
	readbmpInfo("/tmp/save.bmp", pfcolor, bcolor);

	SDL_FreeSurface(text);
	SDL_FreeSurface(tmp);
	TTF_CloseFont(font);
	TTF_Quit();

	return 0;
}
#else
#endif

int get_min_idx(int items[], int num, int *idx)
{
	int tmp;
	tmp = items[0];
	*idx = 0;
	for (int i = 0; i < num; i++) {
		if (tmp > items[i]) {
			tmp = items[i];
			*idx = i;
		}
	}

	return tmp;
}

int get_min(int items[], int num)
{
	int tmp;
	tmp = items[0];
	for (int i = 0; i < num; i++) {
		if (tmp > items[i]) {
			tmp = items[i];
		}
	}

	return tmp;
}

int get_max(int items[], int num)
{
	int tmp;
	tmp = items[0];
	for (int i = 0; i < num; i++) {
		if (tmp < items[i]) {
			tmp = items[i];
		}
	}

	return tmp;
}

void logAllOsdHandle(OsdHandle *phd)
{
	/*this is a log function*/
	printf("(%d, %d)\r\n", phd->width, phd->height);

	for (int i = 0; i < MAX_OSD; i++) {
		printf("region [%d] (%d, %d, %d, %d) \tin %d", i, phd->region[i].startX, phd->region[i].startY,
		       phd->region[i].width, phd->region[i].height, phd->region[i].include_canvas);
	}

	for (int i = 0; i < MAX_CANVAS; i++) {
		printf("canvas[%d](%d, %d, %d, %d) has %d: \t", i, phd->canvas[i].startX, phd->canvas[i].startY,
		       phd->canvas[i].width, phd->canvas[i].height, phd->canvas[i].osd_num);
		if (phd->canvas[i].osd_num != 255) {
			for (int j = 0; (uint32_t)j < phd->canvas[i].osd_num; j++) {
				printf("%d ", phd->canvas[i].osd_list[j]);
			}
		}
		printf("\r\n");
	}
	printf("--------\r\n");
}
