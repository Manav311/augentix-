#include "mp4_dumper.h"

#include <sys/stat.h>
#include <sys/vfs.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

#ifdef MP4_ENABLE
#include <alsa/asoundlib.h>
#include <alsa/hwdep.h>
#include <alsa/error.h>
#include <pcm_interfaces.h>

#include "aac.h"

#include "mp4v2/mp4v2.h"

#include "utlist.h"

#define __DEBUG__ 1

#if (__DEBUG__)
#define DEBUG(fmt, args...) printf("[DEBUG] " fmt, ##args)
#else
#define DEBUG(fmt, args...)
#endif

#define DESCRIPTOR_MAX 32

#define VIDEO_TIME_SCALE (90000)

static char *const DEV_PATH = "/dev/";
static const char *g_device[] = { "default", "hw:0,0" }; /* sound device */
static unsigned char aac_dec_conf[2] = { 0x15, 0x88 };

typedef struct file_item {
	char *path;
	uint32_t mbytes;
	struct file_item *next;
} FileItem;

typedef struct {
	snd_pcm_t *capture;
	char *buf;
	int buf_len;
	int frames;
	int bytes_per_sample;
	int channel;
	int rate;
	int timestamp_enable;
} AudioStreamInfo;

typedef struct mp4_dumper {
	BitStreamSubscriber base_part;
	char previous_output_path[PATH_MAX];
	char dumping_file_path[PATH_MAX];
	const char output_path[PATH_MAX];
	char descriptor[DESCRIPTOR_MAX];
	MP4FileHandle mp4_hd;
	MP4TrackId video_t;
	MP4TrackId audio_t;
	/* max_frame_count=0 means infinity */
	uint32_t max_frame_count;
	uint32_t dumped_frame_count;
	bool dumped_to_black_hole;
	bool repeat;
	/* repeat mode only */
	FileItem *dumped_files;
	int32_t reserved_mbytes; /* negative means percentage */
	int32_t recycle_mbytes; /* negative means percentage */
	int max_dumped_files; /* add to new contructor*/
	/*recent file number in some LL*/
	uint16_t width;
	uint16_t height;
	uint8_t fps;
	ty_media_aac_handle_s hdl;
	AudioStreamInfo audio;
} Mp4Dumper;

static int agtx_pcm_init(snd_pcm_t **pcm_handle, const char *device, snd_pcm_stream_t stream, snd_pcm_format_t format,
                         snd_pcm_uframes_t frame, unsigned int rate, unsigned int channels)
{
	int ret;
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;

	/* return error if already initialized */
	ret = snd_pcm_open(pcm_handle, device, stream, SND_PCM_NONBLOCK);
	if (ret < 0) {
		fprintf(stderr, "fail pcm open: %s\n", snd_strerror(ret));
		*pcm_handle = NULL;
		return -1;
	}

	/* configure alsa devicei, including layout, format, channels, sample rate, and periords */
	ret = snd_pcm_hw_params_malloc(&params);
	if (ret != 0) {
		fprintf(stderr, "Failed to Pcm hw param malloc\n");
		return -1;
	}

	ret = snd_pcm_hw_params_any(*pcm_handle, params);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_access(*pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_format(*pcm_handle, params, format);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_channels(*pcm_handle, params, channels);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_rate_near(*pcm_handle, params, &rate, 0);
	if (ret != 0) {
		goto handle_error;
	}

	/* Thit will only follow /etc/asound.conf */
	ret = snd_pcm_hw_params_set_period_size_near(*pcm_handle, params, &frame, 0);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params(*pcm_handle, params);
	if (ret != 0) {
		goto handle_error;
	}

	snd_pcm_hw_params_free(params);

	snd_pcm_sw_params_alloca(&swparams);
	ret = snd_pcm_sw_params_current(*pcm_handle, swparams);
	if (ret) {
		printf("snd_pcm_sw_params_current %s\n", snd_strerror(ret));
	}

	ret = snd_pcm_sw_params_set_tstamp_mode(*pcm_handle, swparams, SND_PCM_TSTAMP_ENABLE);
	if (ret) {
		printf("set_tstamp_mode %s\n", snd_strerror(ret));
	}

	ret = snd_pcm_sw_params_set_tstamp_type(*pcm_handle, swparams, SND_PCM_TSTAMP_TYPE_MONOTONIC);
	if (ret) {
		printf("set_tstamp_type %s\n", snd_strerror(ret));
	}

	ret = snd_pcm_sw_params(*pcm_handle, swparams);
	if (ret) {
		printf("snd_pcm_sw_params %s\n", snd_strerror(ret));
	}

	return 0;

handle_error:
	snd_pcm_hw_params_free(params);
	perror("configure alsa device failture");
	snd_pcm_close(*pcm_handle);
	*pcm_handle = NULL;
	return -1;
}

static int audio_deinit(Mp4Dumper *dumper)
{
	printf("PCM Bit stream system exited.\n");

	int ret = 0;
	if (dumper->audio.capture != NULL) {
		ret = snd_pcm_drop(dumper->audio.capture);
		if (ret < 0) {
			fprintf(stderr, "Failed snd_pcm_drop, %d\n", ret);
		}
		ret = snd_pcm_close(dumper->audio.capture);
		if (ret < 0) {
			fprintf(stderr, "Failed snd_pcm_close, %d\n", ret);
		}
	}

	dumper->audio.capture = NULL;

	return 0;
}

static int audio_init(Mp4Dumper *dumper)
{
	snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	int dev_id = 0;
	int ret = 0;

	ret = agtx_pcm_init(&dumper->audio.capture, g_device[dev_id], stream, format, dumper->audio.frames,
	                    dumper->audio.rate, dumper->audio.channel);
	if (ret != 0) {
		fprintf(stderr, "Failed to init pcm\n");
		dumper->audio.capture = NULL;
		return ret;
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

static bool MP4DUMPER_interpretLevels(Mp4Dumper *dumper, uint32_t *reserved_mbytes, uint32_t *recycle_mbytes,
                                      uint32_t *available_mbytes, uint32_t *total_mbytes)
{
	uint64_t available, total;
	if (detectAvailableSpace(dumper->dumping_file_path, &available, &total)) {
		available >>= 20; /* to MBytes */
		total >>= 20; /* to MBytes */
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

static void MP4DUMPER_purgeDumpedItems(Mp4Dumper *dumper, uint32_t mbytes)
{
	FileItem *item = NULL;
	FileItem *tmp = NULL;
	uint32_t purged_mbytes = 0;
	LL_FOREACH_SAFE(dumper->dumped_files, item, tmp)
	{
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

static bool MP4DUMPER_prepareNextFile(Mp4Dumper *dumper) // -> successful
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
		if (((access("/dev/mmcblk0p1", F_OK) != 0) || (access("/dev/mmcblk0", F_OK) != 0)) &&
		    strstr(dumper->dumping_file_path, "/mnt/sdcard") != NULL) {
			usleep(10000);
			continue;
		}

		dumper->mp4_hd = MP4CreateEx(dumper->dumping_file_path, 0, 1, 1, 0, 0, 0, 0);
		if (dumper->mp4_hd == MP4_INVALID_FILE_HANDLE) {
			usleep(10000);
			continue;
		}
		break;
	}

	MP4SetTimeScale(dumper->mp4_hd, VIDEO_TIME_SCALE);

	printf("%s: opening file %s ...\n", dumper->descriptor, dumper->dumping_file_path);

	return dumper->mp4_hd != NULL;
}

static uint32_t MP4DUMPER_closeDumpingFile(Mp4Dumper *dumper)
{
	if (dumper->mp4_hd != NULL) {
		MP4Close(dumper->mp4_hd, 0);
		dumper->mp4_hd = NULL;
		dumper->video_t = MP4_INVALID_TRACK_ID;
		dumper->audio_t = MP4_INVALID_TRACK_ID;
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

static bool MP4DUMPER_deliveryWillStart(void *context)
{
	Mp4Dumper *dumper = context;

	if (MP4DUMPER_prepareNextFile(dumper)) {
		if (dumper->repeat) {
			uint32_t reserved_mbytes, recycle_mbytes, total_mbytes;
			if (MP4DUMPER_interpretLevels(dumper, &reserved_mbytes, &recycle_mbytes, NULL, &total_mbytes)) {
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

static long timespecDiffinMillis(struct timespec start, struct timespec end)
{
	long diff_sec = end.tv_sec - start.tv_sec;
	long diff_nsec = end.tv_nsec - start.tv_nsec;

	long diff_millis = diff_sec * 1000 + diff_nsec / 1000000;

	return diff_millis;
}

static int getAudioVideoTimeDiff(const struct timespec *video_timestamp, snd_pcm_t *handle, int interval)
{
	snd_htimestamp_t audio_timestamp;
	snd_pcm_status_t *status;
	snd_pcm_status_alloca(&status);

	snd_pcm_audio_tstamp_config_t audio_tstamp_config_p;
	memset(&audio_tstamp_config_p, 0, sizeof(snd_pcm_audio_tstamp_config_t));
	audio_tstamp_config_p.type_requested = 0;
	audio_tstamp_config_p.report_delay = 0;

	snd_pcm_status_set_audio_htstamp_config(status, &audio_tstamp_config_p);
	int err = 0;
	if ((err = snd_pcm_status(handle, status)) < 0) {
		/*when alsa driver not ready*/
		printf("Stream status error: %s\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_status_get_htstamp(status, &audio_timestamp);
	long diff = timespecDiffinMillis(*video_timestamp, audio_timestamp);
	snd_pcm_status_free(status);

	if (diff < interval && diff > -1 * interval) {
		/*diff in 1 sample interval*/
		return 0;
	} else {
		/*when audio ts not sync video ts*/
		return -1;
	}
}

static bool MP4DUMPER_receiveFrame(void *context, const MPI_STREAM_PARAMS_V2_S *frame)
{
	Mp4Dumper *dumper = context;
	int ret = 0;
	static struct timespec last_ts = { .tv_sec = 0, .tv_nsec = 0 };
	INT32 sample_interval = dumper->audio.frames * 1000 / dumper->audio.rate;

	if (dumper->max_frame_count == 0 || dumper->repeat ||
	    dumper->dumped_frame_count < dumper->max_frame_count /*max frame in single file*/) {
		if (frame->seg[0].type == MPI_FRAME_TYPE_SPS &&
		    dumper->mp4_hd != NULL && dumper->video_t == MP4_INVALID_TRACK_ID) {
			/** MP4 add video track */
			dumper->video_t =
			        MP4AddH264VideoTrack(dumper->mp4_hd, VIDEO_TIME_SCALE, VIDEO_TIME_SCALE / dumper->fps,
			                             dumper->width, dumper->height,
			                             frame->seg[0].uaddr[4 + 1], //sps[1], // AVCProfileIndication
			                             frame->seg[0].uaddr[4 + 2], //sps[2], // profile_compat
			                             frame->seg[0].uaddr[4 + 3], //sps[3], // AVCLevelIndication
			                             3); // 4 bytes length before each NALU
			MP4AddH264SequenceParameterSet(dumper->mp4_hd, dumper->video_t, frame->seg[0].uaddr + 4,
			                               frame->seg[0].size - 4);
			MP4AddH264PictureParameterSet(dumper->mp4_hd, dumper->video_t, frame->seg[1].uaddr + 4,
			                              frame->seg[1].size - 4);
			MP4SetVideoProfileLevel(dumper->mp4_hd, 0x7f);

			/** MP4 add audio track */
			dumper->audio_t = MP4AddAudioTrack(dumper->mp4_hd, dumper->audio.rate, dumper->audio.frames,
			                                   MP4_MPEG4_AUDIO_TYPE);
			MP4SetAudioProfileLevel(dumper->mp4_hd, 0x2);
			MP4SetTrackESConfiguration(dumper->mp4_hd, dumper->audio_t, &aac_dec_conf[0], 2);
		} else if (dumper->video_t == MP4_INVALID_TRACK_ID) {
			return dumper->max_frame_count ? dumper->max_frame_count <= dumper->dumped_frame_count : false;
		}

		int seg_payload_size = 0; // include payload
		uint8_t h264_buf[dumper->width * dumper->height];
		uint8_t frame_start_seg = 0;
		if (frame->seg[0].type == MPI_FRAME_TYPE_SPS) {
			frame_start_seg = 2;
		}

		for (uint8_t i = frame_start_seg; i < frame->seg_cnt; i++) {
			memcpy(&h264_buf[seg_payload_size], frame->seg[i].uaddr, frame->seg[i].size);
			seg_payload_size += frame->seg[i].size;
		}

		/*switch 00 00 00 01 to payload*/
		h264_buf[0] = ((seg_payload_size - 4) >> 24) & 0xFF;
		h264_buf[1] = ((seg_payload_size - 4) >> 16) & 0xFF;
		h264_buf[2] = ((seg_payload_size - 4) >> 8) & 0xFF;
		h264_buf[3] = ((seg_payload_size - 4) >> 0) & 0xFF;

		if (dumper->video_t != MP4_INVALID_TRACK_ID) {
			ret = MP4WriteSample(dumper->mp4_hd, dumper->video_t, (const uint8_t *)&h264_buf[0],
			                     seg_payload_size, VIDEO_TIME_SCALE / dumper->fps, 0,
			                     frame->seg[0].type == MPI_FRAME_TYPE_SPS ? 1 : 0);
			if (ret != true) {
				printf("pack H264 to mp4 failed %d\n", ret);
				return ret;
			}
		}

		if (dumper->audio.capture == NULL) {
			ret = audio_init(dumper);
			if (ret != 0) {
				fprintf(stderr, "pcm init failed:%d\n", ret);
				dumper->audio.capture = NULL;
			}
		}

		ret = 1;
		long diff = timespecDiffinMillis(frame->timestamp, last_ts);

		if (getAudioVideoTimeDiff(&frame->timestamp, dumper->audio.capture, sample_interval) >= 0) {
			ret = snd_pcm_readi((snd_pcm_t *)dumper->audio.capture, dumper->audio.buf,
			                    dumper->audio.frames);
			if (ret == -EPIPE) {
				snd_pcm_prepare((snd_pcm_t *)dumper->audio.capture);
				//fprintf(stderr, "snd -EPIPE\n");
			} else if (ret == -EAGAIN) {
				//fprintf(stderr, "snd -EAGAIN\n");
			} else {
			}
		} else if (diff > sample_interval || diff < -1 * sample_interval) {
			/*fake pcm*/
			memset(dumper->audio.buf, 0x00, dumper->audio.buf_len);
		} else {
			ret = -1;
		}

		/*aac encoder*/
		if (ret > 0) {
			int aac_recv = dumper->audio.buf_len;
			ret = aac_encoder_data(&dumper->hdl, dumper->audio.buf, dumper->audio.buf_len,
			                       dumper->audio.frames, dumper->hdl.aac_buf, &aac_recv);
			if (ret != 0) {
				printf("encode AAC failed %d\n", ret);
			}

			if (dumper->audio_t == MP4_INVALID_TRACK_ID) {
				fprintf(stderr, "dumper->audio_t == MP4_INVALID_TRACK_ID Skip audio\n");
			} else if (ret == 0) {
				ret = MP4WriteSample(dumper->mp4_hd, dumper->audio_t,
				                     (const uint8_t *)&dumper->hdl.aac_buf[7], aac_recv - 7,
				                     dumper->audio.frames, 0, 1);
			}

			last_ts = frame->timestamp;
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
			uint32_t dumped_mbytes = MP4DUMPER_closeDumpingFile(dumper);
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
				if (MP4DUMPER_interpretLevels(dumper, &reserved_mbytes, &recycle_mbytes, &available,
				                              NULL)) {
					DEBUG("%s: storage available: %u MBytes.\n", dumper->descriptor, available);
					if (available < reserved_mbytes) {
						MP4DUMPER_purgeDumpedItems(dumper, recycle_mbytes);
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

			if (MP4DUMPER_prepareNextFile(dumper)) {
				dumper->dumped_frame_count = 0;
			} else {
				/* unable to prepare next file, we signal publisher to stop delivery */
				return true;
			}
		}
	}

	return dumper->max_frame_count ? dumper->max_frame_count <= dumper->dumped_frame_count : false;
}

static void MP4DUMPER_deliveryDidEnd(void *context)
{
	MP4DUMPER_closeDumpingFile(context);

	Mp4Dumper *dumper = context;

	aac_encoder_uninit(&dumper->hdl);

	if (dumper->audio.capture != NULL) {
		audio_deinit(dumper);
		dumper->audio.capture = NULL;
	}

	if (dumper->audio.buf != NULL) {
		free(dumper->audio.buf);
		dumper->audio.buf = NULL;
	}

	/* AAC encoder can reuse in diff mp4 ?*/

	printf("%s: dumped file list -----\n", dumper->descriptor);
	int i = 0;
	FileItem *item = NULL;
	FileItem *tmp = NULL;
	LL_FOREACH_SAFE(dumper->dumped_files, item, tmp)
	{
		++i;
		printf("%d: %s, %u MBytes.\n", i, item->path, item->mbytes);
		LL_DELETE(dumper->dumped_files, item);
		free(item);
	}
}

BitStreamSubscriber *newMp4Dumper(MPI_ECHN encoder_channel, const char *output_path, uint32_t frame_count, bool repeat,
                                  int32_t reservation_level, int32_t recycle_level, int max_dumped_files,
                                  uint16_t width, uint16_t height, uint8_t fps)
{
	Mp4Dumper *dumper = malloc(sizeof(*dumper));
	snprintf((char *)dumper->output_path, sizeof(dumper->output_path), "%s", output_path);
	dumper->max_frame_count = frame_count;
	dumper->mp4_hd = NULL;
	dumper->video_t = MP4_INVALID_TRACK_ID;
	dumper->audio_t = MP4_INVALID_TRACK_ID;
	dumper->audio.capture = NULL;
	dumper->repeat = repeat;
	dumper->dumped_files = NULL;
	dumper->reserved_mbytes = reservation_level;
	dumper->recycle_mbytes = recycle_level;
	dumper->max_dumped_files = max_dumped_files;
	dumper->width = width;
	dumper->height = height;
	dumper->fps = fps;

	/*audio config init*/
	unsigned int rate = 8000;
	int bytes_per_sample = 2;
	int channel = 1;

	snd_pcm_uframes_t frame = 1024;
	memset(&dumper->audio, 0x00, sizeof(dumper->audio));
	dumper->audio.capture = NULL;
	dumper->audio.buf_len = frame * bytes_per_sample * channel;
	dumper->audio.buf = malloc(dumper->audio.buf_len);
	if (dumper->audio.buf == NULL) {
		fprintf(stderr, "failed to alloc pcm buf\n");
		free(dumper);
		return NULL;
	}
	memset(dumper->audio.buf, 0x00, dumper->audio.buf_len);

	dumper->audio.frames = frame;
	dumper->audio.channel = channel;
	dumper->audio.bytes_per_sample = bytes_per_sample;
	dumper->audio.rate = rate;
	dumper->hdl.aac_buf = NULL;

	/* AAC encoder ready before ALSA driver*/
	/* AAC encoder can reuse in diff mp4 */
	int ret = aac_encoder_init(&dumper->hdl, dumper->audio.channel, dumper->audio.rate, dumper->audio.rate,
	                           dumper->audio.buf_len);
	if (ret != 0) {
		printf("init AAC encoder failed %d\n", ret);
		/*ignore*/
	}

	if (dumper->recycle_mbytes == 0) {
		dumper->recycle_mbytes = 1;
	}
	memset(dumper->previous_output_path, 0, sizeof(dumper->previous_output_path));
	snprintf(dumper->descriptor, sizeof(dumper->descriptor), "Mp4Dumper[%d]", encoder_channel.chn);
	if (!dumper->repeat || strncmp(dumper->output_path, DEV_PATH, strlen(DEV_PATH)) == 0) {
		dumper->dumped_to_black_hole = true;
		printf("%s: assume dumping frames to unlimited sink.\n", dumper->descriptor);
	} else {
		dumper->dumped_to_black_hole = false;
	}

	BitStreamSubscriber *base = &dumper->base_part;
	base->context = dumper;
	base->deliveryWillStart = MP4DUMPER_deliveryWillStart;
	base->receiveFrame = MP4DUMPER_receiveFrame;
	base->deliveryDidEnd = MP4DUMPER_deliveryDidEnd;
	return base;
}

#endif
