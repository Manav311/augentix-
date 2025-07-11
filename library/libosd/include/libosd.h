#ifndef LIBOSD_H
#define LIBOSD_H

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
#include "errno.h"
#include <syslog.h>

//#define OSD_DEBUG
#ifdef OSD_DEBUG
#define libosd_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[LIBOSD][Debug] " fmt, ##args)
#else
#define libosd_log_debug(fmt, args...)
#endif
#define libosd_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[LIBOSD][Info] " fmt, ##args)
#define libosd_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[LIBOSD][Error] " fmt, ##args)

typedef enum { TRANSPARENT, WHITE, BLACK } BACKGROUND_COLOR;
typedef enum { PALETTE_8, AYUV_3544 } COLOR_MODE;
typedef enum { YES, NO } UPDATED;
typedef enum { IMG, TXT } OSD_TYPE;
typedef enum { AUTO = 0, MANUAL } KERNING_MODE;
typedef enum { UTF8 = 0, UNICODE } TEXT_MODE;

#define MAX_OSD (8)
#define MAX_CANVAS (4)

#define MAX_TXT_LEN (64)
#define DPI (72)
#define AUTO_KERNING_RATE (2)

/*
* AyuvSrc (for internal use)
* unicode: text of this struct
* width: width of ayuv pixels
* height: height of ayuv pixel
* src: ptr of ayuv
*/
typedef struct {
	uint16_t unicode;
	int width;
	int height;
	int bearingY;
	char *src;
} AyuvSrc;

/*
* AyuvSrcList (for internal use)
* src: each ayuv of unicode
* len: number of fot list
*/
typedef struct {
	AyuvSrc *src;
	int len;
} AyuvSrcList;

/*
*OsdText: metatdata to generate text to ayuv
* txt: array of text to use (only by OSD_setTextUTF8)
* unicode_txt: array of text transfer to UNICODE
* size: font size(unit: pt)
* color: forecolor for text
* background: can be black, white, or transparent
* ttf: ttf, ttc file for font
* mode: select mode 1 (two byte) or mode 9 (one byte) color
* outline_width: pixels for outline, 0 = no outline
* outline_color: if need outline, set outline color
* kerning mode: width betwwen char. has AUTO or MANUAL mode
* keernig: width betwwen char.
*/
typedef struct {
	char txt[MAX_TXT_LEN];
	uint16_t unicode_txt[MAX_TXT_LEN];
	char ttf_path[128];
	COLOR_MODE mode;
	char color[3];
	int size;
	BACKGROUND_COLOR background;
	int outline_width;
	char outline_color[3];
	KERNING_MODE kerning_mode;
	int kerning;
} OsdText;
/*
 * OsdPoint: x,y as a point position
 */
typedef struct {
	int x;
	int y;
} OsdPoint;

/*
 * OsdLine: 
 * start x,y as a point position
 * end x,y as a point position
 * thickness: by pixel add both upper and lower
 * color[4] in [r, g, b, a](mode 1) or [Alpha, color_idx] (mode 9)
 * mode: select mode 1 (two byte) or mode 9 (one byte) color
*/
typedef struct {
	OsdPoint start;
	OsdPoint end;
	int thickness;
	char color[4];
	COLOR_MODE mode;
} OsdLine;

/*
*OsdRegion: OSD in physical canvas in chn, 
*			add by  OSD_addOsd(...)
*			delete by OSD_delOsd(...)
* startx, starty: upper-left cornner of region
*/
typedef struct {
	uint32_t startX;
	uint32_t startY;
	uint32_t width;
	uint32_t height;
	uint8_t include_canvas; /*Read only*/
} OsdRegion;

/*
*OsdCanvas: OSD in physical canvas in chn, 
*			generate by OSD_calcCanvas(...)
*osd_list: contain which osd added by user
*osd_num: number of osd in this canvas by grouping
*/
typedef struct {
	uint32_t startX;
	uint32_t startY;
	uint32_t width;
	uint32_t height;
	uint8_t osd_list[8];
	uint8_t osd_num;
} OsdCanvas;

/*
OsdHandle: create by OSD_create(...)
			release by OSD_destroy(...)
contain all osd information in one channel, Don't EDIT it
*/
typedef struct {
	int width;
	int height;
	int osd_num;
	int osd_index[MAX_OSD];
	OsdRegion region[MAX_OSD];
	OsdCanvas canvas[MAX_CANVAS];
} OsdHandle;

int OSD_init();
int OSD_deinit();

/*
 *@in: width of resolution
 *@in: height of resolution
 */
OsdHandle *OSD_create(int width, int height);

/*
 *@in: phd: add region to one handle
 *@in: osd_idx: region index
　*/
int OSD_addOsd(OsdHandle *phd, int osd_idx, OsdRegion *region);

/*
 *@in: phd: handle of target region
 *@in: osd_idx: region index, will delete by index
 */
int OSD_delOsd(OsdHandle *phd, int osd_idx);

/*
 * calculate region to canvas for one handle if canvas < 4
 *	 startx, starty, width, height will be aligned to 16
 *@in: phd: OsdHandle to calc
 */
int OSD_calcCanvas(OsdHandle *phd);
/*
 * calculate region to canvas for one handle if canvas > 4
 *	 startx, starty, width, height will be aligned to 16
 *@in: phd: OsdHandle to calc
 */
int OSD_calcCanvasbygroup(OsdHandle *phd);

/*
 * OSD_setImage: add ayuv image to a region
 * @in: hd: to select region 
 * @in: osd_idx: to check in which canvas
 * @in: image_path: ayuv source path
 * @in: out: memory ptr to copy ayuv
 */
int OSD_setImage(OsdHandle *hd, int osd_idx, const char *image_path,
                 char *out /*(canvas[cavnas.cavas_idx[soft_idx]].canvas_addr)*/);

/*
 * OSD_setImage: add BMP image to a region
 * @in: hd: to select region 
 * @in: osd_idx: to check in which canvas
 * @in: image_path: ayuv source path
 * @in: out: memory ptr to copy ayuv
 */
int OSD_setImageBmp(OsdHandle *hd, int osd_idx, const char *image_path, char *out);
/*
* OSD_setTextUTF8: add utf8 words to a region
* @in: hd: to select region 
* @in: osd_idx: to check in which canvas
* @in: txt: data to generate text to ayuv
* @in: out: memory ptr to copy ayuv
*/
int OSD_setTextUTF8(OsdHandle *hd, int osd_idx, OsdText *txt, char *out /*canvas[cavas_idx].canvas_addr*/);
/*
* OSD_createTextUTF8Src: create text to ayuv image and return img size
* @in: txt: data to generate text to ayuv
* @out: width: width of ayuv src
* @out: width: height of ayuv src
* @ret: ptr of output ayuv src
*/
char *OSD_createTextUTF8Src(OsdText *txt, int *width, int *height);
/*
* OSD_createTextUTF8Src8bit: create text to ayuv image and return img size in mode 9
* @in: txt: data to generate text to ayuv
* @out: width: width of ayuv src
* @out: width: height of ayuv src
* @ret: ptr of output ayuv src
*/
char *OSD_createTextUTF8Src8bit(OsdText *txt, int *width, int *height);
/*
* OSD_createTextUnicodeSrc: create text to ayuv image and return img size
* @in: txt: data to generate text to ayuv
* @out: width: width of ayuv src
* @out: width: height of ayuv src
* @ret: ptr of output ayuv src
*/
char *OSD_createTextUnicodeSrc(OsdText *txt, int *width, int *height);
/*
* OSD_createTextUnicodeSrc8bit: create text to ayuv image and return img size in mode 9
* @in: txt: data to generate text to ayuv
* @out: width: width of ayuv src
* @out: width: height of ayuv src
* @ret: ptr of output ayuv src
*/
char *OSD_createTextUnicodeSrc8bit(OsdText *txt, int *width, int *height);
/*
* OSD_createTextUTF8AYUV: create text to ayuv image and return img size
* @in: ptr:free ayuv src created by OSD_createTextUTF8Src or OSD_createTextUnicodeSrc
*/
int OSD_destroySrc(char *ptr);
/*
 * OSD_setTextUnicode: add UNICODE words to a region
 * @in: hd: to select region 
 * @in: osd_idx: to check in which canvas
 * @in txt: data to generate text to ayuv (need unicode_list)
 * @in: out: memory ptr to copy ayuv
 */
int OSD_setTextUnicode(OsdHandle *hd, int osd_idx, OsdText *txt, char *out /*canvas[cavas_idx].canvas_addr*/);
/*
 * OSD_setLine: add line to a region
 * @in: hd: to select region 
 * @in: osd_idx: to check in which canvas
 * @in line: data to generate a line
 * @in: out: memory ptr to copy ayuv
 */
int OSD_setLine(OsdHandle *hd, int osd_idx, OsdLine *line, char *out);
/*
 * OSD_setPrivacyMask: fulfill region with single color act as privacy mask
 * @in: hd: to select region 
 * @in: osd_idx: to check in which canvas
 * @in: p_color: ptr of privacy mask color, include [r, g, b, a] (mode 1), [alpha, color_idx] (mode 9)
 * @in: mode: select mode 1 (two byte) or mode 9 (one byte) color
 * @in: out: memory ptr to copy ayuv
 */
int OSD_setPrivacyMask(OsdHandle *hd, int osd_idx, char *p_color, COLOR_MODE mode, char *out);
/*
 *remove all canvas in a OsdHandle
 */
int OSD_destroyCanvas(OsdHandle *phd);

/*
 *to free OsdHandle memory
 * @in phd free this handle 
 */
int OSD_destroy(OsdHandle *phd);

/*
 * support: 0 - 9 , '-', ':', ' ' to unicode
 * 
 * @in txt to translate
 * @return unicode of input txt
 */
uint16_t OSD_trans2Unicode(char txt);
/*
 * let one region to transparent
 *
 *@in hd: osd handle for this index
 *@in: reg_idx: region to become transparent
 *@in: mode: select mode 1 (two byte) or mode 9 (one byte) color
 *@out: memory ptr to write
 */
int OSD_setRegionTransparent(OsdHandle *hd, int reg_idx, COLOR_MODE mode, char *out);
/*
 * OSD_createUnicodeFontList: generate unicode text to ayuv src
 * @in: txt: get text format (color, outline, ...)
 * @in: text_list: all text in unicode will be used
 * @in: len: length of text list
 * ret: the ptr if all ayuv, need to be free by OSD_destroyUnicodeFontList(...)
*/
char *OSD_createUnicodeFontList(OsdText *txt, uint16_t *text_list, int len);
/*
 * OSD_createUnicodeFontList: generate unicode text to ayuv src in mode 9
 * @in: txt: get text format (color, outline, ...)
 * @in: text_list: all text in unicode will be used
 * @in: len: length of text list
 * ret: the ptr if all ayuv, need to be free by OSD_destroyUnicodeFontList(...)
*/
char *OSD_createUnicodeFontList8bit(OsdText *txt, uint16_t *text_list, int len);
/*
 * OSD_getUnicodeSizetoGenerate: before generate ayuv, user need to know size to save ayuv
 * @in: text_list: all text in unicode to generate
 * @in: len: length of text list
 * @out: width: width of ayuv src (pixel)
 * @out: height: height width of ayuv src (pixel)
*/
int OSD_getUnicodeSizetoGenerate(uint16_t *text_list, int len, char *src_ptr, int *width, int *height);
/*
 * OSD_generateUnicodeFromList: generate ayuv src from text_list
 * @in: text_list: all text in unicode to generate
 * @in: len: length of text list
 * @in: src_ptr: ptr if all ayuv, create by OSD_createUnicodeFontList
 * @in: dst_ptr: ptr to save ayuv src
*/
int OSD_generateUnicodeFromList(uint16_t *text_list, int len, char *src_ptr, char *dst_ptr, int dst_width,
                                int dst_height);
/*
 * OSD_generateUnicodeFromList: generate ayuv src from text_list in mode 9
 * @in: text_list: all text in unicode to generate
 * @in: len: length of text list
 * @in: src_ptr: ptr if all ayuv, create by OSD_createUnicodeFontList
 * @in: dst_ptr: ptr to save ayuv src
*/
int OSD_generateUnicodeFromList8bit(uint16_t *text_list, int len, char *src_ptr, char *dst_ptr, int dst_width,
                                    int dst_height);
/*
* OSD_destroyUnicodeFontList: free memory of font list src
* @in: ptr:ptr if all ayuv, create by OSD_createUnicodeFontList
* @in: len: number of text in Unicode Font List
*/
int OSD_destroyUnicodeFontList(char *ptr);
/*
* OSD_setImageAYUVptr: give ptr not file as image OSD src
* @in: hd: to select region 
* @in: osd_idx: to check in which canvas
* @in: src_ptr: src of ayuv
* @in: img_width: width of ayuv (pixel)
* @in: img_height: height of ayuv (pixel)
* @in: mode: select mode 1 (two byte) or mode 9 (one byte) color
* @in: out: memory ptr to copy ayuv
*/
int OSD_setImageAYUVptr(OsdHandle *hd, int osd_idx, char *src_ptr, int img_width, int img_height, COLOR_MODE mode,
                        char *out);
/*
* OSD_moveOsdRegion: move osd src position
* @in: hd: to select region 
* @in: osd_idx: to check in which canvas
* @in: region: new start x, start y
*/
int OSD_moveOsdRegion(OsdHandle *phd, int osd_idx, OsdRegion *region);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
