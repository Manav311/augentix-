/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CC_UTILS_H_
#define CC_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <json.h>

#define CC_DEBUG_MEASURE_TIME 0
#if CC_DEBUG_MEASURE_TIME
#include <stdio.h>
#include <sys/time.h>

#define __TIMEVAL_INITIALISER() \
{                               \
	.tv_sec = 0,            \
	.tv_usec = 0            \
}

#define DECLARE_TIMEVAL(n) \
	struct timeval n = __TIMEVAL_INITIALISER()

#define get_time(p) gettimeofday(p,NULL)
#define print_timediff(b,e)                                                 \
{                                                                           \
	fprintf(stderr, "%s: time diff: %ld us\n", __func__,                \
		1000000 * (e.tv_sec - b.tv_sec) + (e.tv_usec - b.tv_usec)); \
}
#else
#define DECLARE_TIMEVAL(n)
#define get_time(p)
#define print_timediff(b,e)
#endif

int validate_json_string_db(struct json_object **json_obj, char *str, int strlen);
int validate_json_string(struct json_object **json_obj, char *buf, int strlen);
struct json_object* get_db_record_obj(const char *db_path, const int id);
int set_data_to_active_db(struct json_object *ret_obj, struct json_object *cmd_obj, int sd, uint32_t cmd_id, long size);
int get_data_from_active_db(struct json_object *ret_obj, int sd, uint32_t cmd_id, long size);
void update_cal_data(const char *cal_data, const char *flag_file, unsigned int force_update);

/* Returns number of bytes read, 0 on EOF, or –1 on error */
ssize_t readn(int fd, void *buffer, size_t count);

/* Returns number of bytes written, or –1 on error */
ssize_t writen(int fd, void *buffer, size_t count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CC_UTILS_H_ */
