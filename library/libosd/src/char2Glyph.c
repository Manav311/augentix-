#include "char2Glyph.h"
#include "drawline.h"
#include "libosd.h"
#include <asm-generic/errno-base.h>

#ifdef SDL_ENABLE
#else

#define TRUE (1)
#define FALSE (0)

int isUTF8orUnicodeSpace(OsdText *txt, int idx, TEXT_MODE text_mode)
{
	if ((text_mode == UTF8) && txt->txt[idx] == 0x20) {
		return TRUE;
	}

	if ((text_mode == UNICODE) &&
	    (txt->unicode_txt[idx] == 0x0020 || txt->unicode_txt[idx] == 0x3000 || txt->unicode_txt[idx] == 0x00a0)) {
		return TRUE;
	}

	return FALSE;
}

int calcKerningPixel(OsdText *txt, int size, int kerning)
{
	if (txt->kerning_mode != MANUAL) {
		txt->kerning_mode = AUTO;
		txt->kerning = AUTO_KERNING_RATE;
		kerning = AUTO_KERNING_RATE;
	}

	int ret = (size >> 4) * kerning;
	libosd_log_debug("get %d", ret);
	return ret;
}

//获取外接矩形
void Include(const struct vec2 *r, struct Rect *rect)
{
	rect->xmin = MIN(rect->xmin, r->x);
	rect->ymin = MIN(rect->ymin, r->y);
	rect->xmax = MAX(rect->xmax, r->x);
	rect->ymax = MAX(rect->ymax, r->y);
}

void RasterCallback(const int y, const int count, const FT_Span *const spans, void *const user)
{
	PNode sptr = (PNode)user;
	while (sptr->next != NULL)
		sptr = sptr->next;
	int i;
	for (i = 0; i < count; ++i) {
		PNode new = calloc(sizeof(Node), 1);
		if (!new) {
			libosd_log_err("failed to alloc new node for PNode");
			return;
		}
		new->next = NULL;
		new->node.x = spans[i].x;
		new->node.y = y;
		new->node.width = spans[i].len;
		new->node.coverage = 255;
		sptr->next = new;
		sptr = sptr->next;
	}
}

void RenderSpans(FT_Library *library, FT_Outline *const outline, Node *spans)
{
	FT_Raster_Params params;
	memset(&params, 0, sizeof(params));
	params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
	params.gray_spans = RasterCallback;
	params.user = spans;
	FT_Outline_Render(*library, outline, &params);
}

unsigned int getUTF8TxtListNextCharPtrLen(char *txt_ptr)
{
	unsigned int ret = 0;
	int i = 0;
	if (txt_ptr[i] != '\0') {
		if ((txt_ptr[i] > 0xf0) && (txt_ptr[i] <= 0xf7)) {
			ret = 4;
		} else if ((txt_ptr[i] > 0xe0) && (txt_ptr[i] <= 0xef)) {
			ret = 3;
		} else if ((txt_ptr[i] > 0xc0) && (txt_ptr[i] <= 0xdf)) {
			ret = 2;
		} else if ((txt_ptr[i] <= 0x7f)) {
			ret = 1;
		}
		i++;
	}

	return ret;
}

int encOneUTF8toUnicode(const unsigned char *pInput, unsigned long *Unic)
{
	char b1, b2, b3, b4, b5, b6;

	*Unic = 0x0;
	int utfbytes = getUTF8TxtListNextCharPtrLen((char *)pInput);
	unsigned char *pOutput = (unsigned char *)Unic;

	switch (utfbytes) {
	case 0:
		*pOutput = *pInput;
		utfbytes += 1;
		break;
	case 2:
		b1 = *pInput;
		b2 = *(pInput + 1);
		if ((b2 & 0xE0) != 0x80)
			return 0;
		*pOutput = (b1 << 6) + (b2 & 0x3F);
		*(pOutput + 1) = (b1 >> 2) & 0x07;
		break;
	case 3:
		b1 = *pInput;
		b2 = *(pInput + 1);
		b3 = *(pInput + 2);
		if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80))
			return 0;
		*pOutput = (b2 << 6) + (b3 & 0x3F);
		*(pOutput + 1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
		break;
	case 4:
		b1 = *pInput;
		b2 = *(pInput + 1);
		b3 = *(pInput + 2);
		b4 = *(pInput + 3);
		if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80))
			return 0;
		*pOutput = (b3 << 6) + (b4 & 0x3F);
		*(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
		*(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
		break;
	case 5:
		b1 = *pInput;
		b2 = *(pInput + 1);
		b3 = *(pInput + 2);
		b4 = *(pInput + 3);
		b5 = *(pInput + 4);
		if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80))
			return 0;
		*pOutput = (b4 << 6) + (b5 & 0x3F);
		*(pOutput + 1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
		*(pOutput + 2) = (b2 << 2) + ((b3 >> 4) & 0x03);
		*(pOutput + 3) = (b1 << 6);
		break;
	case 6:
		b1 = *pInput;
		b2 = *(pInput + 1);
		b3 = *(pInput + 2);
		b4 = *(pInput + 3);
		b5 = *(pInput + 4);
		b6 = *(pInput + 5);
		if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) ||
		    ((b6 & 0xC0) != 0x80))
			return 0;
		*pOutput = (b5 << 6) + (b6 & 0x3F);
		*(pOutput + 1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
		*(pOutput + 2) = (b3 << 2) + ((b4 >> 4) & 0x03);
		*(pOutput + 3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
		break;
	default:
		return 0;
		break;
	}

	return utfbytes;
}

unsigned int getUTF8TxtListCharNum(OsdText *txt)
{
	unsigned int num = 0;

	int i = 0;
	while (txt->txt[i] != '\0' && i < MAX_TXT_LEN) {
		if ((txt->txt[i] > 0xf0) && (txt->txt[i] <= 0xf7)) {
			libosd_log_debug("4 byte UTF8 char, %0x", txt->txt[i]);
			num += 1;
			i += 4;
		} else if ((txt->txt[i] > 0xe0) && (txt->txt[i] <= 0xef)) {
			libosd_log_debug("3 byte UTF8 char, %0x", txt->txt[i]);
			num += 1;
			i += 3;
		} else if ((txt->txt[i] > 0xc0) && (txt->txt[i] <= 0xdf)) {
			libosd_log_debug("2 byte UTF8 char, %0x", txt->txt[i]);
			num += 1;
			i += 2;
		} else if (txt->txt[i] < 0x7f) {
			libosd_log_debug("1 byte UTF8 char, %0x", txt->txt[i]);
			num += 1;
			i += 1;
		}
	}

	return num;
}

unsigned int getUTF8TxtListLen(OsdText *txt)
{

	return getUTF8TxtListCharNum(txt);

}

unsigned int getUnicodeTxtListLen(OsdText *txt)
{
	unsigned int i;
	for (i = 0; i < MAX_TXT_LEN; i++) {
		if (txt->unicode_txt[i] == 0x00)
			break;
	}

	return i;
}

static int setSpaceBitmapInfo(unsigned int *maxheight, OsdText *txt, unsigned int *width, unsigned int *height,
                              unsigned int *bearingY)
{
	*width = txt->size / 4;
	*height = *maxheight;
	*bearingY = *maxheight;

	libosd_log_debug("space width: %d,%d", *width, *maxheight);

	return 0;
}

int setPalette8Background(OsdText *txt, unsigned int imgWidth, unsigned int imgHeight, char *palette_buf)
{
	int a = 0xff;
	char background_color = 0x00;
	if (txt->background == TRANSPARENT) {
		background_color = MERGE35(0, 0x00);
	} else if (txt->background == WHITE) {
		background_color = MERGE35(a, 0xff);
	} else if (txt->background == BLACK) {
		background_color = MERGE35(a, 0x00);
	}

	memset(palette_buf, background_color, imgWidth * imgHeight);

	return 0;
}

int setAYUVBackground(OsdText *txt, unsigned int imgWidth, unsigned int imgHeight, char *ayuv_buf)
{
	int a = 0xff;
	int r = 0;
	int g = 0;
	int b = 0;
	int y = 0;
	int u = 0;
	int v = 0;

	/*background color*/
	if (txt->background == TRANSPARENT) {
		a = 0x00;
	}

	if (txt->background == BLACK) {
		r = 0x00;
		g = 0x00;
		b = 0x00;
	}

	if (txt->background == WHITE) {
		r = 0xff;
		g = 0xff;
		b = 0xff;
	}
	RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
	normalizeAYUV3544(&a, &y, &u, &v);
	for (unsigned int i = 0; i < imgWidth; i++) {
		libosd_log_debug("[%d]%d. %d", i, MERGE44(u, v), MERGE35(a, y));
		for (unsigned int j = 0; j < imgHeight; j++) {
			ayuv_buf[(imgWidth * j + i) * 2 + 0] = MERGE44(u, v);
			ayuv_buf[(imgWidth * j + i) * 2 + 1] = MERGE35(a, y);
		}
	}

	return 0;
};

int setPalette8(OsdText *txt, PNode ptr, struct Rect rect, int imgWidth, int imgHeight, PNode olsph, PNode sph,
                char *palette_buf)
{
	setPalette8Background(txt, imgWidth, imgHeight, palette_buf);

	/*outline color*/
	for (ptr = olsph; ptr != NULL; ptr = ptr->next) {
		for (int w = 0; w < ptr->node.width; ++w) {
			/*OUTLINE COLOR*/
			unsigned int m = (unsigned int)((imgHeight - 1 - (ptr->node.y - rect.ymin)) * imgWidth +
			                                ptr->node.x - rect.xmin + w);

			palette_buf[m] = MERGE35(txt->outline_color[0], txt->outline_color[1]);
		}
	}

	/*text color*/
	for (ptr = sph; ptr != NULL; ptr = ptr->next) {
		for (int w = 0; w < ptr->node.width; ++w) {
			/*txt color*/
			int m = (int)((imgHeight - 1 - (ptr->node.y - rect.ymin)) * imgWidth + ptr->node.x - rect.xmin +
			              w);
			palette_buf[m] = MERGE35(txt->color[0], txt->color[1]);
		}
	}

	return 0;
}

int setAYUV(OsdText *txt, PNode ptr, struct Rect rect, int imgWidth, int imgHeight, PNode olsph, PNode sph,
            char *ayuv_buf)
{
	int a = 0xff;
	int r = 0;
	int g = 0;
	int b = 0;
	int y = 0;
	int u = 0;
	int v = 0;

	setAYUVBackground(txt, imgWidth, imgHeight, ayuv_buf);

	/*outline color*/
	a = 0xff;
	r = txt->outline_color[0];
	g = txt->outline_color[1];
	b = txt->outline_color[2];
	RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
	normalizeAYUV3544(&a, &y, &u, &v);
	for (ptr = olsph; ptr != NULL; ptr = ptr->next) {
		for (int w = 0; w < ptr->node.width; ++w) {
			/*OUTLINE COLOR*/
			unsigned int m = (unsigned int)((imgHeight - 1 - (ptr->node.y - rect.ymin)) * imgWidth +
			                                ptr->node.x - rect.xmin + w);

			ayuv_buf[m * 2 + 0] = MERGE44(u, v);
			ayuv_buf[m * 2 + 1] = MERGE35(a, y);
		}
	}
	/*text color*/
	a = 0xff;
	r = txt->color[0];
	g = txt->color[1];
	b = txt->color[2];
	RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
	normalizeAYUV3544(&a, &y, &u, &v);
	for (ptr = sph; ptr != NULL; ptr = ptr->next) {
		for (int w = 0; w < ptr->node.width; ++w) {
			/*txt color*/
			int m = (int)((imgHeight - 1 - (ptr->node.y - rect.ymin)) * imgWidth + ptr->node.x - rect.xmin +
			              w);
			ayuv_buf[m * 2 + 0] = MERGE44(u, v);
			ayuv_buf[m * 2 + 1] = MERGE35(a, y);
		}
	}

	alignUVVal(ayuv_buf, imgWidth, imgHeight);

	return 0;
}

int setARGB(char *src_argb, OsdText *txt, PNode ptr, struct Rect rect, int imgWidth, int imgHeight, PNode olsph,
            PNode sph)
{
	for (int i = 0; i < imgWidth * imgHeight; i++) {
		/*background color*/
		src_argb[4 * i] = 0xff;
		src_argb[4 * i + 1] = 0xff;
		src_argb[4 * i + 2] = 0xff;
		src_argb[4 * i + 3] = 0xff;
	}

	for (ptr = olsph; ptr != NULL; ptr = ptr->next) {
		for (int w = 0; w < ptr->node.width; ++w) {
			/*OUTLINE COLOR*/
			unsigned int m = (unsigned int)((imgHeight - 1 - (ptr->node.y - rect.ymin)) * imgWidth +
			                                ptr->node.x - rect.xmin + w);
			src_argb[4 * m] = 0xff;
			src_argb[4 * m + 1] = txt->outline_color[0];
			src_argb[4 * m + 2] = txt->outline_color[1];
			src_argb[4 * m + 3] = txt->outline_color[2];
		}
	}

	for (ptr = sph; ptr != NULL; ptr = ptr->next) {
		for (int w = 0; w < ptr->node.width; ++w) {
			/*txt color*/
			int m = (int)((imgHeight - 1 - (ptr->node.y - rect.ymin)) * imgWidth + ptr->node.x - rect.xmin +
			              w);
			src_argb[4 * m] = 0xff;
			src_argb[4 * m + 1] = txt->color[0];
			src_argb[4 * m + 2] = txt->color[1];
			src_argb[4 * m + 3] = txt->color[2];
		}
	}

	return 0;
}

int monoGraytoARGB(OsdText *txt, char *src, unsigned int w, unsigned int h, char *src_argb)
{
	int a = 0x00;
	if (txt->background != TRANSPARENT) {
		a = 0xff;
	}

	/*this part need to change to AYUV*/

	for (unsigned int i = 0; i < w; i++) {
		for (unsigned int j = 0; j < h; j++) {
			if (*(src + w * j + i) != 0x00) {
				src_argb[(w * j + i) * 4 + 0] = a;
				src_argb[(w * j + i) * 4 + 1] = txt->color[0];
				src_argb[(w * j + i) * 4 + 2] = txt->color[1];
				src_argb[(w * j + i) * 4 + 3] = txt->color[2];
			} else {
				src_argb[(w * j + i) * 4 + 0] = a;
				src_argb[(w * j + i) * 4 + 1] = 0xff;
				src_argb[(w * j + i) * 4 + 2] = 0xff;
				src_argb[(w * j + i) * 4 + 3] = 0xff;
			}
		}
	}

#if 0
	char tmpfile[32];
	snprintf(&tmpfile[0], 32, "./%s.argb", "list-color");
	FILE *fp = fopen(tmpfile, "w");
	fwrite(src_argb, 1, w * h * 4, fp);
	fclose(fp);
#endif
	return 0;
}

int monoGraytoPalette8(OsdText *txt, char *src, unsigned int w, unsigned int h, char *palette8_buf)
{
	/*setPalette8Background*/
	setPalette8Background(txt, w, h, palette8_buf);

	for (unsigned int i = 0; i < w; i++) {
		for (unsigned int j = 0; j < h; j++) {
			if (src[w * j + i] != 0x00) {
				/*text color*/
				palette8_buf[j * w + i] = MERGE35(txt->color[0], txt->color[1]);
			}
		}
	}

	libosd_log_debug("text color: %0x", MERGE35(txt->color[0], txt->color[1]));

	return 0;
}

int monoGraytoAYUV(OsdText *txt, char *src, unsigned int w, unsigned int h, char *ayuv_buf)
{
	int a = 0xff;

	/*this part need to change to AYUV*/
	int r, g, b, y, u, v;

	setAYUVBackground(txt, w, h, ayuv_buf);
#if 0
	char tmpfile[32];
	snprintf(&tmpfile[0], 32, "./%s.8bit", "list-monoGraytoAYUV");
	FILE *fp = fopen(tmpfile, "w");
	fwrite(src, 1, w * h, fp);
	fclose(fp);
#endif

	for (unsigned int i = 0; i < w; i++) {
		for (unsigned int j = 0; j < h; j++) {
			if (src[w * j + i] != 0) {
				/*text color*/
				r = txt->color[0];
				g = txt->color[1];
				b = txt->color[2];
				a = 0xff;

				RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
				normalizeAYUV3544(&a, &y, &u, &v);

				ayuv_buf[(w * j + i) * 2 + 0] = MERGE44(u, v);
				ayuv_buf[(w * j + i) * 2 + 1] = MERGE35(a, y);
			}
		}
	}

	alignUVVal(ayuv_buf, w, h);
	return 0;
}

int monoGrayChartoList(OsdText *txt, TEXT_MODE text_mode, unsigned int *w, unsigned int *h, unsigned int *char_width,
                       unsigned int *char_height, unsigned int *char_bearingY, char **txt_list_buffer, char *src)
{
	int w_cursor = 0;
	int h_cursor = 0;
	int is_jgpqy = 0;
	unsigned int ref_bearingY = 0;
	unsigned int max_bearingY_to_origin = 0;
	int utf8_char_offset = 0;
	uint16_t tmp_enc_unicode = 0x0000;

	int text_len = 0;
	if (text_mode == UTF8) {
		text_len = getUTF8TxtListLen(txt);
	} else if (text_mode == UNICODE) {
		text_len = getUnicodeTxtListLen(txt);
	}
	libosd_log_debug("text len %d, type %d, ", text_len, text_mode);

	/*if JPQYG case, need enlarge height*/
	for (int i = 0; i < text_len; i++) {
		if (text_mode == UTF8) {
			if (getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]) == 1) {
				tmp_enc_unicode = txt->txt[utf8_char_offset];
			} else {
				encOneUTF8toUnicode((unsigned char *)&txt->txt[utf8_char_offset],
				                    (unsigned long *)&tmp_enc_unicode);
			}

			utf8_char_offset += getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]);
		} else if (text_mode == UNICODE) {
			tmp_enc_unicode = txt->unicode_txt[i];
		}

		if ((tmp_enc_unicode == 0x006a /*j*/ || tmp_enc_unicode == 0x0070 /*p*/ ||
		     tmp_enc_unicode == 0x0071 /*q*/ || tmp_enc_unicode == 0x0079 /*y*/ ||
		     tmp_enc_unicode == 0x0067 /*g*/ || tmp_enc_unicode == 0xff4a || tmp_enc_unicode == 0xff50 ||
		     tmp_enc_unicode == 0xff59 || tmp_enc_unicode == 0xff51 || tmp_enc_unicode == 0xff57)) {
			is_jgpqy = 1;
			break;
		}
	}
	for (int i = 0; i < text_len; i++) {
		if (char_bearingY[i] > ref_bearingY) {
			ref_bearingY = char_bearingY[i];
		}
	}
	libosd_log_info("get ref bearing y :%d", ref_bearingY);

	if (is_jgpqy) {
		for (int i = 0; i < text_len; i++) {
			if (char_height[i] > char_bearingY[i]) {
				max_bearingY_to_origin = max_bearingY_to_origin > char_height[i] - char_bearingY[i] ?
				                                 max_bearingY_to_origin :
				                                 char_height[i] - char_bearingY[i];
				libosd_log_debug("change max_bearingY_to_origin to: %d", max_bearingY_to_origin);
			}
		}

		if (max_bearingY_to_origin == 0) {
			max_bearingY_to_origin = *h;
		}

		free(src);
		src = malloc(*w * (ref_bearingY + max_bearingY_to_origin));
		if (src == NULL) {
			libosd_log_err("failed to re-alloc src");
			return -ENOSR;
		}
		memset(src, 0x00, *w * (ref_bearingY + max_bearingY_to_origin));
		libosd_log_debug("Change height to : %d", (ref_bearingY + max_bearingY_to_origin));
	} else {
		memset(src, 0x00, (*w) * (*h));
	}

	/*copy each char to list pixmap*/
	for (int i = 0; i < text_len; i++) {
		libosd_log_debug("[%d]%d %d %d", i, (int)char_width[i], (int)(char_width[i]), (int)(char_bearingY[i]));
		h_cursor = ref_bearingY - char_bearingY[i];

		if (h_cursor + char_height[i] > *h && !is_jgpqy) {
			libosd_log_info("h_cursor: %d + char_height[i]: %d > h:%d", h_cursor, char_height[i], *h);
			h_cursor = *h - char_height[i];
		}

		if (h_cursor + char_height[i] > (ref_bearingY + max_bearingY_to_origin) && is_jgpqy) {
			libosd_log_info("[jgpqy case]h_cursor: %d + char_height[i]: %d > h:%d", h_cursor,
			                char_height[i], *h);
			h_cursor = (ref_bearingY + max_bearingY_to_origin) - char_height[i];
		}

		for (unsigned int j = 0; j < char_height[i]; j++) {
			memcpy((void *)src + *w * (j + h_cursor) + w_cursor,
			       (const void *)(txt_list_buffer[i] + j * (char_width[i])), (char_width[i]));
		}
		w_cursor += (char_width[i]) + calcKerningPixel(txt, txt->size, txt->kerning);
		if (txt_list_buffer[i] != NULL) {
			free(txt_list_buffer[i]);
		}
	}

	if (is_jgpqy == 1) {
		*h = (ref_bearingY + max_bearingY_to_origin);
		libosd_log_info("is jgpqy: h= %d", *h);
	}

#if 0
	char tmpfile[32];
	snprintf(&tmpfile[0], 32, "./%s.8bit", "list-monoGrayChartoList");
	FILE *fp = fopen(tmpfile, "w");
	fwrite(src, 1, *w * *h, fp);
	fclose(fp);
#endif

	return 0;
}

int generateChartoGlyph(OsdText *txt, TEXT_MODE text_mode, FT_Face face, char **txt_list_buffer, unsigned int *w,
                        unsigned int *h, unsigned int *char_width, unsigned int *char_height,
                        unsigned int *char_bearingY)
{
	FT_Bitmap bitmap;
	unsigned int text_len = 0;
	if (text_mode == UTF8) {
		text_len = getUTF8TxtListLen(txt);
	} else if (text_mode == UNICODE) {
		text_len = getUnicodeTxtListLen(txt);
	}

	int utf8_char_offset = 0;

	for (unsigned int i = 0; i < text_len; i++) {
		if (isUTF8orUnicodeSpace(txt, i, text_mode)) {
			libosd_log_info("Char[%d] is space", i);
			/*space Must do after get max height*/
			setSpaceBitmapInfo(h, txt, &char_width[i], &char_height[i], &char_bearingY[i]);
			txt_list_buffer[i] = malloc(char_width[i] * char_height[i]);
			if (txt_list_buffer[i] == NULL) {
				libosd_log_err("failed to alloc glyph bitmap src");
				return -ENOSR;
			}
			memset((void *)txt_list_buffer[i], 0x00, char_width[i] * char_height[i]);
			*w += char_width[i];
			*w += calcKerningPixel(txt, txt->size, txt->kerning);

			libosd_log_debug("[%d]w now = %d, %d %d", i, *w, *(char_width + i), *(char_height + i));
			utf8_char_offset += getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]);
			continue;
		} else {
			if (text_mode == UTF8) {
				if (getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]) == 1) {
					/*0x00 - 0x7f include in ASCII, NO need to encode*/
					FT_Load_Char(face, (FT_ULong)txt->txt[utf8_char_offset], FT_LOAD_RENDER);
				} else {
					uint16_t enc_unicode = 0x0000;
					encOneUTF8toUnicode((unsigned char *)&txt->txt[utf8_char_offset],
					                    (unsigned long *)&enc_unicode);
					libosd_log_debug("[%d]load : %0x, enc unicode: %#x", i,
					                 txt->txt[utf8_char_offset], (uint16_t)enc_unicode);
					FT_Load_Char(face, enc_unicode, FT_LOAD_RENDER);
					libosd_log_debug("get %d", FT_Get_Char_Index(face, (FT_ULong)enc_unicode));
				}

				utf8_char_offset += getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]);

				libosd_log_debug("increase %d, i= %d, next start: %0x", utf8_char_offset, i,
				                 txt->txt[utf8_char_offset]);
			} else if (text_mode == UNICODE) {
				FT_Load_Char(face, txt->unicode_txt[i], FT_LOAD_RENDER);
			}

			bitmap = face->glyph->bitmap;

			libosd_log_debug("[%d] row = %d, h / 64 = %ld underline hori: %ld", i, bitmap.rows,
			                 face->glyph->metrics.height / 64, face->glyph->metrics.horiBearingY / 64);
			if (bitmap.rows == 0 && face->glyph->metrics.height / 64 == 0) {
				libosd_log_err("can't search [%d] char", i);
				setSpaceBitmapInfo(h, txt, &char_width[i], &char_height[i], &char_bearingY[i]);
				txt_list_buffer[i] = malloc(char_width[i] * char_height[i]);
				memset((void *)txt_list_buffer[i], 0x00, char_width[i] * char_height[i]);
				*w += char_width[i];
				*w += calcKerningPixel(txt, txt->size, txt->kerning);

			} else {
				txt_list_buffer[i] = malloc(bitmap.width * bitmap.rows);
			}

			if (txt_list_buffer[i] == NULL) {
				libosd_log_debug("failed to alloc[%d]", i);
				break;
			}

			memcpy((void *)txt_list_buffer[i], &(bitmap.buffer[0]), bitmap.rows * bitmap.width);

			*w += bitmap.width;
			*w += calcKerningPixel(txt, txt->size, txt->kerning);

			*h = (*h > bitmap.rows ? *h : bitmap.rows);
			char_width[i] = bitmap.width;
			char_height[i] = bitmap.rows;
			char_bearingY[i] = face->glyph->metrics.horiBearingY / 64;
			libosd_log_debug("[%d]w now = %d, %d %d", i, *w, char_width[i], char_height[i]);
		}

#if 0
		char tmpfile[32];
		snprintf(&tmpfile[0], 32, "./%d-ChartoGlyph.8bit", i);
		FILE *fp = fopen(tmpfile, "w");
		fwrite(txt_list_buffer[i], 1, bitmap.rows * bitmap.width, fp);
		fclose(fp);
#endif
	}

	return 0;
}

char *genCharWithOutline(OsdText *txt_info, TEXT_MODE text_mode, uint16_t unicode, unsigned int *imgWidth,
                         unsigned int *imgHeight, unsigned int *bearingY)
{
	FT_Library library;
	FT_Face face;
	FT_Error error;
	FT_Glyph glyph;
	FT_GlyphSlot glyphSlot;
	FT_Stroker stroker;
	PNode sp = NULL;
	PNode olsp = NULL;
	PNode sph = NULL;
	PNode olsph = NULL;

	char *src_out = NULL;
	unsigned int w_width = 0;
	unsigned int w_height = 0;

	error = FT_Init_FreeType(&library); /* initialize library */
	if (error) {
		libosd_log_debug("FT_Init_FreeType failed");
		return NULL;
	}

	error = FT_New_Face(library, txt_info->ttf_path, 0, &face); /* create face object */
	if (error) {
		libosd_log_debug("FT_New_Face failed");
		FT_Done_FreeType(library);
		return NULL;
	}

	FT_Set_Char_Size(face, 0, txt_info->size << 6 /* x MAX_TXT_LEN*/, DPI, DPI);

	if (text_mode == UTF8) {
		error = FT_Load_Char(face, unicode, FT_LOAD_NO_BITMAP);
	} else if (text_mode == UNICODE) {
		error = FT_Load_Char(face, unicode, FT_LOAD_NO_BITMAP);
	}

	glyphSlot = face->glyph;
	if (glyphSlot->format != FT_GLYPH_FORMAT_OUTLINE) {
		libosd_log_debug("format is not outline");
		return NULL;
	}

	sp = (PNode)malloc(sizeof(Node));
	if (!sp) {
		libosd_log_debug("failed to malloc Node");
	}

	memset(&(sp->node), 0, sizeof(struct Span));
	sp->next = NULL;
	RenderSpans(&library, &(glyphSlot->outline), sp);

	olsp = (PNode)malloc(sizeof(Node));
	if (!olsp) {
		libosd_log_debug("failed to malloc olsp");
		return NULL;
	}
	memset(&(olsp->node), 0, sizeof(struct Span));
	olsp->next = NULL;

	error = FT_Stroker_New(library, &stroker);
	if (error) {
		libosd_log_err("FT_Stroker_New: %s", FT_Error_String(error));
		free(olsp);
		return NULL;
	}

	FT_Stroker_Set(stroker, txt_info->outline_width * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

	error = FT_Get_Glyph(glyphSlot, &glyph);
	if (error) {
		libosd_log_err("FT_Get_Glyph:%s", FT_Error_String(error));
		free(olsp);
		return NULL;
	}

	error = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
	if (error) {
		libosd_log_err("FT_Glyph_StrokeBorder: %s", FT_Error_String(error));
		free(olsp);
		return NULL;
	}

	// Again, this needs to be an outline to work.
	if (glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		// Render the outline spans to the span list
		FT_Outline *o = &(((FT_OutlineGlyph)glyph)->outline);
		libosd_log_debug("stroke contours are %d, npoints are %d", o->n_contours, o->n_points);
		RenderSpans(&library, o, olsp);
	}
	// Clean up afterwards.

	libosd_log_debug("[%0x] h / 64 = %ld underline hori: %ld", unicode, glyphSlot->metrics.height / 64,
	                 glyphSlot->metrics.horiBearingY / 64);
	*bearingY = glyphSlot->metrics.horiBearingY / 64;

	FT_Stroker_Done(stroker);
	FT_Done_Glyph(glyph);

	// Now we need to put it all together.

	struct Rect rect;
	memset(&rect, 0, sizeof(struct Rect));
	PNode ptr = NULL;

	sph = sp->next;
	olsph = olsp->next;
	if (!sph || !olsph) {
		libosd_log_debug("[%0x]!sph || !olsph, set as space", unicode);
		goto gen_space;
	}

	rect.xmin = sph->node.x;
	rect.xmax = sph->node.y;
	rect.ymin = sph->node.x;
	rect.ymax = sph->node.y;
	//获取实际图形的外接矩形
	for (ptr = sph; ptr != NULL; ptr = ptr->next) {
		struct vec2 vc1, vc2;
		vc1.x = ptr->node.x;
		vc1.y = ptr->node.y;
		vc2.x = ptr->node.x + ptr->node.width - 1;
		vc2.y = ptr->node.y;

		Include(&vc1, &rect);
		Include(&vc2, &rect);
	}
	//获取加宽轮廓图形的外接矩形
	for (ptr = olsph; ptr != NULL; ptr = ptr->next) {
		struct vec2 vc1, vc2;
		vc1.x = ptr->node.x;
		vc1.y = ptr->node.y;
		vc2.x = ptr->node.x + ptr->node.width - 1;
		vc2.y = ptr->node.y;

		Include(&vc1, &rect);
		Include(&vc2, &rect);
	}
	//计算实际图形大小
	*imgWidth = MAX(w_width, rect.xmax - rect.xmin + 1);
	*imgHeight = MAX(w_height, rect.ymax - rect.ymin + 1);
	libosd_log_debug("imgHeight=%d", *imgHeight);
	libosd_log_debug("imgWidth=%d", *imgWidth);

	if (txt_info->mode == AYUV_3544) {
		src_out = malloc((*imgWidth) * (*imgHeight) * 2);
		if (src_out == NULL) {
			libosd_log_err("failed to alloc src argb");
			return NULL;
		}
		setAYUV(txt_info, ptr, rect, *imgWidth, *imgHeight, olsph, sph, src_out);

#if 0
		char tmpfile[32];
		snprintf(&tmpfile[0], 32, "./%0x-outline.ayuv", unicode);
		saveAYUV(&tmpfile[0], *imgWidth, *imgHeight, src_out, (*imgWidth) * (*imgHeight) * 2);
#endif
	} else if (txt_info->mode == PALETTE_8) {
		src_out = malloc((*imgWidth) * (*imgHeight));
		if (src_out == NULL) {
			libosd_log_err("failed to alloc src argb");
			return NULL;
		}
		/*setPalette8*/
		setPalette8(txt_info, ptr, rect, *imgWidth, *imgHeight, olsph, sph, src_out);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	PNode tmp;
	while (olsp != NULL) {
		tmp = olsp;
		olsp = olsp->next;
		free(tmp);
	}

	while (sp != NULL) {
		tmp = sp;
		sp = sp->next;
		free(tmp);
	}

	return src_out;

gen_space:

	return src_out;
}

char *generatOutlineChartoList(OsdText *txt, TEXT_MODE text_mode, unsigned int *imgWidth, unsigned int *imgHeight)
{
	unsigned int w_cursor = 0;
	unsigned int h_cursor = 0;

	unsigned int text_len = 0;
	if (text_mode == UTF8) {
		text_len = getUTF8TxtListLen(txt);
	} else if (text_mode == UNICODE) {
		text_len = getUnicodeTxtListLen(txt);
	}
	libosd_log_debug("text len: %d", text_len);

	char *txt_list_buffer[MAX_TXT_LEN];
	unsigned int char_width[MAX_TXT_LEN];
	unsigned int char_height[MAX_TXT_LEN];
	unsigned int char_bearingY[MAX_TXT_LEN];

	unsigned int w = 0;
	unsigned int h = 0;
	unsigned int bearingY = 0;
	unsigned int max_bearingY_to_origin = 0;
	int utf8_char_offset = 0;
	uint16_t enc_unicode = 0x6587;

	if (txt->kerning_mode != MANUAL) {
		txt->kerning_mode = AUTO;
		txt->kerning = AUTO_KERNING_RATE;
	}

	for (unsigned int i = 0; i < text_len; i++) {
		if (isUTF8orUnicodeSpace(txt, i, text_mode)) {
			if (text_mode == UTF8) {
				utf8_char_offset += 1;
			}
			continue;
		} else {
			if (text_mode == UTF8) {
				if (getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]) == 1) {
					enc_unicode = txt->txt[utf8_char_offset];
				} else {
					encOneUTF8toUnicode((unsigned char *)&txt->txt[utf8_char_offset],
					                    (unsigned long *)&enc_unicode);
				}

				txt_list_buffer[i] = genCharWithOutline(txt, text_mode, enc_unicode, &w, &h, &bearingY);
				utf8_char_offset += getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]);
			} else if (text_mode == UNICODE) {
				txt_list_buffer[i] =
				        genCharWithOutline(txt, text_mode, txt->unicode_txt[i], &w, &h, &bearingY);
			}

			/*can't search in ttf/otf */
			if (txt_list_buffer[i] == NULL) {
				libosd_log_err("Can't search: %d in font library", i);
				continue;
			}

			libosd_log_debug("w, h = %d, %d", w, h);

			char_width[i] = w;
			char_height[i] = h;
			char_bearingY[i] = bearingY;

			*imgWidth += w + calcKerningPixel(txt, txt->size, txt->kerning);
			*imgHeight = h > *imgHeight ? h : *imgHeight;
		}
	}

	for (unsigned int i = 0; i < text_len; i++) {
		if (isUTF8orUnicodeSpace(txt, i, text_mode)) {
			libosd_log_info("[%d] is space", i);
			/*Space case == set backgrround color*/
			setSpaceBitmapInfo(imgHeight, txt, &char_width[i], &char_height[i], &char_bearingY[i]);
			*imgWidth += char_width[i] + calcKerningPixel(txt, txt->size, txt->kerning);
			/*Add color*/
			if (txt->mode == AYUV_3544) {
				txt_list_buffer[i] = malloc(char_width[i] * char_height[i] * 2);
				setAYUVBackground(txt, char_width[i], char_height[i], txt_list_buffer[i]);
			} else if (txt->mode == PALETTE_8) {
				txt_list_buffer[i] = malloc(char_width[i] * char_height[i]);
				setPalette8Background(txt, char_width[i], char_height[i], txt_list_buffer[i]);
			}
		}
	}

	for (unsigned int i = 0; i < text_len; i++) {
		/*can't search in ttf/otf */
		if (txt_list_buffer[i] == NULL) {
			libosd_log_err("Can't search: %d in font library, set as space", i);
			setSpaceBitmapInfo(imgHeight, txt, &char_width[i], &char_height[i], &char_bearingY[i]);
			*imgWidth += char_width[i] + calcKerningPixel(txt, txt->size, txt->kerning);
			/*Add color*/
			if (txt->mode == AYUV_3544) {
				txt_list_buffer[i] = malloc(char_width[i] * char_height[i] * 2);
				setAYUVBackground(txt, char_width[i], char_height[i], txt_list_buffer[i]);
			} else if (txt->mode == PALETTE_8) {
				txt_list_buffer[i] = malloc(char_width[i] * char_height[i]);
				setPalette8Background(txt, char_width[i], char_height[i], txt_list_buffer[i]);
			}
		}
	}
#if 0
	for (unsigned int i = 0; i < text_len; i++) {
		char tmpfile[32];
		snprintf(&tmpfile[0], 32, "./%d.ayuv", i);
		saveAYUV(&tmpfile[0], char_width[i], char_height[i], txt_list_buffer[i],
		         char_width[i] * char_height[i] * 2);
	}
#endif
	char *src = NULL;
	int pixel_color_bytes = 0;
	int is_jgpqy_case = 0;
	unsigned int ref_bearingY = 0;
	utf8_char_offset = 0;
	uint16_t tmp_enc_unicode = 0x0000;

	/*if JPQYG case, need enlarge height*/
	for (unsigned int i = 0; i < text_len; i++) {
		if (text_mode == UTF8) {
			if (getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]) == 1) {
				tmp_enc_unicode = txt->txt[utf8_char_offset];
			} else {
				encOneUTF8toUnicode((unsigned char *)&txt->txt[utf8_char_offset],
				                    (unsigned long *)&tmp_enc_unicode);
			}

			utf8_char_offset += getUTF8TxtListNextCharPtrLen(&txt->txt[utf8_char_offset]);
		} else if (text_mode == UNICODE) {
			tmp_enc_unicode = txt->unicode_txt[i];
		}

		if ((tmp_enc_unicode == 0x006a /*j*/ || tmp_enc_unicode == 0x0070 /*p*/ ||
		     tmp_enc_unicode == 0x0071 /*q*/ || tmp_enc_unicode == 0x0079 /*y*/ ||
		     tmp_enc_unicode == 0x0067 /*g*/ || tmp_enc_unicode == 0xff4a || tmp_enc_unicode == 0xff50 ||
		     tmp_enc_unicode == 0xff59 || tmp_enc_unicode == 0xff51 || tmp_enc_unicode == 0xff57)) {
			libosd_log_debug("jgpqy CASE unicode");
			is_jgpqy_case = 1;
			break;
		}
	}

	for (unsigned int i = 0; i < text_len; i++) {
		if (ref_bearingY < char_bearingY[i]) {
			ref_bearingY = char_bearingY[i];
		}
	}

	libosd_log_debug("Get ref bearing y: %d", ref_bearingY);

	if (is_jgpqy_case == 1) {
		for (unsigned int i = 0; i < text_len; i++) {
			if (char_height[i] > char_bearingY[i]) {
				max_bearingY_to_origin = max_bearingY_to_origin > char_height[i] - char_bearingY[i] ?
				                                 max_bearingY_to_origin :
				                                 char_height[i] - char_bearingY[i];
			}
		}
		if (max_bearingY_to_origin == 0) {
			max_bearingY_to_origin = *imgHeight;
			libosd_log_info("change bearingY to origin: %d", max_bearingY_to_origin);
		}
		*imgHeight = ref_bearingY + max_bearingY_to_origin;
		libosd_log_info("change img height to :%d", *imgHeight);
	}

	if (txt->mode == AYUV_3544) {
		pixel_color_bytes = 2;
		src = malloc((*imgWidth) * (*imgHeight) * pixel_color_bytes);
		setAYUVBackground(txt, (*imgWidth), (*imgHeight), src);
	} else if (txt->mode == PALETTE_8) {
		pixel_color_bytes = 1;
		src = malloc((*imgWidth) * (*imgHeight) * pixel_color_bytes);
		setPalette8Background(txt, (*imgWidth), (*imgHeight), src);
	}

	/*copy each char to list pixmap*/
	for (unsigned int i = 0; i < text_len; i++) {
		h_cursor = ref_bearingY - char_bearingY[i];

		if (h_cursor + char_height[i] > *imgHeight) {
			libosd_log_info("h_cursor: %d + char_height[i]: %d > h:%d", h_cursor, char_height[i],
			                *imgHeight);
			h_cursor = *imgHeight - char_height[i];
		}

		for (unsigned int j = 0; j < char_height[i]; j++) {
			memcpy((void *)src + (*imgWidth) * (j + h_cursor) * pixel_color_bytes + w_cursor,
			       (const void *)(txt_list_buffer[i] + (j * char_width[i]) * pixel_color_bytes),
			       (char_width[i]) * pixel_color_bytes);
		}
		w_cursor += (char_width[i] + calcKerningPixel(txt, txt->size, txt->kerning)) * pixel_color_bytes;

		if (txt_list_buffer[i] != NULL) {
			free(txt_list_buffer[i]);
		}
	}

	return src;
}

int assignAYUVList(OsdText *txt, uint16_t *text_list, unsigned int *char_width, unsigned int *char_height,
                   unsigned int *char_bearingY, char **src_in, AyuvSrcList *ayuv_list)
{
	unsigned int pixel_byte = 0;

	for (int i = 0; i < ayuv_list->len; i++) {
		ayuv_list->src[i].width = char_width[i] + calcKerningPixel(txt, txt->size, txt->kerning);
		ayuv_list->src[i].height = char_height[i];
		ayuv_list->src[i].bearingY = char_bearingY[i];
		ayuv_list->src[i].unicode = text_list[i];

		libosd_log_debug("[%d] unicode %#x", i, text_list[i]);
		if (txt->mode == AYUV_3544) {
			pixel_byte = 2;
			ayuv_list->src[i].src = malloc(ayuv_list->src[i].width * ayuv_list->src[i].height * 2);
			setAYUVBackground(txt, ayuv_list->src[i].width, ayuv_list->src[i].height,
			                  ayuv_list->src[i].src);

		} else if ((txt->mode == PALETTE_8)) {
			pixel_byte = 1;
			ayuv_list->src[i].src = malloc(ayuv_list->src[i].width * ayuv_list->src[i].height);
			setPalette8Background(txt, ayuv_list->src[i].width, ayuv_list->src[i].height,
			                      ayuv_list->src[i].src);
		}

		for (unsigned int j = 0; j < char_height[i]; j++) {
			libosd_log_debug("dst size: %d, goto  %d + %d",
			                 ayuv_list->src[i].width * ayuv_list->src[i].height * pixel_byte,
			                 ayuv_list->src[i].width * j * pixel_byte, char_width[i] * pixel_byte);
			memcpy((void *)(ayuv_list->src[i].src + ayuv_list->src[i].width * j * pixel_byte),
			       (const void *)((src_in[i]) + (j * char_width[i] * pixel_byte)),
			       char_width[i] * pixel_byte);
		}
		free((src_in[i]));

#if 0
		char tmpfile[32];
		snprintf(&tmpfile[0], 32, "./%d-%s.ayuv", i, "outline");

		saveAYUV(&tmpfile[0], ayuv_list->src[i].width, ayuv_list->src[i].height, ayuv_list->src[i].src, 
			ayuv_list->src[i].width * ayuv_list->src[i].height * 2);

#endif
	}

	return 0;
}

int genAYUVListSrcBitmapWithOutline(OsdText *txt, unsigned int *char_width, unsigned int *char_height,
                                    unsigned int *char_bearingY, char **src_bitmap)
{
	unsigned int imgWidth = 0;
	unsigned int imgHeight = 0;
	unsigned int bearingY = 0;

	unsigned int max_height = 0;

	if (txt->kerning_mode != MANUAL) {
		txt->kerning_mode = AUTO;
		txt->kerning = AUTO_KERNING_RATE;
	}
	for (unsigned int i = 0; i < getUnicodeTxtListLen(txt); i++) {
		if (isUTF8orUnicodeSpace(txt, i, UNICODE)) {
			libosd_log_info("Char[%d] is space", i);
			continue;
		} else {
			src_bitmap[i] =
			        genCharWithOutline(txt, UNICODE, txt->unicode_txt[i], &imgWidth, &imgHeight, &bearingY);
			if (src_bitmap[i] == NULL) {
				libosd_log_err("failed to get bitmap with outline");
				return -ENOSR;
			}
			char_width[i] = imgWidth;
			char_height[i] = imgHeight;
			char_bearingY[i] = bearingY;
			libosd_log_debug("[%d]w, h = %d, %d, bearing Y :%d", i, char_width[i], char_height[i],
			                 char_bearingY[i]);

			max_height = char_height[i] > max_height ? char_height[i] : max_height;
		}
	}

	for (unsigned int i = 0; i < getUnicodeTxtListLen(txt); i++) {
		if (isUTF8orUnicodeSpace(txt, i, UNICODE)) {
			setSpaceBitmapInfo(&max_height, txt, &char_width[i], &char_height[i], &char_bearingY[i]);
			if (txt->mode == AYUV_3544) {
				src_bitmap[i] = malloc(char_width[i] * char_height[i] * 2);
				setAYUVBackground(txt, char_width[i], char_height[i], src_bitmap[i]);
			} else if (txt->mode == PALETTE_8) {
				src_bitmap[i] = malloc(char_width[i] * char_height[i]);
				setPalette8Background(txt, char_width[i], char_height[i], src_bitmap[i]);
			}
		}
	}

	return 0;
}

int genAYUVListSrcBitmapNoOutline(OsdText *txt, unsigned int *char_width, unsigned int *char_height,
                                  unsigned int *char_bearingY, char **src_bitmap)
{
	unsigned int len = getUnicodeTxtListLen(txt);
	if (len == 0) {
		libosd_log_err("Failed to get unicode text len");
		return -EINVAL;
	}
	char *txt_list_buffer[MAX_TXT_LEN];
	int ret = 0;

	FT_Library library;
	FT_Face face;
	FT_Error error;

	error = FT_Init_FreeType(&library); /* initialize library */
	if (error) {
		libosd_log_err("FT_Init_FreeType failed, %s", FT_Error_String(error));
		return 0;
	}

	error = FT_New_Face(library, txt->ttf_path, 0, &face); /* create face object */
	if (error) {
		libosd_log_err("FT_New_Face failed, ttf path: %s, %s", txt->ttf_path, FT_Error_String(error));
		FT_Done_FreeType(library);
		return -EINVAL;
	}

	FT_Set_Char_Size(face, 0, txt->size << 6 /* x MAX_TXT_LEN*/, DPI, DPI);
	unsigned int tmp_w = 0, tmp_h = 0;
	ret = generateChartoGlyph(txt, UNICODE, face, &txt_list_buffer[0], &tmp_w, &tmp_h, &char_width[0],
	                          &char_height[0], &char_bearingY[0]);
	if (ret < 0) {
		goto glyph_err;
	}
	for (int i = 0; (unsigned int)i < len; i++) {
		if (txt->mode == AYUV_3544) {
			src_bitmap[i] = malloc(char_width[i] * char_height[i] * 2);
			monoGraytoAYUV(txt, txt_list_buffer[i], char_width[i], char_height[i], src_bitmap[i]);
		} else if (txt->mode == PALETTE_8) {
			src_bitmap[i] = malloc(char_width[i] * char_height[i]);
			monoGraytoPalette8(txt, txt_list_buffer[i], char_width[i], char_height[i], src_bitmap[i]);
		}

#if 0
		char tmpfile[32];
		snprintf(&tmpfile[0], 32, "./%d-%s.ayuv", i, "single");

		saveAYUV(&tmpfile[0],  char_width[i],  char_height[i], src_bitmap[i], 
			char_width[i]* char_height[i] * 2);
		libosd_log_info("[%d] save %d %d", i,  char_width[i], char_height[i]);
#endif
	}

	for (int i = 0; (unsigned int)i < len; i++) {
		if (txt_list_buffer[i] != NULL) {
			free(txt_list_buffer[i]);
		}
	}
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return 0;

glyph_err:

	for (int i = 0; (unsigned int)i < len; i++) {
		if (txt_list_buffer[i] != NULL) {
			free(txt_list_buffer[i]);
		}
	}
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	return -EINVAL;
}

char *generatNoOutlineChartoList(OsdText *txt, TEXT_MODE text_mode, unsigned int *total_w, unsigned int *total_h)
{
	char *txt_list_buffer[MAX_TXT_LEN];
	unsigned int char_width[MAX_TXT_LEN];
	unsigned int char_height[MAX_TXT_LEN];
	unsigned int char_bearingY[MAX_TXT_LEN];

	char *src_out = NULL;
	int ret = 0;

	FT_Library library;
	FT_Face face;
	FT_Error error;

	error = FT_Init_FreeType(&library); /* initialize library */
	if (error) {
		libosd_log_err("FT_Init_FreeType failed, %s", FT_Error_String(error));
		return NULL;
	}

	error = FT_New_Face(library, txt->ttf_path, 0, &face); /* create face object */
	if (error) {
		libosd_log_err("FT_New_Face failed, ttf path: %s, %s", txt->ttf_path, FT_Error_String(error));
		FT_Done_FreeType(library);
		return NULL;
	}

	FT_Set_Char_Size(face, 0, txt->size << 6 /* x MAX_TXT_LEN*/, DPI, DPI);

	ret = generateChartoGlyph(txt, text_mode, face, &txt_list_buffer[0], total_w, total_h, &char_width[0],
	                          &char_height[0], &char_bearingY[0]);
	if (ret < 0) {
		goto glyph_err;
	}

	char *mono_src = malloc((*total_w) * (*total_h));
	if (mono_src == NULL) {
		libosd_log_err("failed to alloc src");
		return NULL;
	}

	ret = monoGrayChartoList(txt, text_mode, total_w, total_h, &char_width[0], &char_height[0], &char_bearingY[0],
	                         &txt_list_buffer[0], mono_src);
	if (ret < 0) {
		goto monoList_err;
	}
	libosd_log_debug("Get total %d %d", *total_w, *total_h);

	if (txt->mode == AYUV_3544) {
		src_out = malloc((*total_w) * (*total_h) * 2);
		if (src_out == NULL) {
			libosd_log_err("failed to alloc src out");
			return NULL;
		}
		monoGraytoAYUV(txt, mono_src, *total_w, *total_h, src_out);
	} else if (txt->mode == PALETTE_8) {
		src_out = malloc((*total_w) * (*total_h));
		if (src_out == NULL) {
			libosd_log_err("failed to alloc src out");
			return NULL;
		}
		monoGraytoPalette8(txt, mono_src, *total_w, *total_h, src_out);
	}

	if (mono_src != NULL) {
		free(mono_src);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return src_out;
monoList_err:
	if (mono_src != NULL) {
		free(mono_src);
	}
glyph_err:
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return NULL;
}

#endif
