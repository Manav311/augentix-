#ifndef TXT2AYUV_H
#define TXT2AYUV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>

#ifdef SDL_ENABLE
#include "SDL.h"
#include "SDL_video.h"
#include "SDL_ttf.h"
#endif

#include "libosd.h"

#define AYUV_HEADER_SIZE (3 * sizeof(uint32_t))

#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

#define GETUPPER4BIT(x) (x >> 4)
#define GETLOWER4BIT(x) (x & 0x0f)

/*ref https://blog.xuite.net/stitchmos/wretch/117148810*/
#define BITMAP_FILE_HEADER_LEN (14)
typedef struct {
	uint16_t identity; //2
	uint32_t fileSize; //4
	uint16_t reserved1; //2
	uint16_t reserved2; //2
	uint32_t data_offset; //4
} BITMAP_FILE_HEADER;
#define BITMAP_INFO_HEADER_LEN (40)
typedef struct {
	uint32_t header_size; //4
	uint32_t width; //4
	uint32_t height; //4
	uint16_t planes; //2
	uint16_t bit_per_pixel; //2
	uint32_t compression; //4
	uint32_t data_size; //4
	int32_t hresolution; //4
	int32_t vresolution; //4
	uint32_t used_colors; //4
	uint32_t important_colors; //4
} BITMAP_INFO_HEADER;

typedef struct {
	char blue;
	char green;
	char red;
	char reserved;
} PALLETTE;

int setImagePtr(OsdHandle *hd, int osd_idx, const char *img_ptr, int width, int height, char *out);
#ifdef SDL_ENABLE
int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol);
#endif
int alignUVVal(char *ayuv_buf, int width, int height);
int RGBTrans2YUV(int *pr, int *pg, int *pb, int *py, int *pu, int *pv);
int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
int checkKeyingBackground(char *pfcolor, int *pr, int *pg, int *pb, int *pa);
int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
int getBmpSize(const char *path, int *width, int *height, int *inline_offset);
int readBmp2AYUV(const char *path, char *pfcolor, BACKGROUND_COLOR bcolor, int width, int height, int inline_offset,
                 char *ptr);
int readbmpInfo(const char *path, char *pfcolor, BACKGROUND_COLOR bcolor);
#ifdef SDL_ENABLE
int generateTxtBmp(char *fontpath, char *pfcolor, BACKGROUND_COLOR bcolor, char *txt, int size);
#endif
int get_max(int items[], int num);
int get_min(int items[], int num);
int get_min_idx(int items[], int num, int *idx);
void logAllOsdHandle(OsdHandle *phd);

#endif