#include "vftr_dump.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <linux/limits.h>
#include <errno.h>
#include <pthread.h>

#include "inf_types.h"
#include "inf_image.h"
#include "vftr_dump_decode.h"
#include "eaif_dump_decode.h"
#include "utlist.h"
#include "rtsp_shm.h"

#define GetImageTypeChn(dtype) (((dtype >> 3) & 0x3) + 1)

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static int g_save_imge_running = 0;
static pthread_t g_tid;

typedef struct image_file_elemt {
	char dir_path[PATH_MAX];
	InfImage img;
	UINT32 group;
	TIMESPEC_S timespec;
	PID_T tid;
	struct image_file_elemt *next, *prev;
} ImageFileElemt;

ImageFileElemt *img_head = NULL;

static void *__save_img_file_thread(void *args)
{
	(void)args;
	char file_name[PATH_MAX];
	char buf[20];

	while (g_save_imge_running) {
		if (img_head != NULL) {
			pthread_mutex_lock(&g_mutex);
			ImageFileElemt *tmp_elemt = img_head;
			DL_DELETE(img_head, img_head);
			pthread_mutex_unlock(&g_mutex);

			struct tm *timeinfo;
			timeinfo = gmtime(&tmp_elemt->timespec.tv_sec);
			strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", timeinfo);
			int chn_no = GetImageTypeChn(tmp_elemt->img.dtype);
			char img_type[20];
#define EXPORT_DUMP(name, type)                           \
	do {                                              \
		if (EAIF_ID_##name == tmp_elemt->group) { \
			strcpy(img_type, #name);          \
			break;                            \
		}                                         \
	} while (0);

			EXPORT_EAIF_DUMP_ARRAY

#undef EXPORT_DUMP

			char file_format[2][4] = { "pgm", "ppm" };

			sprintf(file_name, "%s/%d_%s.%03lu_%s.%s", tmp_elemt->dir_path, tmp_elemt->tid, buf,
			        (unsigned long)tmp_elemt->timespec.tv_nsec / 1000000, img_type,
			        file_format[chn_no / 3]);
			int idx = 1;
			while (!access(file_name, F_OK)) {
				sprintf(file_name, "%s/%d_%s.%03lu_%s%d.%s", tmp_elemt->dir_path, tmp_elemt->tid, buf,
				        (unsigned long)tmp_elemt->timespec.tv_nsec / 1000000, img_type, ++idx,
				        file_format[chn_no / 3]);
			}

			Inf_Imwrite(file_name, &tmp_elemt->img);

			free(tmp_elemt->img.data);
			free(tmp_elemt);
		} else {
			pthread_mutex_lock(&g_mutex);
			pthread_cond_wait(&g_cond, &g_mutex);
			pthread_mutex_unlock(&g_mutex);
		}
	}

	g_save_imge_running = -1;

	return NULL;
}

static bool __shouldDumpMessage(UINT32 flag, const DumpFilterConfig *config, pid_t tid)
{
	VFTR_DUMP_FLAG_V1_U *msg = (VFTR_DUMP_FLAG_V1_U *)&flag;
	int is_found;

	if (config->id_num) {
		is_found = 0;

		for (int i = 0; i < config->id_num; ++i) {
			if (msg->field.id == config->id[i]) {
				is_found = 1;
			}
		}

		if (!is_found) {
			return false;
		}
	}

	if (config->group_num) {
		is_found = 0;

		for (int i = 0; i < config->group_num; ++i) {
			if (msg->field.group == config->group[i]) {
				is_found = 1;
			}
		}

		if (!is_found) {
			return false;
		}
	}

	if (config->no_group_num) {
		is_found = 0;

		for (int i = 0; i < config->no_group_num; ++i) {
			if (msg->field.group == config->no_group[i]) {
				is_found = 1;
			}
		}

		if (is_found) {
			return false;
		}
	}

	if (config->type_num) {
		is_found = 0;

		for (int i = 0; i < config->type_num; ++i) {
			if (msg->field.type == config->type[i]) {
				is_found = 1;
			}
		}

		if (!is_found) {
			return false;
		}
	}

	if (config->cat_num) {
		is_found = 0;

		for (int i = 0; i < config->cat_num; ++i) {
			if (msg->field.category == config->cat[i]) {
				is_found = 1;
			}
		}

		if (!is_found) {
			return false;
		}
	}

	if (config->tid) {
		if (tid != config->tid) {
			return false;
		}
	}

	return true;
}

static int decodeEaifMessage(const char *dir_path, char **buf, size_t count, UINT32 flag, TIMESPEC_S timestamp,
                             PID_T tid)
{
	static InfImage img = { 0 };
	static PID_T img_tid;
	static TIMESPEC_S img_timestamp = { 0 };
	VFTR_DUMP_FLAG_V1_U msg = (VFTR_DUMP_FLAG_V1_U)flag;

	if (msg.field.type & FLAG_TYPE_IMG) {
		if ((msg.field.id == EAIF_ID_InfImage) || (msg.field.id == EAIF_ID_InfFaceImage)) {
			assert(img.w == 0);
			memcpy(&img, *buf, sizeof(img));
			img.data = NULL;
			img_tid = tid;
			img_timestamp = timestamp;
		} else if ((msg.field.group == EAIF_ID_InfImage) || (msg.field.group == EAIF_ID_InfFaceImage)) {
			assert(img.w != 0);

			/* check data length is correct */
			switch (img.dtype) {
			case Inf8U:
			case Inf8S:
			case Inf8UC3:
			case Inf8SC3:
				assert((img.w * img.h * img.c) == (int)count);
				assert(img_tid == tid);
				assert(img_timestamp.tv_nsec == timestamp.tv_nsec);
				img.data = (void *)*buf;
				break;
			default:
				assert(0);
				break;
			}

			/* if image directory is specified */
			if (dir_path) {
				ImageFileElemt *elemt = malloc(sizeof(*elemt));
				if (elemt) {
					strcpy(elemt->dir_path, dir_path);
					elemt->img = img;
					elemt->group = msg.field.group;
					elemt->tid = tid;
					elemt->timespec = timestamp;

					pthread_mutex_lock(&g_mutex);
					DL_APPEND(img_head, elemt);
					pthread_cond_broadcast(&g_cond);
					pthread_mutex_unlock(&g_mutex);

					/* avoid double free in upper layer */
					*buf = NULL;
				} else {
					fprintf(stderr, "failed to save img !\n");
				}
			}

			img.w = 0;
		}
	}

	EAIF_showDump((VOID *)*buf, count, flag, timestamp, tid);

	return 0;
}

static int readMessageFromFile(int fd, VOID **buf, UINT32 *flag, TIMESPEC_S *timestamp, pid_t *tid)
{
	VFTR_DUMP_HEADER_V1_S header;
	int len = 0;
	int cnt = 0;

	/* read header first, check if header is valid */
	len = sizeof(header);
	do {
		cnt = read(fd, ((char *)&header) + (sizeof(header) - len), len);
		if (cnt < 0) {
			fprintf(stderr, "error %d occur while reading !\n", errno);
			goto error;
		} else if (cnt == 0) {
			fprintf(stderr, "reach EOF !\n");
			goto error;
		}

		len -= cnt;
	} while (len > 0);

	assert(len == 0);
	assert(header.init_code == VFTR_HEADER_INITCODE);

	*timestamp = header.timestamp;
	*tid = header.tid;
	*flag = header.flag.val;

	/* read data from file */
	int remain_bytes = len = header.payload_len;
	if (header.payload_len) {
		*buf = malloc(header.payload_len);
		if (*buf == NULL) {
			fprintf(stderr, "failed to allocate memory !\n");
			goto error;
		}

		/* read data loop */
		while (remain_bytes > 0) {
			cnt = read(fd, (char *)*buf + (len - remain_bytes), remain_bytes);

			/* read data until EOF or error occur */
			if (cnt < 0) {
				fprintf(stderr, "error %d occur while reading !\n", errno);
				goto free;
			} else if (cnt == 0) {
				fprintf(stderr, "reach EOF !\n");
				goto free;
			}

			remain_bytes -= cnt;
		}
	}

	assert(remain_bytes == 0);

	return header.payload_len;

free:
	free(*buf);
	*buf = NULL;

error:
	return -1;
}

static int writeDataToStdout(const VOID *buf, size_t len, UINT32 flag, TIMESPEC_S timestamp, PID_T tid)
{
	/* print dump to path */
	VFTR_DUMP_HEADER_V1_S header = { .init_code = VFTR_HEADER_INITCODE,
		                         .ver = 0x0,
		                         .payload_len = len,
		                         .flag = (VFTR_DUMP_FLAG_V1_U){ .val = flag },
		                         .timestamp = timestamp,
		                         .tid = tid };

	/* write data to stdout with headers */
	if (write(fileno(stdout), &header, sizeof(header)) != sizeof(header)) {
		fprintf(stderr, "error writing header !\n");
		return -1;
	}

	assert((len % 4) == 0);

	if (write(fileno(stdout), buf, len) != (int)len) {
		fprintf(stderr, "error writing data !\n");
		return -1;
	}

	return 0;
}

int DUMP_start(int pid, const DumpConfig *config)
{
	char *data = NULL;
	int len = 0;
	UINT32 flag = 0;
	TIMESPEC_S timestamp;
	PID_T tid;
	int fd = -1;

	/* read from file */
	if (config->in_binary_path) {
		if ((fd = open(config->in_binary_path, O_RDONLY)) == -1) {
			fprintf(stderr, "failed to open %s !", config->in_binary_path);
			return -1;
		}
	} else { /* read from dump fifo */
		char buf[64] = { 0 };
		snprintf(buf, 64, "%s.%d", VFTR_DUMP_ENABLE_PATH, pid);

		if (VFTR_linkDump(buf)) {
			fprintf(stderr, "failed to link dump fifo !\n");
			return -1;
		}
	}

	/* create thread for file io */
	g_save_imge_running = 1;
	int ret = pthread_create(&g_tid, NULL, __save_img_file_thread, NULL);
	if (ret) {
		goto exit;
	}

	time_t start_time = time(NULL);

	/* dump message loop */
	while (true) {
		assert(data == NULL);

		if (config->in_binary_path) {
			len = readMessageFromFile(fd, (VOID **)&data, &flag, &timestamp, &tid);
		} else {
			len = VFTR_readDump((VOID **)&data, &flag, &timestamp, &tid);
		}

		/* read error or reach EOF */
		if (len < 0) {
			fprintf(stderr, "stop reading due to error %d !\n", len);
			break;
		} else if (len == 0) {
			assert(data == NULL);
		} else {
			assert(flag);
			assert(data);
		}

		/* decode message or write to stdout */
		if (__shouldDumpMessage(flag, &config->filter, tid)) {
			VFTR_DUMP_FLAG_V1_U msg = (VFTR_DUMP_FLAG_V1_U)flag;

			if (!config->is_binary) {
#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
				if (config->rtsp_app) {
					/* nothing */
					RSHM_updateStatus(config->win, data, len, flag, timestamp);
				}
#endif
				/* show text */
				if (msg.field.category == FLAG_CAT_VFTR) {
					VFTR_showDump(data, len, flag, timestamp, tid);
				} else if (msg.field.category == FLAG_CAT_EAIF) {
					decodeEaifMessage(config->img_directory, &data, len, flag, timestamp, tid);
				} else {
					fprintf(stderr, "unknown category %d !\n", msg.field.category);
					break;
				}
			} else {
				if (writeDataToStdout(data, len, flag, timestamp, tid) < 0) {
					break;
				}
			}
		}

		/* should free memory pointed by data */
		if (data) {
			free(data);
			data = NULL;
		}

		/* stop dump data if interval_in_sec is set */
		time_t end_time = time(NULL);
		if ((config->interval_in_sec) && (difftime(end_time, start_time) > config->interval_in_sec)) {
			break;
		}
	}

	/* release resources */
	if (data) {
		free(data);
		data = NULL;
	}

	/* stop __save_img_file_thread */
	g_save_imge_running = 0;
	while (g_save_imge_running != -1) {
		pthread_mutex_lock(&g_mutex);
		pthread_cond_broadcast(&g_cond);
		pthread_mutex_unlock(&g_mutex);
		usleep(100000);
	}

	pthread_join(g_tid, NULL);

	/* free image file list */
	ImageFileElemt *elemt, *tmp;
	DL_FOREACH_SAFE(img_head, elemt, tmp)
	{
		DL_DELETE(img_head, elemt);
		free(elemt->img.data);
		free(elemt);
	}

exit:
	if (fd != -1) {
		close(fd);
		fd = -1;
	} else {
		VFTR_unlinkDump();
	}

	return 0;
}

void DUMP_stop(void)
{
	/* after calling unlink, VFTR_readDump will return err code */
	VFTR_unlinkDump();

	return;
}

static void dumpVftrGroupId(void)
{
	printf("------------- VFTR GROUP LIST --------------\n");
	printf("group\tname\n");
#define EXPORT_DUMP(name, type)                                                                           \
	do {                                                                                              \
		if ((strstr(#name, "INSTANCE") != NULL) || (strstr(#name, "VFTR_PFM_INPUT_S") != NULL)) { \
			printf("%d\t%s\n", VFTR_ID_##name, #name);                                        \
		}                                                                                         \
	} while (0);

	EXPORT_VFTR_DUMP_ARRAY

#undef EXPORT_DUMP

	printf("\n");
}

static void dumpVftrStructId(void)
{
	printf("--------------- VFTR ID LIST ---------------\n");
	printf("cat\tid\ttype\tname\n");
#define EXPORT_DUMP(name, type) printf("%d\t%d\t0x%02X\t%s\n", FLAG_CAT_VFTR, VFTR_ID_##name, VFTR_TYPE_##name, #name);

	EXPORT_VFTR_DUMP_ARRAY

#undef EXPORT_DUMP
	printf("\n");
}

static void dumpInfGroupId(void)
{
	printf("------------- EAIF GROUP LIST --------------\n");
	printf("group\tname\n");
	printf("\n");
}

static void dumpInfStructId(void)
{
	printf("--------------- EAIF ID LIST ---------------\n");
	printf("cat\tid\ttype\tname\n");
#define EXPORT_DUMP(name, type) printf("%d\t%d\t0x%02X\t%s\n", FLAG_CAT_EAIF, EAIF_ID_##name, EAIF_TYPE_##name, #name);

	EXPORT_EAIF_DUMP_ARRAY

#undef EXPORT_DUMP

	printf("\n");
}

void DUMP_list(void)
{
	dumpVftrGroupId();
	dumpInfGroupId();
	dumpVftrStructId();
	dumpInfStructId();
}
