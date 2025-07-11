#include "frame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "cmd.h"
#include "handler.h"
#include "unicorn_debug.h"

int g_start_flag;
int g_event_interrupt = 0;

UnicornFrame *unicorn_get_valid_frame(unsigned char *buffer, int size)
{
	unsigned int datalen = 0, datasum = 0, cmd_idx = 0;
	unsigned int frame_type = 0;
	unsigned char checksum = 0x0;
	int i = 0, offset = 0, next_info = 0;
	int header_bytes = 0, len = 0;
	UnicornFrame *frame = NULL;

	offset = i;

	/* prefix */
	i += PREFIX_BYTES;

	/* version */
	i += VERSION_BYTES;

	/* frame type */
	frame_type = buffer[i];
	i += FRAME_TYPE_BYTES;

	/* delivery mode */
	i += DELIVERY_MODE_BYTES;

	/* end point idx */
	i += END_POINT_IDX_BYTES;

	/* command index (little-endianness) */
	cmd_idx = buffer[i] + (buffer[i + 1] << 8);
	i += CMDIDX_BYTES;

	/* data length */
	datalen = buffer[i] + (buffer[i + 1] << 8);
	i += PAYLOADLEN_BYTES;

	/* Length sanity check */
	header_bytes = i + CHECKSUM_BYTES;
	if (datalen > UNICORN_FRAME_MAX_LEN || (header_bytes + datalen) > (unsigned)size)
		return NULL;

	/* Data checksum */
	checksum = buffer[i + datalen];
	for (i = 0, datasum = 0; (unsigned)i < (datalen + header_bytes - CHECKSUM_BYTES); i++) {
		datasum += (unsigned char)buffer[i];
	}

	datasum &= 0xff;
	if (datasum != checksum) {
		DBG_MED("Error: Checksum error: calculated 0x%02x, expected 0x%02x\n", checksum, datasum);
		return NULL;
	}

	/* Generate UnicornFrame */
	frame = (UnicornFrame *)calloc(1, sizeof(UnicornFrame));
	if (frame == NULL) {
		DBG_MED("Error: Failed to allocate Unicorn Command Frame!\n");
		return NULL;
	}

	next_info = 0;
	memcpy(frame->prefix, buffer + offset, PREFIX_BYTES);
	next_info += PREFIX_BYTES;
	frame->version = *(buffer + offset + next_info);
	next_info += VERSION_BYTES;
	frame->frame_type = frame_type;
	next_info += FRAME_TYPE_BYTES;
	frame->delivery_mode = *(buffer + offset + next_info);
	next_info += DELIVERY_MODE_BYTES;
	frame->end_point_idx = *(buffer + offset + next_info);
	next_info += END_POINT_IDX_BYTES;
	frame->command = cmd_idx;
	next_info += CMDIDX_BYTES;
	frame->data_len = datalen;
	next_info += PAYLOADLEN_BYTES;

	frame->data = (unsigned char *)calloc((datalen + 1), sizeof(unsigned char));
	if (!frame->data) {
		free(frame);
		DBG_MED("Error: Failed to allocate Unicorn Data!\n");
		return NULL;
	}

	memcpy(frame->data, buffer + offset + next_info, datalen);
	frame->check_sum = checksum;

	len = strnlen((const char *)frame->data, datalen);
	DBG("Received frame: cmd = 0x%02x, strlen(data) = %d\n", frame->command, len);

	return frame;
}

UnicornFrame *unicorn_get_frame(int *client_fd)
{
	unsigned char buf[UNICORN_FRAME_MAX_LEN];
	UnicornFrame *frame = NULL;
	int ret;
	int fd = *client_fd;

	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf));
	if (ret < 0) {
		perror("Failed to read from socket");
		return NULL;
	} else if (ret == 0) {
		perror("Closed by client");
		close(fd);
		*client_fd = -1;
		return NULL;
	} else {
		//__hexdump(buf, ret);
		frame = unicorn_get_valid_frame(buf, ret);
		return frame;
	}
}

static int unicorn_put_out(int fd, char *buf, unsigned int len)
{
	if (buf != NULL) {
		return write(fd, buf, len);
	}
	return -1;
}

int unicorn_put_frame(int fd, int cmd, int ft, char *data, int size)
{
	unsigned int datalen;
	int data_offset, total;
	unsigned int i = 0;
	char buf[UNICORN_FRAME_MAX_LEN] = { 0 };

	/* prefix */
	memcpy(buf + i, PREFIX, strlen(PREFIX));
	i += strlen(PREFIX);

	/* version */
	buf[i++] = 0 & 0xff; //CMD_VERSION;

	/* frame type */
	buf[i++] = ft & 0xff;

	/* delivery mode */
	buf[i++] = 1 & 0xff;

	/* end point idx */
	buf[i++] = 0 & 0xff;

	/* command index */
	buf[i++] = ((cmd >> 0) & 0xff);
	buf[i++] = ((cmd >> 8) & 0xff);

	/* data length */
	datalen = size;
	buf[i++] = ((datalen >> 0) & 0xff);
	buf[i++] = ((datalen >> 8) & 0xff);

	/* data */
	data_offset = i;
	memcpy(buf + data_offset, data, datalen);

	/* checksum */
	total = data_offset + datalen + 1;
	unsigned int datasum = 0;
	for (i = 0; i < (unsigned)total - 1; i++) {
		datasum += buf[i];
	}
	buf[total - 1] = datasum & 0xff;
	return unicorn_put_out(fd, buf, total);
}

void unicorn_free_frame(UnicornFrame *frame)
{
	if (frame == NULL)
		return;
	if (frame->data) {
		free(frame->data);
		frame->data = NULL;
	}
	free(frame);
}

void unicorn_dispatch_cmd(int fd, UnicornFrame *frame)
{
	char buf[UNICORN_FRAME_MAX_LEN] = { 0 };
	unsigned char cmd = frame->command;
	unsigned int ft = frame->frame_type;
	unsigned int ft_out = ft;
	int ret_len = 0;
	int ret = 0;

	g_start_flag = 1;

	if (g_start_flag) {
		switch (cmd) {
		case AGTX_R_ACCESS_MODE:
			g_event_interrupt = cmd;
			ret = unicorn_access_mode(frame, buf, "READ");
			break;
		case AGTX_W_ACCESS_MODE:
			g_event_interrupt = cmd;
			ret = unicorn_access_mode(frame, buf, "WRITE");
			break;
		case AGTX_FAC_RESET:
			g_event_interrupt = cmd;
			ret = unicorn_factory_reset(frame, buf);
			break;
		case AGTX_OTA:
			g_event_interrupt = cmd;
			unicorn_ota(frame, buf);
			break;
		case AGTX_BAD_PIXEL:
			g_event_interrupt = cmd;
			unicorn_bad_pixel(frame, buf);
			break;
		case AGTX_SENSOR_TEST:
			g_event_interrupt = cmd;
			unicorn_sensor_test(frame, buf);
			break;
		case AGTX_LVDS_TEST:
			g_event_interrupt = cmd;
			unicorn_lvds_test(frame, buf);
			break;
		case AGTX_IRCUT_TEST:
			g_event_interrupt = cmd;
			unicorn_ir_cut_ctrl(frame, buf);
			break;
		case AGTX_LIGHT_SENSOR_TEST:
			g_event_interrupt = cmd;
			unicorn_light_sensor(frame, buf);
			break;
		case AGTX_SPEAKER_TEST:
			g_event_interrupt = cmd;
			unicorn_audio_speaker(frame, buf);
			break;
		case AGTX_MIC_TEST:
			g_event_interrupt = cmd;
			unicorn_audio_mic(frame, buf);
			break;
		case AGTX_OC_ADJUST_APPLY:
			g_event_interrupt = cmd;
			unicorn_oc_adjust_apply(frame, buf, fd);
			break;
		case AGTX_OC_ADJUST_PREVIEW:
			g_event_interrupt = cmd;
			unicorn_oc_adjust_preview(frame, buf);
			break;
		case AGTX_R_FW_VERSION:
			g_event_interrupt = cmd;
			ret = unicorn_read_fw_version(frame, buf);
			break;
		case AGTX_R_MB_SN:
			g_event_interrupt = cmd;
			ret = unicorn_mb_number(frame, buf, "READ");
			break;
		case AGTX_R_PID:
			g_event_interrupt = cmd;
			ret = unicorn_product_id(frame, buf, "READ");
			break;
		case AGTX_R_SN:
			g_event_interrupt = cmd;
			ret = unicorn_serial_number(frame, buf, "READ");
			break;
		case AGTX_RSSI:
			g_event_interrupt = cmd;
			unicorn_rssi(frame, buf);
			break;
		case AGTX_SD_ACCESS:
			unicorn_sd_test(frame, buf);
			break;
		case AGTX_W_AC_FREQ_APPLY:
			g_event_interrupt = cmd;
			unicorn_ac_freq_apply(frame, buf, fd);
			break;
		case AGTX_W_AC_FREQ_PREVIEW:
			g_event_interrupt = cmd;
			unicorn_ac_freq_preview(frame, buf);
			break;
		case AGTX_W_MB_SN:
			g_event_interrupt = cmd;
			ret = unicorn_mb_number(frame, buf, "WRITE");
			break;
		case AGTX_W_PID:
			g_event_interrupt = cmd;
			ret = unicorn_product_id(frame, buf, "WRITE");
			break;
		case AGTX_W_SN:
			g_event_interrupt = cmd;
			ret = unicorn_serial_number(frame, buf, "WRITE");
			break;
		case LYON_R_HW_VERSION:
			ERR("LYON_VERSION Not supported!\n");
			break;
		case LYON_R_DEVID:
			ERR("LYON_R_DEVID Not supported!\n");
			break;
		case LYON_W_DEVID:
			ERR("LYON_W_DEVID Not supported!\n");
			break;
		case AGTX_W_CFGINFO:
			g_event_interrupt = cmd;
			ret = unicorn_write_cfg(frame, buf, fd);
			break;
		case AGTX_R_CFGINFO:
			g_event_interrupt = cmd;
			ret = unicorn_read_cfg(frame, buf);
			break;
		case AGTX_W_WIFI_MAC:
			g_event_interrupt = cmd;
			ret = unicorn_wifi_mac_addr(frame, buf, "WRITE");
			break;
		case AGTX_R_WIFI_MAC:
			g_event_interrupt = cmd;
			ret = unicorn_wifi_mac_addr(frame, buf, "READ");
			break;
		case AGTX_W_ETH_MAC:
			g_event_interrupt = cmd;
			ret = unicorn_eth_mac_addr(frame, buf, "WRITE");
			break;
		case AGTX_R_ETH_MAC:
			g_event_interrupt = cmd;
			ret = unicorn_eth_mac_addr(frame, buf, "READ");
			break;
		case AGTX_W_USB_MAC:
			g_event_interrupt = cmd;
			ret = unicorn_usb_mac_addr(frame, buf, "WRITE");
			break;
		case AGTX_R_USB_MAC:
			g_event_interrupt = cmd;
			ret = unicorn_usb_mac_addr(frame, buf, "READ");
			break;
		case AGTX_BUTTON_TEST:
			g_event_interrupt = cmd;
			unicorn_button_ctrl(frame, buf);
			break;
		case AGTX_IRLED_TEST:
			g_event_interrupt = cmd;
			unicorn_ir_led_ctrl(frame, buf);
			break;
		case AGTX_LED_TEST:
			g_event_interrupt = cmd;
			unicorn_led_ctrl(frame, buf);
			break;
		case AGTX_RECEIVE_FILE:
			g_event_interrupt = cmd;
			ret_len = unicorn_recv_file(frame, buf, fd);
			break;
		case AGTX_SEND_FILE:
			g_event_interrupt = cmd;
			ret_len = unicorn_send_file(frame, buf, fd);
			break;
		case AGTX_DAY_NIGHT_MODE:
			g_event_interrupt = cmd;
			unicorn_day_night_mode(frame, buf);
			break;
		case AGTX_FLOODLIGHT_TEST:
			g_event_interrupt = cmd;
			unicorn_floodlight_ctrl(frame, buf);
			break;
		case AGTX_PIR_TEST:
			g_event_interrupt = cmd;
			unicorn_pir_ctrl(frame, buf);
			break;
		default:
			ERR("Error: Unknown command 0x%02x\n", cmd);
			return;
		}
	}

	ft_out = frame->frame_type;
	if (ft_out == FRAME_TYPE_DATA) {
		DBG("data ret_len = %d\n", ret_len);
		ret = unicorn_put_frame(fd, cmd, ft_out, buf, ret_len);
		DBG("Put frame size = %d\n", ret);
	} else {
		DBG("strlen(buf) = %d\n", strlen(buf));
		ret = unicorn_put_frame(fd, cmd, ft_out, buf, strlen(buf));
		DBG("Put frame size = %d\n", ret);
	}
}
