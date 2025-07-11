#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_audio_conf.h"


const char * agtx_audio_codec_e_map[] = {
	"NONE",
	"PCM",
	"ULAW",
	"ALAW",
	"G726"
};

void parse_audio_conf(AGTX_AUDIO_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "codec", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_audio_codec_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_audio_codec_e_map[i], str) == 0) {
				data->codec = (AGTX_AUDIO_CODEC_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gain", &tmp_obj)) {
		data->gain = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sampling_bit", &tmp_obj)) {
		data->sampling_bit = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sampling_frequency", &tmp_obj)) {
		data->sampling_frequency = json_object_get_int(tmp_obj);
	}
}

void comp_audio_conf(struct json_object *ret_obj, AGTX_AUDIO_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_audio_codec_e_map[data->codec];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "codec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "codec");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gain");
	}

	tmp_obj = json_object_new_int(data->sampling_bit);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sampling_bit", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sampling_bit");
	}

	tmp_obj = json_object_new_int(data->sampling_frequency);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sampling_frequency", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sampling_frequency");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
