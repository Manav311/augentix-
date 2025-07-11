#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "soapStub.h"
#include "stdsoap2.h"
#include "augentix.h"

#define MAX_RES_MAP_NUM 5

SOAP_FMAC5 int SOAP_FMAC6
__tr2__GetServiceCapabilities(struct soap *soap, struct _tr2__GetServiceCapabilities *tr2__GetServiceCapabilities,
                              struct _tr2__GetServiceCapabilitiesResponse *tr2__GetServiceCapabilitiesResponse)
{
	struct tr2__Capabilities2 *cap = NULL;

	ONVIF_TRACE("__tr2__GetServiceCapabilities \n");

	tr2__GetServiceCapabilitiesResponse->Capabilities = aux_onvif_malloc(soap, sizeof(struct trt__Capabilities));
	cap = tr2__GetServiceCapabilitiesResponse->Capabilities;
	cap->SnapshotUri = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->SnapshotUri = xsd__boolean__true_;
	cap->OSD = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->OSD = xsd__boolean__true_;
	cap->ProfileCapabilities = aux_onvif_malloc(soap, sizeof(struct trt__ProfileCapabilities));
	cap->ProfileCapabilities->MaximumNumberOfProfiles = aux_onvif_malloc(soap, sizeof(int));
	*cap->ProfileCapabilities->MaximumNumberOfProfiles = 3;
	cap->StreamingCapabilities = aux_onvif_malloc(soap, sizeof(struct trt__StreamingCapabilities));
	cap->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;

	return SOAP_OK;
}
/** Web service operation '__tr2__CreateProfile' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__CreateProfile(struct soap *soap, struct _tr2__CreateProfile *tr2__CreateProfile,
                                               struct _tr2__CreateProfileResponse *tr2__CreateProfileResponse)
{
	ONVIF_TRACE("__tr2__CreateProfile TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetProfiles' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetProfiles(struct soap *soap, struct _tr2__GetProfiles *tr2__GetProfiles,
                                             struct _tr2__GetProfilesResponse *tr2__GetProfilesResponse)
{
	int i = 0;
	int tmpWidth = 0;
	int tmpHeight = 0;
	int tmpFps = 0;
	int tmpGov = 0;
	int tmpQuality = 0;
	int tmpEncType = 0;
	int tmpRcMode = 0;
	int tmpVbrBitrate = 0;
	int tmpBitrate = 0;
	int tmpProfile = 0;
	int stream_num = 0;
	int audio_en = 0;
	int audio_code = 0;
	int audio_bits = 0;
	int audio_freq = 0;
	struct tr2__MediaProfile *profile = NULL;
	struct tr2__ConfigurationSet *cfg_set = NULL;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_AUDIO_CONF_S audio = { 0 };

	ONVIF_TRACE("__tr2__GetProfiles\n");
	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}
	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}
	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}
	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	audio_en = audio.enabled;
	audio_code = audio.codec;
	audio_bits = audio.sampling_bit;
	audio_freq = audio.sampling_frequency;
	stream_num = strm.video_strm_cnt;
	tr2__GetProfilesResponse->__sizeProfiles = stream_num;
	tr2__GetProfilesResponse->Profiles_ = aux_onvif_malloc(soap, sizeof(struct tr2__MediaProfile) * stream_num);

	for (i = 0; i < stream_num; i++) {
		if (!strm.video_strm[i].strm_en) {
			continue;
		}

		tmpWidth = strm.video_strm[i].width;
		tmpHeight = strm.video_strm[i].height;
		tmpFps = strm.video_strm[i].output_fps;
		tmpGov = strm.video_strm[i].gop_size;
		tmpQuality = strm.video_strm[i].vbr_quality_level_index;
		tmpEncType = strm.video_strm[i].venc_type;
		tmpRcMode = strm.video_strm[i].rc_mode;
		tmpVbrBitrate = strm.video_strm[i].vbr_max_bit_rate;
		tmpBitrate = strm.video_strm[i].bit_rate;
		tmpProfile = strm.video_strm[i].venc_profile;
		profile = &tr2__GetProfilesResponse->Profiles_[i];

		profile->fixed = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
		*profile->fixed = xsd__boolean__true_;
		profile->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(profile->token, "0%02d", i);
		profile->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(profile->Name, "Profile_%s", profile->token);
		profile->Configurations = aux_onvif_malloc(soap, sizeof(struct tr2__ConfigurationSet));

		cfg_set = profile->Configurations;
		/*VideoSourceConfiguration*/
		cfg_set->VideoSource = aux_onvif_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
		cfg_set->VideoSource->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(cfg_set->VideoSource->token, "000");
		cfg_set->VideoSource->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(cfg_set->VideoSource->Name, "VideoS_000");
		cfg_set->VideoSource->UseCount = stream_num;
		cfg_set->VideoSource->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(cfg_set->VideoSource->SourceToken, cfg_set->VideoSource->token);
		cfg_set->VideoSource->Bounds = aux_onvif_malloc(soap, sizeof(struct tt__IntRectangle));
		cfg_set->VideoSource->Bounds->x = 0;
		cfg_set->VideoSource->Bounds->y = 0;
		cfg_set->VideoSource->Bounds->width = dev.input_path[0].width;
		cfg_set->VideoSource->Bounds->height = dev.input_path[0].height;

		if (audio_en) {
			/*AudioSourceConfiguration*/
			cfg_set->AudioSource = aux_onvif_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
			cfg_set->AudioSource->token = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy(cfg_set->AudioSource->token, "000");
			cfg_set->AudioSource->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(cfg_set->AudioSource->Name, "Audio_%s", cfg_set->AudioSource->token);
			cfg_set->AudioSource->UseCount = stream_num;
			cfg_set->AudioSource->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy(cfg_set->AudioSource->SourceToken, cfg_set->AudioSource->token);
		}

		/*VideoEncoderConfiguration*/
		cfg_set->VideoEncoder = aux_onvif_malloc(soap, sizeof(struct tt__VideoEncoder2Configuration));
		cfg_set->VideoEncoder->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(cfg_set->VideoEncoder->token, "0%02d", i);
		cfg_set->VideoEncoder->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(cfg_set->VideoEncoder->Name, "VideoE_0%02d", i);
		cfg_set->VideoEncoder->UseCount = 1;
		cfg_set->VideoEncoder->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);

		if (tmpEncType == AGTX_VENC_TYPE_H264) {
			strcpy(cfg_set->VideoEncoder->Encoding, "H264");
		} else if (tmpEncType == AGTX_VENC_TYPE_H265) {
			strcpy(cfg_set->VideoEncoder->Encoding, "H265");
		} else if (tmpEncType == AGTX_VENC_TYPE_MJPEG) {
			strcpy(cfg_set->VideoEncoder->Encoding, "JPEG");
		} else {
			ONVIF_TRACE("unknown encode type %d\n", tmpEncType);
			strcpy(cfg_set->VideoEncoder->Encoding, "UNKNOWN");
		}

		cfg_set->VideoEncoder->Resolution = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution2));
		if (dev.stitch_en) {
			cfg_set->VideoEncoder->Resolution->Height =
			        (tmpWidth == 3840 && tmpHeight == 1080) ?
			                2160 :
			                (tmpHeight == 720) ?
			                1440 :
			                (tmpHeight == 544) ?
			                1080 :
			                (tmpHeight == 272) ? 540 : (tmpHeight == 184) ? 360 : tmpHeight;
		} else {
			cfg_set->VideoEncoder->Resolution->Height = tmpHeight;
		}

		cfg_set->VideoEncoder->Resolution->Width = tmpWidth;
		cfg_set->VideoEncoder->Quality = tmpQuality;
		cfg_set->VideoEncoder->RateControl = aux_onvif_malloc(soap, sizeof(struct tt__VideoRateControl2));
		cfg_set->VideoEncoder->RateControl->FrameRateLimit = tmpFps;
		cfg_set->VideoEncoder->RateControl->ConstantBitRate = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));

		if (tmpRcMode == AGTX_RC_MODE_VBR) {
			*cfg_set->VideoEncoder->RateControl->ConstantBitRate = xsd__boolean__false_;
			cfg_set->VideoEncoder->RateControl->BitrateLimit = tmpVbrBitrate;
		} else if (tmpRcMode == AGTX_RC_MODE_CBR) {
			*cfg_set->VideoEncoder->RateControl->ConstantBitRate = xsd__boolean__true_;
			cfg_set->VideoEncoder->RateControl->BitrateLimit = tmpBitrate;
		} else if (tmpRcMode == AGTX_RC_MODE_SBR) {
			*cfg_set->VideoEncoder->RateControl->ConstantBitRate = xsd__boolean__false_;
			cfg_set->VideoEncoder->RateControl->BitrateLimit = tmpBitrate;
		} else if (tmpRcMode == AGTX_RC_MODE_CQP) {
			*cfg_set->VideoEncoder->RateControl->ConstantBitRate = xsd__boolean__false_;
			cfg_set->VideoEncoder->RateControl->BitrateLimit = INT_MAX;
		} else {
			ONVIF_TRACE("unknown encode rc mode %d\n", tmpRcMode);
			*cfg_set->VideoEncoder->RateControl->ConstantBitRate = xsd__boolean__false_;
			cfg_set->VideoEncoder->RateControl->BitrateLimit = tmpBitrate;
		}

		cfg_set->VideoEncoder->GovLength = aux_onvif_malloc(soap, sizeof(int));
		*cfg_set->VideoEncoder->GovLength = tmpGov;
		cfg_set->VideoEncoder->Profile = aux_onvif_malloc(soap, MAX_STR_LEN);

		if (tmpProfile == AGTX_PRFL_BASELINE) {
			strcpy(cfg_set->VideoEncoder->Profile, "Baseline");
		} else if (tmpProfile == AGTX_PRFL_MAIN) {
			strcpy(cfg_set->VideoEncoder->Profile, "Main");
		} else if (tmpProfile == AGTX_PRFL_HIGH) {
			strcpy(cfg_set->VideoEncoder->Profile, "High");
		} else {
			ONVIF_TRACE("unknown encode venc profile %d\n", tmpProfile);
		}

		if (audio_en) {
			/*AudioEncoderConfiguration*/
			cfg_set->AudioEncoder = aux_onvif_malloc(soap, sizeof(struct tt__AudioEncoder2Configuration));
			cfg_set->AudioEncoder->token = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(cfg_set->AudioEncoder->token, "0%02d", i);
			cfg_set->AudioEncoder->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(cfg_set->AudioEncoder->Name, "AudioE_%s", cfg_set->AudioEncoder->token);
			cfg_set->AudioEncoder->UseCount = stream_num;
			cfg_set->AudioEncoder->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);
			if (audio_code == AGTX_AUDIO_CODEC_ULAW || audio_code == AGTX_AUDIO_CODEC_ALAW) {
				strcpy(cfg_set->AudioEncoder->Encoding, "G711-0");
			} else if (audio_code == AGTX_AUDIO_CODEC_G726 && audio_bits == 2) {
				strcpy(cfg_set->AudioEncoder->Encoding, "G726-16");
			} else if (audio_code == AGTX_AUDIO_CODEC_G726 && audio_bits == 4) {
				strcpy(cfg_set->AudioEncoder->Encoding, "G726-32");
			} else {
				ONVIF_TRACE("unknown audio_code %d\n", audio_code);
			}
			cfg_set->AudioEncoder->Bitrate = audio_freq * audio_bits;
			cfg_set->AudioEncoder->SampleRate = audio_freq;
		}

		/*VideoAnalyticsConfiguration*/
		if (aux_get_iva_setting(soap, &cfg_set->Analytics) != SOAP_OK) {
			return SOAP_USER_ERROR;
		}
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__AddConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__AddConfiguration(struct soap *soap,
                                                  struct _tr2__AddConfiguration *tr2__AddConfiguration,
                                                  struct _tr2__AddConfigurationResponse *tr2__AddConfigurationResponse)
{
	ONVIF_TRACE("__tr2__AddConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__RemoveConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__RemoveConfiguration(struct soap *soap, struct _tr2__RemoveConfiguration *tr2__RemoveConfiguration,
                           struct _tr2__RemoveConfigurationResponse *tr2__RemoveConfigurationResponse)
{
	ONVIF_TRACE("__tr2__RemoveConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__DeleteProfile' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__DeleteProfile(struct soap *soap, struct _tr2__DeleteProfile *tr2__DeleteProfile,
                                               struct _tr2__DeleteProfileResponse *tr2__DeleteProfileResponse)
{
	ONVIF_TRACE("__tr2__DeleteProfile TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetVideoSourceConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetVideoSourceConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetVideoSourceConfigurations,
        struct _tr2__GetVideoSourceConfigurationsResponse *tr2__GetVideoSourceConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetVideoSourceConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetVideoEncoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetVideoEncoderConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetVideoEncoderConfigurations,
        struct _tr2__GetVideoEncoderConfigurationsResponse *tr2__GetVideoEncoderConfigurationsResponse)
{
	int i = 0;
	int num = 0;
	int tmpWidth = 0;
	int tmpHeight = 0;
	int tmpFps = 0;
	int tmpGov = 0;
	int tmpQuality = 0;
	int tmpEncType = 0;
	int tmpRcMode = 0;
	int tmpVbrBitrate = 0;
	int tmpBitrate = 0;
	int tmpProfile = 0;
	int stream_num = 0;
	struct tt__VideoEncoder2Configuration *venc_cfg = NULL;
	AGTX_STRM_CONF_S strm = { 0 };

	ONVIF_TRACE("__tr2__GetVideoEncoderConfigurations\n");

	/*If Token exist, Only return specify channel*/
	if (tr2__GetVideoEncoderConfigurations->ConfigurationToken) {
		num = atoi(tr2__GetVideoEncoderConfigurations->ConfigurationToken);
		if (num >= AGTX_MAX_VIDEO_STRM_NUM) {
			return soap_sender_fault(soap, "Invalid token", NULL);
		}
	} else {
		num = -1;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	stream_num = (num >= 0) ? 1 : strm.video_strm_cnt;

	tr2__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = stream_num;
	tr2__GetVideoEncoderConfigurationsResponse->Configurations_ =
	        aux_onvif_malloc(soap, sizeof(struct tt__VideoEncoder2Configuration) * stream_num);

	if (num >= 0) {
		i = num;

		if (!strm.video_strm[i].strm_en) {
			return soap_sender_fault(soap, "channel not enable", NULL);
		}

		tmpWidth = strm.video_strm[i].width;
		tmpHeight = strm.video_strm[i].height;
		tmpFps = strm.video_strm[i].output_fps;
		tmpGov = strm.video_strm[i].gop_size;
		tmpQuality = strm.video_strm[i].vbr_quality_level_index;
		tmpEncType = strm.video_strm[i].venc_type;
		tmpRcMode = strm.video_strm[i].rc_mode;
		tmpVbrBitrate = strm.video_strm[i].vbr_max_bit_rate;
		tmpBitrate = strm.video_strm[i].bit_rate;
		tmpProfile = strm.video_strm[i].venc_profile;
		venc_cfg = &tr2__GetVideoEncoderConfigurationsResponse->Configurations_[0];

		/*VideoEncoderConfiguration*/
		venc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(venc_cfg->token, "0%02d", i);
		venc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(venc_cfg->Name, "VideoE_0%02d", i);
		venc_cfg->UseCount = 1;
		venc_cfg->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);

		if (tmpEncType == AGTX_VENC_TYPE_H264) {
			strcpy(venc_cfg->Encoding, "H264");
		} else if (tmpEncType == AGTX_VENC_TYPE_H265) {
			strcpy(venc_cfg->Encoding, "H265");
		} else if (tmpEncType == AGTX_VENC_TYPE_MJPEG) {
			strcpy(venc_cfg->Encoding, "JPEG");
		} else {
			ONVIF_TRACE("unknown encode type %d\n", tmpEncType);
			strcpy(venc_cfg->Encoding, "UNKNOWN");
		}

		venc_cfg->Resolution = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution2));
		venc_cfg->Resolution->Width = tmpWidth;
		venc_cfg->Resolution->Height = tmpHeight;
		venc_cfg->Quality = tmpQuality;
		venc_cfg->RateControl = aux_onvif_malloc(soap, sizeof(struct tt__VideoRateControl2));
		venc_cfg->RateControl->FrameRateLimit = tmpFps;
		venc_cfg->RateControl->ConstantBitRate = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));

		if (tmpRcMode == AGTX_RC_MODE_VBR) {
			*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
			venc_cfg->RateControl->BitrateLimit = tmpVbrBitrate;
		} else if (tmpRcMode == AGTX_RC_MODE_CBR) {
			*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__true_;
			venc_cfg->RateControl->BitrateLimit = tmpBitrate;
		} else if (tmpRcMode == AGTX_RC_MODE_SBR) {
			*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
			venc_cfg->RateControl->BitrateLimit = tmpBitrate;
		} else if (tmpRcMode == AGTX_RC_MODE_CQP) {
			*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
			venc_cfg->RateControl->BitrateLimit = INT_MAX;
		} else {
			ONVIF_TRACE("unknown encode rc mode %d\n", tmpRcMode);
			*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
			venc_cfg->RateControl->BitrateLimit = tmpBitrate;
		}

		venc_cfg->GovLength = aux_onvif_malloc(soap, sizeof(int));
		*venc_cfg->GovLength = tmpGov;
		venc_cfg->Profile = aux_onvif_malloc(soap, MAX_STR_LEN);

		if (tmpProfile == AGTX_PRFL_BASELINE) {
			strcpy(venc_cfg->Profile, "Baseline");
		} else if (tmpProfile == AGTX_PRFL_MAIN) {
			strcpy(venc_cfg->Profile, "Main");
		} else if (tmpProfile == AGTX_PRFL_HIGH) {
			strcpy(venc_cfg->Profile, "High");
		} else {
			ONVIF_TRACE("unknown encode venc profile %d\n", tmpProfile);
		}
	} else {
		for (i = 0; i < stream_num; i++) {
			if (!strm.video_strm[i].strm_en) {
				continue;
			}

			tmpWidth = strm.video_strm[i].width;
			tmpHeight = strm.video_strm[i].height;
			tmpFps = strm.video_strm[i].output_fps;
			tmpGov = strm.video_strm[i].gop_size;
			tmpQuality = strm.video_strm[i].vbr_quality_level_index;
			tmpEncType = strm.video_strm[i].venc_type;
			tmpRcMode = strm.video_strm[i].rc_mode;
			tmpVbrBitrate = strm.video_strm[i].vbr_max_bit_rate;
			tmpBitrate = strm.video_strm[i].bit_rate;
			tmpProfile = strm.video_strm[i].venc_profile;
			venc_cfg = &tr2__GetVideoEncoderConfigurationsResponse->Configurations_[i];

			/*VideoEncoderConfiguration*/
			venc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(venc_cfg->token, "0%02d", i);
			venc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(venc_cfg->Name, "VideoE_0%02d", i);
			venc_cfg->UseCount = 1;
			venc_cfg->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);

			if (tmpEncType == AGTX_VENC_TYPE_H264) {
				strcpy(venc_cfg->Encoding, "H264");
			} else if (tmpEncType == AGTX_VENC_TYPE_H265) {
				strcpy(venc_cfg->Encoding, "H265");
			} else if (tmpEncType == AGTX_VENC_TYPE_MJPEG) {
				strcpy(venc_cfg->Encoding, "JPEG");
			} else {
				ONVIF_TRACE("unknown encode type %d\n", tmpEncType);
				strcpy(venc_cfg->Encoding, "UNKNOWN");
			}

			venc_cfg->Resolution = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution2));
			venc_cfg->Resolution->Width = tmpWidth;
			venc_cfg->Resolution->Height = tmpHeight;
			venc_cfg->Quality = tmpQuality;
			venc_cfg->RateControl = aux_onvif_malloc(soap, sizeof(struct tt__VideoRateControl2));
			venc_cfg->RateControl->FrameRateLimit = tmpFps;
			venc_cfg->RateControl->ConstantBitRate = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));

			if (tmpRcMode == AGTX_RC_MODE_VBR) {
				*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
				venc_cfg->RateControl->BitrateLimit = tmpVbrBitrate;
			} else if (tmpRcMode == AGTX_RC_MODE_CBR) {
				*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__true_;
				venc_cfg->RateControl->BitrateLimit = tmpBitrate;
			} else if (tmpRcMode == AGTX_RC_MODE_SBR) {
				*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
				venc_cfg->RateControl->BitrateLimit = tmpBitrate;
			} else if (tmpRcMode == AGTX_RC_MODE_CQP) {
				*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
				venc_cfg->RateControl->BitrateLimit = INT_MAX;
			} else {
				ONVIF_TRACE("unknown encode rc mode %d\n", tmpRcMode);
				*venc_cfg->RateControl->ConstantBitRate = xsd__boolean__false_;
				venc_cfg->RateControl->BitrateLimit = tmpBitrate;
			}

			venc_cfg->GovLength = aux_onvif_malloc(soap, sizeof(int));
			*venc_cfg->GovLength = tmpGov;
			venc_cfg->Profile = aux_onvif_malloc(soap, MAX_STR_LEN);

			if (tmpProfile == AGTX_PRFL_BASELINE) {
				strcpy(venc_cfg->Profile, "Baseline");
			} else if (tmpProfile == AGTX_PRFL_MAIN) {
				strcpy(venc_cfg->Profile, "Main");
			} else if (tmpProfile == AGTX_PRFL_HIGH) {
				strcpy(venc_cfg->Profile, "High");
			} else {
				ONVIF_TRACE("unknown encode venc profile %d\n", tmpProfile);
			}
		}
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioSourceConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioSourceConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioSourceConfigurations,
        struct _tr2__GetAudioSourceConfigurationsResponse *tr2__GetAudioSourceConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioSourceConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioEncoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioEncoderConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioEncoderConfigurations,
        struct _tr2__GetAudioEncoderConfigurationsResponse *tr2__GetAudioEncoderConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioEncoderConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAnalyticsConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAnalyticsConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAnalyticsConfigurations,
        struct _tr2__GetAnalyticsConfigurationsResponse *tr2__GetAnalyticsConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetAnalyticsConfigurations TODO\n");
	tr2__GetAnalyticsConfigurationsResponse->__sizeConfigurations = 1;

	if (aux_get_iva_setting(soap, &tr2__GetAnalyticsConfigurationsResponse->Configurations_) != SOAP_OK) {
		return SOAP_USER_ERROR;
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__GetMetadataConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__GetMetadataConfigurations(struct soap *soap, struct tr2__GetConfiguration *tr2__GetMetadataConfigurations,
                                 struct _tr2__GetMetadataConfigurationsResponse *tr2__GetMetadataConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetMetadataConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioOutputConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioOutputConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioOutputConfigurations,
        struct _tr2__GetAudioOutputConfigurationsResponse *tr2__GetAudioOutputConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioOutputConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioDecoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioDecoderConfigurations(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioDecoderConfigurations,
        struct _tr2__GetAudioDecoderConfigurationsResponse *tr2__GetAudioDecoderConfigurationsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioDecoderConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetVideoSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetVideoSourceConfiguration(
        struct soap *soap, struct _tr2__SetVideoSourceConfiguration *tr2__SetVideoSourceConfiguration,
        struct tr2__SetConfigurationResponse *tr2__SetVideoSourceConfigurationResponse)
{
	ONVIF_TRACE("__tr2__SetVideoSourceConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetVideoEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetVideoEncoderConfiguration(
        struct soap *soap, struct _tr2__SetVideoEncoderConfiguration *tr2__SetVideoEncoderConfiguration,
        struct tr2__SetConfigurationResponse *tr2__SetVideoEncoderConfigurationResponse)
{
	int num = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	struct tt__VideoEncoder2Configuration *enc;

	if (tr2__SetVideoEncoderConfiguration->Configuration &&
	    tr2__SetVideoEncoderConfiguration->Configuration->token) {
		num = atoi(tr2__SetVideoEncoderConfiguration->Configuration->token);
	} else {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__tr2__SetVideoEncoderConfiguration %d\n", num);

	if (num > AGTX_MAX_VIDEO_STRM_NUM) {
		ONVIF_TRACE("Unsupport encode channel %d\n", num);
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}
	enc = tr2__SetVideoEncoderConfiguration->Configuration;
	if (enc == NULL) {
		ONVIF_TRACE("enc = NULL\n");
		return SOAP_FAULT;
	}

	if (enc->Encoding && !strcmp(enc->Encoding, "H264")) {
		strm.video_strm[num].venc_type = AGTX_VENC_TYPE_H264;
	} else if (enc->Encoding && !strcmp(enc->Encoding, "H265")) {
		strm.video_strm[num].venc_type = AGTX_VENC_TYPE_H265;
	} else {
		ONVIF_TRACE("Unsupport encode type\n");
		return SOAP_FAULT;
	}

	if (enc->GovLength) {
		strm.video_strm[num].gop_size = *enc->GovLength;
	}

	if (enc->Resolution) {
		strm.video_strm[num].width = enc->Resolution->Width;
		strm.video_strm[num].height = enc->Resolution->Height;
	}

	if (enc->RateControl) {
		if (enc->RateControl->FrameRateLimit)
			strm.video_strm[num].output_fps = enc->RateControl->FrameRateLimit;

		if (enc->RateControl->ConstantBitRate && enc->RateControl->BitrateLimit) {
			if (*enc->RateControl->ConstantBitRate == xsd__boolean__true_) {
				strm.video_strm[num].bit_rate = enc->RateControl->BitrateLimit;
				strm.video_strm[num].rc_mode = AGTX_RC_MODE_CBR;
			} else {
				if (enc->RateControl->BitrateLimit == INT_MAX) {
					strm.video_strm[num].bit_rate = enc->RateControl->BitrateLimit;
					strm.video_strm[num].rc_mode = AGTX_RC_MODE_CQP;
				} else {
					strm.video_strm[num].vbr_max_bit_rate = enc->RateControl->BitrateLimit;
					strm.video_strm[num].rc_mode = AGTX_RC_MODE_VBR;
				}
			}
		}
	}
	if (aux_set_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__SetAudioSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetAudioSourceConfiguration(
        struct soap *soap, struct _tr2__SetAudioSourceConfiguration *tr2__SetAudioSourceConfiguration,
        struct tr2__SetConfigurationResponse *tr2__SetAudioSourceConfigurationResponse)
{
	ONVIF_TRACE("__tr2__SetAudioSourceConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetAudioEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetAudioEncoderConfiguration(
        struct soap *soap, struct _tr2__SetAudioEncoderConfiguration *tr2__SetAudioEncoderConfiguration,
        struct tr2__SetConfigurationResponse *tr2__SetAudioEncoderConfigurationResponse)
{
	ONVIF_TRACE("__tr2__SetAudioEncoderConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetMetadataConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__SetMetadataConfiguration(struct soap *soap, struct _tr2__SetMetadataConfiguration *tr2__SetMetadataConfiguration,
                                struct tr2__SetConfigurationResponse *tr2__SetMetadataConfigurationResponse)
{
	ONVIF_TRACE("__tr2__SetMetadataConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetAudioOutputConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetAudioOutputConfiguration(
        struct soap *soap, struct _tr2__SetAudioOutputConfiguration *tr2__SetAudioOutputConfiguration,
        struct tr2__SetConfigurationResponse *tr2__SetAudioOutputConfigurationResponse)
{
	ONVIF_TRACE("__tr2__SetAudioOutputConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetAudioDecoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetAudioDecoderConfiguration(
        struct soap *soap, struct _tr2__SetAudioDecoderConfiguration *tr2__SetAudioDecoderConfiguration,
        struct tr2__SetConfigurationResponse *tr2__SetAudioDecoderConfigurationResponse)
{
	ONVIF_TRACE("__tr2__SetAudioDecoderConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetVideoSourceConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetVideoSourceConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetVideoSourceConfigurationOptions,
        struct _tr2__GetVideoSourceConfigurationOptionsResponse *tr2__GetVideoSourceConfigurationOptionsResponse)
{
	ONVIF_TRACE("__tr2__GetVideoSourceConfigurationOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetVideoEncoderConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetVideoEncoderConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetVideoEncoderConfigurationOptions,
        struct _tr2__GetVideoEncoderConfigurationOptionsResponse *tr2__GetVideoEncoderConfigurationOptionsResponse)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int idx = 0;
	int num = 0;
	int codec_idx[MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE] = { 0 };
	int resIndex = 0;
	int availRes = 0;
	int availCodec = 0;
	int res_map[MAX_RES_MAP_NUM][4] = { { 3840, 1080, 3840, 2160 },
		                            { 2560, 720, 2560, 1440 },
		                            { 1920, 544, 1920, 1080 },
		                            { 960, 540, 960, 272 },
		                            { 640, 360, 640, 184 } };
	struct tt__VideoEncoder2ConfigurationOptions *option = NULL;
	AGTX_RES_OPTION_S *devResOptions = aux_get_res();
	AGTX_VENC_OPTION_S *devVencOptions = aux_get_venc();
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };

	if (tr2__GetVideoEncoderConfigurationOptions->ConfigurationToken) {
		num = atoi(tr2__GetVideoEncoderConfigurationOptions->ConfigurationToken);
	} else {
		return SOAP_NO_DATA;
	}

	ONVIF_TRACE("__tr2__GetVideoEncoderConfigurationOptions num %d\n", num);

	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	for (i = 0; i < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; i++) {
		if ((devResOptions->strm[num].res[i].width == strm.video_strm[num].width) &&
		    (devResOptions->strm[num].res[i].height == strm.video_strm[num].height)) {
			resIndex = i;
			break;
		}
	}

	if (i >= MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE) {
		soap->error = 400;
		return SOAP_FAULT;
	}

	for (i = 0; i < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; i++) {
		if (devResOptions->strm[num].res[i].width != 0) {
			availRes++;
		}
	}

	/*Dual lens need to remapping resolution for NVR*/
	if (dev.stitch_en) {
		for (i = 0; i < availRes; i++) {
			for (j = 0; j < MAX_RES_MAP_NUM; j++) {
				if (devResOptions->strm[num].res[i].width == res_map[j][0] &&
				    devResOptions->strm[num].res[i].height == res_map[j][1]) {
					availRes++;
					break;
				}
			}
		}
	}

	/*Ruision NVR can't receive JPEG option that must skip*/
	for (i = 0; i < MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE; i++) {
		if (devVencOptions->strm[num].venc[i].codec == AGTX_VENC_TYPE_H264 ||
		    devVencOptions->strm[num].venc[i].codec == AGTX_VENC_TYPE_H265) {
			codec_idx[availCodec] = i;
			availCodec++;
		}
	}

	tr2__GetVideoEncoderConfigurationOptionsResponse->__sizeOptions = availCodec;
	tr2__GetVideoEncoderConfigurationOptionsResponse->Options_ =
	        aux_onvif_malloc(soap, sizeof(struct tt__VideoEncoder2ConfigurationOptions) * availCodec);

	for (i = 0; i < availCodec; i++) {
		option = &tr2__GetVideoEncoderConfigurationOptionsResponse->Options_[i];
		idx = codec_idx[i];

		option->GovLengthRange = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(option->GovLengthRange, "1 %d", devVencOptions->strm[num].venc[idx].max_gop_size);

		option->FrameRatesSupported = aux_onvif_malloc(soap, MAX_STR_LEN);

		for (j = 0; j < MAX_AGTX_RES_ENTITY_S_FRAME_RATE_LIST_SIZE; j++) {
			if (devResOptions->strm[num].res[resIndex].frame_rate_list[j] == 0) {
				break;
			}
			if (j == 0) {
				sprintf(option->FrameRatesSupported, "%d",
				        (int)devResOptions->strm[num].res[resIndex].frame_rate_list[j]);
			} else {
				sprintf(option->FrameRatesSupported, "%s %d", option->FrameRatesSupported,
				        (int)devResOptions->strm[num].res[resIndex].frame_rate_list[j]);
			}
		}

		option->ProfilesSupported = aux_onvif_malloc(soap, MAX_STR_LEN);
		for (j = 0; j < MAX_AGTX_VENC_ENTITY_S_PROFILE_SIZE; j++) {
			if (devVencOptions->strm[num].venc[idx].profile[j] == AGTX_PRFL_BASELINE) {
				strcat(option->ProfilesSupported, "Baseline ");
			} else if (devVencOptions->strm[num].venc[idx].profile[j] == AGTX_PRFL_MAIN) {
				strcat(option->ProfilesSupported, "Main ");
			} else if (devVencOptions->strm[num].venc[idx].profile[j] == AGTX_PRFL_HIGH) {
				strcat(option->ProfilesSupported, "High ");
			}
		}

		option->ConstantBitRateSupported = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
		*option->ConstantBitRateSupported = xsd__boolean__true_;
		option->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);
		if (devVencOptions->strm[num].venc[idx].codec == AGTX_VENC_TYPE_H264) {
			strcpy(option->Encoding, "H264");
		} else if (devVencOptions->strm[num].venc[idx].codec == AGTX_VENC_TYPE_H265) {
			strcpy(option->Encoding, "H265");
		} else if (devVencOptions->strm[num].venc[idx].codec == AGTX_VENC_TYPE_MJPEG) {
			strcpy(option->Encoding, "JPEG");
		}

		option->QualityRange = aux_onvif_malloc(soap, sizeof(struct tt__FloatRange));
		option->QualityRange->Min = devVencOptions->strm[num].venc[idx].vbr_param.min_quality_range;
		option->QualityRange->Max = devVencOptions->strm[num].venc[idx].vbr_param.max_quality_range;

		option->__sizeResolutionsAvailable = availRes;
		option->ResolutionsAvailable_ = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution2) * availRes);
		for (j = 0; j < availRes; j++) {
			option->ResolutionsAvailable_[j].Width = devResOptions->strm[num].res[j].width;
			option->ResolutionsAvailable_[j].Height = devResOptions->strm[num].res[j].height;
			/*Dual lens need to remapping resolution for NVR*/
			if (dev.stitch_en) {
				for (k = 0; k < MAX_RES_MAP_NUM; k++) {
					if (devResOptions->strm[num].res[j].width == res_map[k][0] &&
					    devResOptions->strm[num].res[j].height == res_map[k][1]) {
						j++;
						option->ResolutionsAvailable_[j].Width = res_map[k][2];
						option->ResolutionsAvailable_[j].Height = res_map[k][3];
						break;
					}
				}
			}
		}

		option->BitrateRange = aux_onvif_malloc(soap, sizeof(struct tt__IntRange));
		option->BitrateRange->Min = (devVencOptions->strm[num].venc[idx].min_bit_rate);
		option->BitrateRange->Max = (devVencOptions->strm[num].venc[idx].max_bit_rate);
	}
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioSourceConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioSourceConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioSourceConfigurationOptions,
        struct _tr2__GetAudioSourceConfigurationOptionsResponse *tr2__GetAudioSourceConfigurationOptionsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioSourceConfigurationOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioEncoderConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioEncoderConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioEncoderConfigurationOptions,
        struct _tr2__GetAudioEncoderConfigurationOptionsResponse *tr2__GetAudioEncoderConfigurationOptionsResponse)
{
	int codec_num = 2;
	struct tt__AudioEncoder2ConfigurationOptions *opt = NULL;

	ONVIF_TRACE("__tr2__GetAudioEncoderConfigurationOptions\n");

	tr2__GetAudioEncoderConfigurationOptionsResponse->__sizeOptions = codec_num;
	tr2__GetAudioEncoderConfigurationOptionsResponse->Options_ =
	        aux_onvif_malloc(soap, sizeof(struct tt__AudioEncoder2ConfigurationOptions));
	opt = &tr2__GetAudioEncoderConfigurationOptionsResponse->Options_[0];
	opt->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(opt->Encoding, "G711-0");
	opt->BitrateList = aux_onvif_malloc(soap, sizeof(struct tt__IntList));
	opt->BitrateList->__sizeItems = 1;
	opt->BitrateList->Items_ = aux_onvif_malloc(soap, sizeof(int) * opt->BitrateList->__sizeItems);
	opt->BitrateList->Items_[0] = 64;
	opt->SampleRateList = aux_onvif_malloc(soap, sizeof(struct tt__IntList));
	opt->SampleRateList->__sizeItems = 1;
	opt->SampleRateList->Items_ = aux_onvif_malloc(soap, sizeof(int) * opt->SampleRateList->__sizeItems);
	opt->SampleRateList->Items_[0] = 8;

	opt = &tr2__GetAudioEncoderConfigurationOptionsResponse->Options_[1];
	opt->Encoding = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(opt->Encoding, "G726");
	opt->BitrateList = aux_onvif_malloc(soap, sizeof(struct tt__IntList));
	opt->BitrateList->__sizeItems = 2;
	opt->BitrateList->Items_ = aux_onvif_malloc(soap, sizeof(int) * opt->BitrateList->__sizeItems);
	opt->BitrateList->Items_[0] = 16;
	opt->BitrateList->Items_[0] = 32;
	opt->SampleRateList = aux_onvif_malloc(soap, sizeof(struct tt__IntList));
	opt->SampleRateList->__sizeItems = 1;
	opt->SampleRateList->Items_ = aux_onvif_malloc(soap, sizeof(int) * opt->SampleRateList->__sizeItems);
	opt->SampleRateList->Items_[0] = 8;

	return SOAP_OK;
}
/** Web service operation '__tr2__GetMetadataConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetMetadataConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetMetadataConfigurationOptions,
        struct _tr2__GetMetadataConfigurationOptionsResponse *tr2__GetMetadataConfigurationOptionsResponse)
{
	ONVIF_TRACE("__tr2__GetMetadataConfigurationOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioOutputConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioOutputConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioOutputConfigurationOptions,
        struct _tr2__GetAudioOutputConfigurationOptionsResponse *tr2__GetAudioOutputConfigurationOptionsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioOutputConfigurationOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetAudioDecoderConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetAudioDecoderConfigurationOptions(
        struct soap *soap, struct tr2__GetConfiguration *tr2__GetAudioDecoderConfigurationOptions,
        struct _tr2__GetAudioDecoderConfigurationOptionsResponse *tr2__GetAudioDecoderConfigurationOptionsResponse)
{
	ONVIF_TRACE("__tr2__GetAudioDecoderConfigurationOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetVideoEncoderInstances' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__GetVideoEncoderInstances(struct soap *soap, struct _tr2__GetVideoEncoderInstances *tr2__GetVideoEncoderInstances,
                                struct _tr2__GetVideoEncoderInstancesResponse *tr2__GetVideoEncoderInstancesResponse)
{
	ONVIF_TRACE("__tr2__GetVideoEncoderInstances TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetStreamUri' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetStreamUri(struct soap *soap, struct _tr2__GetStreamUri *tr2__GetStreamUri,
                                              struct _tr2__GetStreamUriResponse *tr2__GetStreamUriResponse)
{
	int stream_index = 0;
	int audio_en = 0;
	char ip_buf[MAX_STR_LEN] = { 0 };
	AGTX_AUDIO_CONF_S audio = { 0 };

	ONVIF_TRACE("__tr2__GetStreamUri\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (tr2__GetStreamUri->ProfileToken) {
		stream_index = atoi(tr2__GetStreamUri->ProfileToken);
		printf("stream_index %d trt__GetStreamUri->ProfileToken %s\n", stream_index,
		       tr2__GetStreamUri->ProfileToken);
	} else {
		return SOAP_NO_DATA;
	}

	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	audio_en = audio.enabled;

	tr2__GetStreamUriResponse->Uri = aux_onvif_malloc(soap, MAX_STR_LEN);

	// currently only 3 port numbers defined
	assert(stream_index >= 0 && stream_index < 3);

	if (audio_en) {
#if AUDIO_EN
		sprintf(tr2__GetStreamUriResponse->Uri, "rtsp://%s:%d/liveaudio/%d", SYS_Getipaddr("eth0", ip_buf),
		        portNumber[stream_index], stream_index);
#else
		sprintf(tr2__GetStreamUriResponse->Uri, "rtsp://%s:%d/live/%d", SYS_Getipaddr("eth0", ip_buf),
		        portNumber[stream_index], stream_index);
#endif
	} else {
		sprintf(tr2__GetStreamUriResponse->Uri, "rtsp://%s:%d/live/%d", SYS_Getipaddr("eth0", ip_buf),
		        portNumber[stream_index], stream_index);
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__StartMulticastStreaming' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__StartMulticastStreaming(struct soap *soap, struct tr2__StartStopMulticastStreaming *tr2__StartMulticastStreaming,
                               struct tr2__SetConfigurationResponse *tr2__StartMulticastStreamingResponse)
{
	ONVIF_TRACE("__tr2__StartMulticastStreaming TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__StopMulticastStreaming' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__StopMulticastStreaming(struct soap *soap, struct tr2__StartStopMulticastStreaming *tr2__StopMulticastStreaming,
                              struct tr2__SetConfigurationResponse *tr2__StopMulticastStreamingResponse)
{
	ONVIF_TRACE("__tr2__StopMulticastStreaming TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetSynchronizationPoint' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__SetSynchronizationPoint(struct soap *soap, struct _tr2__SetSynchronizationPoint *tr2__SetSynchronizationPoint,
                               struct _tr2__SetSynchronizationPointResponse *tr2__SetSynchronizationPointResponse)
{
	ONVIF_TRACE("__tr2__SetSynchronizationPoint TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetSnapshotUri' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetSnapshotUri(struct soap *soap, struct _tr2__GetSnapshotUri *tr2__GetSnapshotUri,
                                                struct _tr2__GetSnapshotUriResponse *tr2__GetSnapshotUriResponse)
{
	int num = 0;
	char ip_buf[MAX_STR_LEN] = { 0 };
	struct tt__MediaUri *mediaUri = NULL;

	ONVIF_TRACE("__tr2__GetSnapshotUri\n");

	if (tr2__GetSnapshotUri->ProfileToken) {
		num = atoi(tr2__GetSnapshotUri->ProfileToken);
	} else {
		return soap_sender_fault(soap, "Unknown token", NULL);
	}

	tr2__GetSnapshotUriResponse->Uri = aux_onvif_malloc(soap, MAX_STR_LEN);
	sprintf(mediaUri->Uri, "http://%s:8899/%s%02d", SYS_Getipaddr("eth0", ip_buf), SNAPSHOT_URI, num);

	return SOAP_OK;
}
/** Web service operation '__tr2__GetVideoSourceModes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__GetVideoSourceModes(struct soap *soap, struct _tr2__GetVideoSourceModes *tr2__GetVideoSourceModes,
                           struct _tr2__GetVideoSourceModesResponse *tr2__GetVideoSourceModesResponse)
{
	ONVIF_TRACE("__tr2__GetVideoSourceModes TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetVideoSourceMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tr2__SetVideoSourceMode(struct soap *soap, struct _tr2__SetVideoSourceMode *tr2__SetVideoSourceMode,
                          struct _tr2__SetVideoSourceModeResponse *tr2__SetVideoSourceModeResponse)
{
	ONVIF_TRACE("__tr2__SetVideoSourceMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetOSDs' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetOSDs(struct soap *soap, struct _tr2__GetOSDs *tr2__GetOSDs,
                                         struct _tr2__GetOSDsResponse *tr2__GetOSDsResponse)
{
	int i = 0;
	int chn = 0;
	int idx = 0;
	int osd_num = 0;
	int osd_idx[MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE] = { 0 };
	AGTX_OSD_CONF_S osd_conf;
	struct tt__OSDConfiguration *osd = NULL;

	ONVIF_TRACE("__tr2__GetOSDs\n");

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
		if (osd_conf.strm[chn].region[i].enabled && (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_TEXT ||
		                                             osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_INFO)) {
			osd_idx[osd_num] = i;
			osd_num++;
		}
	}

	tr2__GetOSDsResponse->__sizeOSDs = osd_num;
	tr2__GetOSDsResponse->OSDs_ = aux_onvif_malloc(soap, sizeof(struct tt__OSDConfiguration) * osd_num);

	for (i = 0; i < osd_num; i++) {
		idx = osd_idx[i];
		osd = &tr2__GetOSDsResponse->OSDs_[i];

		osd->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		if (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_TEXT) {
			strcpy(osd->token, "osdtoken_txt");
			osd->Type = tt__OSDType__Text;
		} else if (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_INFO) {
			strcpy(osd->token, "osdtoken_info");
			osd->Type = tt__OSDType__Text;
		} else if (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_IMAGE) {
			strcpy(osd->token, (idx == 3) ? "osdtoken_003" : "osdtoken_002");
			osd->Type = tt__OSDType__Image;
		} else {
			ONVIF_TRACE("Unknown OSD type %d\n", osd_conf.strm[chn].region[idx].type);
			continue;
		}

		osd->VideoSourceConfigurationToken = aux_onvif_malloc(soap, sizeof(struct tt__OSDReference));
		osd->VideoSourceConfigurationToken->__item = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(osd->VideoSourceConfigurationToken->__item, "000");
		osd->VideoSourceConfigurationToken->__anyAttribute = NULL;
		osd->Type = (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_IMAGE) ? tt__OSDType__Image :
		                                                                           tt__OSDType__Text;
		osd->Position = aux_onvif_malloc(soap, sizeof(struct tt__OSDPosConfiguration));
		osd->Position->Type = aux_onvif_malloc(soap, sizeof(char) * MAX_STR_LEN);
		strcpy(osd->Position->Type, "Custom");
		osd->Position->Pos = aux_onvif_malloc(soap, sizeof(struct tt__Vector));
		osd->Position->Pos->x = aux_onvif_malloc(soap, sizeof(float));
		*osd->Position->Pos->x = (float)(osd_conf.strm[chn].region[idx].start_x - 50) / 50;
		osd->Position->Pos->y = aux_onvif_malloc(soap, sizeof(float));
		*osd->Position->Pos->y = -((float)(osd_conf.strm[chn].region[idx].start_y - 50) / 50);
		if (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_TEXT) {
			osd->TextString = aux_onvif_malloc(soap, sizeof(struct tt__OSDTextConfiguration));
			osd->TextString->Type = aux_onvif_malloc(soap, sizeof(char) * MAX_STR_LEN);
			strcpy(osd->TextString->Type, "Plain");
			osd->TextString->PlainText = aux_onvif_malloc(soap, MAX_STR_LEN);

			if (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_TEXT) {
				strncpy(osd->TextString->PlainText, (char *)osd_conf.strm[0].region[idx].type_spec,
				        MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE);
			}
		} else if (osd_conf.strm[chn].region[idx].type == AGTX_OSD_TYPE_INFO) {
			osd->TextString = aux_onvif_malloc(soap, sizeof(struct tt__OSDTextConfiguration));
			osd->TextString->Type = aux_onvif_malloc(soap, sizeof(char) * MAX_STR_LEN);
			strcpy(osd->TextString->Type, "DateAndTime");

			osd->TextString->DateFormat = aux_onvif_malloc(soap, MAX_STR_LEN);
			if (strcmp((char *)osd_conf.strm[chn].region[idx].type_spec, "MM-DD-YYYY") == 0) {
				strcpy((char *)osd->TextString->DateFormat, "MM/dd/yyyy");
			} else if (strcmp((char *)osd_conf.strm[chn].region[idx].type_spec, "DD-MM-YYYY") == 0) {
				strcpy((char *)osd->TextString->DateFormat, "dd/MM/yyyy");
			} else if (strcmp((char *)osd_conf.strm[chn].region[idx].type_spec, "YYYY-MM-DD") == 0) {
				strcpy((char *)osd->TextString->DateFormat, "yyyy/MM/dd");
			} else {
				strcpy((char *)osd->TextString->DateFormat, "YYYY-MM-DD");
			}
			osd->TextString->TimeFormat = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy((char *)osd->TextString->TimeFormat, "hh:mm:ss tt");
		} else { //Image osd type
			osd->Image = aux_onvif_malloc(soap, sizeof(struct tt__OSDImgConfiguration));
			osd->Image->ImgPath = aux_onvif_malloc(soap, MAX_STR_LEN);
			strncpy(osd->Image->ImgPath, (char *)osd_conf.strm[0].region[idx].type_spec,
			        MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE);
		}
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__GetOSDOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetOSDOptions(struct soap *soap, struct _tr2__GetOSDOptions *tr2__GetOSDOptions,
                                               struct _tr2__GetOSDOptionsResponse *tr2__GetOSDOptionsResponse)
{
	int i = 0;
	int type_num = 1;
	int pos_num = 1;
	int osd_num = 5;
	int osd_type_num = 5;
	int date_format_num = 3;
	int time_format_num = 4;
	struct tt__OSDConfigurationOptions *osd_opt = NULL;

	ONVIF_TRACE("__tr2__GetOSDOptions\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	tr2__GetOSDOptionsResponse->OSDOptions = aux_onvif_malloc(soap, sizeof(struct tt__OSDConfigurationOptions));
	osd_opt = tr2__GetOSDOptionsResponse->OSDOptions;
	osd_opt->MaximumNumberOfOSDs = aux_onvif_malloc(soap, sizeof(struct tt__MaximumNumberOfOSDs));
	osd_opt->MaximumNumberOfOSDs->DateAndTime = aux_onvif_malloc(soap, sizeof(int));
	*osd_opt->MaximumNumberOfOSDs->DateAndTime = 1;
	osd_opt->MaximumNumberOfOSDs->Date = aux_onvif_malloc(soap, sizeof(int));
	*osd_opt->MaximumNumberOfOSDs->Date = 0;
	osd_opt->MaximumNumberOfOSDs->Time = aux_onvif_malloc(soap, sizeof(int));
	*osd_opt->MaximumNumberOfOSDs->Time = 0;
	osd_opt->MaximumNumberOfOSDs->Image = aux_onvif_malloc(soap, sizeof(int));
	*osd_opt->MaximumNumberOfOSDs->Image = 0;
	osd_opt->MaximumNumberOfOSDs->PlainText = aux_onvif_malloc(soap, sizeof(int));
	*osd_opt->MaximumNumberOfOSDs->PlainText = 1;
	osd_opt->MaximumNumberOfOSDs->Total = osd_num;
	osd_opt->__sizeType = type_num;
	osd_opt->Type_ = aux_onvif_malloc(soap, sizeof(enum tt__OSDType) * type_num);
	osd_opt->Type_[0] = tt__OSDType__Text;

	osd_opt->TextOption = aux_onvif_malloc(soap, sizeof(struct tt__OSDTextOptions));

	osd_opt->TextOption->__sizeType = osd_type_num;
	osd_opt->TextOption->Type_ = (char **)aux_onvif_malloc(soap, sizeof(char *) * osd_type_num);

	for (i = 0; i < osd_num; i++) {
		osd_opt->TextOption->Type_[i] = (char *)aux_onvif_malloc(soap, MAX_STR_LEN);
	}

	strcpy(osd_opt->TextOption->Type_[0], "Plain");
	strcpy(osd_opt->TextOption->Type_[1], "Date");
	strcpy(osd_opt->TextOption->Type_[2], "Time");
	strcpy(osd_opt->TextOption->Type_[3], "DateAndTime");
	strcpy(osd_opt->TextOption->Type_[4], "YYYY/MM/DD");
	osd_opt->TextOption->__sizeDateFormat = date_format_num;
	osd_opt->TextOption->DateFormat_ = (char **)aux_onvif_malloc(soap, sizeof(char *) * date_format_num);

	for (i = 0; i < date_format_num; i++) {
		osd_opt->TextOption->DateFormat_[i] = (char *)aux_onvif_malloc(soap, MAX_STR_LEN);
	}

	strcpy(osd_opt->TextOption->DateFormat_[0], "MM/dd/yyyy");
	strcpy(osd_opt->TextOption->DateFormat_[1], "dd/MM/yyyy");
	strcpy(osd_opt->TextOption->DateFormat_[2], "yyyy/MM/dd");

	osd_opt->TextOption->__sizeTimeFormat = time_format_num;
	osd_opt->TextOption->TimeFormat_ = (char **)aux_onvif_malloc(soap, sizeof(char *) * time_format_num);

	for (i = 0; i < time_format_num; i++) {
		osd_opt->TextOption->TimeFormat_[i] = (char *)aux_onvif_malloc(soap, MAX_STR_LEN);
	}

	strcpy(osd_opt->TextOption->TimeFormat_[0], "h:mm:ss tt");
	strcpy(osd_opt->TextOption->TimeFormat_[0], "hh:mm:ss tt");
	strcpy(osd_opt->TextOption->TimeFormat_[0], "H:mm:ss");
	strcpy(osd_opt->TextOption->TimeFormat_[1], "HH:mm:ss");

	osd_opt->__sizePositionOption = pos_num;
	osd_opt->PositionOption_ = aux_onvif_malloc(soap, sizeof(char *) * pos_num);
	osd_opt->PositionOption_[0] = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(osd_opt->PositionOption_[0], "Custom");

	return SOAP_OK;
}
/** Web service operation '__tr2__SetOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetOSD(struct soap *soap, struct _tr2__SetOSD *tr2__SetOSD,
                                        struct tr2__SetConfigurationResponse *tr2__SetOSDResponse)
{
	int i = 0;
	int chn = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_OSD_CONF_S osd_conf;
	struct tt__OSDConfiguration *osd;

	if (!tr2__SetOSD->OSD || !tr2__SetOSD->OSD->token) {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__tr2__SetOSD\n");

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	osd = tr2__SetOSD->OSD;

	if (strcmp(osd->token, "osdtoken_txt") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_TEXT) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->x) {
			osd_conf.strm[chn].region[i].start_x = ((*osd->Position->Pos->x * 100) + 100) / 2;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->y) {
			osd_conf.strm[chn].region[i].start_y = abs(((*osd->Position->Pos->y * 100) - 100) / 2);
		}

		if (osd->TextString && osd->TextString->PlainText) {
			strncpy((char *)osd_conf.strm[chn].region[i].type_spec, osd->TextString->PlainText,
			        MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE);
		}
	} else if (strcmp(osd->token, "osdtoken_info") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_INFO) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->x) {
			osd_conf.strm[chn].region[i].start_x = ((*osd->Position->Pos->x * 100) + 100) / 2;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->y) {
			osd_conf.strm[chn].region[i].start_y = abs(((*osd->Position->Pos->y * 100) - 100) / 2);
		}

		if (osd->TextString && osd->TextString->DateFormat) {
			ONVIF_TRACE("Set OSD: Date format: %s ", osd->TextString->DateFormat);
			if (strcmp(osd->TextString->DateFormat, "MM/dd/yyyy") == 0) {
				osd_conf.strm[chn].region[i].enabled = 1;
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "MM-DD-YYYY");
			} else if (strcmp(osd->TextString->DateFormat, "dd/MM/yyyy") == 0) {
				osd_conf.strm[chn].region[i].enabled = 1;
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "DD-MM-YYYY");
			} else if (strcmp(osd->TextString->DateFormat, "yyyy/MM/dd") == 0) {
				osd_conf.strm[chn].region[i].enabled = 1;
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "YYYY-MM-DD");
			} else {
				osd_conf.strm[chn].region[i].enabled = 1;
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "YYYY-MM-DD");
			}
		}
		if (osd->TextString && osd->TextString->TimeFormat) {
			if (strcmp(osd->TextString->TimeFormat, "h:mm:ss tt") == 0) { //2:14:21 PM

			} else if (strcmp(osd->TextString->TimeFormat, "hh:mm:ss tt") == 0) { //02:14:21 PM

			} else if (strcmp(osd->TextString->TimeFormat, "H:mm:ss") == 0) { //14:14:21

			} else if (strcmp(osd->TextString->TimeFormat, "HH:mm:ss") == 0) { //14:14:21

			} else {
			}
		}
	} else if (strcmp(osd->token, "osdtoken_002") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_IMAGE) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		if (osd->Image && osd->Image->ImgPath) {
			strncpy((char *)osd_conf.strm[chn].region[i].type_spec, osd->Image->ImgPath,
			        MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE);
		}
		if (osd->Position && osd->Position->Pos && osd->Position->Pos->x) {
			osd_conf.strm[chn].region[i].start_x = ((*osd->Position->Pos->x * 100) + 100) / 2;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->y) {
			osd_conf.strm[chn].region[i].start_y = abs(((*osd->Position->Pos->y * 100) - 100) / 2);
		}
		osd_conf.strm[chn].region[i].enabled = 1;
	}

	/*Mirror channel 0 to other channel*/
	for (i = 1; (unsigned)i < strm.video_strm_cnt; i++) {
		memcpy(osd_conf.strm[i].region, osd_conf.strm[chn].region, sizeof(osd_conf.strm[chn].region));
	}

	if (aux_set_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__CreateOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__CreateOSD(struct soap *soap, struct _tr2__CreateOSD *tr2__CreateOSD,
                                           struct _tr2__CreateOSDResponse *tr2__CreateOSDResponse)
{
	int i = 0;
	int chn = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_OSD_CONF_S osd_conf;
	struct tt__OSDConfiguration *osd;

	if (!tr2__CreateOSD->OSD || !tr2__CreateOSD->OSD->token) {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__tr2__CreateOSD\n");

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	osd = tr2__CreateOSD->OSD;

	if (osd->Type == tt__OSDType__Text && osd->TextString && !strcmp(osd->TextString->Type, "Plain")) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_TEXT) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->x) {
			osd_conf.strm[chn].region[i].start_x = ((*osd->Position->Pos->x * 100) + 100) / 2;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->y) {
			osd_conf.strm[chn].region[i].start_y = abs(((*osd->Position->Pos->y * 100) - 100) / 2);
		}

		if (osd->TextString && osd->TextString->PlainText) {
			osd_conf.strm[chn].region[i].enabled = 1;
			strncpy((char *)osd_conf.strm[chn].region[i].type_spec, osd->TextString->PlainText,
			        MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE);
		}
		tr2__CreateOSDResponse->OSDToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(tr2__CreateOSDResponse->OSDToken, "osdtoken_txt");
	} else if (osd->Type == tt__OSDType__Text && osd->TextString && !strcmp(osd->TextString->Type, "DateAndTime")) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_INFO) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		osd_conf.strm[chn].region[i].enabled = 1;

		if (osd->TextString && osd->TextString->DateFormat) {
			ONVIF_TRACE("Create OSD: Date format: %s \n", osd->TextString->DateFormat);
			if (strcmp(osd->TextString->DateFormat, "MM/dd/yyyy") == 0) {
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "MM-DD-YYYY");
			} else if (strcmp(osd->TextString->DateFormat, "dd/MM/yyyy") == 0) {
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "DD-MM-YYYY");
			} else if (strcmp(osd->TextString->DateFormat, "yyyy/MM/dd") == 0) {
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "YYYY-MM-DD");
			} else {
				strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "YYYY-MM-DD");
			}
		} else {
			strcpy((char *)osd_conf.strm[chn].region[i].type_spec, "YYYY-MM-DD");
		}

		if (osd->TextString && osd->TextString->TimeFormat) {
			if (strcmp(osd->TextString->TimeFormat, "h:mm:ss tt") == 0) { //2:14:21 PM

			} else if (strcmp(osd->TextString->TimeFormat, "hh:mm:ss tt") == 0) { //02:14:21 PM

			} else if (strcmp(osd->TextString->TimeFormat, "H:mm:ss") == 0) { //14:14:21

			} else if (strcmp(osd->TextString->TimeFormat, "HH:mm:ss") == 0) { //14:14:21

			} else {
			}
		} else {
		}
		tr2__CreateOSDResponse->OSDToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(tr2__CreateOSDResponse->OSDToken, "osdtoken_info");
	} else if (strcmp(osd->token, "osdtoken_002") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_IMAGE) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		if (osd->Image && osd->Image->ImgPath) {
			strncpy((char *)osd_conf.strm[chn].region[i].type_spec, osd->Image->ImgPath,
			        MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE);
		}
		if (osd->Position && osd->Position->Pos && osd->Position->Pos->x) {
			osd_conf.strm[chn].region[i].start_x = ((*osd->Position->Pos->x * 100) + 100) / 2;
		}

		if (osd->Position && osd->Position->Pos && osd->Position->Pos->y) {
			osd_conf.strm[chn].region[i].start_y = abs(((*osd->Position->Pos->y * 100) - 100) / 2);
		}
		osd_conf.strm[chn].region[i].enabled = 1;
	}

	/*Mirror channel 0 to other channel*/
	for (i = 1; (unsigned)i < strm.video_strm_cnt; i++) {
		memcpy(osd_conf.strm[i].region, osd_conf.strm[chn].region, sizeof(osd_conf.strm[chn].region));
	}

	if (aux_set_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__DeleteOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__DeleteOSD(struct soap *soap, struct _tr2__DeleteOSD *tr2__DeleteOSD,
                                           struct tr2__SetConfigurationResponse *tr2__DeleteOSDResponse)
{
	int i = 0;
	int chn = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_OSD_CONF_S osd_conf;

	if (!tr2__DeleteOSD->OSDToken) {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__tr2__DeleteOSD \n");

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	if (strcmp(tr2__DeleteOSD->OSDToken, "TEXTOSD") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_TEXT) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		osd_conf.strm[chn].region[i].enabled = 0;
	} else if (strcmp(tr2__DeleteOSD->OSDToken, "INFOOSD") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_INFO) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		osd_conf.strm[chn].region[i].enabled = 0;
	} else if (strcmp(tr2__DeleteOSD->OSDToken, "IMGOSD") == 0) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd_conf.strm[chn].region[i].type == AGTX_OSD_TYPE_IMAGE) {
				break;
			}
		}

		if (i >= MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE) {
			ONVIF_TRACE("No OSD type match\n");
			return SOAP_FAULT;
		}

		osd_conf.strm[chn].region[i].enabled = 0;
	} else {
		ONVIF_TRACE("Unknown token\n");
		return SOAP_FAULT;
	}

	/*Mirror channel 0 to other channel*/
	for (i = 1; (unsigned)i < strm.video_strm_cnt; i++) {
		memcpy(osd_conf.strm[i].region, osd_conf.strm[chn].region, sizeof(osd_conf.strm[chn].region));
	}

	if (aux_set_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
}
/** Web service operation '__tr2__GetMasks' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetMasks(struct soap *soap, struct _tr2__GetMasks *tr2__GetMasks,
                                          struct _tr2__GetMasksResponse *tr2__GetMasksResponse)
{
	ONVIF_TRACE("__tr2__GetMasks TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__GetMaskOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__GetMaskOptions(struct soap *soap, struct _tr2__GetMaskOptions *tr2__GetMaskOptions,
                                                struct _tr2__GetMaskOptionsResponse *tr2__GetMaskOptionsResponse)
{
	ONVIF_TRACE("__tr2__GetMaskOptions TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__SetMask' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__SetMask(struct soap *soap, struct _tr2__SetMask *tr2__SetMask,
                                         struct tr2__SetConfigurationResponse *tr2__SetMaskResponse)
{
	ONVIF_TRACE("__tr2__SetMask TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__CreateMask' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__CreateMask(struct soap *soap, struct _tr2__CreateMask *tr2__CreateMask,
                                            struct _tr2__CreateMaskResponse *tr2__CreateMaskResponse)
{
	ONVIF_TRACE("__tr2__CreateMask TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tr2__DeleteMask' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tr2__DeleteMask(struct soap *soap, struct _tr2__DeleteMask *tr2__DeleteMask,
                                            struct tr2__SetConfigurationResponse *tr2__DeleteMaskResponse)
{
	ONVIF_TRACE("__tr2__DeleteMask TODO\n");
	return SOAP_OK;
}
