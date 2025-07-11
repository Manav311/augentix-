#ifndef DRAWLINE_H
#define DRAWLINE_H

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

#define LINE_OVERLAP_NONE 0x00
#define LINE_OVERLAP_MAJOR 0x01
#define LINE_OVERLAP_MINOR 0x02
#define LINE_OVERLAP_BOTH 0x03

#define LINE_THICKNESS_MIDDLE 0

int calcNewLine(int startx, int starty, int endx, int endy, int thickness, OsdRegion *targetReg, OsdLine *output);
void initTransBack(OsdRegion *region, char *ayuv_buf);
int setColorPixel(int px, int py, char *color, int canvas_width, int x_offset, int y_offset, OsdRegion *region,
                  COLOR_MODE mode, char *ayuvBuf);
void bresenhamLine(int x0, int y0, int x1, int y1, char *color, int canvas_width, int x_offset, int y_offset,
                   OsdRegion *region, COLOR_MODE mode, char *ayuvBuf);
void drawThickLineSimple(unsigned int aXStart, unsigned int aYStart, unsigned int aXEnd, unsigned int aYEnd,
                         unsigned int aThickness, uint8_t aThicknessMode, char *color, int canvas_width, int x_offset,
                         int y_offset, OsdRegion *region, COLOR_MODE mode, char *ayuvBuf);

#endif