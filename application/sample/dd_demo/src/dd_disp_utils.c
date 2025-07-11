/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "dd_disp_utils.h"

char *fbp = NULL;
char *fbp_current = NULL;
int fbfd = 0;
struct fb_var_screeninfo vinfo;
long int screensize_byte = 0;
int fb_num = 0;
int fb_idx = 0;

void fill_color(unsigned long *dest, unsigned long bgra, unsigned long size);
void mem_release(void);

/*
 * Global Functions
*/
int display_process_initial(void)
{
	struct fb_fix_screeninfo finfo;
	fb_idx = 1;

	fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) {
		printf("opening /dev/fb0\n");
		return -1;
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("reading fixed information\n");
		mem_release();
		return -2;
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("reading variable information\n");
		mem_release();
		return -2;
	}

	// Figure out the size of the screen in bytes
	screensize_byte = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

	// Enable multi buffer.
	fb_num = finfo.smem_len / screensize_byte;
	vinfo.yres_virtual = fb_num * vinfo.yres;
	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
		printf("Put variable information\n");
		mem_release();
		return -2;
	}

	// Map the device to memory
	fbp = (char *)mmap(0, screensize_byte * fb_num, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if ((int)fbp == -1) {
		printf("map framebuffer device to memory\n");
		mem_release();
		return -3;
	}

	// fill black on the screan.
	fill_color((unsigned long *)fbp, 0xFF000000, (screensize_byte >> 2));

	// Switch display buffer
	vinfo.yoffset = 0;
	if (ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo)) {
		printf("FBIOPAN_DISPLAY fail\n");
	}

	return 0;
}

int display_process_deinitial(void)
{
	if ((fbfd == 0) && (fbp == NULL)) {
		return 0;
	}

	fill_color((unsigned long *)fbp, 0xFF0000FF, (screensize_byte >> 2));

	// Switch display buffer
	vinfo.yoffset = 0;
	if (ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo)) {
		printf("FBIOPAN_DISPLAY fail\n");
	}

	mem_release();
	return 0;
}

int display_image_update(void *pImage, unsigned long byte_size)
{
	int ret = 0;

	fbp_current = fbp + fb_idx * screensize_byte;
	memcpy(fbp_current, pImage, byte_size);

	// Switch display buffer
	vinfo.yoffset = fb_idx * vinfo.yres;
	if (ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo)) {
		printf("FBIOPAN_DISPLAY fail\n");
		mem_release();
		return -1;
	}
	// wait for Vsync.
	ret = 0;
	ioctl(fbfd, FBIO_WAITFORVSYNC, &ret);
	if (ret < 0) {
		printf("FBIO_WAITFORVSYN\n");
		mem_release();
		return -1;
	}
	fb_idx = ((fb_idx + 1) % fb_num);

	return 0;
}

/*
 * Local Functions
*/
void fill_color(unsigned long *dest, unsigned long bgra, unsigned long size)
{
	/*
	 * bgra is in Little-edian.
	*/
	size--;
	do {
		*(dest + size) = bgra;
	} while (size--);
}

void mem_release(void)
{
	if (fbp != NULL) {
		munmap(fbp, screensize_byte * fb_num);
		fbp = NULL;
	}

	if (fbfd != 0) {
		close(fbfd);
		fbfd = 0;
	}
}
