#include "parse_utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mpi_base_types.h"

/**
 * @brief Modify string to all upper-case letters.
 */
void parse_str_to_upper(char *str)
{
	while ((*str = toupper(*str))) {
		str++;
	}
}

int parse_rect_param(char *tok, MPI_RECT_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "rect_x")) {
		get_value((void *)&p->x, TYPE_UINT16);
	} else if (!strcmp(tok, "rect_y")) {
		get_value((void *)&p->y, TYPE_UINT16);
	} else if (!strcmp(tok, "rect_width")) {
		get_value((void *)&p->width, TYPE_UINT16);
	} else if (!strcmp(tok, "rect_height")) {
		get_value((void *)&p->height, TYPE_UINT16);
	} else {
		hit = 0;
	}

	return hit;
}

void get_value(void *dest, enum parse_types type)
{
	char *val = strtok(NULL, " =");

	switch (type) {
	case TYPE_BOOL:
	case TYPE_STR:
		break;

	case TYPE_INT8:
		*(INT8 *)dest = (INT8)atoi(val);
		break;

	case TYPE_UINT8:
		*(UINT8 *)dest = (UINT8)atoi(val);
		break;

	case TYPE_INT16:
		*(INT16 *)dest = (INT16)atoi(val);
		break;

	case TYPE_UINT16:
		*(UINT16 *)dest = (UINT16)atoi(val);
		break;

	case TYPE_INT32:
		*(INT32 *)dest = (INT32)atoi(val);
		break;

	case TYPE_UINT32:
		*(UINT32 *)dest = (UINT32)atoi(val);
		break;

	case TYPE_FLOAT:
		*(FLOAT *)dest = (FLOAT)atof(val);
		break;

	default:
		break;
	}

	return;
}

void get_res(MPI_SIZE_S *dest)
{
	char *val = strtok(NULL, " =");
	char *tok = NULL;

	tok = strtok(val, " x");
	dest->width = (INT16)atoi(tok);

	val = strtok(NULL, " x");
	dest->height = (INT16)atoi(val);

	return;
}

void get_rotate_type(MPI_ROTATE_TYPE_E *dest)
{
	char *val = strtok(NULL, " =\n");

	parse_str_to_upper(val);

	if (!strcmp(val, "ROTATE_0")) {
		*dest = MPI_ROTATE_0;
	} else if (!strcmp(val, "ROTATE_90")) {
		*dest = MPI_ROTATE_90;
	} else if (!strcmp(val, "ROTATE_180")) {
		*dest = MPI_ROTATE_180;
	} else if (!strcmp(val, "ROTATE_270")) {
		*dest = MPI_ROTATE_270;
	} else {
		fprintf(stderr, "ERROR: Invalid rotate degree (%s)\n", val);
	}

	return;
}