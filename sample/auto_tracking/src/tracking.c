#include "tracking.h"

#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>

#include "mpi_dev.h"
#include "mpi_sys.h"
#include "mpi_osd.h"
#include "log_define.h"

extern int g_run_flag;

#define OSD_COUNT 5
#define DETECT_BOUNDARY_OSD 4
#define ROI_OSD_COUNT OSD_COUNT - 2
#define WAIT_WIN_TIMEOUT 0

typedef struct tracking_info {
	int track_id; /** target tracking object id */
	int idle_time; /** no roi in scene time (unit: ms) */
	int track_delay_frame;
	int track_delay_frame_cnt;
	float detect_edge_left;
	float detect_edge_right;
	float detect_edge_top;
	float detect_edge_bottom;
	float frame_center_x;
	float frame_center_y;
	float frame_interval; /** frame interval time (unit: ms) */
	int mirr_en;
	int flip_en;

	int fps;
	int en_motor;
	float rotate_theta;
	MotorData *motor_data;
	MOTOR_CMD_E motor_cmd;
	pthread_mutex_t track_lock;
} TrackingInfo;

TrackingInfo *g_track_info;

typedef struct tracking_result {
	MPI_RECT_POINT_S rect; /** roi */
	MPI_MOTION_VEC_S mv; /** speed of roi */
	int dx[AXIS_NUM]; /** distance between roi & frame center */
} TrackingResult;

struct tracking_instance {
	union {
		TrackingParam od;
	} param;
	int (*init)(void *param);
	int (*deinit)(void *param);
	int (*run)(TrackingInstance *instance, TrackingResult *res);
};

typedef enum {
	CR_BLACK = 0,
	CR_RED,
	CR_GREEN,
	CR_BLUE,
	CR_YELLOW,
	CR_YELLOW_LIGHT,
	CR_PURPLE,
	CR_ORANGE,
	CR_WHITE,
	CR_RED_LIGHT,
	CR_RED_LIGHT2,
	CR_MAX,
} VIDEO_OSD_CR_E;

UINT16 VIDEO_OSD_getAyuvColor(INT32 color_index)
{
	UINT16 AYUV_TABLE[CR_MAX] = {
		0b1110000010001000, // AYUV_BLACK
		0b1110100101011111, // AYUV_RED
		0b1111001000110001, // AYUV_GREEN
		0b1110001111110110, // AYUV_BLUE
		0b1111101100001001, // AYUV_YELLOW
		0b0101011000101001, // AYUV_YELLOW_LIGHT
		0b1110101010111011, // AYUV_PURPLE
		0b1111011000001011, // AYUV_ORANGE
		0b1111111110001000, // AYUV_WHITE
		0b0111000001111001, // AYUV_RED_LIGHT
		0b1000111001101100, // AYUV_RED_LIGHT2
	};

	if (color_index < CR_MAX) {
		return AYUV_TABLE[color_index];
	}
	return 0;
}

INT32 OSD_createRect(OSD_HANDLE *p_handle, MPI_ECHN chn_idx, INT32 priority)
{
	MPI_OSD_RGN_ATTR_S osd_attr = { 0 };

	osd_attr.show = 0;
	osd_attr.priority = priority;
	osd_attr.qp_enable = FALSE;
	osd_attr.color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544;
	osd_attr.osd_type = MPI_OSD_OVERLAY_L50_POLYGON;
	osd_attr.polygon.point_nums = 4;
	osd_attr.polygon.fill = 0;
	osd_attr.polygon.line_width = MPI_OSD_THICKNESS_MIN + 2;

	UINT16 w = 16, h = 16;
	osd_attr.polygon.point[0].x = 0;
	osd_attr.polygon.point[0].y = 0;
	osd_attr.polygon.point[1].x = w;
	osd_attr.polygon.point[1].y = 0;
	osd_attr.polygon.point[2].x = w;
	osd_attr.polygon.point[2].y = h;
	osd_attr.polygon.point[3].x = 0;
	osd_attr.polygon.point[3].y = h;
	osd_attr.polygon.color = (4 << 13) + (VIDEO_OSD_getAyuvColor(CR_RED) & 0x1fff);
	osd_attr.size.width = w;
	osd_attr.size.height = h;

	INT32 ret = 0;
	ret = MPI_createOsdRgn(p_handle, &osd_attr);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("MPI_createOsdRgn NG:%d\n", ret);
		return -1;
	}

	MPI_OSD_BIND_ATTR_S osd_bind;
	memset(&osd_bind, 0, sizeof(osd_bind));

	osd_bind.point.x = 16;
	osd_bind.point.y = 16;
	osd_bind.idx = chn_idx;
	osd_bind.module = 0;

	ret = MPI_bindOsdToChn(*p_handle, &osd_bind);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("MPI_bindOsdToChn NG:%d\n", ret);
		return -1;
	}

	return 0;
}

INT32 OSD_moveRect(OSD_HANDLE handle, VIDEO_OSD_CR_E enColor, int start_x, int start_y, UINT16 width, UINT16 height)
{
	MPI_OSD_RGN_ATTR_S osd_attr;
	MPI_OSD_BIND_ATTR_S osd_bind;

	MPI_getOsdRgnAttr(handle, &osd_attr);
	MPI_getOsdBindAttr(handle, &osd_bind);

	UINT16 w, h;
	w = ((width + 15) / 16) * 16;
	h = ((height + 15) / 16) * 16;

	osd_attr.show = 1;
	osd_attr.polygon.point[0].x = 0;
	osd_attr.polygon.point[0].y = 0;
	osd_attr.polygon.point[1].x = w;
	osd_attr.polygon.point[1].y = 0;
	osd_attr.polygon.point[2].x = w;
	osd_attr.polygon.point[2].y = h;
	osd_attr.polygon.point[3].x = 0;
	osd_attr.polygon.point[3].y = h;
	osd_attr.polygon.color = (4 << 13) + (VIDEO_OSD_getAyuvColor(enColor) & 0x1fff);
	osd_attr.size.width = w;
	osd_attr.size.height = h;

	start_x = ((start_x + 15) / 16) * 16;
	start_y = ((start_y + 15) / 16) * 16;
	osd_bind.point.x = start_x;
	osd_bind.point.y = start_y;

	MPI_setOsdRgnAttr(handle, &osd_attr);
	MPI_setOsdBindAttr(handle, &osd_bind);

	return 0;
}

INT32 OSD_openCloseRect(OSD_HANDLE handle, bool bOpen)
{
	MPI_OSD_RGN_ATTR_S osd_attr;
	MPI_OSD_BIND_ATTR_S osd_bind;

	MPI_getOsdRgnAttr(handle, &osd_attr);
	MPI_getOsdBindAttr(handle, &osd_bind);

	osd_attr.show = bOpen;

	MPI_setOsdRgnAttr(handle, &osd_attr);
	MPI_setOsdBindAttr(handle, &osd_bind);

	return 0;
}

INT32 OSD_destroyRect(OSD_HANDLE handle)
{
	MPI_OSD_BIND_ATTR_S osd_bind;
	MPI_getOsdBindAttr(handle, &osd_bind);
	MPI_unbindOsdFromChn(handle, &osd_bind);
	MPI_destroyOsdRgn(handle);
	return 0;
}

OSD_HANDLE osdHandle[OSD_COUNT];

static void *runMotor(void *param)
{
	int ret = 0;
	TrackingParam *track_param = (TrackingParam *)param;
	MPI_IVA_OD_MOTOR_PARAM_S od_motor_param;

	while (g_run_flag) {
		pthread_mutex_lock(&g_track_info->track_lock);
		MOTOR_CMD_E motor_cmd = g_track_info->motor_cmd;
		pthread_mutex_unlock(&g_track_info->track_lock);

		if (motor_cmd == MOTOR_CMD_NONE) {
			continue;
		}

		/* motor start rotating processing for tracking thread */
		if (track_param->type == TRACKING_TYPE_GMV_OD) {
			od_motor_param.en_motor = 1;
			MPI_IVA_setObjMotorParam(track_param->win_idx, &od_motor_param);
		} else {
			ret = MPI_IVA_disableObjDet(track_param->win_idx);
			if (ret != MPI_SUCCESS) {
				auto_tracking_log_err("Disable OD failed.\n");
			}
		}

		/* run motor command */
		switch (motor_cmd) {
		case MOTOR_CMD_ROTATE_LEFT:
		case MOTOR_CMD_ROTATE_RIGHT:
			if (track_param->type == TRACKING_TYPE_GMV_OD) {
				MOTOR_rotateXAxisSec(g_track_info->motor_data, motor_cmd, track_param->max_move_time);
			} else {
				auto_tracking_log_debug("rotate_theta: %f\n", g_track_info->rotate_theta);
				MOTOR_rotateXAxis(g_track_info->motor_data, &g_track_info->rotate_theta);
			}
			break;
		case MOTOR_CMD_ROTATE_UP:
		case MOTOR_CMD_ROTATE_DOWN:
			if (track_param->type == TRACKING_TYPE_GMV_OD) {
				MOTOR_rotateYAxisSec(g_track_info->motor_data, motor_cmd, track_param->max_move_time);
			} else {
				MOTOR_rotateYAxis(g_track_info->motor_data, &g_track_info->rotate_theta);
			}
			break;
		case MOTOR_CMD_ALIGN_CENTER:
			MOTOR_alignCenter(g_track_info->motor_data);
			break;
		default:
			// do nothing
			break;
		}

		usleep(track_param->track_delay_time * 1000); // make sure the mv is converged

		/* motor stop rotating processing for tracking thread */
		if (track_param->type == TRACKING_TYPE_GMV_OD) {
			od_motor_param.en_motor = 0;
			MPI_IVA_setObjMotorParam(track_param->win_idx, &od_motor_param);
		} else {
			ret = MPI_IVA_enableObjDet(track_param->win_idx);
			if (ret != MPI_SUCCESS) {
				auto_tracking_log_err("Enable OD failed.\n");
			}

			pthread_mutex_lock(&g_track_info->track_lock);
			g_track_info->motor_cmd = MOTOR_CMD_NONE;
			g_track_info->en_motor = 0;
			pthread_mutex_unlock(&g_track_info->track_lock);
		}
	}

	return NULL;
}

int TRACKING_initOd(void *param)
{
	int ret = 0;
	TrackingParam *track_param = (TrackingParam *)param;
	MPI_IVA_OD_PARAM_S *od_param = &track_param->od_param;
	MPI_SIZE_S *bd = &track_param->boundary;

	ret = MPI_IVA_setObjParam(track_param->win_idx, od_param);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("Failed to set OD param. err: %d\n", ret);
		return ret;
	}

	ret = MPI_IVA_enableObjDet(track_param->win_idx);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("Failed to enable OD. err: %d\n", ret);
		return ret;
	}

	/* create all OSD & draw detect boundary */
	if (track_param->enable_debug_osd) {
		for (int i = 0; i < OSD_COUNT; ++i) {
			OSD_createRect(&osdHandle[i], MPI_ENC_CHN(0), i);
		}

		OSD_moveRect(osdHandle[DETECT_BOUNDARY_OSD], CR_GREEN,
		             bd->width * (0.5 - track_param->detect_boundary[X_AXIS] / 2),
		             bd->height * (0.5 - track_param->detect_boundary[Y_AXIS] / 2),
		             (bd->width * track_param->detect_boundary[X_AXIS]),
		             (bd->height * track_param->detect_boundary[Y_AXIS]));

		OSD_openCloseRect(osdHandle[DETECT_BOUNDARY_OSD], true);
	}

	return 0;
}

int TRACKING_deinitOd(void *param)
{
	int ret = 0;
	TrackingParam *track_param = (TrackingParam *)param;

	ret = MPI_IVA_disableObjDet(track_param->win_idx);

	/* clean & destroy all OSD */
	if (track_param->enable_debug_osd) {
		for (int i = 0; i < OSD_COUNT; ++i) {
			OSD_openCloseRect(osdHandle[i], false);
			OSD_destroyRect(osdHandle[i]);
		}
	}

	return ret;
}

static int getRectSize(MPI_IVA_OBJ_ATTR_S *obj)
{
	int size = (obj->rect.ex - obj->rect.sx) * (obj->rect.ey - obj->rect.sy);
	return size;
}

static void convertObjToTrackRes(MPI_IVA_OBJ_LIST_S *obj_list, MPI_SIZE_S *boundary, TrackingResult *res,
                                 int enable_debug_osd)
{
	int detect_idx = 0; /*idx in obj array, not obj id*/
	bool track_id_founded = false;
	MPI_IVA_OBJ_ATTR_S obj;
	MPI_IVA_OBJ_ATTR_S max_area_obj = obj_list->obj[0];
	int *track_id = &g_track_info->track_id;

	if (obj_list->obj_num == 0) {
		/* accumulate idle time when there's no object in the scene */
		if ((g_track_info->motor_data->status.state[X_AXIS].actual_theta !=
		     g_track_info->motor_data->limit.attr[X_AXIS].center_position) ||
		    (g_track_info->motor_data->status.state[X_AXIS].actual_theta !=
		     g_track_info->motor_data->limit.attr[X_AXIS].center_position)) {
			g_track_info->idle_time += g_track_info->frame_interval;
		}
		/* clean all roi OSD */
		if (enable_debug_osd) {
			for (int i = 0; i < ROI_OSD_COUNT; ++i) {
				OSD_openCloseRect(osdHandle[i], false);
			}
		}
		return;
	}

	g_track_info->idle_time = 0;

	/*case 0: initial*/
	if (*track_id == INIT_TRACKING_ID) {
		*track_id = obj_list->obj[0].id;
	}

	/*case 1: continuous track same obj*/
	for (int i = 0; i < obj_list->obj_num; i++) {
		if (obj_list->obj[i].id == *track_id) {
			detect_idx = i;
			track_id_founded = true;
			break;
		}
	}

	/*case 2: not found previous obj*/
	if (track_id_founded == false) {
		if (obj_list->obj_num > 1) {
			for (int i = 0; i < obj_list->obj_num; i++) {
				obj = obj_list->obj[i];
				if (getRectSize(&obj) > getRectSize(&max_area_obj)) {
					detect_idx = i;
					max_area_obj = obj;
				}
			}
		} else {
			detect_idx = 0;
		}
		*track_id = obj_list->obj[detect_idx].id;
	}

	/** calc result */
	MPI_RECT_POINT_S *rect = &res->rect;
	rect->sx = obj_list->obj[detect_idx].rect.sx;
	rect->sy = obj_list->obj[detect_idx].rect.sy;
	rect->ex = obj_list->obj[detect_idx].rect.ex;
	rect->ey = obj_list->obj[detect_idx].rect.ey;
	memcpy(&res->mv, &obj_list->obj[detect_idx].mv, sizeof(res->mv));

	res->dx[X_AXIS] = (rect->sx + (rect->ex - rect->sx) / 2) - (boundary->width / 2);
	res->dx[Y_AXIS] = (rect->sy + (rect->ey - rect->sy) / 2) - (boundary->height / 2);

	/** show a limited number of objects in obj_list */
	if (enable_debug_osd) {
		/* clean all roi OSD */
		for (int i = 0; i < ROI_OSD_COUNT; ++i) {
			OSD_openCloseRect(osdHandle[i], false);
		}

		/* show other objs */
		for (int i = 0; i < obj_list->obj_num; i++) {
			if (i == detect_idx) {
				continue;
			}

			if (i >= ROI_OSD_COUNT) {
				break;
			}

			OSD_moveRect(osdHandle[i], CR_BLUE, obj_list->obj[i].rect.sx, obj_list->obj[i].rect.sy,
			             obj_list->obj[i].rect.ex - obj_list->obj[i].rect.sx,
			             obj_list->obj[i].rect.ey - obj_list->obj[i].rect.sy);
		}

		OSD_moveRect(osdHandle[0], CR_RED, rect->sx, rect->sy, rect->ex - rect->sx, rect->ey - rect->sy);
		OSD_openCloseRect(osdHandle[0], true);
	}
}

int TRACKING_runOd(TrackingInstance *instance, TrackingResult *res)
{
	const float MIN_ASPECT_RATIO = 0.2f, MAX_ASPECT_RATIO = 5.f;
	float aspect_ratio = 0.f;
	UINT32 timestamp = 0;
	int ret = 0;

	TrackingParam *track_param = &instance->param.od;

	MPI_RECT_POINT_S *bbox;
	MPI_IVA_OBJ_LIST_S obj_list;
	MPI_IVA_OBJ_LIST_S obj_in_bd;
	memset(&obj_in_bd, 0x00, sizeof(obj_in_bd));
	MPI_RECT_POINT_S bd = {
		.sx = 0, .sy = 0, .ex = track_param->boundary.width, .ey = track_param->boundary.height
	};

	/** get obj_list */
	ret = MPI_DEV_waitWin(track_param->win_idx, &timestamp, WAIT_WIN_TIMEOUT);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("wait win fail!\n");
		return ret;
	}

	ret = MPI_IVA_getBitStreamObjList(track_param->win_idx, timestamp, &obj_list);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("get bitstream obj list fail!\n");
		return ret;
	}

	/** modify obj_list */
	for (int i = 0; i < obj_list.obj_num; i++) {
		bbox = &obj_list.obj[i].rect;

		/* ignore obj with low life */
		if (obj_list.obj[i].life < track_param->obj_life_th) {
			continue;
		}

		/* filter by aspect ratio */
		aspect_ratio = ((float)(bbox->ex - bbox->sx + 1)) / ((float)(bbox->ey - bbox->sy + 1));
		if (aspect_ratio < MIN_ASPECT_RATIO || aspect_ratio > MAX_ASPECT_RATIO) {
			continue;
		}

		/* crop obj which is out of FOV */
		if (bbox->sx < bd.sx) {
			bbox->sx = bd.sx;
		}
		if (bbox->sy < bd.sy) {
			bbox->sy = bd.sy;
		}
		if (bbox->ex > bd.ex) {
			bbox->ex = bd.ex;
		}
		if (bbox->ey > bd.ey) {
			bbox->ey = bd.ey;
		}

		memcpy(&obj_in_bd.obj[obj_in_bd.obj_num], &obj_list.obj[i], sizeof(MPI_IVA_OBJ_ATTR_S));
		obj_in_bd.obj_num += 1;
	}

	convertObjToTrackRes(&obj_in_bd, &track_param->boundary, res, track_param->enable_debug_osd);

	return 0;
}

TrackingInstance *newTrackingInstance(const TrackingParam *param, MotorData *motor_data)
{
	int ret = 0;
	MPI_WIN_ATTR_S win_attr;
	MPI_WIN win_idx = param->win_idx;

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		return NULL;
	}

	/* get fps from window attribute */
	ret = MPI_DEV_getWindowAttr(win_idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("Get window attributes failed.\n");
		return NULL;
	}

	/* get resolution from channel attribute */
	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(win_idx.dev, win_idx.chn);
	MPI_SIZE_S *bd = &(((TrackingParam *)param)->boundary);

	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		auto_tracking_log_err("failed to get chn 0 res\n");
		return NULL;
	}
	bd->width = chn_attr.res.width;
	bd->height = chn_attr.res.height;

	/* create tracking instance */
	TrackingInstance *instance = malloc(sizeof(TrackingInstance));
	if (instance == NULL) {
		auto_tracking_log_err("Create tracking instance fail!\n");
		return NULL;
	}

	memcpy(&(instance->param.od), param, sizeof(TrackingParam));

	/* create tracking info */
	g_track_info = malloc(sizeof(TrackingInfo));
	g_track_info->track_id = INIT_TRACKING_ID;
	g_track_info->idle_time = 0;
	g_track_info->track_delay_frame = param->track_delay_time * win_attr.fps / 1000;
	g_track_info->track_delay_frame_cnt = g_track_info->track_delay_frame;
	g_track_info->detect_edge_left = param->boundary.width * (0.5 - param->detect_boundary[X_AXIS] / 2);
	g_track_info->detect_edge_right = param->boundary.width * (0.5 + param->detect_boundary[X_AXIS] / 2);
	g_track_info->detect_edge_top = param->boundary.height * (0.5 - param->detect_boundary[Y_AXIS] / 2);
	g_track_info->detect_edge_bottom = param->boundary.height * (0.5 + param->detect_boundary[Y_AXIS] / 2);
	g_track_info->frame_center_x = param->boundary.width / 2;
	g_track_info->frame_center_y = param->boundary.height / 2;
	g_track_info->frame_interval = 1000.0 / win_attr.fps;
	g_track_info->fps = win_attr.fps;
	g_track_info->mirr_en = win_attr.mirr_en;
	g_track_info->flip_en = win_attr.flip_en;
	g_track_info->en_motor = 0;
	g_track_info->rotate_theta = 0.f;
	g_track_info->motor_data = motor_data;
	g_track_info->motor_cmd = MOTOR_CMD_NONE;

	/* init tracking lock */
	pthread_mutex_init(&g_track_info->track_lock, NULL);

	instance->init = TRACKING_initOd;
	instance->deinit = TRACKING_deinitOd;
	instance->run = TRACKING_runOd;

	/* enable OD */
	instance->init((void *)param);

	return instance;
}

void deleteTrackingInstance(TrackingInstance *instance)
{
	/* disable OD */
	instance->deinit((void *)&(instance->param.od));

	/* destroy tracking lock */
	pthread_mutex_destroy(&g_track_info->track_lock);

	/* release tracking info */
	if (g_track_info != NULL) {
		auto_tracking_log_debug("free tracking info.\n");
		free(g_track_info);
	}

	/* release tracking instance */
	if (instance != NULL) {
		auto_tracking_log_info("free tracking instance.\n");
		free(instance);
	}

	return;
}

static float calcRotateTheta(TrackingInstance *instance, TrackingResult *result, AXIS_TYPE_E axis)
{
	MotorData *motor_data = g_track_info->motor_data;
	TrackingParam *param = &instance->param.od;

	MPI_RECT_POINT_S *obj = &result->rect;
	MPI_MOTION_VEC_S *mv = &result->mv;

	float fps = (float)g_track_info->fps;
	float object_center;
	float resolution;
	float center_diff;
	float speed;
	float prediction;

	if (fps == 0) {
		auto_tracking_log_err("fps is 0.");
		return 0.f;
	}

	if (axis == X_AXIS) {
		object_center = ((float)(obj->sx + obj->ex)) / 2.f;
		resolution = param->boundary.width;
		center_diff = object_center - resolution / 2.f;
		speed = (float)(mv->x);
	} else if (axis == Y_AXIS) {
		object_center = ((float)(obj->sy + obj->ey)) / 2.f;
		resolution = param->boundary.height;
		center_diff = object_center - resolution / 2.f;
		speed = (float)(mv->y);
	} else {
		auto_tracking_log_err("Invalid axis.");
		return 0.f;
	}

	float ptz_speed_factor = 0.f;
	MOTOR_getPtzSpeedFactor(motor_data, axis, &ptz_speed_factor);
	if (ptz_speed_factor == 0.f) {
		auto_tracking_log_err("ptz_speed_factor is 0.");
		return 0.f;
	}
	ptz_speed_factor = resolution / ptz_speed_factor; // unit: pixel per sec

	/* calc rotate_pixel */
	const float MAX_ROTATE_THETA = resolution / 1.5f;
	float rotate_pixel = center_diff;

	float dx_factor = 1.f;
	dx_factor += (object_center < resolution / 2.f) ? (speed / (ptz_speed_factor / fps)) :
	                                                  (-speed / (ptz_speed_factor / fps));

	if (dx_factor == 0.f) {
		prediction = center_diff;
	} else {
		prediction = center_diff / dx_factor;
	}

	if (fabs(dx_factor) > 1e-6 && (fabs(prediction) < MAX_ROTATE_THETA) && (fabs(speed) < ptz_speed_factor / fps)) {
		rotate_pixel = prediction;
	}

	/* convert rotate_pixel to rotate_theta */
	float rotate_theta =
	        rotate_pixel / ptz_speed_factor / ((float)(motor_data->limit.attr[axis].max_velocity << 3) / 1000000.f);

	auto_tracking_log_debug("center_diff:%f speed:%f dx_factor:%f rotate_pixel:%f rotate_theta:%f\n", center_diff,
	                        speed, dx_factor, rotate_pixel, rotate_theta);

	return rotate_theta;
}

static void determineMotorMovement(TrackingInstance *instance, TrackingResult *result)
{
	TrackingParam *track_param = &instance->param.od;

	/** motor status */
	MotorStatus motor_status;
	MOTOR_getStat(g_track_info->motor_data, &motor_status);

	int is_rotate = motor_status.is_rotate;
	int is_reset = motor_status.is_reset;
	int mirr_en = instance->param.od.mirr_en;
	int flip_en = instance->param.od.flip_en;

	/** update tracking status */
	int roi_exist = 0;
	int roi_in_bd = 0;
	int roi_cover_both_edge = 0;
	int exceed_reset_time = 0;
	int roi_reach_center = 0;

	float obj_center_x = (result->rect.sx + result->rect.ex) / 2;
	float obj_center_y = (result->rect.sy + result->rect.ey) / 2;

	if (result->rect.ex != 0 || result->rect.ey != 0) {
		roi_exist = 1;
	}
	if ((result->rect.sx < g_track_info->detect_edge_left && result->rect.ex > g_track_info->detect_edge_right) ||
	    (result->rect.sy < g_track_info->detect_edge_top && result->rect.ey > g_track_info->detect_edge_bottom)) {
		roi_cover_both_edge = 1;
	}
	if (result->rect.sx > g_track_info->detect_edge_left && result->rect.ex < g_track_info->detect_edge_right &&
	    result->rect.sy > g_track_info->detect_edge_top && result->rect.ey < g_track_info->detect_edge_bottom) {
		roi_in_bd = 1;
	}
	if (g_track_info->idle_time > instance->param.od.reset_time * 1000) {
		exceed_reset_time = 1;
	}

	if (track_param->type == TRACKING_TYPE_GMV_OD) {
		float speed_x, speed_y;
		DIRECTION_TYPE_E move_dir;

		MOTOR_getSpeed(g_track_info->motor_data, X_AXIS, &speed_x);
		MOTOR_getSpeed(g_track_info->motor_data, Y_AXIS, &speed_y);

		/** transform move direction into ground-mounted camera view */
		if (speed_x > 0) {
			move_dir = (mirr_en) ? LEFT_DIR : RIGHT_DIR;
		} else if (speed_x < 0) {
			move_dir = (mirr_en) ? RIGHT_DIR : LEFT_DIR;
		} else if (speed_y > 0) {
			move_dir = (flip_en) ? UP_DIR : DOWN_DIR;
		} else if (speed_y < 0) {
			move_dir = (flip_en) ? DOWN_DIR : UP_DIR;
		} else {
			move_dir = NONE_DIR;
		}

		if ((move_dir == LEFT_DIR && obj_center_x >= g_track_info->frame_center_x) ||
		    (move_dir == RIGHT_DIR && obj_center_x <= g_track_info->frame_center_x) ||
		    (move_dir == UP_DIR && obj_center_y >= g_track_info->frame_center_y) ||
		    (move_dir == DOWN_DIR && obj_center_y <= g_track_info->frame_center_y)) {
			roi_reach_center = 1;
		}
	}

	/** set motor command */
	if (is_reset) {
		g_track_info->motor_cmd = MOTOR_CMD_NONE;
		return;
	}

	pthread_mutex_lock(&g_track_info->track_lock);

	if (roi_exist) {
		if (!is_rotate && (roi_in_bd || roi_cover_both_edge)) {
			g_track_info->motor_cmd = MOTOR_CMD_NONE;
		} else if (!is_rotate && !roi_in_bd) {
			if (result->dx[X_AXIS] > 0 && result->rect.ex > g_track_info->detect_edge_right) {
				g_track_info->motor_cmd = (mirr_en) ? MOTOR_CMD_ROTATE_RIGHT : MOTOR_CMD_ROTATE_LEFT;
				if (track_param->type != TRACKING_TYPE_GMV_OD) {
					g_track_info->rotate_theta = calcRotateTheta(instance, result, X_AXIS);
					if (mirr_en) {
						g_track_info->rotate_theta *= (-1);
					}
					g_track_info->en_motor = 1;
					auto_tracking_log_debug("rotate_theta: %f\n", g_track_info->rotate_theta);
				}
			} else if (result->dx[X_AXIS] < 0 && result->rect.sx < g_track_info->detect_edge_left) {
				g_track_info->motor_cmd = (mirr_en) ? MOTOR_CMD_ROTATE_LEFT : MOTOR_CMD_ROTATE_RIGHT;
				if (track_param->type != TRACKING_TYPE_GMV_OD) {
					g_track_info->rotate_theta = calcRotateTheta(instance, result, X_AXIS);
					if (mirr_en) {
						g_track_info->rotate_theta *= (-1);
					}
					g_track_info->en_motor = 1;
					auto_tracking_log_debug("rotate_theta: %f\n", g_track_info->rotate_theta);
				}
			} else if (result->dx[Y_AXIS] > 0 && result->rect.ey > g_track_info->detect_edge_bottom) {
				g_track_info->motor_cmd = (flip_en) ? MOTOR_CMD_ROTATE_DOWN : MOTOR_CMD_ROTATE_UP;
				if (track_param->type != TRACKING_TYPE_GMV_OD) {
					g_track_info->rotate_theta = calcRotateTheta(instance, result, Y_AXIS);
					if (mirr_en) {
						g_track_info->rotate_theta *= (-1);
					}
					g_track_info->en_motor = 1;
				}
			} else if (result->dx[Y_AXIS] < 0 && result->rect.sy < g_track_info->detect_edge_top) {
				g_track_info->motor_cmd = (flip_en) ? MOTOR_CMD_ROTATE_UP : MOTOR_CMD_ROTATE_DOWN;
				if (track_param->type != TRACKING_TYPE_GMV_OD) {
					g_track_info->rotate_theta = calcRotateTheta(instance, result, Y_AXIS);
					if (mirr_en) {
						g_track_info->rotate_theta *= (-1);
					}
					g_track_info->en_motor = 1;
				}
			} else {
				g_track_info->motor_cmd = MOTOR_CMD_NONE;
			}
		} else if (is_rotate && roi_reach_center) {
			MOTOR_stop(g_track_info->motor_data); // TODO: modify as by-axis (axis enum: x/y/z/all)
		} else {
			g_track_info->motor_cmd = MOTOR_CMD_NONE;
		}
	} else { /* roi isn't exist */
		if (!is_rotate && exceed_reset_time) {
			g_track_info->motor_cmd = MOTOR_CMD_ALIGN_CENTER;
			g_track_info->idle_time = 0;
			if (track_param->type != TRACKING_TYPE_GMV_OD) {
				g_track_info->en_motor = 1;
			}
		} else {
			g_track_info->motor_cmd = MOTOR_CMD_NONE;
		}
	}

	pthread_mutex_unlock(&g_track_info->track_lock);

	return;
}

int runAutoTracking(TrackingInstance *instance)
{
	int ret = 0;
	int en_motor = 0;
	pthread_t motor_thread = 0;
	TrackingResult result;
	memset(&result, 0x00, sizeof(result));
	TrackingParam *track_param = &instance->param.od;

	/* create motor thread to execute motor movement */
	if (pthread_create(&motor_thread, NULL, runMotor, (void *)(&instance->param.od)) != 0) {
		auto_tracking_log_err("Create runMotor thread failed.\n");
		return -1;
	}

	while (g_run_flag) {
		pthread_mutex_lock(&g_track_info->track_lock);
		en_motor = g_track_info->en_motor;
		pthread_mutex_unlock(&g_track_info->track_lock);

		if (track_param->type == TRACKING_TYPE_GMV_OD || en_motor == 0) {
			/** determine target obj & calc its distance from frame center */
			ret = instance->run(instance, &result);
			if (ret != MPI_SUCCESS) {
				auto_tracking_log_err("Run OD tracking failed.\n");
				g_run_flag = 0;
				pthread_join(motor_thread, NULL);
				return ret;
			}

			/** determine motor movement */
			determineMotorMovement(instance, &result);

			/** clear all result */
			memset(&result, 0x00, sizeof(result));
		}
	}

	/* join motor thread */
	pthread_join(motor_thread, NULL);

	return 0;
}
