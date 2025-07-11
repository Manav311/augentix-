#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_video.h"
#include "agtx_video_layout_conf.h"
#include "cm_video_layout_conf.h"

#include "agtx_cmd.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(VideoLayout)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_VIDEO_LAYOUT_CONF);
	parse_layout_conf(&g_conf.layout, ret_obj);

	avmain2_log_info("layout layout_num:%d. dev_idx:%d", g_conf.layout.layout_num, g_conf.layout.video_dev_idx);

	json_object_put(ret_obj);

	return 0;
}

int WRITE_DB(VideoLayout)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(VideoLayout)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	int ret = 0;
	int ptr = 0;
	AGTX_LAYOUT_CONF_S *layout = (AGTX_LAYOUT_CONF_S *)data;
	if (len != sizeof(AGTX_LAYOUT_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_LAYOUT_CONF_S));
		return -EINVAL;
	}
	NodeId id = NONE;

	saveOldConftoTmp(&g_conf.layout, layout, sizeof(AGTX_LAYOUT_CONF_S));
	AGTX_LAYOUT_CONF_S *layout_old = (AGTX_LAYOUT_CONF_S *)&g_old_conf_tmp;

	if (layout_old->video_layout[0].window_array[0].pos_x != layout->video_layout[0].window_array[0].pos_x ||
	    layout_old->video_layout[0].window_array[0].pos_y != layout->video_layout[0].window_array[0].pos_y ||
	    layout_old->video_layout[0].window_array[0].pos_width !=
	            layout->video_layout[0].window_array[0].pos_width ||
	    layout_old->video_layout[0].window_array[0].pos_height !=
	            layout->video_layout[0].window_array[0].pos_height) {
		/*win[0,0] size change need to restart VB*/
		id = VB;
		ptr = ((int)node) + (id * sizeof(Node));
		ret = NODES_execRestart((Node *)ptr);
		if (ret != 0) {
			recoverOldConfFromTmp(&g_conf.strm, sizeof(AGTX_LAYOUT_CONF_S));
			return ret;
		}

		return 0;
	}

	/*ALL OTHER case need to restart channels*/
	id = CHN;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execRestart((Node *)ptr);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.layout, sizeof(AGTX_LAYOUT_CONF_S));
		return ret;
	}

	return 0;
}

static JsonConfHandler video_layout_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_VIDEO_LAYOUT_CONF, VideoLayout);

__attribute__((constructor)) void registerVideoLayout(void)
{
	HANDLERS_registerHandlers(&video_layout_ops);
}
