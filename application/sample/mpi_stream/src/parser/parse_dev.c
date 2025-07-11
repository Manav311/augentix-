#include "parse_dev.h"

#include <stdio.h>
#include <string.h>

#include "sample_stream.h"

#include "parse_utils.h"

#define INVALID_DEV_IDX (-1)
#define MIN_DEV_IDX (0)
#define MAX_DEV_IDX (MPI_MAX_VIDEO_DEV_NUM - 1)

#define INVALID_PATH_IDX (-1)
#define MIN_PATH_IDX (0)
#define MAX_PATH_IDX (MPI_MAX_INPUT_PATH_NUM - 1)

#define INVALID_CHN_IDX (-1)
#define MIN_CHN_IDX (0)
#define MAX_CHN_IDX (MPI_MAX_VIDEO_CHN_NUM - 1)

#define INVALID_WINDOW_IDX (-1)
#define MIN_WINDOW_IDX (0)
#define MAX_WINDOW_IDX (MPI_MAX_VIDEO_WIN_NUM - 1)

static void get_hdr_mode(MPI_HDR_MODE_E *dest)
{
	char *val = strtok(NULL, " =\n");

	parse_str_to_upper(val);

	if (!strcmp(val, "NONE")) {
		*dest = MPI_HDR_MODE_NONE;
	} else if (!strcmp(val, "FRAME_PARL")) {
		*dest = MPI_HDR_MODE_FRAME_PARL;
	} else if (!strcmp(val, "FRAME_ITLV")) {
		*dest = MPI_HDR_MODE_FRAME_ITLV;
	} else if (!strcmp(val, "TOP_N_BTM")) {
		*dest = MPI_HDR_MODE_TOP_N_BTM;
	} else if (!strcmp(val, "SIDE_BY_SIDE")) {
		*dest = MPI_HDR_MODE_SIDE_BY_SIDE;
	} else if (!strcmp(val, "LINE_COLOC")) {
		*dest = MPI_HDR_MODE_LINE_COLOC;
	} else if (!strcmp(val, "LINE_ITLV")) {
		*dest = MPI_HDR_MODE_LINE_ITLV;
	} else if (!strcmp(val, "PIX_COLOC")) {
		*dest = MPI_HDR_MODE_PIX_COLOC;
	} else if (!strcmp(val, "PIX_ITLV")) {
		*dest = MPI_HDR_MODE_PIX_ITLV;
	} else if (!strcmp(val, "FRAME_COMB")) {
		*dest = MPI_HDR_MODE_FRAME_COMB;
	} else if (!strcmp(val, "FRAME_ITLV_ASYNC")) {
		*dest = MPI_HDR_MODE_FRAME_ITLV_ASYNC;
	} else if (!strcmp(val, "FRAME_ITLV_SYNC")) {
		*dest = MPI_HDR_MODE_FRAME_ITLV_SYNC;
	} else {
		printf("ERROR: Invalid HDR mode (%s)\n", val);
	}

	return;
}

static void get_bayer_phase(MPI_BAYER_E *dest)
{
	char *val = strtok(NULL, " =\n");

	parse_str_to_upper(val);

	if (!strcmp(val, "PHASE_G0")) {
		*dest = MPI_BAYER_PHASE_G0;
	} else if (!strcmp(val, "PHASE_R")) {
		*dest = MPI_BAYER_PHASE_R;
	} else if (!strcmp(val, "PHASE_B")) {
		*dest = MPI_BAYER_PHASE_B;
	} else if (!strcmp(val, "PHASE_G1")) {
		*dest = MPI_BAYER_PHASE_G1;
	} else {
		printf("ERROR: Invalid bayer phase (%s)\n", val);
	}

	return;
}

static int parse_path_gen_param(char *tok, MPI_PATH_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "sensor_idx")) {
		get_value((void *)&p->sensor_idx, TYPE_INT32);
	} else if (!strcmp(tok, "path_fps")) {
		get_value((void *)&p->fps, TYPE_FLOAT);
	} else if (!strcmp(tok, "input_res")) {
		get_res((void *)&p->res);
	} else if (!strcmp(tok, "eis_strength")) {
		get_value((void *)&p->eis_strength, TYPE_INT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_dev_attr(char *tok, MPI_DEV_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "hdr_mode")) {
		get_hdr_mode(&p->hdr_mode);
	} else if (!strcmp(tok, "stitch_enable")) {
		get_value((void *)&p->stitch_en, TYPE_UINT8);
	} else if (!strcmp(tok, "eis_enable")) {
		get_value((void *)&p->eis_en, TYPE_UINT8);
	} else if (!strcmp(tok, "bayer")) {
		get_bayer_phase(&p->bayer);
	} else if (!strcmp(tok, "input_fps")) {
		get_value((void *)&p->fps, TYPE_FLOAT);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_chn_layout_param(char *tok, MPI_CHN_LAYOUT_S *p)
{
	static INT32 idx = 0;
	int hit = 1;

	if (!strcmp(tok, "window_num")) {
		get_value((void *)&p->window_num, TYPE_INT32);
	} else if (!strcmp(tok, "layout_window_idx")) {
		get_value((void *)&idx, TYPE_INT32);
	} else {
		hit = 0;
	}

	if (!hit) {
		hit = parse_rect_param(tok, &p->window[idx]);
	}

	return hit;
}

static int parse_win_attr(char *tok, MPI_WIN_ATTR_S *p)
{
	static INT32 w_idx = INVALID_WINDOW_IDX;
	int hit = 1;

	if (!strcmp(tok, "window_idx")) {
		get_value((void *)&w_idx, TYPE_INT32);
		return hit;
	}

	if (w_idx >= MIN_WINDOW_IDX && w_idx <= MAX_WINDOW_IDX) {
		if (!strcmp(tok, "path_bmp")) {
			get_value((void *)&p[w_idx].path.bmp, TYPE_UINT32);
		} else if (!strcmp(tok, "window_fps")) {
			get_value((void *)&p[w_idx].fps, TYPE_FLOAT);
		} else if (!strcmp(tok, "rotate")) {
			get_rotate_type((void *)&p[w_idx].rotate);
		} else if (!strcmp(tok, "mirror")) {
			get_value((void *)&p[w_idx].mirr_en, TYPE_UINT8);
		} else if (!strcmp(tok, "flip")) {
			get_value((void *)&p[w_idx].flip_en, TYPE_UINT8);
		} else if (!strcmp(tok, "eis_en")) {
			get_value((void *)&p[w_idx].eis_en, TYPE_UINT8);
		} else if (!strcmp(tok, "win_type")) {
			get_value((void *)&p[w_idx].view_type, TYPE_INT32);
		} else if (!strcmp(tok, "roi_x")) {
			get_value((void *)&p[w_idx].roi.x, TYPE_UINT16);
		} else if (!strcmp(tok, "roi_y")) {
			get_value((void *)&p[w_idx].roi.y, TYPE_UINT16);
		} else if (!strcmp(tok, "roi_width")) {
			get_value((void *)&p[w_idx].roi.width, TYPE_UINT16);
		} else if (!strcmp(tok, "roi_height")) {
			get_value((void *)&p[w_idx].roi.height, TYPE_UINT16);
		} else if (!strcmp(tok, "priority")) {
			get_value((void *)&p[w_idx].prio, TYPE_UINT8);
		} else if (!strcmp(tok, "parent")) {
			get_value((void *)&p[w_idx].src_id, TYPE_INT32);
		} else if (!strcmp(tok, "const_qual")) {
			get_value((void *)&p[w_idx].const_qual, TYPE_UINT8);
		} else if (!strcmp(tok, "dyn_adj")) {
			get_value((void *)&p[w_idx].dyn_adj, TYPE_UINT8);
		} else {
			hit = 0;
		}
	}

	return hit;
}

static int parse_chn_dist_param(char *tok, MPI_STITCH_DIST_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "dist")) {
		get_value((void *)&p->dist, TYPE_UINT16);
	} else if (!strcmp(tok, "ver_disp")) {
		get_value((void *)&p->ver_disp, TYPE_UINT16);
	} else if (!strcmp(tok, "straighten")) {
		get_value((void *)&p->straighten, TYPE_UINT16);
	} else if (!strcmp(tok, "src_zoom")) {
		get_value((void *)&p->src_zoom, TYPE_UINT16);
	} else if (!strcmp(tok, "theta_0")) {
		get_value((void *)&p->theta[0], TYPE_INT16);
	} else if (!strcmp(tok, "theta_1")) {
		get_value((void *)&p->theta[1], TYPE_INT16);
	} else if (!strcmp(tok, "radius_0")) {
		get_value((void *)&p->radius[0], TYPE_UINT16);
	} else if (!strcmp(tok, "radius_1")) {
		get_value((void *)&p->radius[1], TYPE_UINT16);
	} else if (!strcmp(tok, "curvature_0")) {
		get_value((void *)&p->curvature[0], TYPE_UINT16);
	} else if (!strcmp(tok, "curvature_1")) {
		get_value((void *)&p->curvature[1], TYPE_UINT16);
	} else if (!strcmp(tok, "fov_ratio_0")) {
		get_value((void *)&p->fov_ratio[0], TYPE_UINT16);
	} else if (!strcmp(tok, "fov_ratio_1")) {
		get_value((void *)&p->fov_ratio[1], TYPE_UINT16);
	} else if (!strcmp(tok, "ver_scale_0")) {
		get_value((void *)&p->ver_scale[0], TYPE_UINT16);
	} else if (!strcmp(tok, "ver_scale_1")) {
		get_value((void *)&p->ver_scale[1], TYPE_UINT16);
	} else if (!strcmp(tok, "ver_shift_0")) {
		get_value((void *)&p->ver_shift[0], TYPE_INT16);
	} else if (!strcmp(tok, "ver_shift_1")) {
		get_value((void *)&p->ver_shift[1], TYPE_INT16);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_stitch_attr(char *tok, MPI_STITCH_ATTR_S *p)
{
	static int idx = 0;
	int hit = 1;

	if (!strcmp(tok, "stitch")) {
		get_value((void *)&p->enable, TYPE_UINT8);
	} else if (!strcmp(tok, "center_0_x")) {
		get_value((void *)&p->center[0].x, TYPE_UINT16);
	} else if (!strcmp(tok, "center_0_y")) {
		get_value((void *)&p->center[0].y, TYPE_UINT16);
	} else if (!strcmp(tok, "center_1_x")) {
		get_value((void *)&p->center[1].x, TYPE_UINT16);
	} else if (!strcmp(tok, "center_1_y")) {
		get_value((void *)&p->center[1].y, TYPE_UINT16);
	} else if (!strcmp(tok, "dft_dist")) {
		get_value((void *)&p->dft_dist, TYPE_INT32);
	} else if (!strcmp(tok, "dist_tbl_num")) {
		get_value((void *)&p->table_num, TYPE_INT32);
	} else if (!strcmp(tok, "dist_idx")) {
		get_value((void *)&idx, TYPE_INT32);
	} else {
		hit = 0;
	}

	if (!hit) {
		hit = parse_chn_dist_param(tok, &p->table[idx]);
	}

	return hit;
}

static int parse_ldc_attr(char *tok, MPI_LDC_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "ldc_en")) {
		get_value((void *)&p->enable, TYPE_UINT8);
	} else if (!strcmp(tok, "ldc_type")) {
		get_value((void *)&p->view_type, TYPE_INT32);
	} else if (!strcmp(tok, "ldc_x")) {
		get_value((void *)&p->center_offset.x, TYPE_INT16);
	} else if (!strcmp(tok, "ldc_y")) {
		get_value((void *)&p->center_offset.y, TYPE_INT16);
	} else if (!strcmp(tok, "ldc_ratio")) {
		get_value((void *)&p->ratio, TYPE_INT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_panorama_attr(char *tok, MPI_PANORAMA_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "pano_en")) {
		get_value((void *)&p->enable, TYPE_UINT8);
	} else if (!strcmp(tok, "pano_radius")) {
		get_value((void *)&p->radius, TYPE_UINT16);
	} else if (!strcmp(tok, "pano_curvature")) {
		get_value((void *)&p->curvature, TYPE_UINT16);
	} else if (!strcmp(tok, "pano_ldc")) {
		get_value((void *)&p->ldc_ratio, TYPE_UINT16);
	} else if (!strcmp(tok, "pano_straighten")) {
		get_value((void *)&p->straighten, TYPE_UINT16);
	} else if (!strcmp(tok, "pano_x_offs")) {
		get_value((void *)&p->center_offset.x, TYPE_INT16);
	} else if (!strcmp(tok, "pano_y_offs")) {
		get_value((void *)&p->center_offset.y, TYPE_INT16);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_panning_attr(char *tok, MPI_PANNING_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "pann_en")) {
		get_value((void *)&p->enable, TYPE_UINT8);
	} else if (!strcmp(tok, "pann_radius")) {
		get_value((void *)&p->radius, TYPE_UINT16);
	} else if (!strcmp(tok, "pann_ldc")) {
		get_value((void *)&p->ldc_ratio, TYPE_UINT16);
	} else if (!strcmp(tok, "pann_hor_strength")) {
		get_value((void *)&p->hor_strength, TYPE_UINT16);
	} else if (!strcmp(tok, "pann_ver_strength")) {
		get_value((void *)&p->ver_strength, TYPE_UINT16);
	} else if (!strcmp(tok, "pann_x_offs")) {
		get_value((void *)&p->center_offset.x, TYPE_INT16);
	} else if (!strcmp(tok, "pann_y_offs")) {
		get_value((void *)&p->center_offset.y, TYPE_INT16);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_surround_attr(char *tok, MPI_SURROUND_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "surr_en")) {
		get_value((void *)&p->enable, TYPE_UINT8);
	} else if (!strcmp(tok, "surr_rotate")) {
		get_value((void *)&p->rotate, TYPE_INT32);
	} else if (!strcmp(tok, "surr_min_radius")) {
		get_value((void *)&p->min_radius, TYPE_UINT16);
	} else if (!strcmp(tok, "surr_max_radius")) {
		get_value((void *)&p->max_radius, TYPE_UINT16);
	} else if (!strcmp(tok, "surr_ldc")) {
		get_value((void *)&p->ldc_ratio, TYPE_UINT16);
	} else if (!strcmp(tok, "surr_x_offs")) {
		get_value((void *)&p->center_offset.x, TYPE_INT16);
	} else if (!strcmp(tok, "surr_y_offs")) {
		get_value((void *)&p->center_offset.y, TYPE_INT16);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_chn_gen_param(char *tok, MPI_CHN_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "output_res")) {
		get_res(&p->res);
	} else if (!strcmp(tok, "output_fps")) {
		get_value((void *)&p->fps, TYPE_FLOAT);
	} else if (!strcmp(tok, "binding_capability")) {
		get_value((void *)&p->binding_capability, TYPE_INT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_chn_param(char *tok, CONF_DEV_PARAM_S *conf)
{
	static INT32 c_idx = INVALID_CHN_IDX;
	int hit = 0;

	if (!strcmp(tok, "chn_idx")) {
		get_value((void *)&c_idx, TYPE_INT32);

		if (c_idx >= MPI_MAX_VIDEO_CHN_NUM) {
			return 0;
		}

		conf->chn[c_idx].enable = 1;
		return 1;
	}

	/* Parse other channel parameter for valid channel index */
	if (c_idx >= MIN_CHN_IDX && c_idx <= MAX_CHN_IDX) {
		CONF_CHN_PARAM_S *chn = &conf->chn[c_idx];

		/* Parse general parameter */
		hit = parse_chn_gen_param(tok, &chn->attr);
		if (hit) {
			goto end;
		}

		/* Parse channel layout parameter */
		hit = parse_chn_layout_param(tok, &chn->layout);
		if (hit) {
			goto end;
		}

		/* Parse window attr */
		hit = parse_win_attr(tok, &chn->win[0]);
		if (hit) {
			goto end;
		}
	}

end:
	return hit;
}

static int parse_dev_path_param(char *tok, CONF_DEV_PARAM_S *conf)
{
	static INT32 idx = INVALID_PATH_IDX;

	int hit = 0;

	if (!strcmp(tok, "path_idx")) {
		get_value((void *)&idx, TYPE_INT32);

		if (idx >= MPI_MAX_INPUT_PATH_NUM) {
			return 0;
		}

		conf->path[idx].enable = 1;
		return 1;
	}

	/* Parae other path parameter for valid paht index */
	if (idx >= MIN_PATH_IDX && idx <= MAX_PATH_IDX) {
		CONF_PATH_PARAM_S *path = &conf->path[idx];
		hit = 1;

		/* Parse general parameter */
		hit = parse_path_gen_param(tok, &path->attr);
		if (hit) {
			goto end;
		}
	}

end:
	return hit;
}

void print_chn_layout(MPI_CHN_LAYOUT_S *p)
{
	int i;

	printf("window_num = %d\n", p->window_num);
	for (i = 0; i < p->window_num; i++) {
		printf("window[%d]: x = %d, y = %d, width = %d, height = %d\n", i, p->window[i].x, p->window[i].y,
		       p->window[i].width, p->window[i].height);
	}
}

void print_window_attr(int k, MPI_WIN_ATTR_S *p)
{
	printf("Window attr [%d]: path.bmp = %d, fps = %f, view_type = %d.\n", k, p->path.bmp, p->fps, p->view_type);
	printf("                 rotate = %d, mirr_en = %d, flip_en = %d, eis_en = %d.\n", p->rotate, p->mirr_en, p->flip_en, p->eis_en);
	printf("                 roi.x = %d, roi.y = %d, roi.width = %d, roi.height = %d.\n", p->roi.x, p->roi.y,
	       p->roi.width, p->roi.height);
	printf("                 prio = %d, const_qual = %d, dyn_adj = %d, parent = %d.\n", p->prio, p->const_qual,
	       p->dyn_adj, p->src_id.value);
}

void print_stitch_attr(MPI_STITCH_ATTR_S *p)
{
	MPI_STITCH_DIST_S *dist;

	printf("STITCH params:\n");
	printf("Enable = %d\n", p->enable);
	printf("Optical center 0 = (%d, %d)\n", p->center[0].x, p->center[0].y);
	printf("Optical center 1 = (%d, %d)\n", p->center[1].x, p->center[1].y);
	printf("Default index = %d\n", p->dft_dist);
	printf("Table num = %d\n", p->table_num);

	for (int i = 0; i < MPI_STITCH_TABLE_NUM; ++i) {
		dist = &p->table[i];

		printf("--STITCH table %d:\n", i);
		printf("Distance = %d\n", dist->dist);
		printf("Vertical display = %d\n", dist->ver_disp);
		printf("Straighten line  = %d\n", dist->straighten);
		printf("Source image zoom = %d\n", dist->src_zoom);
		printf("Theta 0 = %d, Theta 1 = %d\n", dist->theta[0], dist->theta[1]);
		printf("Radius 0 = %d, Radius 1 = %d\n", dist->radius[0], dist->radius[1]);
		printf("Curvature 0 = %d, Curvature 1 = %d\n", dist->curvature[0], dist->curvature[1]);
		printf("FOV Ratio 0 = %d, Ratio 1 = %d\n", dist->fov_ratio[0], dist->fov_ratio[1]);
		printf("VER Scale 0 = %d, Scale 1 = %d\n", dist->ver_scale[0], dist->ver_scale[1]);
		printf("VER Shift 0 = %d, Shift 1 = %d\n", dist->ver_shift[0], dist->ver_shift[1]);
	}

	printf("\n");
}

void print_ldc_attr(MPI_LDC_ATTR_S *p)
{
	printf("LDC Attributes:\n");
	printf("LDC enable:%d type %d ratio %d\n", p->enable, p->view_type, p->ratio);
	printf("LDC x %d y %d\n", p->center_offset.x, p->center_offset.y);

	printf("\n");
}

void print_panorama_attr(MPI_PANORAMA_ATTR_S *p)
{
	printf("PANORAMA Attributes:\n");
	printf("PANO enable:%d radius %d curvature %d straighten %d idc_ratio %d\n", p->enable, p->radius, p->curvature,
	       p->straighten, p->ldc_ratio);
	printf("PANO x %d y %d\n", p->center_offset.x, p->center_offset.y);

	printf("\n");
}

void print_panning_attr(MPI_PANNING_ATTR_S *p)
{
	printf("PANNING Attributes:\n");
	printf("PANN enable:%d hor_str %d ver_str %d ratio %d\n", p->enable, p->hor_strength, p->ver_strength,
	       p->ldc_ratio);
	printf("PANN x %d y %d\n", p->center_offset.x, p->center_offset.y);

	printf("\n");
}

void print_surround_attr(MPI_SURROUND_ATTR_S *p)
{
	printf("SURROUND Attributes:\n");
	printf("SURR enable:%d rotate %d min_radius %d max_radius %d idc_ratio %d\n", p->enable, p->rotate,
	       p->min_radius, p->max_radius, p->ldc_ratio);
	printf("SURR x %d y %d\n", p->center_offset.x, p->center_offset.y);

	printf("\n");

}

int parse_dev_param(char *tok, SAMPLE_CONF_S *conf)
{
	static INT32 idx = INVALID_DEV_IDX;
	int hit = 0;

	if (!strcmp(tok, "dev_idx")) {
		get_value((void *)&idx, TYPE_INT32);

		if (idx >= MPI_MAX_VIDEO_DEV_NUM) {
			return 0;
		}

		conf->dev[idx].enable = 1;
		return 1;
	}

	/* Parae other device parameter for valid device index */
	if (idx >= MIN_DEV_IDX && idx <= MAX_DEV_IDX) {
		CONF_DEV_PARAM_S *p = &conf->dev[idx];

		/* Parse general parameter */
		hit = parse_dev_attr(tok, &p->attr);
		if (hit) {
			goto end;
		}

		/* Parse path parameter */
		hit = parse_dev_path_param(tok, p);
		if (hit) {
			goto end;
		}

		/* Parse channel parameter */
		hit = parse_chn_param(tok, p);
		if (hit) {
			goto end;
		}

		/* Parse STITCH parameter */
		hit = parse_stitch_attr(tok, &p->stitch);
		if (hit) {
			goto end;
		}

		/* Parse LDC parameter */
		hit = parse_ldc_attr(tok, &p->ldc);
		if (hit) {
			goto end;
		}

		/* Parse Panorama parameter */
		hit = parse_panorama_attr(tok, &p->panorama);
		if (hit) {
			goto end;
		}

		hit = parse_panning_attr(tok, &p->panning);
		if (hit) {
			goto end;
		}

		/* Parse Panorama parameter */
		hit = parse_surround_attr(tok, &p->surround);
		if (hit) {
			goto end;
		}
	}

end:
	return hit;
}
