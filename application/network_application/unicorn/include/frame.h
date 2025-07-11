#ifndef UNICORN_FRAME_H_
#define UNICORN_FRAME_H_

#include <stdint.h>

#define UNICORN_FRAME_MAX_LEN 16384 * 2
#define PREFIX "AGTX"
#define CMD_VERSION 0x00

#define PREFIX_BYTES 4
#define VERSION_BYTES 1
#define FRAME_TYPE_BYTES 1
#define DELIVERY_MODE_BYTES 1
#define END_POINT_IDX_BYTES 1
#define CMDIDX_BYTES 2
#define PAYLOADLEN_BYTES 2
#define PAYLOAD_BYTES 256
#define CHECKSUM_BYTES 1

#define FRAME_TYPE_DATA 0
#define FRAME_TYPE_CMD 1
#define FRAME_TYPE_ACK 2

extern int g_start_flag;
extern int g_event_interrupt;

typedef struct unicorn_frame {
	uint8_t prefix[PREFIX_BYTES];
	uint8_t version;
	uint8_t frame_type;
	uint8_t delivery_mode;
	uint8_t end_point_idx;
	uint16_t command;
	uint16_t data_len; /* actual data length in unsigned int */
	uint8_t *data;
	uint8_t check_sum;
} UnicornFrame;

typedef enum frame_type {
	FT_data = 0,
	FT_command = 1,
	FT_ack = 2,
} FrameType;

typedef enum delivery_mode {
	DM_unicast = 0,
	DM_broadcast = 1,
} DeliveryMoee;

UnicornFrame *unicorn_get_valid_frame(unsigned char *buffer, int size);
UnicornFrame *unicorn_get_frame(int *client_fd);
//static int unicorn_put_out(int fd, char *buf, unsigned int len);
int unicorn_put_frame(int fd, int cmd, int ft, char *data, int size);
void unicorn_free_frame(UnicornFrame *frame);
void unicorn_dispatch_cmd(int fd, UnicornFrame *frame);

#endif
