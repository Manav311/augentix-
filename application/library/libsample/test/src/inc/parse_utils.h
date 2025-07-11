#ifndef PARSE_UTILS_H_
#define PARSE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

#include "mpi_base_types.h"

typedef enum parse_types {
	TYPE_BOOL,
	TYPE_CHAR,
	TYPE_INT8,
	TYPE_UINT8,
	TYPE_INT16,
	TYPE_UINT16,
	TYPE_INT32,
	TYPE_UINT32,
	TYPE_FLOAT,
	TYPE_STR,
	TYPE_NUM,
} PARSE_TYPES;

void parse_str_to_upper(char *str);
int parse_rect_param(char *tok, MPI_RECT_S *p);
void get_value(void *dest, enum parse_types type);
void get_res(MPI_SIZE_S *dest);
void get_rotate_type(MPI_ROTATE_TYPE_E *dest);

#ifdef __cplusplus
}
#endif /* !__cplusplus */

#endif /* !PARSE_UTILS_H_ */