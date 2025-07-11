#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>

#include "libosd.h"
#include "txt2ayuv.h"
#include "drawline.h"

#include "char2Glyph.h"

static int copy2Region(OsdHandle *hd, int osd_idx, int tmp_idx, char *src, int src_width, int src_height,
                       COLOR_MODE mode, char *out)
{
	int x_offset = 0;
	int y_offset = 0;
	int canvas_width = 0;
	int osd_width = 0;
	int osd_height = 0;
	int flag = 0;
	int line = 0;
	int copy_width, copy_height;
	int byte_len = 1;

	for (int i = 0; i < hd->osd_num; i++) {
		for (int j = 0; j < hd->canvas[i].osd_num; j++) {
			if (hd->canvas[i].osd_list[j] == osd_idx) {
				x_offset = hd->region[tmp_idx].startX - hd->canvas[i].startX;

				y_offset = hd->region[tmp_idx].startY - hd->canvas[i].startY;
				canvas_width = hd->canvas[i].width;
				osd_width = hd->region[tmp_idx].width;
				osd_height = hd->region[tmp_idx].height;
				libosd_log_debug("get[%d] : %d, %d %d %d %d %d", i, osd_idx, x_offset, y_offset,
				                 canvas_width, osd_width, osd_height);
				flag = 1;
				break;
			}
		}
		if (flag == 1) {
			break;
		}
	}

	if ((src_width != osd_width) || (src_height != osd_height)) {
		libosd_log_info("Img size != OSD size, (%d, %d) (%d, %d)", src_width, src_height, osd_width,
		                osd_height);
	}

	if (src_width > osd_width) {
		copy_width = osd_width;
	} else {
		copy_width = src_width;
	}

	if (src_height > osd_height) {
		copy_height = osd_height;
	} else {
		copy_height = src_height;
	}

	if (mode == AYUV_3544) {
		byte_len = 2;
	} else {
		byte_len = 1;
	}

	for (int i = y_offset; i < y_offset + copy_height; i++) {
		memcpy((void *)out + (canvas_width * i + x_offset) * byte_len,
		       (void *)src + (line * src_width * byte_len), copy_width * byte_len);
		line++;
	}

	return 0;
}
static int findOSDRegion(OsdHandle *hd, int osd_idx)
{
	int tmp_idx = -1;
	for (int i = 0; i < MAX_OSD; ++i) {
		if (hd->osd_index[i] == osd_idx) {
			tmp_idx = i;
			libosd_log_debug("get tmp_idx = %d", tmp_idx);
			break;
		}
	}

	if (tmp_idx == -1) {
		libosd_log_err("index %d not found", osd_idx);
		return -EINVAL;
	}

	return tmp_idx;
}

int OSD_init()
{
	libosd_log_info("init");
	return 0;
}

int OSD_deinit()
{
	libosd_log_info("deinit");
	return 0;
}

OsdHandle *OSD_create(int width, int height)
{
	OsdHandle *ptr = malloc(sizeof(OsdHandle));
	if (ptr == NULL) {
		libosd_log_err("failed to create OSD handle");
		return NULL;
	}
	memset(ptr, 0xff, sizeof(OsdHandle));
	ptr->width = width;
	ptr->height = height;
	ptr->osd_num = 0;
	for (int i = 0; i < MAX_CANVAS; i++) {
		for (int j = 0; j < MAX_OSD; j++) {
			ptr->canvas[i].osd_list[j] = 0xff;
		}
	}

	for (int i = 0; i < MAX_OSD; i++) {
		ptr->osd_index[i] = -1;
	}

	libosd_log_debug("create (%d, %d)", width, height);

	return ptr;
}

uint32_t CEILINGALIGN16(uint32_t x)
{
	uint32_t tmp = 0;
	tmp = (x + 15) / 16;
	tmp = tmp * 16;
	return tmp;
}

uint32_t ALIGN16(uint32_t x)
{
	uint32_t tmp = 0;
	tmp = x / 16;
	tmp = tmp * 16;

	return tmp;
}

int OSD_destroy(OsdHandle *phd)
{
	libosd_log_debug("detroy handle(%d, %d)", phd->width, phd->height);
	free(phd);

	return 0;
}

int OSD_addOsd(OsdHandle *phd, int osd_idx, OsdRegion *region)
{
	if (phd->osd_num == MAX_OSD) {
		libosd_log_err("handle has 8 osd");
		return -EINVAL;
	}

	if (region->startX > (uint32_t)phd->width) {
		libosd_log_err("startX exceed handle width");
		return -EINVAL;
	}

	if (region->startY > (uint32_t)phd->height) {
		libosd_log_err("startY exceed handle height");
		return -EINVAL;
	}

	if (CEILINGALIGN16(region->startX + region->width) > (uint32_t)phd->width) {
		libosd_log_err("canvas > width after align 16");
		return -EINVAL;
	}

	if (CEILINGALIGN16(region->startY + region->height) > (uint32_t)phd->height) {
		libosd_log_err("canvas > height after align 16");
		return -EINVAL;
	}

	if (phd->osd_index[osd_idx] == -1) {
		phd->osd_num += 1;
		phd->osd_index[osd_idx] = osd_idx;
	}

	memcpy(&phd->region[osd_idx], region, sizeof(OsdRegion));

	libosd_log_info("add [%d] (%d, %d, %d, %d) to hd, now has region:[%d]", osd_idx, region->startX, region->startY,
	                region->width, region->height, phd->osd_num - 1);
	return 0;
}

int OSD_delOsd(OsdHandle *phd, int osd_idx)
{
	if (phd->osd_num == 0) {
		libosd_log_err("handle has no osd");
		return -EINVAL;
	}

	if ((osd_idx > MAX_OSD) || (osd_idx < 0)) {
		libosd_log_err("invalid del idx: %d", osd_idx);
		return -EINVAL;
	}

	int tmp_idx = -1;

	for (int i = 0; i < MAX_OSD; ++i) {
		if (phd->osd_index[i] == osd_idx) {
			tmp_idx = i;
			libosd_log_debug("get osd_index[%d]: %d", i, osd_idx);
			break;
		}
	}

	if (tmp_idx == -1) {
		libosd_log_err("can't find this idx:%d", osd_idx);
		return -EINVAL;
	}

	libosd_log_info("del [%d] from hd->region[%d]", osd_idx, tmp_idx);

	phd->osd_num -= 1;
	phd->osd_index[tmp_idx] = -1;
	memset(&phd->region[tmp_idx], 0xff, sizeof(OsdRegion));

	return 0;
}

int OSD_calcCanvasbygroup(OsdHandle *phd)
{
	if (phd->osd_num == 0) {
		libosd_log_err("handle has no osd");
		return -EINVAL;
	}

	/*0-3: (0,1,2,3) or all*/
	OsdHandle tmp_hd[2];
	memset(&tmp_hd[0], 0xff, sizeof(OsdHandle) * 2);
	int tmp_area[2] = { 0, 0 };

	for (int i = 0; i < MAX_CANVAS; i++) {
		memset(&phd->canvas[i], 0xff, sizeof(OsdCanvas));
	}

	/*grouping algo*/
	int add_list[MAX_OSD] = { 0 };
	int add_idx = 0;
	for (int i = 0; i < MAX_OSD; i++) {
		if (phd->osd_index[i] != -1) {
			libosd_log_debug("add %d", phd->osd_index[i]);
			add_list[add_idx] = phd->osd_index[i];
			add_idx++;
		}
	}

	if (add_idx > phd->osd_num) {
		libosd_log_err("add idx != region num: %d, %d", add_idx, phd->osd_num);
		return -EINVAL;
	}

	for (int i = 0; i < add_idx; i++) {
		phd->region[add_list[i]].include_canvas = 0;
	}

	for (int i = 0; i < MAX_CANVAS; i++) {
		libosd_log_debug("First add reg %d (%d, %d, %d, %d)", add_list[i], phd->region[add_list[i]].startX,
		                 phd->region[add_list[i]].startY, phd->region[add_list[i]].width,
		                 phd->region[add_list[i]].height);
		tmp_hd[0].canvas[i].osd_num = 1;
		tmp_hd[0].canvas[i].osd_list[0] = add_list[i];
		tmp_hd[0].canvas[i].startX = ALIGN16(phd->region[add_list[i]].startX);
		tmp_hd[0].canvas[i].startY = ALIGN16(phd->region[add_list[i]].startY);
		uint32_t width_tmp = phd->region[add_list[i]].startX + phd->region[add_list[i]].width;
		libosd_log_debug("width tmp : %d", width_tmp);
		width_tmp = width_tmp - tmp_hd[0].canvas[i].startX;
		libosd_log_debug("width tmp : %d", width_tmp);
		tmp_hd[0].canvas[i].width = CEILINGALIGN16(width_tmp);
		uint32_t height_tmp = phd->region[add_list[i]].startY + phd->region[add_list[i]].height;
		libosd_log_debug("height tmp : %d", height_tmp);
		height_tmp = height_tmp - tmp_hd[0].canvas[i].startY;
		libosd_log_debug("height tmp : %d, start y = %d", height_tmp, phd->canvas[i].startY);
		tmp_hd[0].canvas[i].height = CEILINGALIGN16(height_tmp);
		tmp_hd[0].region[add_list[i]].include_canvas = i;
		tmp_area[0] += tmp_hd[0].canvas[i].width * tmp_hd[0].canvas[i].height;
		libosd_log_debug("[%d] num:%d, %d, (%d, %d, %d, %d) = %d", i, tmp_hd[0].canvas[i].osd_num,
		                 tmp_hd[0].canvas[i].osd_list[0], tmp_hd[0].canvas[i].startX,
		                 tmp_hd[0].canvas[i].startY, tmp_hd[0].canvas[i].width, tmp_hd[0].canvas[i].height,
		                 tmp_area[0]);
	}

	tmp_hd[1].canvas[0].osd_num = 4;
	int startx_list[4];
	int starty_list[4];
	int endx_list[4];
	int endy_list[4];
	for (int i = 0; i < 4; i++) {
		tmp_hd[1].canvas[0].osd_list[i] = add_list[i];
		tmp_hd[1].region[i].include_canvas = 0;
		startx_list[i] = ALIGN16(phd->region[add_list[i]].startX);
		starty_list[i] = ALIGN16(phd->region[add_list[i]].startY);
		endx_list[i] = CEILINGALIGN16(phd->region[add_list[i]].startX + phd->region[add_list[i]].width);
		endy_list[i] = CEILINGALIGN16(phd->region[add_list[i]].startY + phd->region[add_list[i]].height);
	}
	tmp_hd[1].canvas[0].startX = get_min(&startx_list[0], 4);
	tmp_hd[1].canvas[0].startY = get_min(&starty_list[0], 4);
	tmp_hd[1].canvas[0].width = get_max(&endx_list[0], 4) - tmp_hd[1].canvas[0].startX;
	tmp_hd[1].canvas[0].height = get_max(&endy_list[0], 4) - tmp_hd[1].canvas[0].startY;
	tmp_area[1] = tmp_hd[1].canvas[0].width * tmp_hd[1].canvas[0].height;
	libosd_log_debug("case 2: (%d, %d, %d, %d) area; %d", tmp_hd[1].canvas[0].startX, tmp_hd[1].canvas[0].startY,
	                 tmp_hd[1].canvas[0].width, tmp_hd[1].canvas[0].height, tmp_area[1]);

	if (tmp_area[0] < tmp_area[1]) {
		libosd_log_debug("select 0");
		memcpy(&phd->canvas[0], &tmp_hd[0].canvas[0], sizeof(OsdCanvas) * 4);
		for (int i = 0; i < MAX_OSD; i++) {
			phd->region[i].include_canvas = tmp_hd[0].region[i].include_canvas;
		}
	} else {
		libosd_log_debug("select 1");
		memcpy(&phd->canvas[0], &tmp_hd[1].canvas[0], sizeof(OsdCanvas) * 4);
		for (int i = 0; i < 4; i++) {
			phd->region[i].include_canvas = 0;
		}
	}

#ifdef OSD_DEBUG
	logAllOsdHandle(phd);
#endif
	/*add region[4]*/
	OsdHandle tmp_hd_loop[11];
	int tmp_area_loop[11];
	int loop_idx = 0;
	memset(&tmp_hd_loop[0], 0, sizeof(tmp_hd_loop));
	memset(&tmp_area_loop[0], 0, sizeof(tmp_area_loop));

	/*0,1 means 0,1 merge, new add to 1, 2,3,4 copy*/
	/*0,2 means 0,2 merge, new add to 2, 1,3,4 copy*/
	int old_num, new_num;
	int compare_list[2];
	int new_calc_num = 4;

	for (new_calc_num = 4; new_calc_num < phd->osd_num; new_calc_num++) {
		libosd_log_debug("add: [%d]: idx: %d", new_calc_num, add_list[new_calc_num]);
		for (int merge_num = 0; merge_num < MAX_CANVAS; merge_num++) {
			for (int add_num = merge_num + 1; add_num < MAX_CANVAS + 1; add_num++) {
				libosd_log_debug("[%d]merge num: %d, add num: %d , other num: ", loop_idx, merge_num,
				                 add_num);
				memcpy(&tmp_hd_loop[loop_idx].region[0], &phd->region[0], sizeof(OsdRegion) * MAX_OSD);
				tmp_hd_loop[loop_idx].osd_num = phd->osd_num;

				/*merge with new add*/
				if (add_num + 1 == 5) {
					/* Add osd_list to bottom */
					old_num = phd->canvas[merge_num].osd_num;
					new_num = old_num + 1; /*only new one*/
					tmp_hd_loop[loop_idx].canvas[merge_num].osd_num = new_num;

					/* copy osd_list */
					memcpy(&tmp_hd_loop[loop_idx].canvas[merge_num].osd_list[0],
					       &phd->canvas[merge_num].osd_list[0],
					       sizeof(uint8_t) * phd->canvas[merge_num].osd_num);
					tmp_hd_loop[loop_idx].canvas[merge_num].osd_list[old_num] =
					        add_list[new_calc_num];
#ifdef OSD_DEBUG
					logAllOsdHandle(&tmp_hd_loop[loop_idx]);
#endif
					/* calc new rect (x,y,w,h)*/
					compare_list[0] = (phd->canvas[merge_num].startX);
					compare_list[1] = (phd->region[add_list[new_calc_num]].startX);
					tmp_hd_loop[loop_idx].canvas[merge_num].startX = get_min(&compare_list[0], 2);

					compare_list[0] = (phd->canvas[merge_num].startY);
					compare_list[1] = (phd->region[add_list[new_calc_num]].startY);
					tmp_hd_loop[loop_idx].canvas[merge_num].startY = get_min(&compare_list[0], 2);

					compare_list[0] =
					        (phd->canvas[merge_num].startX + phd->canvas[merge_num].width);
					compare_list[1] = (phd->region[add_list[new_calc_num]].startX +
					                   phd->region[add_list[new_calc_num]].width);
					tmp_hd_loop[loop_idx].canvas[merge_num].width =
					        get_max(&compare_list[0], 2) -
					        tmp_hd_loop[loop_idx].canvas[merge_num].startX;

					compare_list[0] =
					        (phd->canvas[merge_num].startY + phd->canvas[merge_num].height);
					compare_list[1] = (phd->region[add_list[new_calc_num]].startY +
					                   phd->region[add_list[new_calc_num]].height);
					tmp_hd_loop[loop_idx].canvas[merge_num].height =
					        get_max(&compare_list[0], 2) -
					        tmp_hd_loop[loop_idx].canvas[merge_num].startY;

					/* merge together, then use add_num space to add new one*/
					int region_id;
					for (int i = 0; i < tmp_hd_loop[loop_idx].canvas[merge_num].osd_num; i++) {
						region_id = tmp_hd_loop[loop_idx].canvas[merge_num].osd_list[i];
						tmp_hd_loop[loop_idx].region[region_id].include_canvas = merge_num;
					}

					/*copy other canvas as usual*/
					for (int other = 0; other < MAX_CANVAS; other++) {
						if ((other != merge_num) && (other != add_num)) {
							libosd_log_debug(" %d", other);
							memcpy(&tmp_hd_loop[loop_idx].canvas[other],
							       &phd->canvas[other], sizeof(OsdCanvas));
						}
					}

				} else {
					/* Add osd_list to bottom */
					old_num = phd->canvas[merge_num].osd_num;
					new_num = phd->canvas[merge_num].osd_num + phd->canvas[add_num].osd_num;
					tmp_hd_loop[loop_idx].canvas[merge_num].osd_num = new_num;

					/* copy osd_list */
					memcpy(&tmp_hd_loop[loop_idx].canvas[merge_num].osd_list[0],
					       &phd->canvas[merge_num].osd_list[0],
					       sizeof(uint8_t) * phd->canvas[merge_num].osd_num);
					memcpy(&tmp_hd_loop[loop_idx].canvas[merge_num].osd_list[old_num],
					       &phd->canvas[add_num].osd_list[0],
					       sizeof(uint8_t) * phd->canvas[add_num].osd_num);
#ifdef OSD_DEBUG
					//logAllOsdHandle(&tmp_hd_loop[loop_idx]);
#endif

					/* calc new rect (x,y,w,h)*/
					compare_list[0] = (phd->canvas[merge_num].startX);
					compare_list[1] = (phd->canvas[add_num].startX);
					tmp_hd_loop[loop_idx].canvas[merge_num].startX = get_min(&compare_list[0], 2);

					compare_list[0] = (phd->canvas[merge_num].startY);
					compare_list[1] = (phd->canvas[add_num].startY);
					tmp_hd_loop[loop_idx].canvas[merge_num].startY = get_min(&compare_list[0], 2);

					compare_list[0] =
					        (phd->canvas[merge_num].startX + phd->canvas[merge_num].width);
					compare_list[1] = (phd->canvas[add_num].startX + phd->canvas[add_num].width);
					tmp_hd_loop[loop_idx].canvas[merge_num].width =
					        get_max(&compare_list[0], 2) -
					        tmp_hd_loop[loop_idx].canvas[merge_num].startX;

					compare_list[0] =
					        (phd->canvas[merge_num].startY + phd->canvas[merge_num].height);
					compare_list[1] = (phd->canvas[add_num].startY + phd->canvas[add_num].height);
					tmp_hd_loop[loop_idx].canvas[merge_num].height =
					        get_max(&compare_list[0], 2) -
					        tmp_hd_loop[loop_idx].canvas[merge_num].startY;

					/* merge together, then use add_num space to add new one*/
					int region_id;
					for (int i = 0; i < tmp_hd_loop[loop_idx].canvas[merge_num].osd_num; i++) {
						region_id = tmp_hd_loop[loop_idx].canvas[merge_num].osd_list[i];
						tmp_hd_loop[loop_idx].region[region_id].include_canvas = merge_num;
					}

					/*copy new to add_num*/
					tmp_hd_loop[loop_idx].canvas[add_num].startX =
					        phd->region[add_list[new_calc_num]].startX;
					tmp_hd_loop[loop_idx].canvas[add_num].startY =
					        phd->region[add_list[new_calc_num]].startY;
					tmp_hd_loop[loop_idx].canvas[add_num].width =
					        phd->region[add_list[new_calc_num]].width;
					tmp_hd_loop[loop_idx].canvas[add_num].height =
					        phd->region[add_list[new_calc_num]].height;
					tmp_hd_loop[loop_idx].canvas[add_num].osd_num = 1;
					tmp_hd_loop[loop_idx].canvas[add_num].osd_list[0] = add_list[new_calc_num];
					tmp_hd_loop[loop_idx].region[add_list[new_calc_num]].include_canvas = add_num;

					/*copy other canvas as usual*/
					for (int other = 0; other < MAX_CANVAS; other++) {
						if ((other != merge_num) && (other != add_num)) {
							libosd_log_debug(" %d", other);
							memcpy(&tmp_hd_loop[loop_idx].canvas[other],
							       &phd->canvas[other], sizeof(OsdCanvas));
						}
					}
				}

				/*calc area of this loop*/
				for (int i = 0; i < MAX_CANVAS; i++) {
					tmp_area_loop[loop_idx] += tmp_hd_loop[loop_idx].canvas[i].width *
					                           tmp_hd_loop[loop_idx].canvas[i].height;
				}

				libosd_log_debug(", area: %d", tmp_area_loop[loop_idx]);
				logAllOsdHandle(&tmp_hd_loop[loop_idx]);

				loop_idx += 1;
			}
		}

		/*all*/
		tmp_hd_loop[10].canvas[0].osd_num = new_calc_num + 1;
		int startx_list_all[new_calc_num + 1];
		int starty_list_all[new_calc_num + 1];
		int endx_list_all[new_calc_num + 1];
		int endy_list_all[new_calc_num + 1];
		for (int i = 0; i < new_calc_num + 1; i++) {
			tmp_hd_loop[10].canvas[0].osd_list[i] = phd->osd_index[add_list[i]];
			tmp_hd_loop[10].region[add_list[i]].include_canvas = 0;
			startx_list_all[i] = (phd->region[add_list[i]].startX);
			starty_list_all[i] = (phd->region[add_list[i]].startY);
			endx_list_all[i] = (phd->region[add_list[i]].startX + phd->region[add_list[i]].width);
			endy_list_all[i] = (phd->region[add_list[i]].startY + phd->region[add_list[i]].height);
		}
		tmp_hd_loop[10].canvas[0].startX = get_min(&startx_list_all[0], new_calc_num + 1);
		tmp_hd_loop[10].canvas[0].startY = get_min(&starty_list_all[0], new_calc_num + 1);
		tmp_hd_loop[10].canvas[0].width =
		        get_max(&endx_list_all[0], new_calc_num + 1) - tmp_hd_loop[10].canvas[0].startX;
		tmp_hd_loop[10].canvas[0].height =
		        get_max(&endy_list_all[0], new_calc_num + 1) - tmp_hd_loop[10].canvas[0].startY;
		tmp_area_loop[10] = tmp_hd_loop[10].canvas[0].width * tmp_hd_loop[10].canvas[0].height;
		libosd_log_debug("case all: (%d, %d, %d, %d) area; %d", tmp_hd_loop[10].canvas[0].startX,
		                 tmp_hd_loop[10].canvas[0].startY, tmp_hd_loop[10].canvas[0].width,
		                 tmp_hd_loop[10].canvas[0].height, tmp_area_loop[10]);
#ifdef OSD_DEBUG
		logAllOsdHandle(phd);
#endif

		int min_idx;
		get_min_idx(&tmp_area_loop[0], 11, &min_idx);
		libosd_log_debug("min[%d]:%d", min_idx, tmp_area_loop[min_idx]);

		memcpy(&phd->canvas[0], &tmp_hd_loop[min_idx].canvas[0], sizeof(OsdCanvas) * 4);
		for (int i = 0; i < new_calc_num + 1; i++) {
			phd->region[add_list[i]].include_canvas =
			        tmp_hd_loop[min_idx].region[add_list[i]].include_canvas;
		}
#ifdef OSD_DEBUG
		logAllOsdHandle(phd);
#endif
		loop_idx = 0;
		memset(&tmp_area_loop[0], 0, sizeof(tmp_area_loop));
		memset(&tmp_hd_loop[0], 0, sizeof(tmp_hd_loop));
	}

	for (int i = 0; i < MAX_CANVAS; i++) {
		phd->canvas[i].width =
		        CEILINGALIGN16(phd->canvas[i].startX + phd->canvas[i].width - ALIGN16(phd->canvas[i].startX));
		phd->canvas[i].startX = ALIGN16(phd->canvas[i].startX);
		phd->canvas[i].height =
		        CEILINGALIGN16(phd->canvas[i].startY + phd->canvas[i].height - ALIGN16(phd->canvas[i].startY));
		phd->canvas[i].startY = ALIGN16(phd->canvas[i].startY);
	}
#ifdef OSD_DEBUG
	logAllOsdHandle(phd);
#endif
	return 0;
}

int OSD_calcCanvas(OsdHandle *phd)
{
	if (phd->osd_num == 0) {
		libosd_log_err("handle has no osd");
		return -EINVAL;
	}

	if (phd->osd_num > 4) {
		libosd_log_err("handle osd > 4");
		return -EINVAL;
	}

	for (int i = 0; i < MAX_CANVAS; i++) {
		memset(&phd->canvas[i], 0xff, sizeof(OsdCanvas));
	}
	/*grouping algo*/
	int add_list[8] = { 0 };
	int add_idx = 0;
	for (int i = 0; i < MAX_OSD; i++) {
		if (phd->osd_index[i] != -1) {
			libosd_log_debug("add %d", phd->osd_index[i]);
			add_list[add_idx] = phd->osd_index[i];
			add_idx++;
		}
	}

	if (add_idx > phd->osd_num) {
		libosd_log_err("add idx != region num: %d, %d", add_idx, phd->osd_num);
		return -EINVAL;
	}
	for (int i = 0; i < phd->osd_num; i++) {
		phd->canvas[i].osd_num = 1;
		phd->canvas[i].osd_list[0] = add_list[i];
		phd->canvas[i].startX = ALIGN16(phd->region[add_list[i]].startX);
		phd->canvas[i].startY = ALIGN16(phd->region[add_list[i]].startY);
		phd->canvas[i].width = CEILINGALIGN16(phd->region[add_list[i]].startX + phd->region[add_list[i]].width -
		                                      phd->canvas[i].startX);
		phd->canvas[i].height = CEILINGALIGN16(phd->region[add_list[i]].startY +
		                                       phd->region[add_list[i]].height - phd->canvas[i].startY);
		phd->region[add_list[i]].include_canvas = i;

		libosd_log_debug("Assign canvas[%d] = osd [%d] %d , (%d, %d, %d, %d)", i, i, add_list[i],
		                 phd->canvas[i].startX, phd->canvas[i].startY, phd->canvas[i].width,
		                 phd->canvas[i].height);
	}
	return 0;
}

int OSD_setTextUnicodewithOutLine(OsdHandle *hd, int osd_idx, OsdText *txt, char *out)
{
#ifdef SDL_ENABLE

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdl2ttf");
		SDL_Quit();
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	fontout = TTF_OpenFont(txt->ttf_path, txt->size);
	TTF_SetFontOutline(fontout, txt->outline_width);

	if ((font == NULL) || (fontout == NULL)) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		TTF_Quit();
		return -EINVAL;
	}

	SDL_Surface *text, *textout;

	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color outcol = { txt->outline_color[0], txt->outline_color[1], txt->outline_color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };
	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}

	libosd_log_debug("for (%d, %d, %d) bak (%d, %d, %d) out (%d, %d, %d), size: %d", forecol.r, forecol.g,
	                 forecol.b, backcol.r, backcol.g, backcol.b, outcol.r, outcol.g, outcol.b, txt->outline_width);

	text = TTF_RenderUNICODE_Blended(font, (uint16_t *)&txt->unicode_txt[0], forecol);
	textout = TTF_RenderUNICODE_Blended(fontout, (uint16_t *)&txt->unicode_txt[0], outcol);
	SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

	SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
	SDL_BlitSurface(text, NULL, textout, &rect);

	if (out == NULL) {
		libosd_log_debug("ptr == NULL, save to /mnt/nfs/ethnfs/text-out-unicode.bmp");
		SDL_SaveBMP(textout, "/mnt/nfs/ethnfs/text-out-unicode.bmp");
	}

	libosd_log_debug("load bmp (w,h) = %d %d", textout->w, textout->h);

	char *bgra = (char *)textout->pixels;
	int width, height, y, u, v, i, j;
	width = textout->w;
	height = textout->h;
	int r;
	int g;
	int b;
	int a = 255;

	char ayuv_buf[width * height * 3];
	int ayuv_idx = 0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width * 4; j += 4) {
			b = (int)bgra[i * width * 4 + j];
			g = (int)bgra[i * width * 4 + j + 1];
			r = (int)bgra[i * width * 4 + j + 2];
			a = (int)bgra[i * width * 4 + j + 3];

			if (txt->background != TRANSPARENT) {
				if (a == 0) {
					b = backcol.b;
					g = backcol.g;
					r = backcol.r;
					a = 255;
				}
			}

			RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
			normalizeAYUV3544(&a, &y, &u, &v);

			ayuv_buf[ayuv_idx] = MERGE44(u, v);
			ayuv_idx += 1;
			ayuv_buf[ayuv_idx] = MERGE35(a, y);
			ayuv_idx += 1;
		}
	}

	alignUVVal(&ayuv_buf[0], width, height);

	/*load bmp & convert*/
	if (out == NULL) {
		libosd_log_debug("out ptr not found, save: /mnt/nfs/ethnfs/save-out-unicode.ayuv");
		saveAYUV("/mnt/nfs/ethnfs/save-out-unicode.ayuv", width, height, &ayuv_buf[0], width * height * 2);
		SDL_FreeSurface(text);
		SDL_FreeSurface(textout);
		TTF_CloseFont(font);
		TTF_CloseFont(fontout);
		TTF_Quit();

		return -EINVAL;
	}

	SDL_FreeSurface(text);
	SDL_FreeSurface(textout);
	TTF_CloseFont(font);
	TTF_CloseFont(fontout);
	TTF_Quit();
#else
	unsigned int width = 0;
	unsigned int height = 0;
	int tmp_idx = -1;

	char *ayuv_buf = generatOutlineChartoList(txt, UNICODE, &width, &height);
	if (ayuv_buf == NULL) {
		libosd_log_err("get outline list NULL");
		return -EINVAL;
	}
#endif
	tmp_idx = findOSDRegion(hd, osd_idx);
	libosd_log_debug("get region idx:%d", tmp_idx);
#ifdef SDL_ENABLE
	copy2Region(hd, osd_idx, tmp_idx, &ayuv_buf[0], (int)width, (int)height, AYUV_3544, out);
#else
	copy2Region(hd, osd_idx, tmp_idx, ayuv_buf, (int)width, (int)height, AYUV_3544, out);
#endif
	return 0;
}

int OSD_setTextUnicode(OsdHandle *hd, int osd_idx, OsdText *txt, char *out)
{
	int ret = 0;
	if (txt->outline_width > 0) {
		libosd_log_debug("go to outline ");
		OSD_setTextUnicodewithOutLine(hd, osd_idx, txt, out);
		return 0;
	}

#ifdef SDL_ENABLE

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf, %s", SDL_GetError());
		SDL_Quit();
		return -EPERM;
	}

	TTF_Font *font;
	font = TTF_OpenFont(txt->ttf_path, txt->size);

	SDL_Surface *text, *tmp;
	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };

	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}

	text = TTF_RenderUNICODE_Shaded(font, (uint16_t *)&txt->unicode_txt[0], forecol, backcol);

	if (text == NULL) {
		libosd_log_err("Invaild Unicode word");
		SDL_FreeSurface(text);
		ret = -EINVAL;
		return ret;
	}

	SDL_PixelFormat *fmt;
	fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
	if (fmt == NULL) {
		libosd_log_err("failed to alloc a SDL_PixelFormat");
		return -EINTR;
	}
	memset(fmt, 0, sizeof(SDL_PixelFormat));

	fmt->BitsPerPixel = 32;
	fmt->BytesPerPixel = 4;

	tmp = SDL_ConvertSurface(text, fmt, 0);
	if (tmp == NULL) {
		libosd_log_err("failed convert interface, %s", SDL_GetError());
		free(fmt);
		return -EINTR;
	}

	free(fmt);

	libosd_log_debug("save %s, fcolor: %0x %0x %0x", "/tmp/save.bmp", txt->color[0], txt->color[1], txt->color[2]);

	int width, height;
	width = tmp->w;
	height = tmp->h;
	char *bgra = (char *)tmp->pixels;
	libosd_log_debug("get (%d, %d)", width, height);

	char ayuv_buf[width * height * 2];
	int y, u, v, i, j;
	int r;
	int g;
	int b;
	int a = 255;
	int ayuv_idx = 0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width * 4; j += 4) {
			b = (int)bgra[i * width * 4 + j];
			g = (int)bgra[i * width * 4 + j + 1];
			r = (int)bgra[i * width * 4 + j + 2];
			a = 255;

			if (txt->background == TRANSPARENT) {
				if (((b == backcol.b) && (g == backcol.g)) && (r == backcol.r)) {
					a = 0x00;
				} else {
					a = 255;
				}
			}

			RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
			normalizeAYUV3544(&a, &y, &u, &v);

			ayuv_buf[ayuv_idx] = MERGE44(u, v);
			ayuv_idx += 1;
			ayuv_buf[ayuv_idx] = MERGE35(a, y);
			ayuv_idx += 1;
		}
	}

	alignUVVal(&ayuv_buf[0], width, height);

	SDL_FreeSurface(text);
	SDL_FreeSurface(tmp);
	TTF_CloseFont(font);
	TTF_Quit();
#else

	unsigned int len = getUnicodeTxtListLen(txt);
	if (len == 0) {
		libosd_log_err("txt has no unicode");
	}

	char *ayuv_buf;

	unsigned int width = 0;
	unsigned int height = 0;

	ayuv_buf = generatNoOutlineChartoList(txt, UNICODE, &width, &height);
	if (ayuv_buf == NULL) {
		libosd_log_err("failed to get ayuv output");
		return -EINVAL;
	}

#endif

	int tmp_idx = -1;
	tmp_idx = findOSDRegion(hd, osd_idx);

	if (out == NULL) {
		libosd_log_debug("out ptr is null");
		saveAYUV("/tmp/save-1.ayuv", width, height, &ayuv_buf[0], width * height * 2);

		ret = -EINVAL;
		return ret;
	}
#ifdef SDL_ENABLE
	copy2Region(hd, osd_idx, tmp_idx, &ayuv_buf[0], width, height, AYUV_3544, out);
#else
	copy2Region(hd, osd_idx, tmp_idx, ayuv_buf, width, height, AYUV_3544, out);
#endif
	return 0;
}

int OSD_setImage(OsdHandle *hd, int osd_idx, const char *image_path, char *out)
{
	int tmp_idx = -1;

	FILE *fp;
	fp = fopen(image_path, "rb");
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

	if (out == NULL) {
		saveAYUV("/mnt/nfs/ethnfs/save-logo.ayuv", img_width, img_height, &ayuv_buf[0],
		         img_width * img_height * 2);
		libosd_log_info("save to save-logo.ayuv");
		return 0;
	}

	tmp_idx = findOSDRegion(hd, osd_idx);
	if (tmp_idx == -EINVAL) {
		libosd_log_err("index %d not found", osd_idx);
		return -EINVAL;
	}

	copy2Region(hd, osd_idx, tmp_idx, &ayuv_buf[0], img_width, img_height, AYUV_3544, out);
	libosd_log_debug("start cp");
	return 0;
}

int OSD_setImageBmp(OsdHandle *hd, int osd_idx, const char *image_path, char *out)
{
	int tmp_idx = -1;
	tmp_idx = findOSDRegion(hd, osd_idx);

	if (tmp_idx == -1) {
		libosd_log_err("index %d not found", osd_idx);
		return -EINVAL;
	}

	int width = 0;
	int height = 0;
	int inline_offset = 0;
	getBmpSize(image_path, &width, &height, &inline_offset);
	char ayuv_buf[width * height * 3];
	readBmp2AYUV(image_path, NULL, WHITE, width, height, inline_offset, &ayuv_buf[0]);

	if (out == NULL) {
		libosd_log_err("out ptr not found");
		saveAYUV("/mnt/nfs/ethnfs/save-out.ayuv", width, height, &ayuv_buf[0], width * height * 2);
		return -EINVAL;
	}

	copy2Region(hd, osd_idx, tmp_idx, &ayuv_buf[0], width, height, AYUV_3544, out);
	libosd_log_info("get size %d %d, at %d", width, height, tmp_idx);

	return 0;
}

int OSD_setTextUTF8withOutLine(OsdHandle *hd, int osd_idx, OsdText *txt, char *out)
{
#ifdef SDL_ENABLE
	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdl2ttf: %s", SDL_GetError());
		SDL_Quit();
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	fontout = TTF_OpenFont(txt->ttf_path, txt->size);
	TTF_SetFontOutline(fontout, txt->outline_width);

	if ((font == NULL) || (fontout == NULL)) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		TTF_Quit();
		return -EINVAL;
	}

	SDL_Surface *text, *textout;

	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color outcol = { txt->outline_color[0], txt->outline_color[1], txt->outline_color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };
	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}

	libosd_log_debug("for (%d, %d, %d) bak (%d, %d, %d) out (%d, %d, %d), size: %d", forecol.r, forecol.g,
	                 forecol.b, backcol.r, backcol.g, backcol.b, outcol.r, outcol.g, outcol.b, txt->outline_width);

	char txt_tmp[64];
	snprintf(&txt_tmp[0], 64, "%s", txt->txt);

	text = TTF_RenderUTF8_Blended(font, txt_tmp, forecol);
	textout = TTF_RenderUTF8_Blended(fontout, txt_tmp, outcol);
	if ((text == NULL) || (textout == NULL)) {
		libosd_log_err("Invaild UTF8 word");
	}
	SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

	SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
	SDL_BlitSurface(text, NULL, textout, &rect);

	if (out == NULL) {
		SDL_SaveBMP(textout, "/mnt/nfs/ethnfs/text-out.bmp");
	}

	libosd_log_debug("load bmp (w,h) = %d %d", textout->w, textout->h);

	char *bgra = (char *)textout->pixels;
	int width, height, y, u, v, i, j;
	width = textout->w;
	height = textout->h;
	int r;
	int g;
	int b;
	int a = 0x07;

	char ayuv_buf[width * height * 3];
	int ayuv_idx = 0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width * 4; j += 4) {
			b = (int)bgra[i * width * 4 + j];
			g = (int)bgra[i * width * 4 + j + 1];
			r = (int)bgra[i * width * 4 + j + 2];
			a = (int)bgra[i * width * 4 + j + 3];

			if (txt->background != TRANSPARENT) {
				if (a == 0) {
					b = backcol.b;
					g = backcol.g;
					r = backcol.r;
					a = 255;
				}
			}

			RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
			normalizeAYUV3544(&a, &y, &u, &v);

			ayuv_buf[ayuv_idx] = MERGE44(u, v);
			ayuv_idx += 1;
			ayuv_buf[ayuv_idx] = MERGE35(a, y);
			ayuv_idx += 1;
		}
	}

	alignUVVal(&ayuv_buf[0], width, height);

	/*load bmp & convert*/
	if (out == NULL) {
		libosd_log_err("out ptr not found, save: /mnt/nfs/ethnfs/save-out.ayuv");
		saveAYUV("/mnt/nfs/ethnfs/save-out.ayuv", width, height, &ayuv_buf[0], width * height * 2);
		SDL_FreeSurface(text);
		SDL_FreeSurface(textout);
		TTF_CloseFont(font);
		TTF_CloseFont(fontout);
		TTF_Quit();

		return -EINVAL;
	}

	SDL_FreeSurface(text);
	SDL_FreeSurface(textout);
	TTF_CloseFont(font);
	TTF_CloseFont(fontout);
	TTF_Quit();
#else
	unsigned int width, height;
	char *ayuv_buf = generatOutlineChartoList(txt, UTF8, &width, &height);
	if (ayuv_buf == NULL) {
		libosd_log_err("get outline list NULL");
		return -EINVAL;
	}
#endif

	int tmp_idx = findOSDRegion(hd, osd_idx);
#ifdef SDL_ENABLE
	copy2Region(hd, osd_idx, tmp_idx, &ayuv_buf[0], (int)width, (int)height, AYUV_3544, out);
#else
	copy2Region(hd, osd_idx, tmp_idx, ayuv_buf, (int)width, (int)height, AYUV_3544, out);
#endif
	return 0;
}

int OSD_setTextUTF8(OsdHandle *hd, int osd_idx, OsdText *txt, char *out)
{
	if (txt->outline_width > 0) {
		libosd_log_debug("go to outline ");
		OSD_setTextUTF8withOutLine(hd, osd_idx, txt, out);
		return 0;
	}
#ifdef SDL_ENABLE

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdl2ttf:%s", SDL_GetError());
		SDL_Quit();
	}

	TTF_Font *font;
	font = TTF_OpenFont(txt->ttf_path, txt->size);

	if (font == NULL) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		TTF_Quit();
		return -EINVAL;
	}

	SDL_PixelFormat *fmt;
	SDL_Surface *text, *tmp;

	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };

	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}

	libosd_log_debug("fcolor(%d, %d, %d), bcolor(%d, %d, %d)", forecol.r, forecol.g, forecol.b, backcol.r,
	                 backcol.g, backcol.b);

	char txt_tmp[64];
	snprintf(&txt_tmp[0], 64, "%s", txt->txt);

	text = TTF_RenderUTF8_Shaded(font, txt_tmp, forecol, backcol);
	if (text == NULL) {
		libosd_log_err("Invaild UTF8 word: %s", SDL_GetError());
	}

	fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
	if (fmt == NULL) {
		libosd_log_err("failed to alloc a SDL_PixelFormat");
	}
	memset(fmt, 0, sizeof(SDL_PixelFormat));

	fmt->BitsPerPixel = 32;
	fmt->BytesPerPixel = 4;

	tmp = SDL_ConvertSurface(text, fmt, 0);
	if (tmp == NULL) {
		libosd_log_err("failed convert interface, %s", SDL_GetError());
		free(fmt);
		return -EINVAL;
	}

	free(fmt);

	libosd_log_debug("load bmp (w,h) = %d %d", tmp->w, tmp->h);

	int width, height, y, u, v, i, j;
	int r;
	int g;
	int b;
	int a = 255;

	width = tmp->w;
	height = tmp->h;

	char *bgra = (char *)tmp->pixels;

	char ayuv_buf[width * height * 2];
	int ayuv_idx = 0;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width * 4; j += 4) {
			b = (int)bgra[i * width * 4 + j];
			g = (int)bgra[i * width * 4 + j + 1];
			r = (int)bgra[i * width * 4 + j + 2];
			a = 255;

			if (txt->background == TRANSPARENT) {
				if (((b == backcol.b) && (g == backcol.g)) && (r == backcol.r)) {
					a = 0x00;
				} else {
					a = 255;
				}
			}

			RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
			normalizeAYUV3544(&a, &y, &u, &v);

			ayuv_buf[ayuv_idx] = MERGE44(u, v);
			ayuv_idx += 1;
			ayuv_buf[ayuv_idx] = MERGE35(a, y);
			ayuv_idx += 1;
		}
	}

	alignUVVal(&ayuv_buf[0], width, height);

	/*load bmp & convert*/
	if (out == NULL) {
		libosd_log_debug("out ptr not found");
		saveAYUV("/mnt/nfs/ethnfs/save-out.ayuv", width, height, &ayuv_buf[0], width * height * 2);
		SDL_FreeSurface(tmp);
		return -EINVAL;
	}

	SDL_FreeSurface(text);
	SDL_FreeSurface(tmp);
	TTF_CloseFont(font);
	TTF_Quit();
#else
	if (getUTF8TxtListLen(txt) <= 0) {
		libosd_log_err("txt has no unicode");
		return -EINVAL;
	}

	char *ayuv_buf;

	unsigned int width = 0;
	unsigned int height = 0;

	ayuv_buf = generatNoOutlineChartoList(txt, UTF8, &width, &height);
	if (ayuv_buf == NULL) {
		libosd_log_err("failed to get ayuv output");
		return -EINVAL;
	}

	/*load bmp & convert*/
	if (out == NULL) {
		libosd_log_debug("out ptr not found");
		saveAYUV("/mnt/nfs/ethnfs/save-utf8.ayuv", width, height, &ayuv_buf[0], width * height * 2);
		return -EINVAL;
	}

#endif

	int tmp_idx = findOSDRegion(hd, osd_idx);
#ifdef SDL_ENABLE
	copy2Region(hd, osd_idx, tmp_idx, &ayuv_buf[0], (int)width, (int)height, AYUV_3544, out);
#else
	copy2Region(hd, osd_idx, tmp_idx, ayuv_buf, (int)width, (int)height, AYUV_3544, out);
#endif
	return 0;
}

char *OSD_createTextUTF8Src(OsdText *txt, int *width, int *height)
{
	char *src;
#ifdef SDL_ENABLE
	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf:%s", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);

	SDL_Surface *text = NULL;
	SDL_Surface *tmp = NULL;
	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };
	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}

	if (txt->outline_width == 0) {
		SDL_PixelFormat *fmt;
		fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
		if (fmt == NULL) {
			libosd_log_err("failed to alloc a SDL_PixelFormat");
			return NULL;
		}
		memset(fmt, 0, sizeof(SDL_PixelFormat));
		fmt->BitsPerPixel = 32;
		fmt->BytesPerPixel = 4;

		text = TTF_RenderUTF8_Shaded(font, &txt->txt[0], forecol, backcol);
		if (text == NULL) {
			libosd_log_err("Invaild utf8 word");
			SDL_FreeSurface(text);
			TTF_CloseFont(font);
			free(fmt);
			return NULL;
		}
		tmp = SDL_ConvertSurface(text, fmt, 0);
		if (tmp == NULL) {
			libosd_log_err("failed convert interface, %s", SDL_GetError());
			return NULL;
		}
#ifdef OSD_DEBUG
		char bmp_name[32];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%s.bmp", &txt->txt[0]);

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
		free(fmt);
	} else {
		fontout = TTF_OpenFont(txt->ttf_path, txt->size);
		TTF_SetFontOutline(fontout, txt->outline_width);

		if (fontout == NULL) {
			libosd_log_err("failed to open font: %s", SDL_GetError());
			TTF_Quit();
			return NULL;
		}

		SDL_Color outcol = { txt->outline_color[0], txt->outline_color[1], txt->outline_color[2] };
		text = TTF_RenderUTF8_Blended(font, &txt->txt[0], forecol);
		tmp = TTF_RenderUTF8_Blended(fontout, &txt->txt[0], outcol);
		if ((text == NULL) || (tmp == NULL)) {
			libosd_log_err("Invaild UTF8 word. %s", SDL_GetError());
			return -EINVAL;
		}

		SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

		SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
		SDL_BlitSurface(text, NULL, tmp, &rect);
#ifdef OSD_DEBUG
		char bmp_name[32];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%s-outline.bmp", &txt->txt[0]);

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
	}

	if (tmp == NULL) {
		libosd_log_err("Invaild UTF8 word. %s", SDL_GetError());
		return -EINVAL;
	}

	libosd_log_debug("alloc size: %d %d %d", tmp->w, tmp->h, tmp->w * tmp->h * 2);
	src = malloc(tmp->w * tmp->h * 2);
	int ayuv_idx = 0;
	*width = tmp->w;
	*height = tmp->h;
	char *bgra = (char *)tmp->pixels;
	int i, j, a, y, u, v, r, g, b;

	for (i = 0; i < tmp->h; i++) {
		for (j = 0; j < tmp->w * 4; j += 4) {
			b = (int)bgra[i * tmp->w * 4 + j];
			g = (int)bgra[i * tmp->w * 4 + j + 1];
			r = (int)bgra[i * tmp->w * 4 + j + 2];
			a = 255;

			if (txt->outline_width == 0) {
				a = 255;
				if (txt->background == TRANSPARENT) {
					if (((b == backcol.b) && (g == backcol.g)) && (r == backcol.r)) {
						a = 0x00;
					} else {
						a = 255;
					}
				}
			} else {
				a = (int)bgra[i * tmp->w * 4 + j + 3];
				if (txt->background != TRANSPARENT) {
					if (a == 0) {
						b = backcol.b;
						g = backcol.g;
						r = backcol.r;
						a = 255;
					}
				}
			}

			RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
			normalizeAYUV3544(&a, &y, &u, &v);
			src[ayuv_idx] = MERGE44(u, v);
			ayuv_idx += 1;
			src[ayuv_idx] = MERGE35(a, y);
			ayuv_idx += 1;
		}
	}

	alignUVVal(&src[0], tmp->w, tmp->h);

	SDL_FreeSurface(tmp);
	SDL_FreeSurface(text);

	TTF_CloseFont(font);

	if (txt->outline_width > 0) {
		TTF_CloseFont(fontout);
	}

	TTF_Quit();
#else
	unsigned int w = 0;
	unsigned int h = 0;
	if (txt->outline_width == 0) {
		src = generatNoOutlineChartoList(txt, UTF8, &w, &h);
		if (src == NULL) {
			libosd_log_err("failed to get ayuv output");
			return NULL;
		}

		*width = w;
		*height = h;
	} else {
		/*need change to AYUV*/
		src = generatOutlineChartoList(txt, UTF8, &w, &h);
		if (src == NULL) {
			libosd_log_err("get outline list NULL");
			return NULL;
		}

		*width = w;
		*height = h;
	}

#if 0
	char tmpfile[32];
	snprintf(&tmpfile[0], 32, "./%s.ayuv", "list-OSD_createTextUTF8Src");
	FILE *fp = fopen(tmpfile, "w");
	fwrite(src, 1, (*width) * (*height), fp);
	fclose(fp);

	saveAYUV(&tmpfile[0], (*width), (*height), src, (*width) * (*height) * 2);
#endif
#endif
	return src;
}

char *OSD_createTextUTF8Src8bit(OsdText *txt, int *width, int *height)
{
	char *src;
#ifdef SDL_ENABLE
	if (txt->mode != PALETTE_8) {
		libosd_log_err("wrong format");
		return NULL;
	}

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf");
		SDL_Quit();
		return NULL;
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	if (font == NULL) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		TTF_Quit();
		return NULL;
	}

	SDL_Surface *text = NULL;
	SDL_Surface *tmp = NULL;
	SDL_Color forecol = { 0xff, 0x00, 0x00 };
	SDL_Color backcol = { 0x00, 0x00, 0x00 };
	if (txt->outline_width > 0) {
		fontout = TTF_OpenFont(txt->ttf_path, txt->size);
		TTF_SetFontOutline(fontout, txt->outline_width);

		if (fontout == NULL) {
			libosd_log_err("failed to open font: %s", SDL_GetError());
			TTF_Quit();
			return NULL;
		}
		SDL_Color outcol = { 0x00, 0xff, 0x00 };

		text = TTF_RenderUTF8_Blended(font, &txt->txt[0], forecol);
		tmp = TTF_RenderUTF8_Blended(fontout, &txt->txt[0], outcol);
		if ((text == NULL) || (tmp == NULL)) {
			libosd_log_err("Invaild UTF8 word");
		}

		SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

		SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
		SDL_BlitSurface(text, NULL, tmp, &rect);
#ifdef OSD_DEBUG
		char bmp_name[64];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%s-outline-8bpp.bmp", &txt->txt[0]);

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
	} else {
		SDL_PixelFormat *fmt;
		fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
		if (fmt == NULL) {
			libosd_log_err("failed to alloc a SDL_PixelFormat");
		}
		memset(fmt, 0, sizeof(SDL_PixelFormat));
		fmt->BitsPerPixel = 32;
		fmt->BytesPerPixel = 4;

		text = TTF_RenderUTF8_Shaded(font, &txt->txt[0], forecol, backcol);
		if (text == NULL) {
			libosd_log_err("Invaild utf8 word: %s", SDL_GetError());
			SDL_FreeSurface(text);
			TTF_CloseFont(font);
			free(fmt);
			return NULL;
		}
		tmp = SDL_ConvertSurface(text, fmt, 0);
		if (tmp == NULL) {
			libosd_log_err("failed convert interface, %s", SDL_GetError());
			return -EINVAL;
		}
#ifdef OSD_DEBUG
		char bmp_name[32];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%s-8bpp.bmp", &txt->txt[0]);

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
		free(fmt);
	}

	libosd_log_debug("alloc size: %d %d %d", tmp->w, tmp->h, tmp->w * tmp->h);
	src = malloc(tmp->w * tmp->h);
	int ayuv_idx = 0;
	*width = tmp->w;
	*height = tmp->h;
	char *bgra = (char *)tmp->pixels;
	int i, j, a, r, g, b;

	for (i = 0; i < tmp->h; i++) {
		for (j = 0; j < tmp->w * 4; j += 4) {
			b = (int)bgra[i * tmp->w * 4 + j];
			g = (int)bgra[i * tmp->w * 4 + j + 1];
			r = (int)bgra[i * tmp->w * 4 + j + 2];
			a = (int)bgra[i * tmp->w * 4 + j + 3];

			if (txt->outline_width > 0) {
				if ((r > g) && (r > b)) {
					src[ayuv_idx] = MERGE35(txt->color[0], txt->color[1]);
				} else if (a != 0) {
					/*outline*/
					src[ayuv_idx] = MERGE35(txt->outline_color[0], txt->outline_color[1]);
				} else {
					a = 255;
					if (txt->background == TRANSPARENT) {
						src[ayuv_idx] = MERGE35(0, 0x00);
					} else if (txt->background == WHITE) {
						src[ayuv_idx] = MERGE35(a, 0xff);
					} else if (txt->background == BLACK) {
						src[ayuv_idx] = MERGE35(a, 0x00);
					}
				}
			} else if (txt->outline_width == 0) {
				a = 255;
				if ((r > g) && (r > b)) {
					src[ayuv_idx] = MERGE35(txt->color[0], txt->color[1]);
				} else {
					if (txt->background == TRANSPARENT) {
						src[ayuv_idx] = MERGE35(0, 0x00);
					} else if (txt->background == WHITE) {
						src[ayuv_idx] = MERGE35(a, 0xff);
					} else if (txt->background == BLACK) {
						src[ayuv_idx] = MERGE35(a, 0x00);
					}
				}
			}
			ayuv_idx += 1;
		}
	}

	SDL_FreeSurface(tmp);
	SDL_FreeSurface(text);

	TTF_CloseFont(font);

	if (txt->outline_width > 0) {
		TTF_CloseFont(fontout);
	}

	TTF_Quit();
#else

	unsigned int w = 0;
	unsigned int h = 0;
	if (txt->outline_width == 0) {
		src = generatNoOutlineChartoList(txt, UTF8, &w, &h);
		if (src == NULL) {
			libosd_log_err("failed to get ayuv output");
			return NULL;
		}
		*width = w;
		*height = h;
	} else {
		/*need change to Palatte 8*/
		src = generatOutlineChartoList(txt, UTF8, &w, &h);
		if (src == NULL) {
			libosd_log_err("get outline list NULL");
			return NULL;
		}
		*width = w;
		*height = h;
	}
#endif

#if 0
	char tmpfile[32];
	snprintf(&tmpfile[0], 32, "./%s.bin", "list-UTF8-palette");
	FILE *fp = fopen(tmpfile, "w");
	if (txt->mode == AYUV_3544) {
		fwrite(src, 1, (*width) * (*height) *2, fp);
	} else if (txt->mode == PALETTE_8) {
		fwrite(src, 1, (*width) * (*height), fp);
	}
	fclose(fp);
#endif

	return src;
}

char *OSD_createTextUnicodeSrc(OsdText *txt, int *width, int *height)
{
	char *src = NULL;

#ifdef SDL_ENABLE
	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf");
		SDL_Quit();
		return NULL;
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	if (font == NULL) {
		libosd_log_err("failed to open font, %s", SDL_GetError());
		return NULL;
	}

	SDL_Surface *text = NULL;
	SDL_Surface *tmp = NULL;
	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };
	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}

	if (txt->outline_width == 0) {
		SDL_PixelFormat *fmt;
		fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
		if (fmt == NULL) {
			libosd_log_err("failed to alloc a SDL_PixelFormat");
		}
		memset(fmt, 0, sizeof(SDL_PixelFormat));
		fmt->BitsPerPixel = 32;
		fmt->BytesPerPixel = 4;
		text = TTF_RenderUNICODE_Shaded(font, (uint16_t *)&txt->unicode_txt[0], forecol, backcol);
		if (text == NULL) {
			libosd_log_err("Invaild Unicode word, %s", SDL_GetError());
			SDL_FreeSurface(text);
			TTF_CloseFont(font);
			free(fmt);
			return NULL;
		}
		tmp = SDL_ConvertSurface(text, fmt, 0);
		if (tmp == NULL) {
			libosd_log_err("failed convert interface, %s", SDL_GetError());
		}
#ifdef OSD_DEBUG
		char bmp_name[32];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%d.bmp", (int)&txt->unicode_txt[0]);

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
		free(fmt);
	} else {
		fontout = TTF_OpenFont(txt->ttf_path, txt->size);
		TTF_SetFontOutline(fontout, txt->outline_width);

		if (fontout == NULL) {
			libosd_log_err("failed to open font: %s", SDL_GetError());
			TTF_Quit();
			return NULL;
		}

		SDL_Color outcol = { txt->outline_color[0], txt->outline_color[1], txt->outline_color[2] };
		text = TTF_RenderUNICODE_Blended(font, (uint16_t *)&txt->unicode_txt[0], forecol);
		tmp = TTF_RenderUNICODE_Blended(fontout, (uint16_t *)&txt->unicode_txt[0], outcol);
		if ((text == NULL) || (tmp == NULL)) {
			libosd_log_err("Invaild Unicode word");
			SDL_FreeSurface(text);
			TTF_CloseFont(font);
			TTF_CloseFont(fontout);
			free(tmp);
			return NULL;
		}

		SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

		SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
		SDL_BlitSurface(text, NULL, tmp, &rect);
#ifdef OSD_DEBUG
		char bmp_name[32];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%s-outline.bmp", &txt->txt[0]);

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
	}

	if (tmp == NULL) {
		libosd_log_err("Invaild UTF8 word. %s", SDL_GetError());
		return -EINVAL;
	}

	src = malloc(tmp->w * tmp->h * 2);
	int ayuv_idx = 0;
	*width = tmp->w;
	*height = tmp->h;
	char *bgra = (char *)tmp->pixels;
	int i, j, a, y, u, v, r, g, b;

	for (i = 0; i < tmp->h; i++) {
		for (j = 0; j < tmp->w * 4; j += 4) {
			b = (int)bgra[i * tmp->w * 4 + j];
			g = (int)bgra[i * tmp->w * 4 + j + 1];
			r = (int)bgra[i * tmp->w * 4 + j + 2];
			a = 255;

			if (txt->outline_width == 0) {
				a = 255;
				if (txt->background == TRANSPARENT) {
					if (((b == backcol.b) && (g == backcol.g)) && (r == backcol.r)) {
						a = 0x00;
					} else {
						a = 255;
					}
				}
			} else {
				a = (int)bgra[i * tmp->w * 4 + j + 3];
				if (txt->background != TRANSPARENT) {
					if (a == 0) {
						b = backcol.b;
						g = backcol.g;
						r = backcol.r;
						a = 255;
					}
				}
			}

			RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
			normalizeAYUV3544(&a, &y, &u, &v);
			src[ayuv_idx] = MERGE44(u, v);
			ayuv_idx += 1;
			src[ayuv_idx] = MERGE35(a, y);
			ayuv_idx += 1;
		}
	}

	SDL_FreeSurface(tmp);
	SDL_FreeSurface(text);

	TTF_CloseFont(font);

	if (txt->outline_width > 0) {
		TTF_CloseFont(fontout);
	}

	TTF_Quit();
#else
	unsigned int w = 0;
	unsigned int h = 0;
	if (txt->outline_width == 0) {
		src = generatNoOutlineChartoList(txt, UNICODE, &w, &h);
		if (src == NULL) {
			libosd_log_err("failed to get ayuv output");
			return NULL;
		}
		*width = w;
		*height = h;

	} else {
		src = generatOutlineChartoList(txt, UNICODE, &w, &h);
		if (src == NULL) {
			libosd_log_err("get outline list NULL");
			return NULL;
		}
		*width = w;
		*height = h;
	}

#endif
	return src;
}

char *OSD_createTextUnicodeSrc8bit(OsdText *txt, int *width, int *height)
{
	char *src = NULL;
#ifdef SDL_ENABLE
	if (txt->mode != PALETTE_8) {
		libosd_log_err("wrong format");
		return NULL;
	}

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf, %s", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	if (font == NULL) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		TTF_Quit();
		return NULL;
	}

	SDL_Surface *text;
	SDL_Surface *tmp;
	SDL_Color forecol = { 0xff, 0x00, 0x00 };
	SDL_Color backcol = { 0x00, 0x00, 0x00 };
	if (txt->outline_width > 0) {
		fontout = TTF_OpenFont(txt->ttf_path, txt->size);
		TTF_SetFontOutline(fontout, txt->outline_width);

		if (fontout == NULL) {
			libosd_log_err("failed to open font: %s", SDL_GetError());
			TTF_Quit();
			return NULL;
		}
		SDL_Color outcol = { 0x00, 0xff, 0x00 };

		text = TTF_RenderUNICODE_Blended(font, (uint16_t *)&txt->unicode_txt[0], forecol);
		tmp = TTF_RenderUNICODE_Blended(fontout, (uint16_t *)&txt->unicode_txt[0], outcol);
		if ((text == NULL) || (tmp == NULL)) {
			libosd_log_err("Invaild UTF8 word");
		}

		SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

		SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
		SDL_BlitSurface(text, NULL, tmp, &rect);
#ifdef OSD_DEBUG
		char bmp_name[64];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/unicode-outline-8bpp.bmp");

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
	} else {
		SDL_PixelFormat *fmt;
		fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
		if (fmt == NULL) {
			libosd_log_err("failed to alloc a SDL_PixelFormat");
		}
		memset(fmt, 0, sizeof(SDL_PixelFormat));
		fmt->BitsPerPixel = 32;
		fmt->BytesPerPixel = 4;

		text = TTF_RenderUNICODE_Shaded(font, (uint16_t *)&txt->unicode_txt[0], forecol, backcol);
		if (text == NULL) {
			libosd_log_err("Invaild utf8 word: %s", SDL_GetError());
			SDL_FreeSurface(text);
			TTF_CloseFont(font);
			free(fmt);
			return NULL;
		}
		tmp = SDL_ConvertSurface(text, fmt, 0);
		if (tmp == NULL) {
			libosd_log_err("failed convert interface, %s", SDL_GetError());
			return -EINVAL;
		}
#ifdef OSD_DEBUG
		char bmp_name[64];
		sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/unicode-8bpp.bmp");

		SDL_SaveBMP(tmp, &bmp_name[0]);
#endif
		free(fmt);
	}

	libosd_log_debug("alloc size: %d %d %d", tmp->w, tmp->h, tmp->w * tmp->h);
	src = malloc(tmp->w * tmp->h);
	int ayuv_idx = 0;
	*width = tmp->w;
	*height = tmp->h;
	char *bgra = (char *)tmp->pixels;
	int i, j, a, r, g, b;

	for (i = 0; i < tmp->h; i++) {
		for (j = 0; j < tmp->w * 4; j += 4) {
			b = (int)bgra[i * tmp->w * 4 + j];
			g = (int)bgra[i * tmp->w * 4 + j + 1];
			r = (int)bgra[i * tmp->w * 4 + j + 2];
			a = (int)bgra[i * tmp->w * 4 + j + 3];

			if (txt->outline_width > 0) {
				if ((r > g) && (r > b)) {
					src[ayuv_idx] = MERGE35(txt->color[0], txt->color[1]);
				} else if (a != 0) {
					/*outline*/
					src[ayuv_idx] = MERGE35(txt->outline_color[0], txt->outline_color[1]);
				} else {
					a = 255;
					if (txt->background == TRANSPARENT) {
						src[ayuv_idx] = MERGE35(0, 0x00);
					} else if (txt->background == WHITE) {
						src[ayuv_idx] = MERGE35(a, 0xff);
					} else if (txt->background == BLACK) {
						src[ayuv_idx] = MERGE35(a, 0x00);
					}
				}
			} else if (txt->outline_width == 0) {
				a = 255;
				if ((r > g) && (r > b)) {
					src[ayuv_idx] = MERGE35(txt->color[0], txt->color[1]);
				} else {
					if (txt->background == TRANSPARENT) {
						src[ayuv_idx] = MERGE35(0, 0x00);
					} else if (txt->background == WHITE) {
						src[ayuv_idx] = MERGE35(a, 0xff);
					} else if (txt->background == BLACK) {
						src[ayuv_idx] = MERGE35(a, 0x00);
					}
				}
			}
			ayuv_idx += 1;
		}
	}

	SDL_FreeSurface(tmp);
	SDL_FreeSurface(text);

	TTF_CloseFont(font);

	if (txt->outline_width > 0) {
		TTF_CloseFont(fontout);
	}

	TTF_Quit();
#else
	unsigned int w = 0;
	unsigned int h = 0;
	if (txt->outline_width == 0) {
		src = generatNoOutlineChartoList(txt, UNICODE, &w, &h);
		if (src == NULL) {
			libosd_log_err("failed to get ayuv output");
			return NULL;
		}
		*width = w;
		*height = h;

	} else {
		src = generatOutlineChartoList(txt, UNICODE, &w, &h);
		if (src == NULL) {
			libosd_log_err("get outline list NULL");
			return NULL;
		}
		*width = w;
		*height = h;
	}
#endif

#if 0
	char tmpfile[32];
	snprintf(&tmpfile[0], 32, "./%s.bin", "list-unicode-palette");
	FILE *fp = fopen(tmpfile, "w");
	if (txt->mode == AYUV_3544) {
		fwrite(src, 1, (*width) * (*height) *2, fp);
	} else if (txt->mode == PALETTE_8) {
		fwrite(src, 1, (*width) * (*height), fp);
	}
	fclose(fp);
#endif
	return src;
}

int OSD_destroySrc(char *ptr)
{
	if (ptr == NULL) {
		libosd_log_err("ayuv src == null, can't destroy");
		return -EINVAL;
	} else {
		libosd_log_info("need to free");
	}
	free(ptr);
	return 0;
}

int OSD_setLine(OsdHandle *hd, int osd_idx, OsdLine *line, char *out)
{
	int tmp_idx = -1;
	for (int i = 0; i < MAX_OSD; ++i) {
		if (hd->osd_index[i] == osd_idx) {
			tmp_idx = i;
		}
	}

	if (tmp_idx == -1) {
		libosd_log_err("index %d not found", osd_idx);
		return -EINVAL;
	}

	/* trans to related position in canvas*/
	OsdPoint p[2];

	p[0].x = line->start.x - hd->region[tmp_idx].startX;
	p[0].y = line->start.y - hd->region[tmp_idx].startY;
	p[1].x = line->end.x - hd->region[tmp_idx].startX;
	p[1].y = line->end.y - hd->region[tmp_idx].startY;

	int include_canvas = hd->region[tmp_idx].include_canvas;
	int x_offset = hd->region[tmp_idx].startX - hd->canvas[include_canvas].startX;
	int y_offset = hd->region[tmp_idx].startY - hd->canvas[include_canvas].startY;
	/*move to start of region, not start of canvas*/

	out = out + (y_offset * hd->canvas[include_canvas].width + x_offset) * 2;

	libosd_log_debug("related canvas[%d], p(%d, %d), (%d, %d), offset: %d, %d", tmp_idx, p[0].x, p[0].y, p[1].x,
	                 p[1].y, x_offset, y_offset);

	if (out == NULL) {
		char ayuv_buf[hd->region[tmp_idx].width * hd->region[tmp_idx].height * 2];
		memset(&ayuv_buf[0], 0x00, sizeof(ayuv_buf));
		drawThickLineSimple(p[0].x, p[0].y, p[1].x, p[1].y, line->thickness, LINE_THICKNESS_MIDDLE,
		                    &line->color[0], hd->region[tmp_idx].width, 0, 0, &hd->region[tmp_idx], line->mode,
		                    &ayuv_buf[0]);

		saveAYUV("/mnt/nfs/ethnfs/tmp-1.ayuv", hd->region[tmp_idx].width, hd->region[tmp_idx].height,
		         &ayuv_buf[0], hd->region[tmp_idx].width * hd->region[tmp_idx].height * 2);

		return 0;
	}

	drawThickLineSimple(p[0].x, p[0].y, p[1].x, p[1].y, line->thickness, LINE_THICKNESS_MIDDLE, &line->color[0],
	                    hd->canvas[include_canvas].width, x_offset, y_offset, &hd->region[tmp_idx], line->mode,
	                    out);

	return 0;
}

int OSD_setPrivacyMask(OsdHandle *hd, int osd_idx, char *p_color, COLOR_MODE mode, char *out)
{
	int tmp_idx = -1;
	for (int i = 0; i < MAX_OSD; ++i) {
		if (hd->osd_index[i] == osd_idx) {
			tmp_idx = i;
			libosd_log_debug("get tmp_idx = %d", tmp_idx);
		}
	}

	if (tmp_idx == -1) {
		libosd_log_err("index %d not found", osd_idx);
		return -EINVAL;
	}

	int x_offset = 0;
	int y_offset = 0;
	int canvas_width = 0;
	int osd_width = 0;
	int osd_height = 0;
	int flag = 0;

	for (int i = 0; i < hd->osd_num; i++) {
		for (int j = 0; j < hd->canvas[i].osd_num; j++) {
			if (hd->canvas[i].osd_list[j] == osd_idx) {
				x_offset = hd->region[tmp_idx].startX - hd->canvas[i].startX;

				y_offset = hd->region[tmp_idx].startY - hd->canvas[i].startY;
				canvas_width = hd->canvas[i].width;
				osd_width = hd->region[tmp_idx].width;
				osd_height = hd->region[tmp_idx].height;
				libosd_log_debug("get[%d] : %d, %d %d %d %d %d", i, osd_idx, x_offset, y_offset,
				                 canvas_width, osd_width, osd_height);
				flag = 1;
				break;
			}
		}
		if (flag == 1) {
			break;
		}
	}

	if (mode == AYUV_3544) {
		char ayuv_buf[osd_width * osd_height * 2];
		int ayuv_idx = 0;
		char *rgba = p_color;
		int r, g, b, a, y, u, v;
		r = rgba[0];
		g = rgba[1];
		b = rgba[2];
		a = rgba[3];

		RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
		normalizeAYUV3544(&a, &y, &u, &v);

		for (int i = 0; i < osd_height; i++) {
			for (int j = 0; j < osd_width * 4; j += 4) {
				ayuv_buf[ayuv_idx] = MERGE44(u, v);
				ayuv_idx += 1;
				ayuv_buf[ayuv_idx] = MERGE35(a, y);
				ayuv_idx += 1;
			}
		}

		/*load bmp & convert*/
		if (out == NULL) {
			libosd_log_debug("out ptr not found");
			saveAYUV("/mnt/nfs/ethnfs/save-out-privacy.ayuv", osd_width, osd_height, &ayuv_buf[0],
			         osd_width * osd_height * 2);
			return -EINVAL;
		}

		int line = 0;
		for (int i = y_offset; i < y_offset + osd_height; i++) {
			memcpy((void *)out + (canvas_width * i + x_offset) * 2, (void *)&ayuv_buf[line * osd_width * 2],
			       osd_width * 2);

			line++;
		}
	} else if (mode == PALETTE_8) {
		char color_by_idx = MERGE35(p_color[0] /*Alpha*/, p_color[1] /*mode 9 idx*/);
		char ayuv_buf[osd_width * osd_height];
		memset(&ayuv_buf[0], color_by_idx, osd_width * osd_height);
		if (out == NULL) {
			libosd_log_err("out ptr not found");
			saveAYUV("/mnt/nfs/ethnfs/save-out-privacy-8bpp.ayuv", osd_width, osd_height, &ayuv_buf[0],
			         osd_width * osd_height);
			return -EINVAL;
		}

		int line = 0;
		for (int i = y_offset; i < y_offset + osd_height; i++) {
			memcpy((void *)out + (canvas_width * i + x_offset), (void *)&ayuv_buf[line * osd_width],
			       osd_width);

			line++;
		}
	}

	return 0;
}

uint16_t OSD_trans2Unicode(char txt)
{
	uint16_t ret = 0x0020;
	switch (txt) {
	case '0':
		ret = 0x0030;
		break;
	case '1':
		ret = 0x0031;
		break;
	case '2':
		ret = 0x0032;
		break;
	case '3':
		ret = 0x0033;
		break;
	case '4':
		ret = 0x0034;
		break;
	case '5':
		ret = 0x0035;
		break;
	case '6':
		ret = 0x0036;
		break;
	case '7':
		ret = 0x0037;
		break;
	case '8':
		ret = 0x0038;
		break;
	case '9':
		ret = 0x0039;
		break;
	case ':':
		ret = 0x003a;
		break;
	case '/':
		ret = 0x002f;
		break;
	case ' ':
		ret = 0x0020;
		break;
	case '-':
		ret = 0x002d;
		break;
	default:
		break;
	}

	return ret;
}

int OSD_setRegionTransparent(OsdHandle *hd, int reg_idx, COLOR_MODE mode, char *out)
{
	int tmp_idx = -1;
	for (int i = 0; i < MAX_OSD; ++i) {
		if (hd->osd_index[i] == reg_idx) {
			tmp_idx = i;
		}
	}

	if (tmp_idx == -1) {
		libosd_log_err("index %d not found", reg_idx);
		return -EINVAL;
	}

	/* trans to related position in canvas*/
	int include_canvas = hd->region[tmp_idx].include_canvas;
	int x_offset = hd->region[tmp_idx].startX - hd->canvas[include_canvas].startX;
	int y_offset = hd->region[tmp_idx].startY - hd->canvas[include_canvas].startY;
	int canvas_width = hd->canvas[include_canvas].width;

	if (mode == AYUV_3544) {
		char transparentAYUV[2] = { 0xff, 0x1f };

		for (int i = y_offset; (uint32_t)i < y_offset + hd->region[tmp_idx].height; i++) {
			for (int j = 0; (uint32_t)j < hd->region[tmp_idx].width; j++) {
				memcpy((void *)out + (canvas_width * i + x_offset + j) * 2, (void *)&transparentAYUV[0],
				       2);
			}
		}

	} else if (mode == PALETTE_8) {
		char transparentAYUV = 0x00;

		for (int i = y_offset; (uint32_t)i < y_offset + hd->region[tmp_idx].height; i++) {
			for (int j = 0; (uint32_t)j < hd->region[tmp_idx].width; j++) {
				memcpy((void *)out + (canvas_width * i + x_offset + j), (void *)&transparentAYUV, 1);
			}
		}
	}

	return 0;
}

char *OSD_createUnicodeFontList(OsdText *txt, uint16_t *text_list, int len)
{
	AyuvSrcList *ayuv_list;
	ayuv_list = malloc(sizeof(AyuvSrcList));
	ayuv_list->len = len;
	ayuv_list->src = malloc(sizeof(AyuvSrc) * len);
#ifdef SDL_ENABLE
	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf, %s", SDL_GetError());
		SDL_Quit();
		free(ayuv_list->src);
		free(ayuv_list);
		return NULL;
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	SDL_Surface *tmp[len];

	SDL_Surface *text;
	SDL_Color forecol = { txt->color[0], txt->color[1], txt->color[2] };
	SDL_Color backcol = { 0xff, 0xff, 0xff };
	if (txt->background != WHITE) {
		setBackgroundColor(&txt->color[0], txt->background, &backcol);
	}
	uint16_t tmp_unicode[2] = { 0x0000, 0x0000 };

	if (txt->outline_width == 0) {
		SDL_PixelFormat *fmt;
		fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
		if (fmt == NULL) {
			libosd_log_err("failed to alloc a SDL_PixelFormat");
			free(ayuv_list->src);
			free(ayuv_list);
			return NULL;
		}
		memset(fmt, 0, sizeof(SDL_PixelFormat));
		fmt->BitsPerPixel = 32;
		fmt->BytesPerPixel = 4;

		for (int i = 0; i < len; i++) {
			tmp_unicode[0] = text_list[i];
			text = TTF_RenderUNICODE_Shaded(font, (uint16_t *)&tmp_unicode[0], forecol, backcol);
			tmp_unicode[0] = 0x0000;
			if (text == NULL) {
				libosd_log_err("Invaild Unicode word, %s", SDL_GetError());
				SDL_FreeSurface(text);
				break;
			}

			tmp[i] = SDL_ConvertSurface(text, fmt, 0);
			if (tmp[i] == NULL) {
				libosd_log_err("failed convert interface, %s", SDL_GetError());
			}

			libosd_log_debug("save %s, fcolor: %0x %0x %0x", "/tmp/save.bmp", txt->color[0], txt->color[1],
			                 txt->color[2]);
			libosd_log_err("get (%d, %d)", tmp[i]->w, tmp[i]->h);
			SDL_FreeSurface(text);
#ifdef OSD_DEBUG
			char bmp_name[32];
			sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%d.bmp", text_list[i]);

			SDL_SaveBMP(tmp[i], &bmp_name[0]);
#endif
		}

		free(fmt);
	} else {
		fontout = TTF_OpenFont(txt->ttf_path, txt->size);
		TTF_SetFontOutline(fontout, txt->outline_width);

		if (fontout == NULL) {
			libosd_log_err("failed to open font: %s", SDL_GetError());
			TTF_Quit();
			free(ayuv_list->src);
			return NULL;
		}

		SDL_Color outcol = { txt->outline_color[0], txt->outline_color[1], txt->outline_color[2] };

		for (int i = 0; i < len; i++) {
			tmp_unicode[0] = text_list[i];
			text = TTF_RenderUNICODE_Blended(font, (uint16_t *)&tmp_unicode[0], forecol);
			tmp[i] = TTF_RenderUNICODE_Blended(fontout, (uint16_t *)&tmp_unicode[0], outcol);
			tmp_unicode[0] = 0x0000;
			if ((text == NULL) || (tmp[i] == NULL)) {
				libosd_log_err("Invaild UTF8 word, %s", SDL_GetError());
				free(ayuv_list->src);
				free(ayuv_list);
				return -EINVAL;
			}
			SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

			SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
			SDL_BlitSurface(text, NULL, tmp[i], &rect);
#ifdef DEBUG
			char bmp_name[32];
			sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%d-outline.bmp", text_list[i]);

			SDL_SaveBMP(tmp[i], &bmp_name[0]);
#endif
			SDL_FreeSurface(text);
		}
	}

	int ayuv_idx = 0;
	char *tmp_ptr;
	int i, j, k, r, g, b, a, y, u, v;
	for (i = 0; i < ayuv_list->len; i++) {
		ayuv_idx = 0;
		libosd_log_debug("gen [%d], idx: %d, loc: %d", i, ayuv_idx, tmp[i]->w * tmp[i]->h * 2);

		ayuv_list->src[i].width = tmp[i]->w;
		ayuv_list->src[i].height = tmp[i]->h;
		ayuv_list->src[i].unicode = text_list[i];
		ayuv_list->src[i].src = malloc(tmp[i]->w * tmp[i]->h * 2);

		tmp_ptr = tmp[i]->pixels;
		for (j = 0; j < tmp[i]->h; j++) {
			for (k = 0; k < tmp[i]->w * 4; k += 4) {
				b = (int)tmp_ptr[j * tmp[i]->w * 4 + k];
				g = (int)tmp_ptr[j * tmp[i]->w * 4 + k + 1];
				r = (int)tmp_ptr[j * tmp[i]->w * 4 + k + 2];

				if (txt->outline_width == 0) {
					a = 255;
					if (txt->background == TRANSPARENT) {
						if (((b == backcol.b) && (g == backcol.g)) && (r == backcol.r)) {
							a = 0x00;
						} else {
							a = 255;
						}
					}
				} else {
					a = (int)tmp_ptr[j * tmp[i]->w * 4 + k + 3];
					if (txt->background != TRANSPARENT) {
						if (a == 0) {
							b = backcol.b;
							g = backcol.g;
							r = backcol.r;
							a = 255;
						}
					}
				}

				RGBTrans2YUV(&r, &g, &b, &y, &u, &v);
				normalizeAYUV3544(&a, &y, &u, &v);

				ayuv_list->src[i].src[ayuv_idx] = MERGE44(u, v);
				ayuv_idx += 1;
				ayuv_list->src[i].src[ayuv_idx] = MERGE35(a, y);
				ayuv_idx += 1;
			}
		}
#ifdef OSD_DEBUG
		char ayuv_name[32];
		sprintf(&ayuv_name[0], "/mnt/nfs/ethnfs/%d-outline.bmp", i);
		saveAYUV(&ayuv_name[0], tmp[i]->w, tmp[i]->h, &ayuv_list->src[i].src[0], tmp[i]->w * tmp[i]->w * 2);
#endif
	}

	/*sdl save exit*/
	for (int i = 0; i < ayuv_list->len; i++) {
		SDL_FreeSurface(tmp[i]);
	}

	TTF_CloseFont(font);
	if (txt->outline_width > 0) {
		TTF_CloseFont(fontout);
	}
	TTF_Quit();
#else

	unsigned int char_width[len];
	unsigned int char_height[len];
	unsigned int char_bearingY[len];
	memset(&char_width[0], 0x00, sizeof(char_width));
	memset(&char_height[0], 0x00, sizeof(char_height));

	char *src_ayuv[len];

	OsdText txt_copy;
	memcpy(&txt_copy, txt, sizeof(OsdText));
	memset(&txt_copy.unicode_txt[0], 0x0000, sizeof(txt_copy.unicode_txt));

	for (int i = 0; i < len; i++) {
		txt_copy.unicode_txt[i] = text_list[i];
		txt_copy.txt[i] = 0x00;
	}
	txt_copy.unicode_txt[len] = 0x00;

	if (txt->outline_width == 0) {
		if (genAYUVListSrcBitmapNoOutline(&txt_copy, &char_width[0], &char_height[0], &char_bearingY[0],
		                                  &src_ayuv[0]) < 0) {
			free(ayuv_list->src);
			free(ayuv_list);
			return NULL;
		}
	} else {
		if (genAYUVListSrcBitmapWithOutline(&txt_copy, &char_width[0], &char_height[0], &char_bearingY[0],
		                                    &src_ayuv[0]) < 0) {
			free(ayuv_list->src);
			free(ayuv_list);
			return NULL;
		}
	}
	assignAYUVList(txt, &text_list[0], &char_width[0], &char_height[0], &char_bearingY[0], &src_ayuv[0], ayuv_list);

#endif
	return (char *)ayuv_list;
}

char *OSD_createUnicodeFontList8bit(OsdText *txt, uint16_t *text_list, int len)
{
	AyuvSrcList *ayuv_list;
	ayuv_list = malloc(sizeof(AyuvSrcList));
	ayuv_list->len = len;
	ayuv_list->src = malloc(sizeof(AyuvSrc) * ayuv_list->len);

#ifdef SDL_ENABLE
	if (txt->mode != PALETTE_8) {
		libosd_log_err("wrong format");
		free(ayuv_list->src);
		free(ayuv_list);
		return NULL;
	}

	if (TTF_Init() < 0) {
		libosd_log_err("failed to init sdlttf, %s", SDL_GetError());
		SDL_Quit();
		free(ayuv_list->src);
		free(ayuv_list);
		return NULL;
	}

	TTF_Font *font = NULL, *fontout = NULL;
	font = TTF_OpenFont(txt->ttf_path, txt->size);
	if (font == NULL) {
		libosd_log_err("failed to open font: %s", SDL_GetError());
		TTF_Quit();
		free(ayuv_list->src);
		free(ayuv_list);
		return NULL;
	}

	SDL_Surface *tmp[len];
	SDL_Surface *text;
	SDL_Color forecol = { 0xff, 0x00, 0x00 };
	SDL_Color backcol = { 0x00, 0x00, 0x00 };
	uint16_t tmp_unicode[2] = { 0x0000, 0x0000 };

	if (txt->outline_width == 0) {
		SDL_PixelFormat *fmt;
		fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
		if (fmt == NULL) {
			libosd_log_err("failed to alloc a SDL_PixelFormat");
		}
		memset(fmt, 0, sizeof(SDL_PixelFormat));
		fmt->BitsPerPixel = 32;
		fmt->BytesPerPixel = 4;

		for (int i = 0; i < len; i++) {
			tmp_unicode[0] = text_list[i];
			text = TTF_RenderUNICODE_Shaded(font, (uint16_t *)&tmp_unicode[0], forecol, backcol);
			tmp_unicode[0] = 0x00;

			if (text == NULL) {
				libosd_log_err("Invaild Unicode word");
				SDL_FreeSurface(text);
				break;
			}

			tmp[i] = SDL_ConvertSurface(text, fmt, 0);
			if (tmp[i] == NULL) {
				libosd_log_err("failed convert interface, %s", SDL_GetError());
			}

			libosd_log_debug("save %s, fcolor: %0x %0x %0x", "/tmp/save.bmp", txt->color[0], txt->color[1],
			                 txt->color[2]);
			libosd_log_debug("get (%d, %d)", tmp[i]->w, tmp[i]->h);
			SDL_FreeSurface(text);
#ifdef OSD_DEBUG
			char bmp_name[32];
			sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%d.bmp", text_list[i]);

			SDL_SaveBMP(tmp[i], &bmp_name[0]);
#endif
		}

		free(fmt);
	} else {
		fontout = TTF_OpenFont(txt->ttf_path, txt->size);
		TTF_SetFontOutline(fontout, txt->outline_width);

		if (fontout == NULL) {
			libosd_log_err("failed to open font: %s", SDL_GetError());
			TTF_Quit();
			free(ayuv_list->src);
			free(ayuv_list);
			return NULL;
		}

		SDL_Color outcol = { 0x00, 0xff, 0x00 };

		for (int i = 0; i < len; i++) {
			tmp_unicode[0] = text_list[i];
			text = TTF_RenderUNICODE_Blended(font, &tmp_unicode[0], forecol);
			tmp[i] = TTF_RenderUNICODE_Blended(fontout, &tmp_unicode[0], outcol);
			tmp_unicode[0] = 0x0000;
			if ((text == NULL) || (tmp[i] == NULL)) {
				libosd_log_err("Invaild UTF8 word");
			}
			SDL_Rect rect = { txt->outline_width, txt->outline_width, text->w, text->h };

			SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_BLEND);
			SDL_BlitSurface(text, NULL, tmp[i], &rect);
#ifdef DEBUG

			char bmp_name[32];
			sprintf(&bmp_name[0], "/mnt/nfs/ethnfs/%0x-outline.bmp", text_list[i]);

			SDL_SaveBMP(tmp[i], &bmp_name[0]);
#endif
			SDL_FreeSurface(text);
		}
	}

	int ayuv_idx = 0;
	char *tmp_ptr;
	int i, j, k, r, g, b, a;
	for (i = 0; i < ayuv_list->len; i++) {
		ayuv_idx = 0;
		libosd_log_debug("gen [%d], idx: %d, loc: %d", i, ayuv_idx, tmp[i]->w * tmp[i]->h);

		ayuv_list->src[i].width = tmp[i]->w;
		ayuv_list->src[i].height = tmp[i]->h;
		ayuv_list->src[i].unicode = text_list[i];
		ayuv_list->src[i].src = malloc(tmp[i]->w * tmp[i]->h);

		libosd_log_debug("-->%d %d %d %d", ayuv_list->len, ayuv_list->src[i].width, ayuv_list->src[i].height,
		                 ayuv_list->src[i].unicode);

		tmp_ptr = tmp[i]->pixels;
		for (j = 0; j < tmp[i]->h; j++) {
			for (k = 0; k < tmp[i]->w * 4; k += 4) {
				b = (int)tmp_ptr[j * tmp[i]->w * 4 + k];
				g = (int)tmp_ptr[j * tmp[i]->w * 4 + k + 1];
				r = (int)tmp_ptr[j * tmp[i]->w * 4 + k + 2];
				a = (int)tmp_ptr[j * tmp[i]->w * 4 + k + 3];

				if (txt->outline_width == 0) {
					a = 255;
					if ((r > g) && (r > b)) {
						ayuv_list->src[i].src[ayuv_idx] = MERGE35(txt->color[0], txt->color[1]);
					} else {
						if (txt->background == TRANSPARENT) {
							ayuv_list->src[i].src[ayuv_idx] = MERGE35(0, 0x00);
						} else if (txt->background == WHITE) {
							ayuv_list->src[i].src[ayuv_idx] = MERGE35(a, 0xff);
						} else if (txt->background == BLACK) {
							ayuv_list->src[i].src[ayuv_idx] = MERGE35(a, 0x00);
						}
					}
				} else {
					if ((r > g) && (r > b)) {
						ayuv_list->src[i].src[ayuv_idx] = MERGE35(txt->color[0], txt->color[1]);
					} else if (a != 0) {
						/*outline*/
						ayuv_list->src[i].src[ayuv_idx] =
						        MERGE35(txt->outline_color[0], txt->outline_color[1]);
					} else {
						a = 255;
						if (txt->background == TRANSPARENT) {
							ayuv_list->src[i].src[ayuv_idx] = MERGE35(0, 0x00);
						} else if (txt->background == WHITE) {
							ayuv_list->src[i].src[ayuv_idx] = MERGE35(a, 0xff);
						} else if (txt->background == BLACK) {
							ayuv_list->src[i].src[ayuv_idx] = MERGE35(a, 0x00);
						}
					}
				}

				ayuv_idx += 1;
			}
		}
#ifdef OSD_DEBUG
		char ayuv_name[32];
		sprintf(&ayuv_name[0], "/mnt/nfs/ethnfs/%d-outline.bmp", i);
#endif
	}

	/*sdl save exit*/
	for (int i = 0; i < ayuv_list->len; i++) {
		SDL_FreeSurface(tmp[i]);
	}

	TTF_CloseFont(font);
	if (txt->outline_width > 0) {
		TTF_CloseFont(fontout);
	}
	TTF_Quit();
#else
	unsigned int char_width[len];
	unsigned int char_height[len];
	unsigned int char_bearingY[len];

	char *src_palette8[len];

	OsdText txt_copy;
	memcpy(&txt_copy, txt, sizeof(OsdText));
	for (int i = 0; i < len; i++) {
		txt_copy.unicode_txt[i] = text_list[i];
		txt_copy.txt[i] = 0x00;
	}

	txt_copy.unicode_txt[len] = 0x00;

	if (txt->outline_width == 0) {
		if (genAYUVListSrcBitmapNoOutline(&txt_copy, &char_width[0], &char_height[0], &char_bearingY[0],
		                                  &src_palette8[0]) < 0) {
			free(ayuv_list->src);
			free(ayuv_list);
			return NULL;
		}
	} else {
		if (genAYUVListSrcBitmapWithOutline(&txt_copy, &char_width[0], &char_height[0], &char_bearingY[0],
		                                    &src_palette8[0]) < 0) {
			free(ayuv_list->src);
			free(ayuv_list);
			return NULL;
		}
	}

	assignAYUVList(txt, &text_list[0], &char_width[0], &char_height[0], &char_bearingY[0], &src_palette8[0],
	               ayuv_list);
#endif
	return (char *)ayuv_list;
}

int OSD_getUnicodeSizetoGenerate(uint16_t *text_list, int len, char *src_ptr, int *width, int *height)
{
	int total_width = 0;
	int max_height = 0;
	int ref_bearingY = 0;
	int max_bearingY_to_origin = 0;
	AyuvSrcList *ayuv_list = (AyuvSrcList *)src_ptr;

	for (int i = 0; i < len; i++) {
		for (int j = 0; j < ayuv_list->len; j++) {
			uint16_t text_unicode = text_list[i];
			uint16_t search_unicode = ayuv_list->src[j].unicode;
			if (text_unicode == search_unicode) {
				total_width += ayuv_list->src[j].width;
				if (max_height < ayuv_list->src[j].height) {
					max_height = ayuv_list->src[j].height;
				}
				if (ayuv_list->src[j].bearingY > ref_bearingY) {
					ref_bearingY = ayuv_list->src[j].bearingY;
				}

				if (ayuv_list->src[j].height > ayuv_list->src[j].bearingY) {
					max_bearingY_to_origin =
					        max_bearingY_to_origin > (ayuv_list->src[j].height -
					                                  ayuv_list->src[j].bearingY) ?
					                max_bearingY_to_origin :
					                (ayuv_list->src[j].height - ayuv_list->src[j].bearingY);
				}

				break;
			}
		}
	}

	if (max_bearingY_to_origin == 0) {
		max_bearingY_to_origin = max_height;
	}

	*width = total_width;
	*height = ref_bearingY + max_bearingY_to_origin;
#if 0
	int y_offset = 0;
	for (int i = 0; i < len; i++) {
		y_offset = total_height - ayuv_list->src[i].bearingY;
		if (y_offset + ayuv_list->src[i].height > total_height) {
			total_height = y_offset + ayuv_list->src[i].height;
		}
	}
#endif
	return 0;
}

static int generateUnicodeFromAYUVList(uint16_t *text_list, COLOR_MODE mode, int len, char *src_ptr, char *dst_ptr,
                                       int dst_width, int dst_height)
{
	int x_offset = 0;
	int x_end_offset = 0;
	int y_offset = 0;
	int ref_bearingY = 0;
	int tmp_idx = 0;
	char *ayuv_src = ((AyuvSrcList *)src_ptr)->src[0].src;
	int pixel_byte_len = 0;

	for (int i = 0; i < len; i++) {
		if (ref_bearingY < ((AyuvSrcList *)src_ptr)->src[i].bearingY) {
			ref_bearingY = ((AyuvSrcList *)src_ptr)->src[i].bearingY;
			libosd_log_debug("[%d]Get ref bearing Y hori: %d, %#x", i, ref_bearingY,
			                 ((AyuvSrcList *)src_ptr)->src[i].unicode);
		}
	}

	/*assign background for both  upper and lower offset*/
	if (mode == AYUV_3544) {
		pixel_byte_len = 2;
		for (int i = 0; i < dst_width; i++) {
			for (int j = 0; j < dst_height; j++) {
				dst_ptr[(dst_width * j + i) * 2 + 0] = ayuv_src[0];
				dst_ptr[(dst_width * j + i) * 2 + 1] = ayuv_src[1];
			}
		}

	} else if (mode == PALETTE_8) {
		pixel_byte_len = 1;
		for (int i = 0; i < dst_width; i++) {
			for (int j = 0; j < dst_height; j++) {
				dst_ptr[dst_width * j + i] = ayuv_src[0];
			}
		}
	}

	for (int i = 0; i < len; i++) {
		/*search for text in ayuv list*/
		for (int j = 0; j < ((AyuvSrcList *)src_ptr)->len; j++) {
			if (text_list[i] == ((AyuvSrcList *)src_ptr)->src[j].unicode) {
				tmp_idx = j;
				ayuv_src = (char *)((AyuvSrcList *)src_ptr)->src[j].src;
				break;
			}
		}

		x_end_offset = ((AyuvSrcList *)src_ptr)->src[tmp_idx].width;

		y_offset = ref_bearingY - ((AyuvSrcList *)src_ptr)->src[tmp_idx].bearingY;
		if (y_offset > 0) {
			libosd_log_debug("height < dst height, offset: %d", y_offset);
		} else if (y_offset < 0) {
			y_offset = 0;
		}

		if ((y_offset + ((AyuvSrcList *)src_ptr)->src[tmp_idx].height) > dst_height) {
			/*if some font ypqjg is higher then dst_height - special y_offset */
			y_offset = dst_height - ((AyuvSrcList *)src_ptr)->src[tmp_idx].height;
			libosd_log_debug("[%d] h: %d,  bearing %d, change y offset: %d,", i, dst_height,
			                 ((AyuvSrcList *)src_ptr)->src[tmp_idx].bearingY, y_offset);
		}

		for (int j = 0; j < ((AyuvSrcList *)src_ptr)->src[tmp_idx].height; j++) {
			libosd_log_debug("[%d]copy to %d %d len %d, ", j,
			                 (dst_width * (y_offset + j) + x_offset) * pixel_byte_len,
			                 j * ((AyuvSrcList *)src_ptr)->src[tmp_idx].width * pixel_byte_len,
			                 ((AyuvSrcList *)src_ptr)->src[tmp_idx].width * pixel_byte_len);
			memcpy(dst_ptr + (dst_width * (y_offset + j) + x_offset) * pixel_byte_len,
			       ayuv_src + j * ((AyuvSrcList *)src_ptr)->src[tmp_idx].width * pixel_byte_len,
			       ((AyuvSrcList *)src_ptr)->src[tmp_idx].width * pixel_byte_len);
		}

		x_offset += x_end_offset;
	}

	return 0;
}

int OSD_generateUnicodeFromList(uint16_t *text_list, int len, char *src_ptr, char *dst_ptr, int dst_width,
                                int dst_height)
{
	generateUnicodeFromAYUVList(text_list, AYUV_3544, len, src_ptr, dst_ptr, dst_width, dst_height);
	return 0;
}

int OSD_generateUnicodeFromList8bit(uint16_t *text_list, int len, char *src_ptr, char *dst_ptr, int dst_width,
                                    int dst_height)
{
	generateUnicodeFromAYUVList(text_list, PALETTE_8, len, src_ptr, dst_ptr, dst_width, dst_height);
	return 0;
}

int OSD_destroyUnicodeFontList(char *ptr)
{
	AyuvSrcList *ayuv_list = (AyuvSrcList *)ptr;
	for (int i = 0; i < ayuv_list->len; i++) {
		free(ayuv_list->src[i].src);
	}
	free(ayuv_list->src);
	free(ayuv_list);

	return 0;
}

int OSD_setImageAYUVptr(OsdHandle *hd, int osd_idx, char *src_ptr, int img_width, int img_height, COLOR_MODE mode,
                        char *out)
{
	int tmp_idx = -1;
	tmp_idx = findOSDRegion(hd, osd_idx);

	if (out == NULL) {
		libosd_log_err("out ptr not found");
		return -EINVAL;
	}
#if 0
	int x_offset = 0;
	int y_offset = 0;
	int canvas_width = 0;
	int osd_width = 0;
	int osd_height = 0;
	int flag = 0;

	for (int i = 0; i < hd->osd_num; i++) {
		for (int j = 0; (uint32_t)j < hd->canvas[i].osd_num; j++) {
			if (hd->canvas[i].osd_list[j] == osd_idx) {
				x_offset = hd->region[tmp_idx].startX - hd->canvas[i].startX;
				y_offset = hd->region[tmp_idx].startY - hd->canvas[i].startY;
				canvas_width = hd->canvas[i].width;
				osd_width = hd->region[tmp_idx].width;
				osd_height = hd->region[tmp_idx].height;

				libosd_log_debug("get[%d] : %d, %d %d %d %d %d", i, osd_idx, x_offset, y_offset, canvas_width,
				       osd_width, osd_height);
				flag = 1;
				break;
			}
		}
		if (flag) {
			break;
		}
	}

	if ((img_width != osd_width) || (img_height != osd_height)) {
		libosd_log_debug("Img size != OSD size, (%d, %d) (%d, %d)", img_width, img_height, osd_width, osd_height);
	}

	int copy_width, copy_height;
	if (img_width > osd_width) {
		copy_width = osd_width;
	} else {
		copy_width = img_width;
	}

	if (img_height > osd_height) {
		copy_height = osd_height;
	} else {
		copy_height = img_height;
	}

	if (src_ptr == NULL) {
		libosd_log_err("SRC is null");
		return -EINVAL;
	}

	if (out == NULL) {
		libosd_log_err("ptr == NULL");
		return -EINVAL;
	}

	int line = 0;
	if (mode == AYUV_3544) {
		alignUVVal(src_ptr, img_width, img_height);

		for (int i = y_offset; i < y_offset + copy_height; i++) {
			libosd_log_debug("cp %d len %d to %d", line * img_width * 2, copy_width * 2,
			       ((i * canvas_width + x_offset)) * 2);

			memcpy((void *)out + (canvas_width * i + x_offset) * 2, (void *)&src_ptr[line * img_width * 2],
			       copy_width * 2);

			line++;
		}
	} else if (mode == PALETTE_8) {
		for (int i = y_offset; i < y_offset + copy_height; i++) {
			libosd_log_debug("cp %d len %d to %d", line * img_width, copy_width, ((i * canvas_width + x_offset)));
			memcpy((void *)out + (canvas_width * i + x_offset), (void *)&src_ptr[line * img_width],
			       copy_width);

			line++;
		}
	}
#else
	copy2Region(hd, osd_idx, tmp_idx, &src_ptr[0], img_width, img_height, mode, out);

#endif
	return 0;
}

int OSD_moveOsdRegion(OsdHandle *phd, int osd_idx, OsdRegion *region)
{
	if (region->startX > (uint32_t)phd->width) {
		libosd_log_err("startX exceed handle width");
		return -EINVAL;
	}

	if (region->startY > (uint32_t)phd->height) {
		libosd_log_err("startY exceed handle height");
		return -EINVAL;
	}

	if (CEILINGALIGN16(region->startX + region->width) > (uint32_t)phd->width) {
		libosd_log_err("canvas > width after align 16");
		return -EINVAL;
	}

	if (CEILINGALIGN16(region->startY + region->height) > (uint32_t)phd->height) {
		libosd_log_err("canvas > height after align 16");
		return -EINVAL;
	}

	if ((osd_idx > MAX_OSD) || (osd_idx < 0)) {
		libosd_log_err("invalid del idx: %d", osd_idx);
		return -EINVAL;
	}

	int tmp_idx = -1;
	for (int i = 0; (uint32_t)i < MAX_OSD; ++i) {
		if (phd->osd_index[i] == osd_idx) {
			tmp_idx = i;
			libosd_log_debug("get osd_index[%d]: %d", i, osd_idx);
			break;
		}
	}

	if (tmp_idx == -1) {
		libosd_log_err("can't find this idx:%d", osd_idx);
		return -EINVAL;
	}

	int old_x, new_x, old_y, new_y;
	old_x = phd->region[tmp_idx].startX;
	old_y = phd->region[tmp_idx].startY;
	new_x = region->startX;
	new_y = region->startY;

	/*Mod 16 should be 0*/
	if ((((new_x - old_x) & 0x0f) != 0) || (((new_y - old_y) & 0x0f) != 0)) {
		libosd_log_err("move step not align 16 (%d, %d)", (new_x - old_x), (new_y - old_y));
		return -EINVAL;
	}

	libosd_log_debug("move [%d] (%d, %d) --> (%d, %d)", osd_idx, phd->region[tmp_idx].startX,
	                 phd->region[tmp_idx].startY, region->startX, region->startY);

	phd->region[tmp_idx].startX = region->startX;
	phd->region[tmp_idx].startY = region->startY;

	return 0;
}
