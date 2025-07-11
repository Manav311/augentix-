#ifndef FLV_MUXER_H_
#define FLV_MUXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "mpi_dip_sns.h"
#include "mpi_enc.h"
#include "agtx_types.h"

typedef struct {
	uint8_t configuration_version;
	uint8_t general_profile_space;
	uint8_t general_tier_flag;
	uint8_t general_profile_idc;
	uint32_t general_profile_compatibility_flags;
	uint64_t general_constraint_indicator_flags;
	uint8_t general_level_idc;
	uint16_t min_spatial_segmentation_idc;
	uint8_t parallelism_type;
	uint8_t chroma_format;
	uint8_t bitDepthLumaMinus8;
	uint8_t bitDepthChromaMinus8;
	uint16_t avg_framerate;
	uint8_t const_framerate;
	uint8_t num_temporal_layers;
	uint8_t temporal_id_nested;
	uint8_t length_size_minus_one;
	uint8_t num_of_arrays;
} HEVCDecoderConfigurationRecord;

#define MAX_SPATIAL_SEGMENTATION (4096)

typedef struct {
	bool has_video;
	bool has_audio;
	int chn_num;
	char audio_codec;
	MPI_VENC_TYPE_E video_codec;
	char video_profile;
	char video_level;
	int frame_len;
	int output_fd;
	bool (*fCheckCodecChange)(char /*chn_num*/);
	int (*fWriteFlv)(void *, uint32_t, int /*flv socketfd*/);
	int (*fFlvOpen)(char *);
	int (*fFlvClose)();
} MediaSrcInfo;

typedef struct {
	MPI_STREAM_PARAMS_V2_S params;
	unsigned char *p_sei_nalu;
	int sei_nalu_len;
} VideoStreamData;

int FLV_writeFLVHeader(MediaSrcInfo *info, bool is_have_audio, bool is_have_video);

int FLV_writeAACSeqHeaderTag(MediaSrcInfo *info, int sample_rate, int channel, uint32_t timestamp);

int FLV_writeAVCSeqHeaderTag(MediaSrcInfo *info, const uint8_t *vps, uint32_t vps_len, const uint8_t *sps,
                             uint32_t sps_len, const uint8_t *pps, uint32_t pps_len, uint32_t timestamp);

int FLV_writeAACDataTag(MediaSrcInfo *info, const uint8_t *data, uint32_t data_len, uint32_t timestamp);

int FLV_writeAVCDataTag(MediaSrcInfo *info, const uint8_t *vps, uint32_t vps_len, const uint8_t *sps, uint32_t sps_len,
                        const uint8_t *pps, uint32_t pps_len, const uint8_t *data, uint32_t data_len,
                        uint32_t timestamp, int is_keyframe);

#endif // FLV_MUXER_H_
