#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "b64_to_hex.h"
#include "augentix.h"
#include "md_grid.h"

//static void MD_transGridFromAvmainToPackbits(char* data, AGTX_IVA_MD_CONF_S *md_conf)
//{
//	int i = 0;
//	int w_unit = 100 / AGTX_MD_ROW;
//	int h_unit = 100 / AGTX_MD_COL;
//	AGTX_IVA_MD_REGION_S *rgn_list = NULL;
//	char *cell = malloc(((AGTX_MD_COL * AGTX_MD_ROW) / 8) + 1);
//
//	cell = malloc((AGTX_MD_COL * AGTX_MD_ROW) / 8);
//	if (!cell) {
//		printf("%s no memory",__func__);
//		return;
//	}
//
//	rgn_list = md_conf->rgn_list;
//	for (i = 0; i < md_conf->rgn_cnt;i++) {
//		rgn_list[i].sx
//		rgn_list[i].ex
//	}
//
//	if (cell) {
//		free(cell);
//	}
//}

static void MD_transGridFromonvifToAvmain(ONVIF_CONF *onvif, MD_REG_LIST *reg_list)
{
	int x_start[MD_MAX_REGION_CNT] = { 0 };
	int x_end[MD_MAX_REGION_CNT] = { 0 };
	int x_seg_active = 0;
	int x_seg_cnt = 0;
	int reg_found = 0;
	int grid_height = onvif->row;
	int grid_width = onvif->col;
	float x_scale = onvif->x_scale;
	float y_scale = onvif->y_scale;
	int8_t *mask = onvif->maskarry;
	RECT *rect = NULL;

	reg_list->region_n = 0;

	for (int y = 0; y < grid_height; y++) {
		//DBG("y = %d\n", y);

		x_seg_active = 0;
		x_seg_cnt = 0;
		for (int x = 0; x < grid_width; x++) {
			//DBG("x = %d\n", x);
			//			printf("x = %d, y = %d, mask = %d\n", x, y, mask[x + grid_width*y]);
			if (mask[x + grid_width * y]) {
				if (x_seg_cnt >= MD_MAX_REGION_CNT) {
					printf("Number of SEG %d is larger than MD_MAX_REGION_CNT%d.\n", x_seg_cnt,
					       MD_MAX_REGION_CNT);
					continue;
				}

				if (!x_seg_active) {
					x_start[x_seg_cnt] = x;
					x_seg_active = 1;
				}
				x_end[x_seg_cnt] = x;
			} else {
				if (x_seg_active) {
					x_seg_active = 0;
					x_seg_cnt++;
				}
			}
		}

		if (x_seg_active) {
			x_seg_cnt++;
		}

		if (x_seg_cnt >= MD_MAX_REGION_CNT) {
			printf("Number of SEG %d is too large.\n", x_seg_cnt);
			x_seg_cnt = MD_MAX_REGION_CNT;
		}

		if (x_seg_cnt) {
			for (int i = 0; i < x_seg_cnt; i++) {
				reg_found = 0;
				for (int o = 0; o < reg_list->region_n; o++) {
					rect = &reg_list->reg[o];
					if ((rect->ey == (y - 1) || rect->ey == y) &&
					    IS_OVERLAP(x_start[i], x_end[i], rect->sx, rect->ex)) {
						reg_found = 1;
						rect->sx = MIN(x_start[i], rect->sx);
						rect->ex = MAX(x_end[i], rect->ex);
						rect->ey = y;
						break;
					}
				}

				if (!reg_found) {
					if (reg_list->region_n >= MD_MAX_REGION_CNT) {
						goto fill;
					}

					rect = &reg_list->reg[reg_list->region_n];
					rect->sx = x_start[i];
					rect->ex = x_end[i];
					rect->sy = y;
					rect->ey = y;
					reg_list->region_n++;
				}
			}
		}
	}

fill:

	//	printf("x_scale = %f, y_scale = %f\n", x_scale, y_scale);
	for (int i = 0; i < reg_list->region_n; i++) {
		//		printf("reg_n id %d\n", i);
		rect = &reg_list->reg[i];
		//DBG("sx:%d sy:%d, ex:%d, ey:%d\n", rect->sx, rect->sy, rect->ex, rect->ey);
		rect->sx = MD_COOR_ONVIF_TO_AVMAIN(rect->sx, 50 * x_scale);
		rect->ex = MD_COOR_ONVIF_TO_AVMAIN(rect->ex + 1, 50 * x_scale);
		rect->sy = MD_COOR_ONVIF_TO_AVMAIN(rect->sy, 50 * y_scale);
		rect->ey = MD_COOR_ONVIF_TO_AVMAIN(rect->ey + 1, 50 * y_scale);
		//		printf("   sx:%d sy:%d, ex:%d, ey:%d\n", rect->sx, rect->sy, rect->ex, rect->ey);
	}
}

static void hexToArray(long hex, int8_t *mask)
{
	for (int j = 0; j < 8; j++) {
		//	DBG("hexToArray hex = %d\n", hex >> (7-j));
		mask[j] = ((hex >> (7 - j)) & 0x1);
	}
}

static void unPackBits(ONVIF_CONF *onvif, char *data)
{
	int i = 0;
	int j = 0;
	long hex = 0; // Header
	long output = 0;
	int8_t *maskarry = onvif->maskarry;
	char *tok = strtok(data, " ");

	/* concept
	int n = 0;
	while (n < MaxSize) {
		//Read the next source byte into B.
		if (0 ≤ B ≤ 127) {
			//		read the next B + 1 bytes.n += B + 1;
		} else if (-127 ≤ B ≤ - 1) {
			//read the next byte - B + 1 times.n += (-B + 1)
		}
	}
	*/

	//	printf("line: %d, tok = %s\n", __LINE__, tok);

	while (tok != NULL) {
		hex = strtol(tok, NULL, 16);
		//		printf("line: %d, hex = %lX\n", __LINE__, hex);
		if (hex >= 128) {
			hex = 256 - hex;
			tok = strtok(NULL, " ");
			for (j = 0; j <= hex; j++) {
				//				printf("line: %d, tok = %s\n", __LINE__, tok);
				if (tok != NULL) { // For protection
					/* Number of tok is "hex" */
					output = strtol(tok, NULL, 16);
					//					printf("line: %d, output = %lX\n", __LINE__, output);
					if (i * 8 >= AGTX_MD_COL * AGTX_MD_ROW) {
						//						printf("\n\n\n******** \n ERROR: i = %d ********\n\n\n", i);
						break;
					}
					hexToArray(output, &maskarry[i * 8]);
					i++;
				} else {
					printf("ERROR: tok == NULL\n");
				}
			}
		} else {
			for (j = 0; j <= hex; j++) {
				tok = strtok(NULL, " ");
				//				printf("line: %d, tok = %s\n", __LINE__, tok);
				if (tok != NULL) { // For protection
					/* Number of tok is "hex" */
					output = strtol(tok, NULL, 16);
					if (i * 8 >= AGTX_MD_COL * AGTX_MD_ROW) {
						//												printf("\n\n\n******** \n ERROR: i = %d ********\n\n\n", i);
						break;
					}
					//					printf("line: %d, output = %lX\n", __LINE__, output);
					hexToArray(output, &maskarry[i * 8]);
					i++;
				} else {
					printf("ERROR: tok == NULL\n");
				}
			}
		}
		tok = strtok(NULL, " ");
	}
}

int MDGRID_decodeActiveCells(char *packbit, MD_REG_LIST *list)
{
	char data[256] = "";
	int len_data = 0;
	int array_n = 0;
	size_t len_str = strlen(packbit);
	RECT *rect = { 0 };
	ONVIF_CONF onvif = { 0 };

	printf("Packbit: %s , length: %zu\n", packbit, len_str);

	/*decode base64*/
	int ret = decodeChar(&packbit[0], &data[0], (int)len_str, &len_data);
	if (ret) {
		printf("fails to decode packbit\n");
		return -1;
	}

	printf("Decoded Hex String: %s, length: %d\n", &data[0], len_data);

	onvif.col = AGTX_MD_COL;
	onvif.row = AGTX_MD_ROW;
	onvif.x_scale = 2.0 / onvif.col;
	onvif.y_scale = 2.0 / onvif.row;
	array_n = ((onvif.col * onvif.row + 7) / 8) * 8;
	onvif.array_n = array_n;
	onvif.maskarry = (int8_t *)calloc(array_n, sizeof(int8_t));
	if (onvif.maskarry == NULL) {
		printf("ERROR: calloc allocate failed\n");
		return -1;
	}

	printf("arry ptr = %p\n", onvif.maskarry);

	unPackBits(&onvif, data);

	printf("array number = %d\n", array_n);

	//	for (int i = 0; i < array_n; i++) {
	//		printf("array[%d] = %d\n", i, onvif.maskarry[i]);
	//	}

	MD_transGridFromonvifToAvmain(&onvif, list);

	printf("Final region list: reg_n = %d\n", list->region_n);

	for (int i = 0; i < list->region_n; i++) {
		rect = &list->reg[i];
		printf("reg: sx=%d, sy=%d, ex=%d, ey=%d\n", rect->sx, rect->sy, rect->ex, rect->ey);
	}

	free(onvif.maskarry);

	return 0;
}
