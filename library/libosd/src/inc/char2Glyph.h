#ifndef CHAR2GLYPH_H
#define CHAR2GLYPH_H
#ifdef SDL_ENABLE
#else
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <malloc.h>
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
#include FT_IMAGE_H

#include "libosd.h"

#define DPI (72)

#define uint8 unsigned char
#define uint16 unsigned short

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

struct Span {
	int x;
	int y;
	int width;
	int coverage;
};

typedef struct Lspan {
	struct Span node;
	struct Lspan *next;
} Node, *PNode;

struct Rect {
	float xmin;
	float xmax;
	float ymin;
	float ymax;
};

struct vec2 {
	float x;
	float y;
};

void Include(const struct vec2 *r, struct Rect *rect);
void RasterCallback(const int y, const int count, const FT_Span *const spans, void *const user);
void RenderSpans(FT_Library *library, FT_Outline *const outline, Node *spans);
int isUTF8orUnicodeSpace(OsdText *txt, int idx, TEXT_MODE text_mode);
unsigned int getUTF8TxtListLen(OsdText *txt);
unsigned int getUnicodeTxtListLen(OsdText *txt);
int calcKerningPixel(OsdText *txt, int size, int kerning);
int setAYUVBackground(OsdText *txt, unsigned int imgWidth, unsigned int imgHeight, char *ayuv_buf);
int setPalette8Background(OsdText *txt, unsigned int imgWidth, unsigned int imgHeight, char *palette_buf);
int monoGraytoARGB(OsdText *txt, char *src, unsigned int w, unsigned int h, char *src_argb);
int monoGraytoAYUV(OsdText *txt, char *src, unsigned int w, unsigned int h, char *ayuv_buf);
int monoGraytoPalette8(OsdText *txt, char *src, unsigned int w, unsigned int h, char *palette8_buf);
int monoGrayChartoList(OsdText *txt, TEXT_MODE text_mode, unsigned int *w, unsigned int *h, unsigned int *char_width,
                       unsigned int *char_height, unsigned int *char_bearingY, char **txt_list_buffer, char *src);
int generateChartoGlyph(OsdText *txt, TEXT_MODE text_mode, FT_Face face, char **txt_list_buffer, unsigned int *w,
                        unsigned int *h, unsigned int *char_width, unsigned int *char_height,
                        unsigned int *char_bearingY);
char *genCharWithOutline(OsdText *txt_info, TEXT_MODE text_mode, uint16_t unicode, unsigned int *imgWidth,
                         unsigned int *imgHeight, unsigned int *bearingY);
char *generatOutlineChartoList(OsdText *txt, TEXT_MODE text_mode, unsigned int *imgWidth, unsigned int *imgHeight);
char *generatNoOutlineChartoList(OsdText *txt, TEXT_MODE text_mode, unsigned int *total_w, unsigned int *total_h);
int assignAYUVList(OsdText *txt, uint16_t *text_list, unsigned int *char_width, unsigned int *char_height,
                   unsigned int *char_bearingY, char **src_in, AyuvSrcList *ayuv_list);
int genAYUVListSrcBitmapNoOutline(OsdText *txt, unsigned int *char_width, unsigned int *char_height,
                                  unsigned int *char_bearingY, char **src_bitmap);
int genAYUVListSrcBitmapWithOutline(OsdText *txt, unsigned int *char_width, unsigned int *char_height,
                                    unsigned int *char_bearingY, char **src_bitmap);

#endif
#endif