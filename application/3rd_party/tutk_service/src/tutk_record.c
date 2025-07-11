#include <netinet/in.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/vfs.h>
#include <linux/stat.h>
#define gettid() syscall(__NR_gettid)
#define _GNU_SOURCE
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <utime.h>
#include <sys/types.h>
#include <dirent.h>

#include "mpi_base_types.h"
#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_osd.h"
#include "mpi_iva.h"

#include "tutk_aac.h"
#include "tutk_audio.h"

#include "tutk_record.h"
#include "tutk_sdmonit.h"
#include "mp4v2/mp4v2.h"
#include "extract.h"

#include "tutk_linked_list.h"
#include "tutk_querylist.h"
#include "tutk_define.h"
#include "tutk_bubble_sort.h"

#include "log_define.h"

extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];

/* TUTK Notification */
extern int gTime_;
extern int n_time_;
extern int gNotify;
extern int gDelayInterval;

MPI_BCHN g_bchn;
static int run_flag = 0;
int is_first_I = 0;
pthread_t check_sd_pid;
pthread_t record_tid;
const char g_device[] = "default"; /* sound device */
List *clip_list = NULL;

static unsigned char sps[12] = { 0x67, 0x64, 0x0, 0x28, 0xac, 0x1b, 0x1a, 0x80, 0x78, 0x02, 0x27, 0xe5 };
static unsigned char pps[4] = { 0x68, 0xee, 0x3c, 0xb0 };

static audio_ctrl g_capture_ctrl = {
	.channels = 1,
	.rate = 8000,
	.device = g_device,
	.params = NULL,
	.stream = SND_PCM_STREAM_CAPTURE,
	.format = SND_PCM_FORMAT_S16_LE,
	.frames = 1024,
	.play_bmp = 0,
	.volume = AUDIO_DEFAULT_INPUT_VOLUME,
};

static int aux_set_pcm_codec(codec_mode_t mode)
{
	const char *devicename = "default";
	snd_hwdep_t *hwdep;
	int err;
	int codec_mode = mode;

	tutkservice_log_info("codec_mode = %d\n", codec_mode);

	if ((err = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		tutkservice_log_err("hwdep interface open error: %s \n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_ENC_MODE, &codec_mode)) < 0) {
		tutkservice_log_err("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_ENC_MODE, &codec_mode)) < 0) {
		tutkservice_log_err("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	snd_hwdep_close(hwdep);

	return 0;
}

static int aux_pcm_init(unsigned int codec, audio_ctrl *p_audio_ctrl)
{
	int err = 0;

	/* Open PCM device for playback. */
	err = snd_pcm_open(&p_audio_ctrl->handle, p_audio_ctrl->device, p_audio_ctrl->stream, 0);
	if (err < 0) {
		tutkservice_log_err("unable to open pcm device: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_malloc(&(p_audio_ctrl->params));

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(p_audio_ctrl->handle, p_audio_ctrl->params);

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(p_audio_ctrl->handle, p_audio_ctrl->params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->format);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->channels);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_channels:%s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->rate, 0);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->frames,
	                                             0);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(p_audio_ctrl->handle, p_audio_ctrl->params);
	if (err < 0) {
		tutkservice_log_err("unable to set hw parameters: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	aux_set_pcm_codec(codec);
	return err;
}

static int __check_sdCard_avail_space(unsigned long newClipDate)
{
	int ret = 0;
	struct statfs diskInfo;
	memset(&diskInfo, 0, sizeof(diskInfo));
	if (statfs("/mnt/sdcard", &diskInfo) == -1) {
		tutkservice_log_err("failed to get sdcard stat\n");
		return -1;
	}
	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long availDisk = diskInfo.f_bavail * blocksize;
	tutkservice_log_info("availed sd: %llu MB\n", availDisk >> 20);

	while ((availDisk >> 20) <= SD_MIN_AVAIL) {
		KeyPair rm_key;
		memset(&rm_key, 0, sizeof(rm_key));

		list_pop(clip_list, &rm_key);
		if (rm_key.key <= 0) {
			tutkservice_log_err("No available space, but have no old mp4\n");
			break;
		}

		char cmd[128] = { 0 };
		char cmd1[128] = { 0 };
		char cmdToday[128] = { 0 };
		/*get localtime*/
		int jetLag = 0;
		FILE *fp;
		char buffer[80];

		fp = popen("cat /etc/localtime", "r");
		fgets(buffer, sizeof(buffer), fp);
		sscanf(buffer, "UTC-%d", &jetLag);
		ret = pclose(fp);
		if (ret == -1) {
			tutkservice_log_err("failed to close /etc/localtime\n");
		}

		unsigned long save_local_time = rm_key.key + jetLag * 60 * 60;
		struct tm tm = *localtime((time_t *)(&(save_local_time)));
		sprintf(cmd, "%s%d-%02d-%02d/", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		sprintf(cmd, "%s%lu.mp4", cmd, rm_key.key);
		tutkservice_log_info("TRY del %s\n", cmd);
		ret = remove(cmd);
		if (ret == -1) {
			if (gNotify) {
				nebula_device_push_notification(NULL, 304);
			}
			tutkservice_log_err("failed to delete %s, %s\n", cmd, strerror(errno));
		}

		save_local_time = clip_list->head->data.key + jetLag * 60 * 60;
		tm = *localtime((time_t *)&(save_local_time));
		sprintf(cmd1, "%s%d-%02d-%02d/", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

		save_local_time = newClipDate + jetLag * 60 * 60;
		tm = *localtime((time_t *)&(save_local_time));
		sprintf(cmdToday, "%s%d-%02d-%02d/", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

		if (strncmp(cmd, cmdToday, 30) == 0) {
			tutkservice_log_err("oldest file in today, only del oldest file\n");
		} else {
			while ((strncmp(cmd, cmd1, 30) == 0) && (clip_list->Keynum >= 1)) {
				list_pop(clip_list, &rm_key);
				sprintf(cmd1, "%s%lu.mp4", cmd1, rm_key.key);
				tutkservice_log_info("rm %s, remain clip:%d \n", cmd1, clip_list->Keynum);

				ret = remove(cmd1);
				if (ret == -1) {
					if (gNotify) {
						nebula_device_push_notification(NULL, 304);
					}
					tutkservice_log_err("failed to delete %s, %s\n", cmd1, strerror(errno));
				}

				if (clip_list->Keynum == 0) {
					break;
				}

				save_local_time = clip_list->head->data.key + jetLag * 60 * 60;
				tm = *localtime((time_t *)&(save_local_time));
				sprintf(cmd1, "%s%d-%02d-%02d/", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1,
				        tm.tm_mday);
			}
			tutkservice_log_info("del files in %s\n", cmd1);
		}

		save_local_time = rm_key.key + jetLag * 60 * 60;
		tm = *localtime((time_t *)&(save_local_time));
		sprintf(cmd, "%s%d-%02d-%02d", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		ret = remove(cmd);
		if (ret == -1) {
			if (gNotify) {
				nebula_device_push_notification(NULL, 304);
			}
			tutkservice_log_err("failed to delete %s, %s\n", cmd, strerror(errno));
		}

		if (clip_list->Keynum >= 1) {
			tutkservice_log_info("head %lu \n", clip_list->head->data.key);
		}
		tutkservice_log_info("sd remain:%llu, del:%s\n", availDisk >> 20, cmd);
		TUTK_queryListRefresh();

		statfs("/mnt/sdcard", &diskInfo);
		blocksize = diskInfo.f_bsize;
		availDisk = diskInfo.f_bavail * blocksize;
	}

	return 0;
}

static int __save_mp4_bank_to_file(unsigned long clip_name)
{
	int ret = 0;
	char date_path[64] = { 0 };
	/*get local time*/
	int jetLag = 0;
	FILE *fp;
	char buffer[80];

	fp = popen("cat /etc/localtime", "r");
	fgets(buffer, sizeof(buffer), fp);
	sscanf(buffer, "UTC-%d", &jetLag);
	ret = pclose(fp);
	if (ret == -1) {
		tutkservice_log_err("failed to close /etc/localtime\n");
	}

	unsigned long save_local_time = clip_name + jetLag * 60 * 60;
	struct tm tm = *localtime((time_t *)&save_local_time);
	sprintf(&date_path[0], "%s%d-%02d-%02d", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	if (access(date_path, F_OK) == -1) {
		mkdir(date_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	char cmd[128] = { 0 };
	sprintf(&cmd[0], "mv %s %s/%lu.mp4", TMP_MP4_FILE_0, date_path, clip_name);

	tutkservice_log_info("save to %s\n", cmd);

	ret = system(cmd);
	if (ret == -1) {
		tutkservice_log_err("failed to mv clip %lu\n", clip_name);
		if (gNotify) {
			nebula_device_push_notification(NULL, 304);
		}

		return -1;
	}

	/*modify create time to local time*/
	time_t utc = 0;
	struct utimbuf m_time;
	memset(&m_time, 0, sizeof(m_time));
	utc = (time_t)clip_name;

	fp = popen("cat /etc/localtime", "r");
	fgets(buffer, sizeof(buffer), fp);
	sscanf(buffer, "UTC-%d", &jetLag);
	ret = pclose(fp);
	if (ret == -1) {
		tutkservice_log_err("failed to close /etc/localtime\n");
	}
	tutkservice_log_info("jet lag: %d, offset: %d\n", jetLag, jetLag * 60 * 60);

	m_time.actime = utc + jetLag * 60 * 60; /* 3600s = 1hr*/
	m_time.modtime = m_time.actime;

	/*modify mp4 time*/
	char fileName[64];
	memset(&fileName[0], 0, sizeof(fileName));
	sprintf(&fileName[0], "%s/%lu.mp4", date_path, clip_name);
	ret = utime(&fileName[0], &m_time);
	if (ret == -1) {
		tutkservice_log_err("failed to change file time: %s", strerror(errno));
	}
	/*modify reocrd path time*/
	char filePath[64] = { 0 };
	memset(&filePath[0], 0, sizeof(filePath));
	sprintf(&filePath[0], "%s", date_path);
	utime(&filePath[0], &m_time);
	if (ret == -1) {
		tutkservice_log_err("failed to change path time: %s", strerror(errno));
	}

	TUTK_queryListRefresh();
	tutkservice_log_info("add query time %s %s\n", fileName, filePath);

	KeyPair add_key = { clip_name, NULL, 0 };
	if (clip_list != NULL) {
		list_add(&add_key, clip_list);
	} else {
		tutkservice_log_err("failed to add key, clip_list not exist\n");
	}
	tutkservice_log_debug("jchange time %s %s\n", fileName, filePath);
	return 0;
}

void *run_record_mp4(void *arg)
{
	(void)(arg);

	INT32 ret = MPI_FAILURE;
	MPI_BCHN bchn = g_bchn;
	MPI_STREAM_PARAMS_S params;
	memset(&params, 0, sizeof(params));
	int gop_cnt = 0;
	int h264_buf_len;
	int pay_load_len = 0;

	int buf_len = g_capture_ctrl.frames * 2 * g_capture_ctrl.channels;
	char pcm_buf[buf_len];

	snd_pcm_nonblock(g_capture_ctrl.handle, 1);

	MP4FileHandle mp4_handler = MP4_INVALID_FILE_HANDLE;
	MP4TrackId video_track = MP4_INVALID_TRACK_ID;
	MP4TrackId audio_track = MP4_INVALID_TRACK_ID;

	while (run_flag) {
		if (gop_cnt == 0) {
			/*new mp4*/
			mp4_handler = MP4CreateEx(TMP_MP4_FILE_0, 0, 1, 1, 0, 0, 0, 0);
			MP4SetTimeScale(mp4_handler, VIDEO_TIME_SCALE);

			video_track = MP4AddH264VideoTrack(mp4_handler, VIDEO_TIME_SCALE, VIDEO_SAMPLE_DURATION, 1920,
			                                   1080,
			                                   sps[1], // AVCProfileIndication
			                                   sps[2], // profile_compat
			                                   sps[3], // AVCLevelIndication
			                                   3); // 4 bytes length before each NALU
			MP4AddH264SequenceParameterSet(mp4_handler, video_track, sps, SPS_START);
			MP4AddH264PictureParameterSet(mp4_handler, video_track, pps, PPS_START);
			MP4SetVideoProfileLevel(mp4_handler, 0x0a);

			audio_track = MP4AddAudioTrack(mp4_handler, g_capture_ctrl.rate, g_capture_ctrl.frames,
			                               MP4_PCM16_LITTLE_ENDIAN_AUDIO_TYPE);
			MP4SetAudioProfileLevel(mp4_handler, 0x2);
		}

		/*get video src*/
	get_video:
		/* get bit-stream from mpi */
		ret = MPI_getBitStream(bchn, &params, 10 /*ms*/);

		if ((is_first_I == 0) && (params.seg[0].type == MPI_FRAME_TYPE_SPS)) {
			is_first_I = 1;
			tutkservice_log_debug("find first I...\n");
		}

		if (is_first_I == 0) {
			tutkservice_log_debug("not find first I...\n");
			MPI_releaseBitStream(bchn, &params);
			MP4Close(mp4_handler, 0);
			goto get_video;
		}

		/* check frame type is 264 or 265 */
		if (params.seg_cnt < 1) {
			tutkservice_log_debug("Warning, return empty segment !\n");
			continue;
		}

		if (ret == -ETIMEDOUT || ret == -EAGAIN) {
			goto get_audio;
		} else if (ret == -ENODATA) {
			break;
		} else if (ret != MPI_SUCCESS) {
			tutkservice_log_debug("Failed to get bitstream with error, %d\n", ret);
			continue;
		}

		if (params.seg[0].type == MPI_FRAME_TYPE_SPS) {
			h264_buf_len = params.seg[2].size - SPS_LEN - PPS_LEN;
			pay_load_len = h264_buf_len - 4;
			params.seg[0].uaddr[0] = pay_load_len >> 24;
			params.seg[0].uaddr[1] = pay_load_len >> 16;
			params.seg[0].uaddr[2] = pay_load_len >> 8;
			params.seg[0].uaddr[3] = pay_load_len & 0xff;

			MP4WriteSample(mp4_handler, video_track, (uint8_t *)params.seg[0].uaddr, h264_buf_len,
			               VIDEO_SAMPLE_DURATION, 0, 1);
		} else {
			h264_buf_len = params.seg[0].size;
			pay_load_len = h264_buf_len - 4;
			params.seg[0].uaddr[0] = pay_load_len >> 24;
			params.seg[0].uaddr[1] = pay_load_len >> 16;
			params.seg[0].uaddr[2] = pay_load_len >> 8;
			params.seg[0].uaddr[3] = pay_load_len & 0xff;

			MP4WriteSample(mp4_handler, video_track, (uint8_t *)params.seg[0].uaddr, h264_buf_len,
			               VIDEO_SAMPLE_DURATION, 0, 0);
		}

		gop_cnt += 1;
		MPI_releaseBitStream(bchn, &params);
	get_audio:
		/*get audio src*/
		ret = snd_pcm_readi(g_capture_ctrl.handle, pcm_buf, g_capture_ctrl.frames);
		if (ret == -EPIPE) {
			snd_pcm_prepare(g_capture_ctrl.handle);
			tutkservice_log_err("-EPIPE\n");
			goto end_mp4;
		} else if (ret == -EAGAIN) {
			//usleep(100000);
			//fprintf(stdout, "-EAGAIN\n");
			goto end_mp4;
		} else if (ret < 0) {
			break;
		}

		ret = MP4WriteSample(mp4_handler, audio_track, (uint8_t *)pcm_buf, buf_len, 1000, 0, 1);

	end_mp4:
		/*end mp4*/
		if (gop_cnt == VIDEO_FPS * CLIP_SIZE /*SPS & PPS*/ || run_flag == 0 /*exit thread case*/) {
			MP4Close(mp4_handler, 0);
			gop_cnt = 0;

			/*rename mp4*/
			unsigned long clip_time = (unsigned long)time(NULL);
			__check_sdCard_avail_space(clip_time);
			__save_mp4_bank_to_file(clip_time);
		}
	}

	MPI_destroyBitStreamChn(bchn);

	ret = snd_pcm_drop(g_capture_ctrl.handle);
	if (ret < 0) {
		tutkservice_log_err("Failed snd_pcm_drop, %d\n", ret);
	}

	ret = snd_pcm_close(g_capture_ctrl.handle);
	if (ret < 0) {
		tutkservice_log_err("Failed snd_pcm_close, %d\n", ret);
	}

	tutkservice_log_info("exit record mp4 thread\n");

	return NULL;
}

static int TUTK_checkSDCardExist(void)
{
	if ((access("/dev/mmcblk0p1", F_OK) != -1) && (access("/dev/mmcblk0", F_OK) != -1)) {
		tutkservice_log_info("find sdcard\n");
		if (sd_format != SD_FORMAT_VFAT) {
			return -1;
		}
		return 0;
	} else {
		tutkservice_log_err("not found sdcard\n");
		return -1;
	}
}

static void saveOldDir(char *dir, int depth)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	memset(&statbuf, 0, sizeof(statbuf));
	struct tm tm;
	char cmd[128];

	if ((dp = opendir(dir)) == NULL) {
		tutkservice_log_err("Can`t open directory %s/n", dir);
		return;
	}

	chdir(dir);
	while ((entry = readdir(dp)) != NULL) {
		lstat(entry->d_name, &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			saveOldDir(entry->d_name, depth + 4);
		} else {
			if (strstr(entry->d_name, ".mp4") != 0) {
				unsigned long clip_time_in_name;
				sscanf(entry->d_name, "%lu.mp4", &clip_time_in_name);
				KeyPair add_key = { clip_time_in_name, NULL, 0 };
				tm = *localtime((time_t *)&(add_key.key));
				sprintf(cmd, "%s%d-%02d-%02d/", MP4_SAVE_PATH, tm.tm_year + 1900, tm.tm_mon + 1,
				        tm.tm_mday);
				tutkservice_log_debug("save %lu:%s\n", add_key.key, cmd);
				list_add(&add_key, clip_list);
			}
		}
	}
	chdir("..");
	closedir(dp);

	bubbleSort(clip_list->head);
	tutkservice_log_info("Total clip %d\n", clip_list->Keynum);
}

static int _check_sdcard_space_at_begin()
{
	struct statfs diskInfo;

	if (statfs("/mnt/sdcard", &diskInfo) == -1) {
		tutkservice_log_err("failed to get sdcard stat\n");
		if (gNotify) {
			nebula_device_push_notification(NULL, 304);
		}

		return -1;
	}

	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long availDisk = diskInfo.f_bavail * blocksize;
	tutkservice_log_info("availed sd: %llu MB\n", availDisk >> 20);

	if ((availDisk >> 20) <= (SD_MIN_AVAIL)) {
		tutkservice_log_err("No available space before record start\n");
		if (gNotify) {
			nebula_device_push_notification(NULL, 305);
		}

		return -1;
	}

	return 0;
}

void *run_check_sdcard_for_init_step(void *arg)
{
	(void)(arg);
	pthread_detach(pthread_self());

	int ret = 0;
	ret = TUTK_checkSDCardExist();
	ret = _check_sdcard_space_at_begin();
	if (ret == -1) {
		return (void *)-1;
	}

	if ((access(MP4_SAVE_PATH, F_OK) != -1)) {
		tutkservice_log_info("find sdcard\n");
	} else {
		mkdir(MP4_SAVE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	MPI_ECHN chn_idx = MPI_ENC_CHN(0);
	ret = MPI_FAILURE;

	MPI_SYS_init();

	g_bchn = MPI_createBitStreamChn(chn_idx);
	tutkservice_log_info("bchn.value = %08X\n", g_bchn.value);

	/*audio init*/

	ret = aux_pcm_init(RAW, &g_capture_ctrl);

	/* create file fifo list */
	clip_list = list_create();
	if (!clip_list) {
		tutkservice_log_err("failed to create list\n");
		goto snd_err;
	}

	remove(TMP_MP4_FILE_0);

	chdir("..");
	saveOldDir(MP4_SAVE_PATH, 0);
	chdir("/");

	tutkservice_log_info("ready save old event record\n");

	is_first_I = 0;

	run_flag = 1;
	if (pthread_create(&record_tid, NULL, run_record_mp4, (void *)NULL)) {
		tutkservice_log_err("Failed to create thread of channel!\n");
		MPI_destroyBitStreamChn(g_bchn);
		goto snd_err;
	}

	return (void *)0;
snd_err:
	snd_pcm_close(g_capture_ctrl.handle);
	return (void *)-1;
}

#define ENABLE 1
#define DISABLE 0

static int OpenRecordFile(AV_Client *info)
{
	/*real demuxer*/
	AVC_getSampleNum(info->filename_with_path, &info->playback_v_sample);
	printf("vsample num = %d\r\n", info->playback_v_sample);

	AAC_getSampleNum(info->filename_with_path, &info->playback_a_sample);
	printf("asample num = %d\r\n", info->playback_a_sample);

	info->aIdx = 0;
	info->vIdx = 0;

	return 0;
}

static int CloseRecordFile(AV_Client *info)
{
	/*real release*/
	printf("[%s:%d] release Demuxer\n", __func__, __LINE__);
	info->playback_a_sample = 0;
	info->playback_v_sample = 0;
	info->aIdx = 0;
	info->vIdx = 0;
	int ret;
	ret = AVC_releaseExtractor();
	if (ret != 0) {
		fprintf(stderr, "failed to release AVC demuxer\r\n");
	}
	ret = AAC_releaseExtractor();
	if (ret != 0) {
		fprintf(stderr, "failed to release AAC demuxer\r\n");
	}

	return 0;
}

static int GetClientPlaybackMode(int sid)
{
	AV_Client *p = &gClientInfo[sid];
	return p->enable_record_video;
}

void *ThreadRecordFileData(void *arg)
{
	int sid = *((int *)arg);
	free(arg);
	printf("%s start SID[%d]\n", __func__, sid);

	int ret = 0;
	int loopsleep = 0;

	struct sched_param tparam;
	pthread_t tid = pthread_self();
	tparam.sched_priority = 51;
	ret = pthread_setschedparam(tid, SCHED_RR, &tparam);
	if (ret) {
		fprintf(stderr, "Failed to set Vplayback policy to SCHED_RR\n");
	}

	AV_Client *info = &gClientInfo[sid];

	info->playback_v_sample = 9999; //magic number for checking ThreadRecordFileData is starting.
	RegEditClientPlaybackMode(sid, PLAYBACK_START);
	int av_index = StartAvServer(sid, AV_PLAYBACK_CHANNEL, 30, 1024, NULL);
	if (av_index < 0) {
		printf("StartAvServer error!! avIndex[%d]\n", av_index);
		goto THREAD_RECORD_EXIT;
	} else {
		printf("StartAvServer ok!! oriIdx[%d] avIndex[%d]\n", info->av_index, av_index);
		info->av_index = av_index;
	}

	ret = OpenRecordFile(info);
	if (ret != 0) {
		printf("Can't open record file correctly\n");
		goto THREAD_RECORD_EXIT;
	}

	int end_of_file_or_file_broken = 0;
	int cache_data = 0;

	/*video buf & tag*/
	char vdata_buf[RECORD_BUF_SIZE] = { 0 };
	int vdata_size = 0;
	int vsleep_ms = 67;
	int vStartSendTimeMs = GetTimeStampMs();

	FRAMEINFO_t vframe_info;
	memset(&vframe_info, 0, sizeof(FRAMEINFO_t));
	vframe_info.codec_id = MEDIA_CODEC_VIDEO_H264;

	/*audio buf & tag*/
	char adata_buf[AUDIO_BUF_SIZE] = { 0 };
	int adata_size = 0;
	int asleep_ms = 128;
	int aStartSendTimeMs = GetTimeStampMs();

	FRAMEINFO_t aframe_info;
	memset(&aframe_info, 0, sizeof(FRAMEINFO_t));
	aframe_info.codec_id = MEDIA_CODEC_AUDIO_AAC_ADTS;
	aframe_info.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;

	while (GetClientPlaybackMode(sid) != PLAYBACK_STOP) {
		if (GetClientPlaybackMode(sid) == PLAYBACK_PAUSE || 1 == end_of_file_or_file_broken) {
			usleep(500 * 1000);
			continue;
		}
#if 0
		if (info->aIdx <= 8) {
			goto PLAYBACK_AUDIO;
		}
#endif
		/*read and send video*/
		if (cache_data == 0) {
			if (info->vIdx == info->playback_v_sample) {
				printf("last video frame\n");
				vframe_info.tags = 1;
				end_of_file_or_file_broken = 1;
				continue;
			} else if (info->vIdx > info->playback_v_sample) {
				fprintf(stderr, "has ended H264 demux\r\n");
				end_of_file_or_file_broken = 1;
				continue;
			}

			ret = AVC_getFrameData(info->filename_with_path, info->vIdx, &vdata_buf[0], &vdata_size);
			if (ret == -1) {
				fprintf(stderr, "failed in V demuxer\r\n");
				continue;
			}
			vStartSendTimeMs += vsleep_ms;
			vframe_info.timestamp = vStartSendTimeMs;

			if (vdata_size < 0) {
				fprintf(stderr, "vdata size < 0, end_of_file_or_file_broken\r\n");
				end_of_file_or_file_broken = 1;
				continue;
			} else if (vdata_size == 0) {
				fprintf(stderr, "vdata size == 0, end_of_file_or_file_broken\r\n");
				continue;
			}
		}

		if (info->vIdx % 15 == 0) {
			vframe_info.flags = IPC_FRAME_FLAG_IFRAME;
		} else {
			vframe_info.flags = IPC_FRAME_FLAG_PBFRAME;
		}

		//printf("->vidx: %d\r\n", info->vIdx);
		ret = avSendFrameData(av_index, vdata_buf, vdata_size, (void *)&vframe_info, sizeof(FRAMEINFO_t));

		if (ret == AV_ER_EXCEED_MAX_SIZE) {
			fprintf(stderr, "[SendRecordData]AV_ER_EXCEED_MAX_SIZE\r\n");
			// AV resend buffer is full.
			// Need to keep and re-send this frame until avSendFrameData or avSendAudioData retrun AV_ER_NoERROR
			cache_data = 1;
			usleep(5000);
			continue;
		} else if (ret < 0) {
			printf("[SendRecordData] avIndex[%d] error[%d]\n", av_index, ret);
			break;
		} else {
			cache_data = 0;
			info->vIdx++;
		}

		if (((info->vIdx - 1) % 15) % 2 == 1) {
			continue;
		}
#if 0 /** v1.0.0 playback no audio*/
	PLAYBACK_AUDIO:
		/*read and send audio*/
		if (cache_data == 0) {
			if (info->aIdx == info->playback_a_sample - 1) {
				printf("last audio frame\n");
				aframe_info.tags = 1;
				continue;
			} else if (info->aIdx > info->playback_a_sample) {
				fprintf(stderr, "has ended AAC demux\r\n");
				continue;
			}

			ret = AAC_getFrameData(info->aIdx, &adata_buf[0], &adata_size);
			if (ret != 1) {
				fprintf(stderr, "failed in A demuxer\r\n");
				continue;
			}
			aStartSendTimeMs += asleep_ms;
			aframe_info.timestamp = aStartSendTimeMs;

			if (adata_size < 0) {
				fprintf(stderr, "adata size < 0, end_of_file_or_file_broken\r\n");
				end_of_file_or_file_broken = 1;
				continue;
			} else if (adata_size == 0) {
				fprintf(stderr, "adata size == 0, end_of_file_or_file_broken\r\n");
				continue;
			}
		}
		//printf("aidx: %d\r\n", info->aIdx);
		ret = avSendAudioData(av_index, adata_buf, adata_size, (void *)&aframe_info, sizeof(FRAMEINFO_t));

		if (ret == AV_ER_EXCEED_MAX_SIZE) {
			fprintf(stderr, "[SendRecordData]AV_ER_EXCEED_MAX_SIZE\r\n");
			// AV resend buffer is full.
			// Need to keep and re-send this frame until avSendFrameData or avSendAudioData retrun AV_ER_NoERROR
			cache_data = 1;
			usleep(5000);
			continue;
		} else if (ret < 0) {
			fprintf(stderr, "[SendRecordData] avIndex[%d] error[%d]\n", av_index, ret);
			break;
		} else {
			cache_data = 0;
			info->aIdx++;
		}
		loopsleep = vStartSendTimeMs - GetTimeStampMs();
		if (loopsleep > 0) {
			usleep(loopsleep * 1000);
		} else {
			fprintf(stderr, "Send time > v interval, maybe delay:%d\r\n", loopsleep);
		}
#endif
	}

THREAD_RECORD_EXIT:
	pthread_rwlock_wrlock(&info->sLock);
	if (av_index >= 0) {
		CloseRecordFile(info);
	}
	pthread_rwlock_unlock(&info->sLock);

	if (av_index >= 0) {
		avServStop(av_index);
		printf("[%s:%d] avServStop playback avIndex[%d]\n", __func__, __LINE__, av_index);
		info->av_index = 0; //clear setting
	} else if (info->playback_v_sample == 9999) {
		info->playback_v_sample = 0;
		printf("reset playback_v_sample as 0\n");
	}

	printf("%s: exit!! SID[%d]\n", __func__, sid);
	RegEditClientPlaybackMode(sid, PLAYBACK_STOP);
	pthread_exit(0);
}

int TUTK_record_init(void)
{
	if (pthread_create(&check_sd_pid, NULL, run_check_sdcard_for_init_step, (void *)NULL)) {
		tutkservice_log_err("Failed to create thread of channel!\n");
		MPI_destroyBitStreamChn(g_bchn);
		goto err;
	}

	return 0;
err:

	return -1;
}

int TUTK_record_stop(void)
{
	tutkservice_log_info("start to end record");

	int ret = 0;
	if (run_flag == 1) {
		run_flag = 0;
	}

	tutkservice_log_info("end record");

	if (record_tid != 0) {
		ret = pthread_join(record_tid, NULL);
		if (ret < 0) {
			tutkservice_log_err("failed to join record_tid %d\n", ret);
		}
	}

	if (clip_list != NULL) {
		list_destroy(clip_list);
		clip_list = NULL;
	}

	tutkservice_log_info("record stop\n");

	return 0;
}

int HandlePlaybackControl(int sid, int ctrl_value, const char *file_name)
{
	AV_Client *p = &gClientInfo[sid];
	int ret = 0;

	if (ctrl_value == PLAYBACK_START) {
		if (GetClientPlaybackMode(sid) == PLAYBACK_START) {
			printf("Playback already started!!\n");
			return -1;
		} else if (GetClientPlaybackMode(sid) == PLAYBACK_PAUSE) {
			RegEditClientPlaybackMode(sid, PLAYBACK_START);
			return 0;
		} else if ((p->playback_v_sample == 9999) || (p->playback_v_sample > 0) || (p->av_index != 0)) {
			printf("ThreadRecordFileData is till on-going v sample %d av_index %d!!\n",
			       p->playback_v_sample, p->av_index);
			return -1;
		} else {
			printf("Playback is going to be started!!\n");
		}
		// check if the record file exist or not
		sprintf(p->filename_with_path, "%s", file_name);
		printf("file name = %s\n", p->filename_with_path);

		pthread_t thread_id;
		int *tmp_sid = (int *)malloc(sizeof(int));
		*tmp_sid = sid;

		int waitTime = 0;
		while (IOTC_Session_Channel_Check_ON_OFF(sid, AV_PLAYBACK_CHANNEL) == 1) {
			sleep(1);
			if (waitTime == 10)
				break;
			waitTime++;
		}

		if ((ret = pthread_create(&thread_id, NULL, &ThreadRecordFileData, (void *)tmp_sid)) < 0) {
			printf("%s() pthread_create ThreadRecordFileData ret[%d]\n", __func__, ret);
			free(tmp_sid);
			return -1;
		} else {
			pthread_detach(thread_id);
		}

		/* Create Audio Thread Here */

	} else if (ctrl_value == PLAYBACK_STOP) {
		RegEditClientPlaybackMode(sid, PLAYBACK_STOP);
	} else if (ctrl_value == PLAYBACK_PAUSE) {
		RegEditClientPlaybackMode(sid, PLAYBACK_PAUSE);
	}

	return 0;
}

void GetRecordFileList(unsigned int start_time, unsigned int end_time, const char *event_type,
                       NebulaJsonObject **response)
{
	// TODO: this is fake data
	// In this sample we ignore the search criteria and response the hard coded file list.
	//char json[] = "{\"value\": [{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames\", \"fileSize\": 6661927}]}";

	(void)start_time;
	(void)end_time;
	(void)event_type;
	(void)response;

	char json[] =
	        "{\"value\": "
	        "["
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames1\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames2\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames3\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames4\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames5\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames6\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames7\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames8\", \"fileSize\": 6661927},"
	        "{\"eventType\": \"motion\",\"timestamp\": 1559754000,\"duration\": 300,\"fileName\": \"frames9\", \"fileSize\": 6661927}"
	        "]}";

	Nebula_Json_Obj_Create_From_String(json, response);
}

void RegEditClientPlaybackMode(int sid, int playback_mode)
{
	AV_Client *p = &gClientInfo[sid];
	p->enable_record_video = playback_mode;
}
