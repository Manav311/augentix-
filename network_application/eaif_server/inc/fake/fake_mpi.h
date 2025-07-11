#ifndef FAKE_MPI_H_
#define FAKE_MPI_H_

#include <stdint.h>

#define MPI_IVA_OBJ_NUM_MAX (10)

typedef struct {
	int16_t x;
	int16_t y;
} MPI_MOTION_VEC_S;

typedef struct {
	int16_t sx;
	int16_t sy;
	int16_t ex;
	int16_t ey;
} MPI_RECT_POINT_S;

typedef struct {
	int id;
	int life;
	MPI_MOTION_VEC_S mv;
	MPI_RECT_POINT_S rect;
} MPI_IVA_OBJ_ATTR_S;

typedef struct {
	uint32_t timestamp;
	int obj_num;
	MPI_IVA_OBJ_ATTR_S obj[MPI_IVA_OBJ_NUM_MAX];
} MPI_IVA_OBJ_LIST_S;

#endif // FAKE_MPI_H_