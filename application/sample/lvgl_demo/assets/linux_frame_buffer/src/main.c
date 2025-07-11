#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// #define _IMG_ARRAY_
#ifdef _IMG_ARRAY_
#include "image_bmp.h"
#endif

char *fbp = NULL;
int line_width_byte = 305 * 4;
int pixel_width_byte = 4;
int x_offset_byte_left = 7 * 4;
int x_offset_byte_right = 8 * 4;
int y_offset_line = 5;
struct fb_var_screeninfo vinfo;

void lcd_put_pixel(unsigned int x, unsigned int y, unsigned int color);
int disp_time = 1;
int bmp_hdr_size = 54;
int main(int argc, char **argv)
{
	printf("start lfb\n");
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	int ret;
	char fn[64] = { 0 };

	int fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1) {
		perror("opening /dev/fb0");
		return -1;
	}

#ifndef _IMG_ARRAY_
	printf("open file\n");
	FILE *image;
	if (argc > 1 && strlen(argv[1]) < 64)
		strcpy(fn, argv[1]);
	if (argc > 2)
		disp_time = atol(argv[2]);
	if (argc > 3)
		bmp_hdr_size = atol(argv[3]);

	image = fopen(fn, "rb");
#endif

	printf("load disp param\n");
	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("Error reading fixed information.\n");
		return -2;
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("Error reading variable information.\n");
		return -3;
	}

	// // 2. 使能多 buffer
	// var.yres_virtual = buffer_num * var.yres;
	// ioctl(fd_fb, FBIOPUT_VSCREENINFO, &var);

	printf("%dx%d, %dbpp [virtual(x,y) %d, %d][offset(x,y) %d, %d]\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel,
	       vinfo.xres_virtual, vinfo.yres_virtual, vinfo.xoffset, vinfo.yoffset);

	// Figure out the size of the screen in bytes
	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

	// Map the device to memory
	//  printf("alloc mmap\n");
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	intptr_t x = (intptr_t)fbp;
	if (x == -1) {
		printf("Error: failed to map framebuffer device to memory.\n");
		return -4;
	}
	printf("The framebuffer device was mapped to memory successfully.\n");

	// /* 清屏: 全部设为白色 */
	memset(fbp, 0xFF, screensize);

#ifndef _IMG_ARRAY_
	//    printf("read file buffer\n");
	// 305*229 = 69845
	// header size = 138
	unsigned int buffer[69845];
	size_t numread;

	//offset header
	fseek(image, bmp_hdr_size, SEEK_SET);

	numread = fread(buffer, 4, 69845, image);
	printf("read %lu bytes\n", (unsigned long int)numread);
#else
	size_t numread;
	numread = 69845;
#endif

	/* 设置100个像素为红色 */
	unsigned int x_image = 0;
	unsigned int y_image = 0;

	//    printf("set color\n");
	for (unsigned int i = 0; i < numread; i++) {
		x_image = i % 305;
		y_image = i / 305;
#ifndef _IMG_ARRAY_
		lcd_put_pixel(x_image, y_image, buffer[i]);
#else
		lcd_put_pixel(x_image, y_image, iBmp[i]);
#endif
	}

	// vinfo.yoffset = buf_idx * vinfo.yres;
	if (ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo)) {
		printf("FBIOPAN_DISPLAY fail\n");
		return -1;
	}

	// 5. 等待帧同步完成
	ret = 0;
	ioctl(fbfd, FBIO_WAITFORVSYNC, &ret);
	if (ret < 0) {
		perror("ioctl() / FBIO_WAITFORVSYNC");
	}

	printf("after FBIO_WAITFORVSYNC\n");
	sleep(disp_time);

	munmap(fbp, screensize);
#ifndef _IMG_ARRAY_
	fclose(image);
#endif
	close(fbfd);
	return 0;
}

void lcd_put_pixel(unsigned int x, unsigned int y, unsigned int color)
{
	//unsigned char *pen_8 = (unsigned char *)fbp + x_offset_byte_left + y_offset_line * 320 * 4 + y * (line_width_byte + x_offset_byte_right + x_offset_byte_left) + x * pixel_width_byte;
	unsigned char *pen_8 =
	        (unsigned char *)fbp + x_offset_byte_left + (y_offset_line + y) * 1280 + x * pixel_width_byte;

	unsigned int *pen_32;
	pen_32 = (unsigned int *)pen_8;
	*pen_32 = color;
}
