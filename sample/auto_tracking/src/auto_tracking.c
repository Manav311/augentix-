#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <json.h>
#include "getopt.h"

#include "gpio_motor.h"
#include "tracking.h"

int g_run_flag = 0;

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE\n");
	} else {
		perror("Unexpected signal!\n");
	}

	g_run_flag = 0;
}

static void help(const char *name)
{
	printf("Usage: %s [options] ...\n"
	       "Options:\n"
	       "  -i <file>         auto_tracking config. Expected .json file\n",
	       name);
}

static int parseGpioMotorParam(const char *file_name, GpioMotor *gpio_motor, MotorLimit *limit)
{
	json_object *root = NULL;
	json_object *motor_child = NULL;
	json_object *child = NULL;
	json_object *limit_child = NULL;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	if (!json_object_object_get_ex(root, "motor", &motor_child)) {
		perror("motor parameter is not found!\n");
		return -EINVAL;
	}

	if (json_object_object_get_ex(motor_child, "limit", &child)) {
		for (int i = 0; i < AXIS_NUM; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				json_object_object_get_ex(json_object_array_get_idx(child, i), "min_theta",
				                          &limit_child);
				limit->attr[i].min_theta = json_object_get_int(limit_child);

				json_object_object_get_ex(json_object_array_get_idx(child, i), "max_theta",
				                          &limit_child);
				limit->attr[i].max_theta = json_object_get_int(limit_child);

				json_object_object_get_ex(json_object_array_get_idx(child, i), "min_velocity",
				                          &limit_child);
				limit->attr[i].min_velocity = json_object_get_int(limit_child);

				json_object_object_get_ex(json_object_array_get_idx(child, i), "max_velocity",
				                          &limit_child);
				limit->attr[i].max_velocity = json_object_get_int(limit_child);
			}
		}
	}

	if (json_object_object_get_ex(motor_child, "step_angle", &child)) {
		gpio_motor->step_angle = json_object_get_double(child);
	} else {
		json_object_object_add(motor_child, "step_angle", json_object_new_double(gpio_motor->step_angle));
	}

	if (json_object_object_get_ex(root, "motor_direction_factor", &child)) {
		char direction[8];
		limit->attr[X_AXIS].plus_coordinates = NONE_DIR;
		limit->attr[Y_AXIS].plus_coordinates = NONE_DIR;
		limit->attr[Z_AXIS].plus_coordinates = NONE_DIR;

		for (int i = 0; i < AXIS_NUM; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				sprintf(&direction[0], "%s",
				        json_object_get_string(json_object_array_get_idx(child, i)));
				printf("coord:%s\n", direction);

				if (strcmp("none", direction) == 0) {
					limit->attr[i].plus_coordinates = NONE_DIR;
				}

				if (strcmp("left", direction) == 0) {
					limit->attr[X_AXIS].plus_coordinates = LEFT_DIR;
				}

				if (strcmp("right", direction) == 0) {
					limit->attr[X_AXIS].plus_coordinates = RIGHT_DIR;
				}

				if (strcmp("up", direction) == 0) {
					limit->attr[Y_AXIS].plus_coordinates = UP_DIR;
				}

				if (strcmp("down", direction) == 0) {
					limit->attr[Y_AXIS].plus_coordinates = DOWN_DIR;
				}

				if (strcmp("forward", direction) == 0) {
					limit->attr[Z_AXIS].plus_coordinates = FORWARD_DIR;
				}

				if (strcmp("backward", direction) == 0) {
					limit->attr[Z_AXIS].plus_coordinates = BACKWARD_DIR;
				}
			}
		}
	} else {
		limit->attr[X_AXIS].plus_coordinates = LEFT_DIR;
		limit->attr[Y_AXIS].plus_coordinates = UP_DIR;
		limit->attr[Z_AXIS].plus_coordinates = FORWARD_DIR;
	}

	if (json_object_object_get_ex(motor_child, "ptz_speed_factor", &child)) {
		for (int i = 0; i < AXIS_NUM; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				/*1280 pixel need 5 sec*/
				gpio_motor->ptz_speed_factor[i] =
				        json_object_get_double(json_object_array_get_idx(child, i));
			}
		}
	}

	if (json_object_object_get_ex(motor_child, "x_gpio", &child)) {
		for (int i = 0; i < MAX_GPIO_CNT; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				gpio_motor->x_gpio[i].id = json_object_get_int(json_object_array_get_idx(child, i));
			}
		}
	}

	if (json_object_object_get_ex(motor_child, "y_gpio", &child)) {
		for (int i = 0; i < MAX_GPIO_CNT; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				gpio_motor->y_gpio[i].id = json_object_get_int(json_object_array_get_idx(child, i));
			}
		}
	}

	json_object_put(root);

	return 0;
}

static int parseTrackingParam(const char *file_name, TrackingParam *param)
{
	json_object *root = NULL;
	json_object *child = NULL;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	char type[8];
	if (json_object_object_get_ex(root, "type", &child)) {
		sprintf(type, "%s", json_object_get_string(child));
		if (strcmp("od", type) == 0) {
			param->type = TRACKING_TYPE_OD;
		} else if (strcmp("gmvod", type) == 0) {
			param->type = TRACKING_TYPE_GMV_OD;
		} else {
			param->type = TRACKING_TYPE_OD;
		}
	} else {
		param->type = TRACKING_TYPE_OD;
	}

	if (json_object_object_get_ex(root, "chn", &child)) {
		param->win_idx.chn = json_object_get_int(child);
	} else {
		json_object_object_add(root, "chn", json_object_new_int(param->win_idx.chn));
	}

	if (json_object_object_get_ex(root, "win", &child)) {
		param->win_idx.win = json_object_get_int(child);
	} else {
		json_object_object_add(root, "win", json_object_new_int(param->win_idx.win));
	}

	if (json_object_object_get_ex(root, "od_qual", &child)) {
		param->od_param.od_qual = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_qual", json_object_new_int(param->od_param.od_qual));
	}

	if (json_object_object_get_ex(root, "od_track_refine", &child)) {
		param->od_param.od_track_refine = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_track_refine", json_object_new_int(param->od_param.od_track_refine));
	}

	if (json_object_object_get_ex(root, "od_size_th", &child)) {
		param->od_param.od_size_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_size_th", json_object_new_int(param->od_param.od_size_th));
	}

	if (json_object_object_get_ex(root, "od_sen", &child)) {
		param->od_param.od_sen = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_sen", json_object_new_int(param->od_param.od_sen));
	}

	if (json_object_object_get_ex(root, "en_stop_det", &child)) {
		param->od_param.en_stop_det = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "en_stop_det", json_object_new_boolean(param->od_param.en_stop_det));
	}

	if (json_object_object_get_ex(root, "en_gmv_det", &child)) {
		param->od_param.en_gmv_det = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "en_gmv_det", json_object_new_boolean(param->od_param.en_gmv_det));
	}

	if (json_object_object_get_ex(root, "obj_life_th", &child)) {
		param->obj_life_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "obj_life_th", json_object_new_int(param->obj_life_th));
	}

	if (json_object_object_get_ex(root, "detect_boundary", &child)) {
		for (int i = 0; i < AXIS_NUM; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				param->detect_boundary[i] = json_object_get_double(json_object_array_get_idx(child, i));
			}
		}
	}

	if (json_object_object_get_ex(root, "track_delay_time", &child)) {
		param->track_delay_time = json_object_get_int(child);
	} else {
		json_object_object_add(root, "track_delay_time", json_object_new_int(param->track_delay_time));
	}

	if (json_object_object_get_ex(root, "max_move_time", &child)) {
		param->max_move_time = json_object_get_int(child);
	} else {
		json_object_object_add(root, "max_move_time", json_object_new_int(param->max_move_time));
	}

	if (json_object_object_get_ex(root, "reset_time", &child)) {
		param->reset_time = json_object_get_int(child);
	} else {
		json_object_object_add(root, "reset_time", json_object_new_int(param->reset_time));
	}

	if (json_object_object_get_ex(root, "mirr_en", &child)) {
		param->mirr_en = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "mirr_en", json_object_new_int(param->mirr_en));
	}

	if (json_object_object_get_ex(root, "flip_en", &child)) {
		param->flip_en = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "flip_en", json_object_new_int(param->flip_en));
	}

	if (json_object_object_get_ex(root, "enable_debug_osd", &child)) {
		param->enable_debug_osd = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "enable_debug_osd", json_object_new_int(param->enable_debug_osd));
	}

	printf("%s\n", json_object_to_json_string(root));
	json_object_put(root);

	return 0;
}

static void checkGpioMotorParam(const GpioMotor *gpio_motor, const MotorLimit *limit)
{
	printf("---\n");
	for (int i = 0; i < AXIS_NUM; i++) {
		printf("axis[%d] theta:%f~%f, velocity: %d ~%d \n", i, limit->attr[i].min_theta,
		       limit->attr[i].max_theta, limit->attr[i].min_velocity, limit->attr[i].max_velocity);
	}

	printf("step angle: %f\n", gpio_motor->step_angle);
	printf("ptz_speed_factor: ");
	for (int i = 0; i < AXIS_NUM; i++) {
		printf("%f, ", gpio_motor->ptz_speed_factor[i]);
	}
	printf("\n");

	printf("x gpio:");
	for (int i = 0; i < MAX_GPIO_CNT; i++) {
		printf("%d ", gpio_motor->x_gpio[i].id);
	}

	printf("\n");

	printf("y gpio:");
	for (int i = 0; i < MAX_GPIO_CNT; i++) {
		printf("%d ", gpio_motor->y_gpio[i].id);
	}

	printf("\n");

	printf("---\n");
}

int main(int argc, char **argv)
{
	/* init tracking parameters */
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);
	MPI_IVA_OD_PARAM_S od_param = {
		.od_qual = 63, .od_track_refine = 63, .od_size_th = 20, .od_sen = 253, .en_stop_det = 0, .en_gmv_det = 1
	};
	MPI_IVA_OD_MOTOR_PARAM_S od_motor_param = { .en_motor = 0 };
	TrackingParam track_param = { .win_idx = win_idx,
		                      .type = TRACKING_TYPE_OD,
		                      .od_param = od_param,
		                      .od_motor_param = od_motor_param,
		                      .obj_life_th = 16,
		                      .track_delay_time = 1000,
		                      .max_move_time = 4,
		                      .reset_time = 12,
		                      .detect_boundary = { 0.6, 1.0, -1 },
		                      .mirr_en = 0,
		                      .flip_en = 0,
		                      .enable_debug_osd = 0 };

	/* init motor parameters */
	GpioMotor gpio_motor; /* set motor parameters by config */
	memset(&gpio_motor, 0x00, sizeof(gpio_motor));
	MotorLimit limit; /* set motor parameters by config */
	memset(&limit, 0x00, sizeof(limit));

	char config_fname[256] = { 0 };
	int ret = 0;
	int opt;

	if (argc < 2) {
		help(argv[0]);
		exit(0);
	}

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	while ((opt = getopt(argc, argv, "i:h")) != -1) {
		switch (opt) {
		case 'i':
			sprintf(config_fname, optarg);
			break;
		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	g_run_flag = 1;

	/** 1. enable motor */
	ret = parseGpioMotorParam(config_fname, &gpio_motor, &limit);
	if (ret != 0) {
		perror("Parse GPIO motor parameters fail.\n");
		exit(1);
	}

	checkGpioMotorParam(&gpio_motor, &limit);

	MotorData *motor_data = newGpioMotor(&limit, &gpio_motor);
	if (motor_data == NULL) {
		perror("failed to new gpio motor\n");
		return -1;
	}

	/** 2. create tracking instance */
	parseTrackingParam(config_fname, &track_param);

	TrackingInstance *track_instance = newTrackingInstance(&track_param, motor_data);

	/** 3. run auto-tracking */
	ret = runAutoTracking(track_instance);
	if (ret != 0) {
		perror("Run auto-tracking fail.\n");
	}

	/** 4. delete tracking instance */
	deleteTrackingInstance(track_instance);

	/** 5. disable motor */
	deleteGpioMotor(motor_data);

	return 0;
}