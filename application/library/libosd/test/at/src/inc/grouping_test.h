#ifndef GROUPING_TEST_H
#define GROUPING_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

typedef struct {
	uint8_t prio;
	uint16_t startx;
	uint16_t starty;
	uint16_t startw;
	uint16_t starth;
} OSD_RECT;

typedef struct {
	char isUsed;
	uint8_t prio_min;
	uint8_t prio_max;
	uint16_t startx;
	uint16_t starty;
	uint16_t startw;
	uint16_t starth;
	uint32_t areaSize;
	uint8_t member[8];
	uint8_t memberNum;
} CANVAS_RECT;

OSD_RECT test1[4] = { { .prio = 0, .startx = 0, .starty = 0, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 1800, .starty = 0, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 1800, .starty = 1000, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 0, .starty = 1000, .startw = 160, .starth = 80 } };

OSD_RECT test2[4] = { { .prio = 0, .startx = 0, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 1, .startx = 0, .starty = 0, .startw = 1920, .starth = 80 },
	              { .prio = 2, .startx = 1800, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 3, .startx = 0, .starty = 1000, .startw = 1920, .starth = 80 } };

OSD_RECT test3[8] = { { .prio = 0, .startx = 0, .starty = 0, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 1800, .starty = 0, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 1800, .starty = 1000, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 0, .starty = 1000, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 200, .starty = 200, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 1600, .starty = 200, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 1600, .starty = 800, .startw = 160, .starth = 80 },
	              { .prio = 0, .startx = 200, .starty = 800, .startw = 160, .starth = 80 } };

OSD_RECT test4[8] = { { .prio = 0, .startx = 0, .starty = 0, .startw = 160, .starth = 80 },
	              { .prio = 1, .startx = 0, .starty = 0, .startw = 1920, .starth = 80 },
	              { .prio = 2, .startx = 1800, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 3, .startx = 0, .starty = 1000, .startw = 1920, .starth = 80 },
	              { .prio = 0, .startx = 200, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 1, .startx = 0, .starty = 200, .startw = 1920, .starth = 80 },
	              { .prio = 2, .startx = 1600, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 3, .startx = 0, .starty = 800, .startw = 1920, .starth = 80 } };

OSD_RECT test5[8] = { { .prio = 0, .startx = 0, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 1, .startx = 0, .starty = 0, .startw = 1920, .starth = 80 },
	              { .prio = 2, .startx = 1800, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 3, .startx = 0, .starty = 1000, .startw = 1920, .starth = 80 },
	              { .prio = 7, .startx = 200, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 6, .startx = 0, .starty = 200, .startw = 1920, .starth = 80 },
	              { .prio = 5, .startx = 1600, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 4, .startx = 0, .starty = 800, .startw = 1920, .starth = 80 } };

OSD_RECT test6[8] = { { .prio = 0, .startx = 0, .starty = 0, .startw = 160, .starth = 1080 },
	              { .prio = 7, .startx = 56, .starty = 56, .startw = 160, .starth = 80 },
	              { .prio = 6, .startx = 48, .starty = 48, .startw = 160, .starth = 80 },
	              { .prio = 1, .startx = 8, .starty = 8, .startw = 160, .starth = 80 },
	              { .prio = 2, .startx = 16, .starty = 16, .startw = 160, .starth = 80 },
	              { .prio = 5, .startx = 40, .starty = 40, .startw = 160, .starth = 80 },
	              { .prio = 4, .startx = 32, .starty = 32, .startw = 160, .starth = 80 },
	              { .prio = 3, .startx = 24, .starty = 24, .startw = 160, .starth = 80 } };

OSD_RECT *testOsd[6] = { &test1[0], &test2[0], &test3[0], &test4[0], &test5[0], &test6[0] };
int OsdSize[6] = { 4, 4, 8, 8, 8, 8 };

#endif