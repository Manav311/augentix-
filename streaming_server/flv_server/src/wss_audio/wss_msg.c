#include "wss_msg.h"

#include <stdlib.h>
#include <string.h>
#include <json.h>
#include "errno.h"

#include "log_define.h"

int freeParseJson(json_object *config_obj)
{
	json_object_put(config_obj);
	return 0;
}

int WSS_parseWssPcmMessage(char *fd, PcmMessage *msg)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	char dataBuf[64];

	/* load json config from json file */
	test_obj = json_tokener_parse(fd);
	if (!test_obj) {
		flv_server_log_err("Cannot open fd, invaild json");
		goto end;
	}

	json_object_object_get_ex(test_obj, "id", &tmp_obj);
	if (!tmp_obj) {
		flv_server_log_err("Failed to find ID, invalid json");
		goto end;
	}
	msg->id = json_object_get_int(tmp_obj);
	flv_server_log_info("Date: %d", (int)msg->id);

	json_object_object_get_ex(test_obj, "type", &tmp_obj);
	if (!tmp_obj) {
		flv_server_log_err("Failed to find msg type, invalid json");
		goto end;
	}
	sprintf(&(msg->msg_type[0]), "%s", json_object_get_string(tmp_obj));
	flv_server_log_info("type: %s", msg->msg_type);

	if (0 == strcmp(msg->msg_type, "ping")) {
		goto save_end;
	} else if (0 == strcmp(msg->msg_type, "pong")) {
		goto save_end;
	} else if (0 == strcmp(msg->msg_type, "call_start")) {
		goto save_end;
	} else if (0 == strcmp(msg->msg_type, "data_available")) {
	} else if (0 == strcmp(msg->msg_type, "call_end")) {
		goto save_end;
	} else {
		flv_server_log_err("Unknown type, invalid json");
		goto end;
	}
	json_object_object_get_ex(test_obj, "mimeType", &tmp_obj);
	if (!tmp_obj) {
		flv_server_log_err("Failed to find mime type, invalid json\n");
		goto end;
	}
	sprintf(&dataBuf[0], "%s", json_object_get_string(tmp_obj));
	flv_server_log_info("mime type: %s.", dataBuf);

	if (0 == strcmp(dataBuf, "audio/l16")) {
		msg->type = S16LE;
	} else if (0 == strcmp(dataBuf, "audio/pcma")) {
		msg->type = ALAW;
	} else if (0 == strcmp(dataBuf, "audio/pcmu")) {
		msg->type = MULAW;
	} else {
		flv_server_log_debug("failed to get pcm format");
		goto end;
	}

	json_object_object_get_ex(test_obj, "length", &tmp_obj);
	if (!tmp_obj) {
		flv_server_log_err("Failed to find pcm length, invalid json");
		goto end;
	}
	msg->length = json_object_get_int(tmp_obj);
	flv_server_log_info("len: %d.", msg->length);

	json_object_object_get_ex(test_obj, "data", &tmp_obj);
	if (!tmp_obj) {
		flv_server_log_err("Failed to find pcm data, invalid json");
		goto end;
	}

	int array_len = json_object_array_length(tmp_obj);
	if (array_len < 0) {
		flv_server_log_err("Failed to parse pcm array, invalid json");
		goto end;
	}
	msg->data = malloc(msg->length);
	if (msg->data == NULL) {
		flv_server_log_err("Failed to alloc pcm buf");
		goto end;
	}
	if (array_len != msg->length) {
		flv_server_log_err("pcm array and data length diff(%d, %d).", array_len, msg->length);
	}
	flv_server_log_info("data len: %d.", array_len);
	for (int i = 0; i < msg->length; i++) {
		tmp1_obj = json_object_array_get_idx(tmp_obj, i);
		if (!tmp1_obj) {
			flv_server_log_err("Failed to get chn id.");
			break;
		}

		msg->data[i] = atoi(json_object_get_string(tmp1_obj));
	}
save_end:
	freeParseJson(test_obj);
	return 0;
end:
	freeParseJson(test_obj);
	return -EINVAL;
}