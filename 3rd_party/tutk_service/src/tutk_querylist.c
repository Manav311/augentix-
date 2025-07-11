#include "tutk_querylist.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <json.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include "mp4v2/mp4v2.h"

#include <tutk_define.h>
#include "log_define.h"

typedef enum { TUTK_EVENT_ALL, TUTK_EVENT_MOTION, TUTK_EVENT_ALARM } TUTK_EVENT_STATE;

const char *g_event_list[] = { "all", "motion", "pir" };

typedef struct {
	TUTK_EVENT_STATE type;
	long ts;
	long dur;
	char name[128];
} FileInfo;

typedef struct {
	char need_to_fresh;
	int old_st;
	int old_et;
	json_object *response;
	pthread_mutex_t lock;
} Filelist;

Filelist g_filelist = {
	.need_to_fresh = 1,
	.old_st = 0,
	.old_et = 0,
	.response = NULL,
	.lock = PTHREAD_MUTEX_INITIALIZER,
};

json_object *generateFile(const TUTK_EVENT_STATE type, const long ts, const long dur, const char *name)
{
	json_object *ret = json_object_new_object();
	if (ret == NULL) {
		tutkservice_log_err("[tutk_sys]generate_file error\n");
		return NULL;
	}
	json_object *array = json_object_new_array();
	json_object *obj = json_object_new_string(g_event_list[type]);
	json_object_array_add(array, obj);
	json_object_object_add(ret, "eventTypes", array);
	json_object_object_add(ret, "timestamp", json_object_new_int(ts));
	json_object_object_add(ret, "duration", json_object_new_int(dur));
	json_object_object_add(ret, "fileName", json_object_new_string(name));
	json_object_object_add(ret, "channel", json_object_new_int(1));
	json_object_object_add(ret, "thumbnailUrl", json_object_new_string("xxx"));

	return ret;
}

static void addJsonToArray(const FileInfo *pfile, json_object *array)
{
	json_object *obj = generateFile(pfile->type, pfile->ts, pfile->dur, pfile->name);
	json_object_array_add(array, obj);
}

static json_object *makeArrayToResponse(json_object *array)
{
	json_object *ret = json_object_new_object();
	if (ret == NULL) {
		tutkservice_log_err("[tutk_sys]generate_file error\n");
		return NULL;
	}

	json_object_object_add(ret, "results", array);

	return ret;
}

// static void __show_file(const FileInfo *pfile){
//     printf("name = %s\t", pfile->name);
//     printf("ts =%ld\n", pfile->ts);
//     printf("size = %d\t",pfile->size);
//     printf("dur = %ld\n", pfile->dur);
//     printf("-----------------------------\n");
// }
static void free_scandir(int n, struct dirent **namelist)
{
	if (namelist == NULL)
		return;
	while (n-- > 0) {
		free(namelist[n]);
	}
	free(namelist);
}
static int dir_filter(const struct dirent *namelist)
{
	return (namelist->d_type == DT_DIR);
}
static int buildList(const int start_time, const int listnum, const int order, json_object *array)
{
	FileInfo file_info;
	char buff[1278];
	int count = 0;

	file_info.dur = 60000;
	file_info.type = TUTK_EVENT_ALARM;
	struct dirent **dirlist;
	struct dirent **filellist = NULL;
	int n_dir, dir, n_file = 0, file = -1;
	int year, mon, day, search = 1;
	n_dir = scandir(SDCARD_RECORD_DIR, &dirlist, dir_filter, alphasort);
	if (n_dir == -1) {
		tutkservice_log_err("[tutk_sys] open dir error");
		return -1;
	}
	for (dir = 0; dir < n_dir; dir++) {
		if (dirlist[dir]->d_type == DT_DIR &&
		    (sscanf(dirlist[dir]->d_name, "%d-%02d-%02d", &year, &mon, &day)) == 3) {
			struct tm t = { .tm_year = year - 1900, .tm_mon = mon - 1, .tm_mday = day };
			time_t t_of_day = mktime(&t);
			if (((long)t_of_day > (long)start_time)) {
				break;
			}
		}
	}
	dir--;
	while (count < listnum && search) {
		printf("state dir %d nfile %d file %d count %d order %d\n", dir, n_file, file, count, order);
		if (order == 0) { //descending
			if (n_file < 0) {
				search = 0;
			} else if (file < 0) {
				free_scandir(n_file, filellist);
				filellist = NULL;
				n_file = -1;
				if (dir >= 0) {
					snprintf(buff, 128, SDCARD_RECORD_DIR "/%s", dirlist[dir]->d_name);
					printf("scan dir name %s\n", buff);
					n_file = scandir(buff, &filellist, NULL, alphasort);
					file = n_file - 1;
				}
				dir--;
			} else {
				int date;
				if (filellist[file]->d_type == DT_REG &&
				    (sscanf(filellist[file]->d_name, "%d.mp4", &date) == 1)) {
					if (date <= start_time) {
						char buff2[256];
						file_info.ts = date; //wow
						snprintf(buff2, 255, "%s/%s", buff, filellist[file]->d_name);
						strcpy(file_info.name, buff2); //send full path name
						{
							MP4FileHandle hFile = MP4Read(file_info.name);
							if (hFile != MP4_INVALID_FILE_HANDLE) {
								int trackid = MP4FindTrackId(hFile, 0, NULL, 0);
								file_info.dur = MP4GetTrackDuration(hFile, trackid) /
								                VIDEO_TIME_SCALE;
								MP4Close(hFile, 0);
							}
						}
						printf("add %s \n", file_info.name);
						addJsonToArray(&file_info, array);
						count++;
					}
				}
				file--;
			}
		} else { //ascending
			if (n_file < 0) {
				search = 0;
			} else if (file < 0 || file >= n_file) {
				free_scandir(n_file, filellist);
				filellist = NULL;
				n_file = -1;
				if (dir < 0) {
					dir = 0;
				}
				if (dir < n_dir && dirlist[dir]->d_type == DT_DIR) {
					snprintf(buff, 128, SDCARD_RECORD_DIR "/%s", dirlist[dir]->d_name);
					printf("scan dir name %s\n", buff);
					n_file = scandir(buff, &filellist, NULL, alphasort);
					file = 0;
				}
				dir++;
			} else {
				int date;
				if (filellist[file]->d_type == DT_REG &&
				    (sscanf(filellist[file]->d_name, "%d.mp4", &date) == 1)) {
					if (date >= start_time) {
						char buff2[256];
						file_info.ts = date; //wow
						snprintf(buff2, 255, "%s/%s", buff, filellist[file]->d_name);
						strcpy(file_info.name, buff2); //send full path name
						{
							MP4FileHandle hFile = MP4Read(file_info.name);
							if (hFile != MP4_INVALID_FILE_HANDLE) {
								int trackid = MP4FindTrackId(hFile, 0, NULL, 0);
								file_info.dur = MP4GetTrackDuration(hFile, trackid) /
								                VIDEO_TIME_SCALE;
								MP4Close(hFile, 0);
							}
						}
						printf("add %s \n", file_info.name);
						addJsonToArray(&file_info, array);
						count++;
					}
				}
				file++;
			}
		}
	}
	free_scandir(n_file, filellist);
	free_scandir(n_dir, dirlist);

	return count;
}

void TUTK_queryListInit()
{
	pthread_mutex_init(&g_filelist.lock, NULL);
	g_filelist.need_to_fresh = 1;
}

void TUTK_queryListRefresh()
{
	pthread_mutex_lock(&g_filelist.lock);
	g_filelist.need_to_fresh = 1;
	pthread_mutex_unlock(&g_filelist.lock);
}

const char *TUTK_querylist_build_response(int start_time, int listnum, int order)
{
	pthread_mutex_lock(&g_filelist.lock);
	{
		g_filelist.need_to_fresh = 0;
		pthread_mutex_unlock(&g_filelist.lock);

		g_filelist.old_st = start_time;
		json_object_put(g_filelist.response);
		json_object *array = json_object_new_array();
		if (buildList(start_time, listnum, order, array) < 0) {
			tutkservice_log_err("Read SD card failed\n");
		}
		g_filelist.response = makeArrayToResponse(array);
	}

	return json_object_to_json_string_ext(g_filelist.response, JSON_C_TO_STRING_PLAIN);
}
