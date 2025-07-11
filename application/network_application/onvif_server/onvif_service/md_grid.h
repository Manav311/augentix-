#ifndef MD_GRID_H_
#define MD_GRID_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MD_MAX_REGION_CNT 64

#define AGTX_MD_COL 16
#define AGTX_MD_ROW 12

#define IS_OVERLAP(sx0, ex0, sx1, ex1) (((sx0) == (sx1)) && (((ex0) == (ex1))))
#define MD_COOR_ONVIF_TO_AVMAIN(x, x_scale) ((x) * (x_scale))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
	int sx;
	int sy;
	int ex;
	int ey;
} RECT;

typedef struct {
	int region_n;
	RECT reg[MD_MAX_REGION_CNT];
} MD_REG_LIST;

typedef struct {
	int row;
	int col;
	float x_scale;
	float y_scale;
	int array_n;
	int8_t *maskarry;
} ONVIF_CONF;

int MDGRID_decodeActiveCells(char *packbit, MD_REG_LIST *list);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
