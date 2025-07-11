#include <lvgl/lvgl.h>

const uint8_t mouse_cursor_icon_map[] = {
#if LV_COLOR_DEPTH == 1 || LV_COLOR_DEPTH == 8
	/*Pixel format: Alpha 8 bit, Red: 3 bit, Green: 3 bit, Blue: 2 bit*/
	0x24,
	0xb8,
	0x24,
	0xc8,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcc,
	0xdb,
	0xff,
	0x49,
	0xcc,
	0x00,
	0x24,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xc8,
	0xff,
	0xff,
	0xff,
	0xff,
	0x49,
	0xe0,
	0x00,
	0x3b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcb,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xff,
	0xff,
	0x6d,
	0xf3,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x92,
	0xff,
	0x00,
	0x73,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x92,
	0xff,
	0x00,
	0x90,
	0x00,
	0x00,
	0x00,
	0x00,
	0xb6,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xb7,
	0xff,
	0x24,
	0xab,
	0x00,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xdb,
	0xff,
	0x24,
	0xbb,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xdb,
	0xff,
	0x49,
	0xd8,
	0x00,
	0x37,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x6d,
	0xef,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x92,
	0xff,
	0x00,
	0x6b,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xdb,
	0xff,
	0x92,
	0xf7,
	0x92,
	0xf8,
	0x6e,
	0xfb,
	0x92,
	0xf8,
	0x6d,
	0xff,
	0x00,
	0xb3,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xdb,
	0xff,
	0x24,
	0xb7,
	0x00,
	0x1b,
	0x00,
	0x14,
	0x00,
	0x13,
	0x00,
	0x0c,
	0x25,
	0x07,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x6d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x6e,
	0xf0,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0xcc,
	0xff,
	0xff,
	0xff,
	0xff,
	0x49,
	0xd8,
	0x00,
	0x78,
	0x92,
	0xfb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xdb,
	0xff,
	0x00,
	0x8b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x6d,
	0xd3,
	0xff,
	0xff,
	0x6d,
	0xef,
	0x00,
	0x34,
	0x00,
	0x00,
	0x49,
	0xc7,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x6d,
	0xdc,
	0x00,
	0x14,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x49,
	0xe0,
	0x6d,
	0xff,
	0x00,
	0x40,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x57,
	0x92,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xb7,
	0xff,
	0x00,
	0x78,
	0x00,
	0x00,
	0x00,
	0x04,
	0x00,
	0x00,
	0x00,
	0x68,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x10,
	0x49,
	0xd0,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xff,
	0xff,
	0x6d,
	0xd8,
	0x00,
	0x18,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x24,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x6f,
	0xb7,
	0xff,
	0xff,
	0xff,
	0x92,
	0xff,
	0x49,
	0xac,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x25,
	0xd7,
	0x49,
	0xc7,
	0x00,
	0x47,
	0x00,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 0
	/*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit*/
	0xc3,
	0x18,
	0xb8,
	0xe4,
	0x20,
	0xc8,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x49,
	0x4a,
	0xcc,
	0x96,
	0xb5,
	0xff,
	0xc7,
	0x39,
	0xcc,
	0x00,
	0x00,
	0x24,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xe7,
	0x39,
	0xc8,
	0xbf,
	0xff,
	0xff,
	0xfb,
	0xde,
	0xff,
	0x28,
	0x42,
	0xe0,
	0x00,
	0x00,
	0x3b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xe7,
	0x39,
	0xcb,
	0x3d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xfc,
	0x3d,
	0xef,
	0xff,
	0xcb,
	0x5a,
	0xf3,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xe7,
	0x39,
	0xcb,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xbf,
	0xff,
	0xff,
	0x8e,
	0x73,
	0xff,
	0x00,
	0x00,
	0x73,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xe8,
	0x41,
	0xcb,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x51,
	0x8c,
	0xff,
	0x00,
	0x00,
	0x90,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xd3,
	0x9c,
	0x00,
	0x20,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xe8,
	0x41,
	0xcb,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x14,
	0xa5,
	0xff,
	0xa2,
	0x10,
	0xab,
	0x00,
	0x00,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x08,
	0x42,
	0xcb,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xd7,
	0xbd,
	0xff,
	0x04,
	0x21,
	0xbb,
	0x00,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x08,
	0x42,
	0xcc,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x59,
	0xce,
	0xff,
	0xe8,
	0x41,
	0xd8,
	0x00,
	0x00,
	0x37,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x08,
	0x42,
	0xcc,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xe6,
	0xff,
	0xab,
	0x5a,
	0xef,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x08,
	0x42,
	0xcc,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xbe,
	0xf7,
	0xff,
	0xaf,
	0x7b,
	0xff,
	0x00,
	0x00,
	0x6b,
	0x28,
	0x42,
	0xcc,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x7a,
	0xd6,
	0xff,
	0x10,
	0x84,
	0xf7,
	0xae,
	0x73,
	0xf8,
	0x6e,
	0x73,
	0xfb,
	0x8e,
	0x73,
	0xf8,
	0xcb,
	0x5a,
	0xff,
	0x61,
	0x08,
	0xb3,
	0x28,
	0x42,
	0xcc,
	0x7d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x59,
	0xce,
	0xff,
	0xa2,
	0x10,
	0xb7,
	0x00,
	0x00,
	0x1b,
	0x00,
	0x00,
	0x14,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x0c,
	0x45,
	0x29,
	0x07,
	0x29,
	0x4a,
	0xcc,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xdb,
	0xde,
	0xff,
	0xec,
	0x62,
	0xff,
	0x1c,
	0xe7,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x0c,
	0x63,
	0xf0,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x29,
	0x4a,
	0xcc,
	0xdf,
	0xff,
	0xff,
	0x7d,
	0xef,
	0xff,
	0x49,
	0x4a,
	0xd8,
	0x00,
	0x00,
	0x78,
	0x51,
	0x8c,
	0xfb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x38,
	0xc6,
	0xff,
	0x00,
	0x00,
	0x8b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xcb,
	0x5a,
	0xd3,
	0xdb,
	0xde,
	0xff,
	0xec,
	0x62,
	0xef,
	0x00,
	0x00,
	0x34,
	0x00,
	0x00,
	0x00,
	0xe7,
	0x39,
	0xc7,
	0x5d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xff,
	0xbe,
	0xf7,
	0xff,
	0xaa,
	0x52,
	0xdc,
	0x00,
	0x00,
	0x14,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0xe8,
	0x41,
	0xe0,
	0xaa,
	0x52,
	0xff,
	0x00,
	0x00,
	0x40,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x57,
	0x72,
	0x94,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x96,
	0xb5,
	0xff,
	0x00,
	0x00,
	0x78,
	0x00,
	0x00,
	0x00,
	0x61,
	0x08,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x68,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x10,
	0x69,
	0x4a,
	0xd0,
	0x7d,
	0xef,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xbe,
	0xf7,
	0xff,
	0xaa,
	0x52,
	0xd8,
	0x00,
	0x00,
	0x18,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xe4,
	0x20,
	0x00,
	0x20,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x6f,
	0x75,
	0xad,
	0xff,
	0xbf,
	0xff,
	0xff,
	0x10,
	0x84,
	0xff,
	0x86,
	0x31,
	0xac,
	0x00,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0x08,
	0x03,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x66,
	0x31,
	0xd7,
	0xc7,
	0x39,
	0xc7,
	0x00,
	0x00,
	0x47,
	0x00,
	0x00,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
#endif
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0
	/*Pixel format: Alpha 8 bit, Red: 5 bit, Green: 6 bit, Blue: 5 bit  BUT the 2  color bytes are swapped*/
	0x18,
	0xc3,
	0xb8,
	0x20,
	0xe4,
	0xc8,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x4a,
	0x49,
	0xcc,
	0xb5,
	0x96,
	0xff,
	0x39,
	0xc7,
	0xcc,
	0x00,
	0x00,
	0x24,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x08,
	0x41,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x39,
	0xe7,
	0xc8,
	0xff,
	0xbf,
	0xff,
	0xde,
	0xfb,
	0xff,
	0x42,
	0x28,
	0xe0,
	0x00,
	0x00,
	0x3b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x08,
	0x41,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x39,
	0xe7,
	0xcb,
	0xef,
	0x3d,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xef,
	0x3d,
	0xff,
	0x5a,
	0xcb,
	0xf3,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x39,
	0xe7,
	0xcb,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xbf,
	0xff,
	0x73,
	0x8e,
	0xff,
	0x00,
	0x00,
	0x73,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0xe8,
	0xcb,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x8c,
	0x51,
	0xff,
	0x00,
	0x00,
	0x90,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x9c,
	0xd3,
	0x00,
	0x00,
	0x20,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0xe8,
	0xcb,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xa5,
	0x14,
	0xff,
	0x10,
	0xa2,
	0xab,
	0x00,
	0x00,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x42,
	0x08,
	0xcb,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xbd,
	0xd7,
	0xff,
	0x21,
	0x04,
	0xbb,
	0x00,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x42,
	0x08,
	0xcc,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xce,
	0x59,
	0xff,
	0x41,
	0xe8,
	0xd8,
	0x00,
	0x00,
	0x37,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x42,
	0x08,
	0xcc,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xe6,
	0xfc,
	0xff,
	0x5a,
	0xab,
	0xef,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x42,
	0x08,
	0xcc,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xf7,
	0xbe,
	0xff,
	0x7b,
	0xaf,
	0xff,
	0x00,
	0x00,
	0x6b,
	0x42,
	0x28,
	0xcc,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xd6,
	0x7a,
	0xff,
	0x84,
	0x10,
	0xf7,
	0x73,
	0xae,
	0xf8,
	0x73,
	0x6e,
	0xfb,
	0x73,
	0x8e,
	0xf8,
	0x5a,
	0xcb,
	0xff,
	0x08,
	0x61,
	0xb3,
	0x42,
	0x28,
	0xcc,
	0xef,
	0x7d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xce,
	0x59,
	0xff,
	0x10,
	0xa2,
	0xb7,
	0x00,
	0x00,
	0x1b,
	0x00,
	0x00,
	0x14,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x0c,
	0x29,
	0x45,
	0x07,
	0x4a,
	0x29,
	0xcc,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xde,
	0xdb,
	0xff,
	0x62,
	0xec,
	0xff,
	0xe7,
	0x1c,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x63,
	0x0c,
	0xf0,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x4a,
	0x29,
	0xcc,
	0xff,
	0xdf,
	0xff,
	0xef,
	0x7d,
	0xff,
	0x4a,
	0x49,
	0xd8,
	0x00,
	0x00,
	0x78,
	0x8c,
	0x51,
	0xfb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xc6,
	0x38,
	0xff,
	0x00,
	0x00,
	0x8b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x5a,
	0xcb,
	0xd3,
	0xde,
	0xdb,
	0xff,
	0x62,
	0xec,
	0xef,
	0x00,
	0x00,
	0x34,
	0x00,
	0x00,
	0x00,
	0x39,
	0xe7,
	0xc7,
	0xef,
	0x5d,
	0xff,
	0xff,
	0xff,
	0xff,
	0xf7,
	0xbe,
	0xff,
	0x52,
	0xaa,
	0xdc,
	0x00,
	0x00,
	0x14,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x41,
	0xe8,
	0xe0,
	0x52,
	0xaa,
	0xff,
	0x00,
	0x00,
	0x40,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x57,
	0x94,
	0x72,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xb5,
	0x96,
	0xff,
	0x00,
	0x00,
	0x78,
	0x00,
	0x00,
	0x00,
	0x08,
	0x61,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x68,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x10,
	0x4a,
	0x69,
	0xd0,
	0xef,
	0x7d,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xf7,
	0xbe,
	0xff,
	0x52,
	0xaa,
	0xd8,
	0x00,
	0x00,
	0x18,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x20,
	0xe4,
	0x00,
	0x00,
	0x20,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x6f,
	0xad,
	0x75,
	0xff,
	0xff,
	0xbf,
	0xff,
	0x84,
	0x10,
	0xff,
	0x31,
	0x86,
	0xac,
	0x00,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x08,
	0x41,
	0x03,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x31,
	0x66,
	0xd7,
	0x39,
	0xc7,
	0xc7,
	0x00,
	0x00,
	0x47,
	0x00,
	0x00,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
#endif
#if LV_COLOR_DEPTH == 32
	0x19,
	0x19,
	0x19,
	0xb8,
	0x1e,
	0x1e,
	0x1e,
	0xc8,
	0x00,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x48,
	0x48,
	0x48,
	0xcc,
	0xb2,
	0xb2,
	0xb2,
	0xff,
	0x3a,
	0x3a,
	0x3a,
	0xcc,
	0x00,
	0x00,
	0x00,
	0x24,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x0a,
	0x0a,
	0x0a,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3b,
	0x3b,
	0x3b,
	0xc8,
	0xf6,
	0xf6,
	0xf6,
	0xff,
	0xdc,
	0xdc,
	0xdc,
	0xff,
	0x43,
	0x43,
	0x43,
	0xe0,
	0x00,
	0x00,
	0x00,
	0x3b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x0a,
	0x0a,
	0x0a,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3b,
	0x3b,
	0x3b,
	0xcb,
	0xe6,
	0xe6,
	0xe6,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xe5,
	0xe5,
	0xe5,
	0xff,
	0x59,
	0x59,
	0x59,
	0xf3,
	0x00,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3c,
	0x3c,
	0x3c,
	0xcb,
	0xe9,
	0xe9,
	0xe9,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xf5,
	0xf5,
	0xf5,
	0xff,
	0x72,
	0x72,
	0x72,
	0xff,
	0x00,
	0x00,
	0x00,
	0x73,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3d,
	0x3d,
	0x3d,
	0xcb,
	0xe9,
	0xe9,
	0xe9,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x8a,
	0x8a,
	0x8a,
	0xff,
	0x00,
	0x00,
	0x00,
	0x90,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x99,
	0x99,
	0x99,
	0x00,
	0x04,
	0x04,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3e,
	0x3e,
	0x3e,
	0xcb,
	0xe9,
	0xe9,
	0xe9,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfe,
	0xfe,
	0xfe,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xa2,
	0xa2,
	0xa2,
	0xff,
	0x13,
	0x13,
	0x13,
	0xab,
	0x00,
	0x00,
	0x00,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3f,
	0x3f,
	0x3f,
	0xcb,
	0xe9,
	0xe9,
	0xe9,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfe,
	0xfe,
	0xfe,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xb7,
	0xb7,
	0xb7,
	0xff,
	0x1f,
	0x1f,
	0x1f,
	0xbb,
	0x00,
	0x00,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0x41,
	0x41,
	0xcc,
	0xea,
	0xea,
	0xea,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfe,
	0xfe,
	0xfe,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xca,
	0xca,
	0xca,
	0xff,
	0x3d,
	0x3d,
	0x3d,
	0xd8,
	0x00,
	0x00,
	0x00,
	0x37,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x41,
	0x41,
	0x41,
	0xcc,
	0xea,
	0xea,
	0xea,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xde,
	0xde,
	0xde,
	0xff,
	0x56,
	0x56,
	0x56,
	0xef,
	0x00,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x42,
	0x42,
	0x42,
	0xcc,
	0xea,
	0xea,
	0xea,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfe,
	0xfe,
	0xfe,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xf3,
	0xf3,
	0xf3,
	0xff,
	0x76,
	0x76,
	0x76,
	0xff,
	0x00,
	0x00,
	0x00,
	0x6b,
	0x43,
	0x43,
	0x43,
	0xcc,
	0xea,
	0xea,
	0xea,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xce,
	0xce,
	0xce,
	0xff,
	0x80,
	0x80,
	0x80,
	0xf7,
	0x74,
	0x74,
	0x74,
	0xf8,
	0x6d,
	0x6d,
	0x6d,
	0xfb,
	0x72,
	0x72,
	0x72,
	0xf8,
	0x57,
	0x57,
	0x57,
	0xff,
	0x0c,
	0x0c,
	0x0c,
	0xb3,
	0x44,
	0x44,
	0x44,
	0xcc,
	0xeb,
	0xeb,
	0xeb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xfb,
	0xfb,
	0xfb,
	0xff,
	0xfe,
	0xfe,
	0xfe,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xc9,
	0xc9,
	0xc9,
	0xff,
	0x13,
	0x13,
	0x13,
	0xb7,
	0x00,
	0x00,
	0x00,
	0x1b,
	0x00,
	0x00,
	0x00,
	0x14,
	0x00,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x0c,
	0x29,
	0x29,
	0x29,
	0x07,
	0x45,
	0x45,
	0x45,
	0xcc,
	0xe8,
	0xe8,
	0xe8,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xd9,
	0xd9,
	0xd9,
	0xff,
	0x5e,
	0x5e,
	0x5e,
	0xff,
	0xe2,
	0xe2,
	0xe2,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x62,
	0x62,
	0x62,
	0xf0,
	0x00,
	0x00,
	0x00,
	0x13,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x45,
	0x45,
	0x45,
	0xcc,
	0xf9,
	0xf9,
	0xf9,
	0xff,
	0xec,
	0xec,
	0xec,
	0xff,
	0x4a,
	0x4a,
	0x4a,
	0xd8,
	0x00,
	0x00,
	0x00,
	0x78,
	0x8a,
	0x8a,
	0x8a,
	0xfb,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xc3,
	0xc3,
	0xc3,
	0xff,
	0x00,
	0x00,
	0x00,
	0x8b,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x58,
	0x58,
	0x58,
	0xd3,
	0xd9,
	0xd9,
	0xd9,
	0xff,
	0x5e,
	0x5e,
	0x5e,
	0xef,
	0x00,
	0x00,
	0x00,
	0x34,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3b,
	0x3b,
	0x3b,
	0xc7,
	0xe9,
	0xe9,
	0xe9,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xf4,
	0xf4,
	0xf4,
	0xff,
	0x54,
	0x54,
	0x54,
	0xdc,
	0x00,
	0x00,
	0x00,
	0x14,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3e,
	0x3e,
	0x3e,
	0xe0,
	0x54,
	0x54,
	0x54,
	0xff,
	0x00,
	0x00,
	0x00,
	0x40,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x57,
	0x8e,
	0x8e,
	0x8e,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xb0,
	0xb0,
	0xb0,
	0xff,
	0x00,
	0x00,
	0x00,
	0x78,
	0x00,
	0x00,
	0x00,
	0x00,
	0x0c,
	0x0c,
	0x0c,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x68,
	0x00,
	0x00,
	0x00,
	0x4f,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x10,
	0x4c,
	0x4c,
	0x4c,
	0xd0,
	0xec,
	0xec,
	0xec,
	0xff,
	0xff,
	0xff,
	0xff,
	0xfc,
	0xf4,
	0xf4,
	0xf4,
	0xff,
	0x53,
	0x53,
	0x53,
	0xd8,
	0x00,
	0x00,
	0x00,
	0x18,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1e,
	0x1e,
	0x1e,
	0x00,
	0x04,
	0x04,
	0x04,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x6f,
	0xab,
	0xab,
	0xab,
	0xff,
	0xf6,
	0xf6,
	0xf6,
	0xff,
	0x80,
	0x80,
	0x80,
	0xff,
	0x31,
	0x31,
	0x31,
	0xac,
	0x00,
	0x00,
	0x00,
	0x17,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x09,
	0x09,
	0x09,
	0x03,
	0x02,
	0x02,
	0x02,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x2e,
	0x2e,
	0x2e,
	0xd7,
	0x38,
	0x38,
	0x38,
	0xc7,
	0x00,
	0x00,
	0x00,
	0x47,
	0x00,
	0x00,
	0x00,
	0x08,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
#endif
};

lv_img_dsc_t mouse_cursor_icon = {
    .header = {
        .always_zero = 0,
        .w = 14,
        .h = 20,
    },
    .data_size = 280 * LV_IMG_PX_SIZE_ALPHA_BYTE,
    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
    .data = mouse_cursor_icon_map,
};
