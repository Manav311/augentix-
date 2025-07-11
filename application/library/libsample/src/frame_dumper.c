#include "frame_dumper.h"

#include <sys/stat.h>
#include <sys/vfs.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

#include "utlist.h"

#define __DEBUG__ 1

#if (__DEBUG__)
#define DEBUG(fmt, args...) printf("[DEBUG] " fmt, ##args)
#else
#define DEBUG(fmt, args...)
#endif

#define DESCRIPTOR_MAX 32

static char *const DEV_PATH = "/dev/";

typedef struct file_item {
	char *path;
	uint32_t mbytes;
	struct file_item *next;
} FileItem;

typedef struct frame_dumper {
	BitStreamSubscriber base_part;
	char previous_output_path[PATH_MAX];
	char dumping_file_path[PATH_MAX];
	const char output_path[PATH_MAX];
	char descriptor[DESCRIPTOR_MAX];
	FILE *fp;
	/* max_frame_count=0 means infinity */
	uint32_t max_frame_count;
	uint32_t dumped_frame_count;
	bool dumped_to_black_hole;
	bool repeat;
	/* repeat mode only */
	FileItem *dumped_files;
	int32_t reserved_mbytes;  /* negative means percentage */
	int32_t recycle_mbytes;  /* negative means percentage */
	int max_dumped_files; /* add to new contructor*/
	/*recent file number in some LL*/
} FrameDumper;

static int dumpFrameToFile(const MPI_STREAM_PARAMS_V2_S *frame, FILE *dest)
{
	for (UINT32 i = 0; i < frame->seg_cnt; ++i) {
		if (!fwrite(frame->seg[i].uaddr, frame->seg[i].size, 1, dest)) {
			perror("fwrite FAILED");
			return errno;
		}
	}
	return 0;
}

static bool detectAvailableSpace(const char *path, uint64_t *available, uint64_t *total)
{
	struct statfs fs_info;
	if (statfs(path, &fs_info)) {
		perror("statfs");
		return false;
	} else {
		if (available) {
			*available = fs_info.f_bavail * fs_info.f_bsize;
		}
		if (total) {
			*total = fs_info.f_blocks * fs_info.f_bsize;
		}
		return true;
	}
}

static bool FRAMEDUMPER_interpretLevels(FrameDumper *dumper,
                                        uint32_t *reserved_mbytes, uint32_t *recycle_mbytes,
                                        uint32_t *available_mbytes, uint32_t *total_mbytes)
{
	uint64_t available, total;
	if (detectAvailableSpace(dumper->dumping_file_path, &available, &total)) {
		available >>= 20;  /* to MBytes */
		total >>= 20;  /* to MBytes */
		if (available_mbytes) {
			*available_mbytes = available;
		}
		if (total_mbytes) {
			*total_mbytes = total;
		}
		if (dumper->reserved_mbytes > 0) {
			*reserved_mbytes = dumper->reserved_mbytes;
		} else {
			*reserved_mbytes = total * -dumper->reserved_mbytes / 100;
		}

		if (dumper->recycle_mbytes > 0) {
			*recycle_mbytes = dumper->recycle_mbytes;
		} else {
			*recycle_mbytes = total * -dumper->recycle_mbytes / 100;
		}

		return true;
	}

	return false;
}

static void FRAMEDUMPER_purgeDumpedItems(FrameDumper *dumper, uint32_t mbytes)
{
	FileItem *item = NULL;
	FileItem *tmp = NULL;
	uint32_t purged_mbytes = 0;
	LL_FOREACH_SAFE(dumper->dumped_files, item, tmp) {
		if (remove(item->path)) {
			perror("remove");
			switch (errno) {
			case EACCES:
			case ELOOP:
			case ENAMETOOLONG:
			case ENOENT:
				/* assume we are NOT capable to remove this file */
				LL_DELETE(dumper->dumped_files, item);
				free(item->path);
				free(item);
				break;
			}
		} else {
			purged_mbytes += item->mbytes;
			LL_DELETE(dumper->dumped_files, item);
			free(item->path);
			free(item);
		}

		if (purged_mbytes >= mbytes) {
			break;
		}
	}
	DEBUG("%s: %u MBytes files have purged.\n", dumper->descriptor, purged_mbytes);
}

static bool FRAMEDUMPER_prepareNextFile(FrameDumper *dumper) // -> successful
{
	time_t t;
	struct tm *tm;
	t = time(NULL);
	tm = localtime(&t);
	bool append_serial_to_path;

	if (!tm) {
		perror("localtime");
		/* we CANNOT construct next output path from current timestamp */
		append_serial_to_path = true;
		strncpy(dumper->previous_output_path, dumper->output_path, sizeof(dumper->previous_output_path));
		fprintf(stderr, "%s: CANNOT get localtime.\n", dumper->descriptor);
	} else {
		/* try to construct next output path from current timestamp */
		char next_path[sizeof(dumper->dumping_file_path)];
		size_t s = strftime(next_path, sizeof(next_path), dumper->output_path, tm);
		if (!s) {
			append_serial_to_path = true;
			strncpy(dumper->previous_output_path, dumper->output_path,
			        sizeof(dumper->previous_output_path));
			printf("%s: dumping file path is exceed buffer size(%d)!\n", dumper->descriptor,
			       sizeof(next_path));
		} else if (strcmp(next_path, dumper->previous_output_path) != 0) {
			/* OK, output path is alternating */
			strncpy(dumper->previous_output_path, next_path, sizeof(dumper->previous_output_path));
			strncpy(dumper->dumping_file_path, next_path, sizeof(dumper->dumping_file_path));
			append_serial_to_path = false;
		} else {
			append_serial_to_path = true;
		}
	}

	if (append_serial_to_path) {
		snprintf(dumper->dumping_file_path, sizeof(dumper->dumping_file_path), "%s.%ld",
		         dumper->previous_output_path, t);
	}

	while (1) {
		dumper->fp = fopen(dumper->dumping_file_path, "wb");
		if (dumper->fp == NULL && strstr(dumper->dumping_file_path, "/mnt/sdcard") != NULL) {
			continue;
		}

		break;
	}

	printf("%s: opening file %s ...\n", dumper->descriptor, dumper->dumping_file_path);

	return dumper->fp != NULL;
}

static uint32_t FRAMEDUMPER_closeDumpingFile(FrameDumper *dumper)
{
	if (dumper->fp != NULL) {
		fclose(dumper->fp);
		dumper->fp = NULL;
	}

	struct stat file_stat;
	if (stat(dumper->dumping_file_path, &file_stat)) {
		perror("stat");
		return 0;
	} else {
		uint32_t mbytes = file_stat.st_size >> 20;
		printf("%s: %u MBytes have dumped to %s.\n", dumper->descriptor, mbytes, dumper->dumping_file_path);
		return mbytes;
	}
}

static bool FRAMEDUMPER_deliveryWillStart(void *context)
{
	FrameDumper *dumper = context;

	if (FRAMEDUMPER_prepareNextFile(dumper)) {
		if (dumper->repeat) {
			uint32_t reserved_mbytes, recycle_mbytes, total_mbytes;
			if (FRAMEDUMPER_interpretLevels(dumper, &reserved_mbytes, &recycle_mbytes, NULL,
			                                &total_mbytes)) {
				if (dumper->reserved_mbytes > 0) {
					printf("%s: reservation level: %u MBytes.\n", dumper->descriptor,
					       reserved_mbytes);
				} else if (dumper->reserved_mbytes < 0) {
					printf("%s: reservation level(%d%%): %u/%u MBytes.\n", dumper->descriptor,
					       -dumper->reserved_mbytes, reserved_mbytes, total_mbytes);
				}

				if (dumper->recycle_mbytes > 0) {
					printf("%s: recycle policy: %u MBytes.\n", dumper->descriptor, recycle_mbytes);
				} else if (dumper->recycle_mbytes < 0) {
					printf("%s: recycle policy(%d%%): %u/%u MBytes.\n", dumper->descriptor,
					       -dumper->recycle_mbytes, recycle_mbytes, total_mbytes);
				}
			}
		}

		dumper->dumped_frame_count = 0;
		return false;
	} else {
		/* If the dumping file is NOT ready, we cancel the subscription */
		return true;
	}
}

static bool FRAMEDUMPER_receiveFrame(void *context, const MPI_STREAM_PARAMS_V2_S *frame)
{
	FrameDumper *dumper = context;
	if (dumper->fp == NULL) {
		return true;
	}

	if (dumper->max_frame_count == 0 || dumper->repeat ||
	    dumper->dumped_frame_count < dumper->max_frame_count /*max frame in single file*/) {
		int error = dumpFrameToFile(frame, dumper->fp);
		if (error) {
			fprintf(stderr, "%s: dump frame to file %s FAILED (err: %d).\n", dumper->descriptor,
			        dumper->dumping_file_path, error);
			fclose(dumper->fp);
			dumper->fp = NULL;
			return true;
		}
		++dumper->dumped_frame_count; /*recent frames number in single file*/
		if (dumper->dumped_frame_count % 10 == 0) {
			char limit[11];
			if (dumper->max_frame_count) {
				snprintf(limit, sizeof(limit), "%u", dumper->max_frame_count);
			} else {
				snprintf(limit, sizeof(limit), "---");
			}
			printf("%s: %u/%s frames are saved!\n", dumper->descriptor, dumper->dumped_frame_count, limit);
		}

		if (dumper->repeat && dumper->max_frame_count <= dumper->dumped_frame_count) {
			uint32_t dumped_mbytes = FRAMEDUMPER_closeDumpingFile(dumper);
			FileItem *new_item = malloc(sizeof(FileItem));
			size_t path_len = strlen(dumper->dumping_file_path) + 1;
			new_item->path = malloc(path_len);
			strncpy(new_item->path, dumper->dumping_file_path, path_len);
			new_item->mbytes = dumped_mbytes;
			LL_APPEND(dumper->dumped_files, new_item);

			/* try to purge some dumped files if storage running low */
			if (dumper->reserved_mbytes && dumper->recycle_mbytes) {
				uint32_t reserved_mbytes;
				uint32_t recycle_mbytes;
				uint32_t available;
				if (FRAMEDUMPER_interpretLevels(dumper, &reserved_mbytes, &recycle_mbytes, &available,
				                                NULL)) {
					DEBUG("%s: storage available: %u MBytes.\n", dumper->descriptor, available);
					if (available < reserved_mbytes) {
						FRAMEDUMPER_purgeDumpedItems(dumper, recycle_mbytes);
					}
				}
			}

			/*now add a new file, add max file number check here, if max return true(true means unsuscribe)*/
			int dumped_files_count = 0;
			FileItem *item;
			LL_COUNT(dumper->dumped_files, item, dumped_files_count);
			if (dumped_files_count >= dumper->max_dumped_files && dumper->max_dumped_files > 0) {
				printf("%s: get max %d dumped files\n", dumper->descriptor, dumped_files_count);
				return true;
			}

			if (FRAMEDUMPER_prepareNextFile(dumper)) {
				dumper->dumped_frame_count = 0;
			} else {
				/* unable to prepare next file, we signal publisher to stop delivery */
				return true;
			}
		}
	}

	return dumper->max_frame_count ? dumper->max_frame_count <= dumper->dumped_frame_count : false;
}

static void FRAMEDUMPER_deliveryDidEnd(void *context)
{
	FRAMEDUMPER_closeDumpingFile(context);

#if 1
	FrameDumper *dumper = context;
	printf("%s: dumped file list -----\n", dumper->descriptor);
	int i = 0;
	FileItem *item = NULL;
	FileItem *tmp = NULL;
	LL_FOREACH_SAFE(dumper->dumped_files, item, tmp) {
		++i;
		printf("%d: %s, %u MBytes.\n", i, item->path, item->mbytes);
		LL_DELETE(dumper->dumped_files, item);
		free(item);
	}
#endif
}

// output_path MUST be non-null
BitStreamSubscriber *newFrameDumper(MPI_ECHN encoder_channel, const char *output_path, uint32_t frame_count,
                                    bool repeat, int32_t reservation_level, int32_t recycle_level, int max_dumped_files)
{
	FrameDumper *dumper = malloc(sizeof(*dumper));
	snprintf((char *)dumper->output_path, sizeof(dumper->output_path), "%s", output_path);
	dumper->max_frame_count = frame_count;
	dumper->fp = NULL;
	dumper->repeat = repeat;
	dumper->dumped_files = NULL;
	dumper->reserved_mbytes = reservation_level;
	dumper->recycle_mbytes = recycle_level;
	dumper->max_dumped_files = max_dumped_files;

	if (dumper->recycle_mbytes == 0) {
		dumper->recycle_mbytes = 1;
	}
	memset(dumper->previous_output_path, 0, sizeof(dumper->previous_output_path));
	snprintf(dumper->descriptor, sizeof(dumper->descriptor), "FrameDumper[%d]", encoder_channel.chn);
	if (!dumper->repeat || strncmp(dumper->output_path, DEV_PATH, strlen(DEV_PATH)) == 0) {
		dumper->dumped_to_black_hole = true;
		printf("%s: assume dumping frames to unlimited sink.\n", dumper->descriptor);
	} else {
		dumper->dumped_to_black_hole = false;
	}

	BitStreamSubscriber *base = &dumper->base_part;
	base->context = dumper;
	base->deliveryWillStart = FRAMEDUMPER_deliveryWillStart;
	base->receiveFrame = FRAMEDUMPER_receiveFrame;
	base->deliveryDidEnd = FRAMEDUMPER_deliveryDidEnd;
	return base;
}