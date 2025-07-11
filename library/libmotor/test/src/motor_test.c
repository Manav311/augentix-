#include "stdio.h"
#include "stdlib.h"

#include <json.h>
#include "getopt.h"
#include <unistd.h>
#include <pthread.h>
#include "string.h"

#include "gpio_motor.h"
#include "libmotor.h"

MotorData *g_motor_data;

static void help(void)
{
	printf("Usage: motor_test -i <CONFIG> [options]\n"
	       "\t-i <file>         auto_tracking config. Expected .json file\n"
	       "Options:\n"
	       "\t-m <movement>     test motor movement.\n"
	       "\t0: rotate_x, 1: rotate_y, 2: reset, \n"
	       "\t3: align to center, 4: move > end then align to center\n"
	       "\t-d <degree>        motor rotate degree.\n"
	       "For example:\n"
	       "\tmotor_test -i agt_ma804043_v1.json -m 0 -d 50\n"
	       "\tmotor_test -i agt_ma804043_v1.json -m 2\n"
	       "\tmotor_test -i agt_ma804043_v1.json -m 3\n");
}

static int parseGpioMotorParam(const char *file_name, GpioMotor *gpio_motor, MotorLimit *limit)
{
	json_object *root = NULL;
	json_object *child = NULL;
	json_object *limit_child = NULL;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	if (json_object_object_get_ex(root, "limit", &child)) {
		for (int i = 0; i < AXIS_NUM; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				json_object_object_get_ex(json_object_array_get_idx(child, i), "min_theta",
				                          &limit_child);
				limit->attr[i].min_theta = json_object_get_double(limit_child);

				json_object_object_get_ex(json_object_array_get_idx(child, i), "max_theta",
				                          &limit_child);
				limit->attr[i].max_theta = json_object_get_double(limit_child);

				json_object_object_get_ex(json_object_array_get_idx(child, i), "min_velocity",
				                          &limit_child);
				limit->attr[i].min_velocity = json_object_get_int(limit_child);

				json_object_object_get_ex(json_object_array_get_idx(child, i), "max_velocity",
				                          &limit_child);
				limit->attr[i].max_velocity = json_object_get_int(limit_child);

				if (json_object_object_get_ex(json_object_array_get_idx(child, i), "default_velocity",
				                              &limit_child)) {
					limit->attr[i].default_velocity = json_object_get_int(limit_child);
				} else {
					json_object_object_add(json_object_array_get_idx(child, i), "default_velocity",
					                       json_object_new_int(limit->attr[i].default_velocity));
				}
				json_object_object_get_ex(json_object_array_get_idx(child, i), "center_position",
				                          &limit_child);
				limit->attr[i].center_position = json_object_get_int(limit_child);
			}
		}
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

	if (json_object_object_get_ex(root, "step_angle", &child)) {
		gpio_motor->step_angle = json_object_get_double(child);
	} else {
		json_object_object_add(root, "step_angle", json_object_new_double(gpio_motor->step_angle));
	}

	if (json_object_object_get_ex(root, "ptz_speed_factor", &child)) {
		for (int i = 0; i < AXIS_NUM; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				gpio_motor->ptz_speed_factor[i] =
				        json_object_get_double(json_object_array_get_idx(child, i));
			}
		}
	}

	if (json_object_object_get_ex(root, "x_gpio", &child)) {
		for (int i = 0; i < MAX_GPIO_CNT; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				gpio_motor->x_gpio[i].id = json_object_get_int(json_object_array_get_idx(child, i));
			}
		}
	}

	if (json_object_object_get_ex(root, "y_gpio", &child)) {
		for (int i = 0; i < MAX_GPIO_CNT; i++) {
			if (json_object_array_get_idx(child, i) != NULL) {
				gpio_motor->y_gpio[i].id = json_object_get_int(json_object_array_get_idx(child, i));
			}
		}
	}

	printf("%s\n", json_object_to_json_string(root));
	json_object_put(root);

	return 0;
}

static void checkGpioMotorParam(GpioMotor *gpio_motor, MotorLimit *limit)
{
	printf("---\n");

	for (int i = 0; i < AXIS_NUM; i++) {
		printf("axis[%d] theta:%f~%f, velocity: %d ~%d \n", i, limit->attr[i].min_theta,
		       limit->attr[i].max_theta, limit->attr[i].min_velocity, limit->attr[i].max_velocity);
	}

	printf("step angle: %f\n", gpio_motor->step_angle);

	printf("coordination_system: ");
	for (int i = 0; i < AXIS_NUM; i++) {
		printf("%d, ", limit->attr[i].plus_coordinates);
	}
	printf("\n");

	printf("center position:");
	for (int i = 0; i < AXIS_NUM; i++) {
		printf("%f, ", limit->attr[i].center_position);
	}
	printf("\n");

	printf("ptx_speed_factor: ");
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

void rotateXAxis(MotorData *data, float rotate_degree)
{
	float inv_rotate_degree = (-1) * rotate_degree;

	for (int i = 0; i < 10; i++) {
		printf("x repeate move[%d]:\n", i);

		printf("moving left...\n");
		MOTOR_rotateXAxis(data, &rotate_degree); // left
		sleep(2);

		printf("moving right...\n");
		MOTOR_rotateXAxis(data, &inv_rotate_degree); // right
		sleep(2);
	}
}

void rotateYAxis(MotorData *data, float rotate_degree)
{
	float inv_rotate_degree = (-1) * rotate_degree;

	for (int i = 0; i < 10; i++) {
		printf("y repeate move[%d]:\n", i);

		printf("moving up...\n");
		MOTOR_rotateYAxis(data, &rotate_degree); // up
		sleep(2);

		printf("moving down...\n");
		MOTOR_rotateYAxis(data, &inv_rotate_degree); // down
		sleep(2);
	}
}

void reset(MotorData *data)
{
	MotorState *state_x = &data->status.state[X_AXIS];
	MotorState *state_y = &data->status.state[Y_AXIS];
	float theta;

	sleep(2);
	theta = 80;
	MOTOR_rotateXAxis(data, &theta);
	sleep(2);
	theta = 40;
	MOTOR_rotateYAxis(data, &theta);
	printf("move to preset position (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta);
	printf("return to default position after 5 seconds.\n");

	sleep(5);

	MOTOR_reset(data);
	printf("return to default position.");
}

void alignCenter(MotorData *data)
{
	MotorState *state_x = &data->status.state[X_AXIS];
	MotorState *state_y = &data->status.state[Y_AXIS];
	float theta;

	printf("center position (%f,%f) (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta, state_x->actual_theta,
	       state_y->actual_theta);

	printf("----\n");

	sleep(2);
	theta = 80;
	MOTOR_rotateXAxis(data, &theta);
	sleep(2);
	theta = 40;
	MOTOR_rotateYAxis(data, &theta);
	printf("finished move to preset position (%f,%f) (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta,
	       state_x->actual_theta, state_y->actual_theta);

	printf("\nreturn to default position after 5 seconds.\n\n");

	sleep(5);

	MOTOR_alignCenter(data);

	printf("align center position (%f,%f) (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta,
	       state_x->actual_theta, state_y->actual_theta);

	printf("return to default position.");
}

void moveToEdgeThenAlignCenter(MotorData *data)
{
	MotorState *state_x = &data->status.state[X_AXIS];
	MotorState *state_y = &data->status.state[Y_AXIS];
	float theta;

	printf("center position (%f,%f) (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta, state_x->actual_theta,
	       state_y->actual_theta);

	printf("----\n");

	sleep(2);

	for (int i = 0; i < 5; i++) {
		printf("test time[%d]:\n", i);
		/** move to max edge */
		{
			theta = data->limit.attr[X_AXIS].max_theta + 80;
			printf("ask x axis move to: %f\n", theta);
			MOTOR_rotateXAxis(data, &theta);

			sleep(2);

			theta = data->limit.attr[Y_AXIS].max_theta + 40;
			printf("ask y axis move to: %f\n", theta);
			MOTOR_rotateYAxis(data, &theta);

			printf("finished move to preset position (%f,%f) (%f,%f).\n", state_x->ideal_theta,
			       state_y->ideal_theta, state_x->actual_theta, state_y->actual_theta);

			printf("\nreturn to default position after 5 seconds.\n\n");

			sleep(5);

			MOTOR_alignCenter(data);

			printf("align center position (%f,%f) (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta,
			       state_x->actual_theta, state_y->actual_theta);
		}

		/** move to min edge */
		{
			theta = data->limit.attr[X_AXIS].min_theta - 80;
			printf("ask x axis move to: %f\n", theta);
			MOTOR_rotateXAxis(data, &theta);

			sleep(2);

			theta = data->limit.attr[Y_AXIS].min_theta - 40;
			printf("ask y axis move to: %f\n", theta);
			MOTOR_rotateYAxis(data, &theta);

			printf("finished move to preset position (%f,%f) (%f,%f).\n", state_x->ideal_theta,
			       state_y->ideal_theta, state_x->actual_theta, state_y->actual_theta);

			printf("\nreturn to default position after 5 seconds.\n\n");

			sleep(5);

			MOTOR_alignCenter(data);

			printf("align center position (%f,%f) (%f,%f).\n", state_x->ideal_theta, state_y->ideal_theta,
			       state_x->actual_theta, state_y->actual_theta);
		}
	}

	printf("return to default position.\n");
}

int main(int argc, char **argv)
{
	char config_fname[256] = { 0 };
	int opt;
	int movement = 0;
	float rotate_degree = 50;

	while ((opt = getopt(argc, argv, "i:m:d:h")) != -1) {
		switch (opt) {
		case 'i':
			sprintf(config_fname, optarg);
			break;
		case 'm':
			movement = atoi(optarg);
			break;
		case 'd':
			rotate_degree = atof(optarg);
			break;
		case 'h':
			help();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	GpioMotor gpio_motor = { 0 };
	MotorLimit limit = { 0 };

	parseGpioMotorParam(config_fname, &gpio_motor, &limit);
	checkGpioMotorParam(&gpio_motor, &limit);

	g_motor_data = newGpioMotor(&limit, &gpio_motor);
	if (g_motor_data == NULL) {
		fprintf(stderr, "failed to new gpio motor\n");
		return -1;
	}

	switch (movement) {
	case 0:
		for (int i = 0; i < 10; i++) {
			printf("x repeate move[%d]:\n", i);

			rotate_degree = 310;
			MOTOR_rotateXAxis(g_motor_data, &rotate_degree);
			sleep(2);

			rotate_degree = -310;
			MOTOR_rotateXAxis(g_motor_data, &rotate_degree);
			sleep(2);
		}
		break;
	case 1:
		rotateYAxis(g_motor_data, rotate_degree);
		break;
	case 2:
		reset(g_motor_data);
		break;
	case 3:
		alignCenter(g_motor_data);
		break;
	case 4:
		moveToEdgeThenAlignCenter(g_motor_data);
		break;
	default:
		perror("Invalid motor movement!\n");
		break;
	}
	deleteGpioMotor(g_motor_data);

	return 0;
}