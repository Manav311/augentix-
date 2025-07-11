#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#include <stdbool.h>
//#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "sqllib.h"

#include "json.h"

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_cmd.h"
#include "agtx_sys_db_info.h"

#include "cm_sys_db_info.h"

#include "cc_common.h"
#include "cc_data.h"
#include "cc_utils.h"

static int overwrite_db_record(const char *factory_db_path, const char *active_db_path,
                               int cmd_id)
{
	int ret = 0;
	struct json_object *obj = NULL;
	char *factory_rec = NULL, *active_rec = NULL;

//	DECLARE_TIMEVAL(btime);
//	DECLARE_TIMEVAL(etime);

//	get_time(&btime);

	/* Check if record exists in new factory default database */
	factory_rec = get_sqldata_by_id_str_path(factory_db_path, "json_tbl", "jstr", cmd_id);
	if (!factory_rec) {
		/* Do nothing if the record does NOT exist in new factory default database */
		fprintf(stderr, "Record (id 0x%08X) is not in DB %s\n", cmd_id, factory_db_path);
		goto end;
	}

	ret = validate_json_string_db(&obj, factory_rec, strlen(factory_rec));
	if (ret < 0) {
		fprintf(stderr, "No a valid JSON string\n");
		goto end;
	}

	json_object_put(obj);

	/* Check if record exists in active setting database */
	active_rec = get_sqldata_by_id_str_path(active_db_path, "json_tbl", "jstr", cmd_id);

	if (active_rec) {
		/* Overwrite firmware settings in active setting database */
		ret = update_sqldata_by_id_str_path(active_db_path, "json_tbl", cmd_id,
		                                    "jstr", factory_rec);
		free(active_rec);
	} else {
		/* Add record into active setting database */
		ret = add_sqldata_by_id_str_path(active_db_path, "json_tbl", cmd_id,
						 "jstr", factory_rec);
	}

	free(factory_rec);
end:
	if (ret) {
		fprintf(stderr, "Failed to update record (0x%08X) into active database.\n",
			cmd_id);
	}

//	get_time(&etime);
//	print_timediff(btime, etime);

	return ret;
}

static int merge_db_record(const char *factory_db_path, const char *active_db_path,
                           int cmd_id, long size)
{
	int ret = 0;
	struct json_object *factory_obj = NULL, *active_obj = NULL, *obj = NULL;
	void *data = NULL;
	PARSE_FUNC_S parse_func = NULL;
	COMP_FUNC_S comp_func = NULL;

//	DECLARE_TIMEVAL(btime);
//	DECLARE_TIMEVAL(etime);

//	get_time(&btime);

	/* Check if record exists in new factory default database */
	factory_obj = get_db_record_obj(factory_db_path, cmd_id);
	if (!factory_obj) {
		/* Do nothing if the record does NOT exist in new factory default database */
		goto end;
	}

	/* Check if the record exists in active setting database */
	active_obj = get_db_record_obj(active_db_path, cmd_id);

	if (active_obj) {
		obj = json_object_new_object();

		if (obj) {
			data = malloc(size);

			if (data) {
				memset(data, 0, size);

				ret = determine_func(&parse_func, &comp_func, cmd_id);

				if (!ret) {
					parse_func(data, factory_obj);
					parse_func(data, active_obj);

					comp_func(obj, data);

					/* Update the existing record in active setting database */
					ret = update_sqldata_by_id_str_path(active_db_path, "json_tbl", cmd_id,
									    "jstr", json_object_to_json_string(obj));
				} else {
					fprintf(stderr, "%s: failed to find parsing and composing func\n", __func__);
					goto end;
				}

				free(data);
			} else {
				fprintf(stderr, "%s: failed to allocate memory\n", __func__);
				ret = -1;
			}

			json_object_put(obj);
		} else {
			fprintf(stderr, "%s: failed to new json object\n", __func__);
			ret = -1;
		}

		json_object_put(active_obj);
	} else {
		/* Add the new record into active setting database */
		ret = add_sqldata_by_id_str_path(active_db_path, "json_tbl", cmd_id,
						 "jstr", json_object_to_json_string(factory_obj));
	}

	json_object_put(factory_obj);
end:
	if (ret) {
		fprintf(stderr, "%s: failed to merge db record (0x%08X)\n", __func__, cmd_id);
	}

//	get_time(&etime);
//	print_timediff(btime, etime);

	return ret;
}

int merge_database(void)
{
	int ret = 0;
	int i, j, is_ow;
	struct json_object *factory_obj = NULL;
	AGTX_SYS_DB_INFO_S *db_info;
	AGTX_FW_SETTING_PARAM_S *fw_setting = NULL;
	const CC_CMD_INFO_S *info = NULL;

	DECLARE_TIMEVAL(btime);
	DECLARE_TIMEVAL(etime);

	get_time(&btime);

	db_info = (AGTX_SYS_DB_INFO_S *)malloc(sizeof(AGTX_SYS_DB_INFO_S));
	if (!db_info) {
		fprintf(stderr, "Failed to allocate memory\n");
		ret = -1;
		goto end;
	}

	memset(db_info, 0, sizeof(AGTX_SYS_DB_INFO_S));

	/* Check if DB info exists in new factory default database */
	factory_obj = get_db_record_obj(FACTORY_DEFAULT_DB, AGTX_CMD_SYS_DB_INFO);
	if (factory_obj) {
		parse_sys_db_info(db_info, factory_obj);
		json_object_put(factory_obj);
	}

	for (i = 0; i < cmd_table_size; ++i) {
		info = &cmd_table[i];
	//	fprintf(stderr, "cmd string: %s\n", info->str);

		if (1 /*!info->is_cal_data*/) {
			is_ow = 0;

			for (j = 0; (unsigned)j < CC_NELEMS(db_info->fw_setting_list); ++j) {
				fw_setting = &db_info->fw_setting_list[j];

				if (strcmp((char *)fw_setting->name, "") == 0) {
				//	fprintf(stderr, "cmd: %s hit null\n", info->str);
					break;
				} else if (strcmp((char *)fw_setting->name, info->str) == 0) {
					if (fw_setting->update_rule == AGTX_UPDATE_RULE_OVERWRITE) {
						is_ow = 1;
					}

				//	fprintf(stderr, "cmd: %s hit overwrite\n", info->str);
					break;
				}
			}

			if (is_ow) {
			//	fprintf(stderr, "overwrite record cmd: %s\n", info->str);
				overwrite_db_record(FACTORY_DEFAULT_DB, TMP_ACTIVE_DB, info->cmd_id);
			} else {
			//	fprintf(stderr, "merge record cmd: %s\n", info->str);
				merge_db_record(FACTORY_DEFAULT_DB, TMP_ACTIVE_DB, info->cmd_id, info->size);
			}
		}
	}

//	fprintf(stderr, "merge done\n");
	free(db_info);
end:
	get_time(&etime);
	print_timediff(btime, etime);

	return ret;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
