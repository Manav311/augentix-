#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <lvgl/lvgl.h>
#include <lv_drivers/display/fbdev.h>
#include <lv_drivers/indev/evdev.h>

#include "lv_example_get_started.h"

#ifndef DISP_HOR_RES
#define DISP_HOR_RES 320
#endif

#ifndef DISP_VER_RES
#define DISP_VER_RES 240
#endif

#define DISP_BUF_SIZE (DISP_HOR_RES * DISP_VER_RES)

int main(void)
{
	static lv_disp_draw_buf_t disp_buf;
	static lv_disp_drv_t disp_drv;
	static lv_indev_drv_t indev_drv;
	static lv_color_t buf_1[DISP_BUF_SIZE];
	static lv_color_t buf_2[DISP_BUF_SIZE];
	lv_indev_t *pointer_indev = NULL;
	lv_obj_t *cursor_obj = NULL;

	lv_init();
	fbdev_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, DISP_BUF_SIZE);

	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
	disp_drv.hor_res = DISP_HOR_RES;
	disp_drv.ver_res = DISP_VER_RES;
	disp_drv.full_refresh = 1;
	lv_disp_drv_register(&disp_drv);

	evdev_init();
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = evdev_read;
	pointer_indev = lv_indev_drv_register(&indev_drv);

	/* Set a cursor for the mouse */
	LV_IMG_DECLARE(mouse_cursor_icon)
	cursor_obj = lv_img_create(lv_scr_act());
	lv_img_set_src(cursor_obj, &mouse_cursor_icon);
	lv_indev_set_cursor(pointer_indev, cursor_obj);
#if 0
	lv_example_get_started_1();
#else
	lv_demo_widgets();
#endif
	/* Handle LVGL tasks (tickless mode) */
	while (1) {
		lv_timer_handler();
		usleep(5000);
	}

	return EXIT_SUCCESS;
}

/* Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR` */
uint32_t custom_tick_get(void)
{
	static uint64_t start_ms = 0;
	if (start_ms == 0) {
		struct timeval tv_start;
		gettimeofday(&tv_start, NULL);
		start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
	}

	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	uint64_t now_ms;
	now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

	uint32_t time_ms = now_ms - start_ms;
	return time_ms;
}
