#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "augentix.h"

int waitFlag = 0;
const int portNumber[3] = {9554, 7554, 6554};

/** Web service operation '__trt__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__GetServiceCapabilities(struct soap *soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities,
                              struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
	struct trt__Capabilities *cap = NULL;

	ONVIF_TRACE("__trt__GetServiceCapabilities\n");

	trt__GetServiceCapabilitiesResponse->Capabilities = aux_onvif_malloc(soap, sizeof(struct trt__Capabilities));
	cap = trt__GetServiceCapabilitiesResponse->Capabilities;
	cap->SnapshotUri = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->SnapshotUri = xsd__boolean__true_;
	cap->OSD = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->OSD = xsd__boolean__true_;
	cap->ProfileCapabilities = aux_onvif_malloc(soap, sizeof(struct trt__ProfileCapabilities));
	cap->ProfileCapabilities->MaximumNumberOfProfiles = aux_onvif_malloc(soap, sizeof(int));
	*cap->ProfileCapabilities->MaximumNumberOfProfiles = 3;
	cap->StreamingCapabilities = aux_onvif_malloc(soap, sizeof(struct trt__StreamingCapabilities));
	cap->StreamingCapabilities->RTP_USCORETCP = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
	cap->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*cap->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;

	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoSources' (returns SOAP_OK or error code) */
/*  List all available physical video inputs of the device*/
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources,
                                                 struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	int i = 0;
	int tmpfps = 0;
	int tmpWid = 0;
	int tmpHeight = 0;
	int brightness = 0;
	int saturation = 0;
	int contrast = 0;
	int sharpness = 0;
	AGTX_IMG_PREF_S img = { 0 };
	AGTX_DEV_CONF_S dev = { 0 };

	ONVIF_TRACE("__trt__GetVideoSources\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	/*Get image config*/
	if (aux_get_cc_config(AGTX_CMD_IMG_PREF, &img) < 0) {
		return SOAP_NO_DATA;
	}

	/*Get DEV config*/
	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_NO_DATA;
	}

	brightness = img.brightness;
	saturation = img.saturation;
	contrast = img.contrast;
	sharpness = img.sharpness;
	tmpfps = dev.input_fps;
	tmpWid = dev.input_path[0].width;
	tmpHeight = dev.input_path[0].height;

	trt__GetVideoSourcesResponse->VideoSources_ = soap_malloc(soap, sizeof(struct tt__VideoSource));
	memset(trt__GetVideoSourcesResponse->VideoSources_, 0x00, sizeof(struct tt__VideoSource));
	trt__GetVideoSourcesResponse->VideoSources_->token = soap_malloc(soap, MAX_STR_LEN);
	memset(trt__GetVideoSourcesResponse->VideoSources_->token, 0x00, MAX_STR_LEN);
	sprintf(trt__GetVideoSourcesResponse->VideoSources_->token, "0%02d", i);
	trt__GetVideoSourcesResponse->VideoSources_->Framerate = tmpfps;
	trt__GetVideoSourcesResponse->VideoSources_->Resolution = soap_malloc(soap, sizeof(struct tt__VideoResolution));
	trt__GetVideoSourcesResponse->VideoSources_->Resolution->Width = tmpWid;
	trt__GetVideoSourcesResponse->VideoSources_->Resolution->Height = tmpHeight;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging = soap_malloc(soap, sizeof(struct tt__ImagingSettings));
	memset(trt__GetVideoSourcesResponse->VideoSources_->Imaging, 0x00, sizeof(struct tt__ImagingSettings));
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Brightness = soap_malloc(soap, sizeof(float));
	*trt__GetVideoSourcesResponse->VideoSources_->Imaging->Brightness = brightness;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->ColorSaturation = soap_malloc(soap, sizeof(float));
	*trt__GetVideoSourcesResponse->VideoSources_->Imaging->ColorSaturation = saturation;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Contrast = soap_malloc(soap, sizeof(float));
	*trt__GetVideoSourcesResponse->VideoSources_->Imaging->Contrast = contrast;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Focus =
	        soap_malloc(soap, sizeof(struct tt__FocusConfiguration));
	memset(trt__GetVideoSourcesResponse->VideoSources_->Imaging->Focus, 0x00,
	       sizeof(struct tt__FocusConfiguration));
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Focus->AutoFocusMode = tt__AutoFocusMode__MANUAL;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Focus->DefaultSpeed = 1;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Focus->FarLimit = 1;
	trt__GetVideoSourcesResponse->VideoSources_->Imaging->Sharpness = soap_malloc(soap, sizeof(float));
	*trt__GetVideoSourcesResponse->VideoSources_->Imaging->Sharpness = sharpness;
	trt__GetVideoSourcesResponse->__sizeVideoSources = 1;

	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioSources' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources,
                                                 struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	ONVIF_TRACE("__trt__GetAudioSources TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioOutputs' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs,
                                                 struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	ONVIF_TRACE("__trt__GetAudioOutputs TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__CreateProfile' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile,
                                               struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
	ONVIF_TRACE("__trt__CreateProfile TODO\n");
	return SOAP_OK;
};

/** Web service operation '__trt__GetProfile' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile,
                                            struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	int num = 0;
	int tmpHeight = 0;
	int audio_en = 0;
	int audio_bits = 0;
	int audio_freq = 0;
	AGTX_AUDIO_CODEC_E audio_code = 0;
	struct tt__VideoEncoderConfiguration *venc_cfg = NULL;
	struct tt__VideoSourceConfiguration *vsc_cfg = NULL;
	struct tt__AudioSourceConfiguration *audio_cfg = NULL;
	struct tt__AudioEncoderConfiguration *aenc_cfg = NULL;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_AUDIO_CONF_S audio = { 0 };

	ONVIF_TRACE("__trt__GetProfile\n");

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
	audio_bits = audio.sampling_bit;
	audio_freq = audio.sampling_frequency;
	audio_code = audio.codec;

	if (trt__GetProfile->ProfileToken) {
		num = atoi(trt__GetProfile->ProfileToken);
	} else {
		return SOAP_ERR;
	}

	printf("ProfileToken %s i %d \n", trt__GetProfile->ProfileToken, num);

	if (!strm.video_strm[num].strm_en) {
		return SOAP_FAULT;
	}

	trt__GetProfileResponse->Profile = aux_onvif_malloc(soap, sizeof(struct tt__Profile));
	trt__GetProfileResponse->Profile->fixed = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
	*(trt__GetProfileResponse->Profile->fixed) = xsd__boolean__true_;

	trt__GetProfileResponse->Profile->token = aux_onvif_malloc(soap, MAX_STR_LEN);
	sprintf(trt__GetProfileResponse->Profile->token, "0%02d", num);

	trt__GetProfileResponse->Profile->Name = (char *)aux_onvif_malloc(soap, MAX_STR_LEN);
	sprintf(trt__GetProfileResponse->Profile->Name, "Profile_%s", trt__GetProfileResponse->Profile->token);

	/*VideoSourceConfiguration*/
	trt__GetProfileResponse->Profile->VideoSourceConfiguration =
	        aux_onvif_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
	vsc_cfg = trt__GetProfileResponse->Profile->VideoSourceConfiguration;
	vsc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(vsc_cfg->token, "000");

	vsc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(vsc_cfg->Name, "VideoS_000");

	vsc_cfg->UseCount = 3;
	vsc_cfg->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(vsc_cfg->SourceToken, trt__GetProfileResponse->Profile->token);

	vsc_cfg->Bounds = aux_onvif_malloc(soap, sizeof(struct tt__IntRectangle));
	vsc_cfg->Bounds->x = 0;
	vsc_cfg->Bounds->y = 0;
	vsc_cfg->Bounds->width = dev.input_path[0].width;
	vsc_cfg->Bounds->height = dev.input_path[0].height;

	/*FIXME onlu support single stream now.*/
	if (audio_en && num == 0) {
		/*AudioSourceConfiguration*/
		trt__GetProfileResponse->Profile->AudioSourceConfiguration =
		        aux_onvif_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
		audio_cfg = trt__GetProfileResponse->Profile->AudioSourceConfiguration;
		audio_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(audio_cfg->token, trt__GetProfileResponse->Profile->token);
		audio_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(audio_cfg->Name, "Audio_%s", trt__GetProfileResponse->Profile->token);
		audio_cfg->UseCount = 2;
		audio_cfg->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(audio_cfg->SourceToken, trt__GetProfileResponse->Profile->token);
	}

	/*VideoEncoderConfiguration*/
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration =
	        aux_onvif_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
	venc_cfg = trt__GetProfileResponse->Profile->VideoEncoderConfiguration;
	venc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(venc_cfg->token, trt__GetProfileResponse->Profile->token);
	venc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
	sprintf(venc_cfg->Name, "VideoE_%s", trt__GetProfileResponse->Profile->token);
	venc_cfg->UseCount = 1;
	venc_cfg->Encoding = tt__VideoEncoding__H264;
	venc_cfg->Resolution = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution));

	tmpHeight = strm.video_strm[num].height;
	if (dev.stitch_en) {
		venc_cfg->Resolution->Height = (strm.video_strm[num].width == 3840 && tmpHeight == 1080) ?
		                                       2160 :
		                                       (tmpHeight == 720) ?
		                                       1440 :
		                                       (tmpHeight == 544) ?
		                                       1080 :
		                                       (tmpHeight == 272) ? 540 : (tmpHeight == 184) ? 360 : tmpHeight;
	} else {
		venc_cfg->Resolution->Height = tmpHeight;
	}
	venc_cfg->Resolution->Width = strm.video_strm[num].width;
	venc_cfg->Quality = strm.video_strm[num].vbr_quality_level_index;
	venc_cfg->RateControl = aux_onvif_malloc(soap, sizeof(struct tt__VideoRateControl));
	venc_cfg->RateControl->FrameRateLimit = strm.video_strm[num].output_fps;
	venc_cfg->RateControl->EncodingInterval = 1;

	if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_VBR) {
		venc_cfg->RateControl->BitrateLimit = strm.video_strm[num].vbr_max_bit_rate;
	} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_CBR) {
		venc_cfg->RateControl->BitrateLimit = strm.video_strm[num].bit_rate;
	} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_SBR) {
		venc_cfg->RateControl->BitrateLimit = strm.video_strm[num].bit_rate;
	} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_CQP) {
		venc_cfg->RateControl->BitrateLimit = INT_MAX;
	} else {
		venc_cfg->RateControl->BitrateLimit = strm.video_strm[num].bit_rate;
	}

	venc_cfg->H264 = aux_onvif_malloc(soap, sizeof(struct tt__H264Configuration));
	venc_cfg->H264->GovLength = strm.video_strm[num].gop_size;
	venc_cfg->H264->H264Profile = tt__H264Profile__Baseline;

	venc_cfg->Multicast = aux_onvif_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	venc_cfg->Multicast->Address = aux_onvif_malloc(soap, sizeof(struct tt__IPAddress));
	venc_cfg->Multicast->Address->Type = tt__IPType__IPv4;
	venc_cfg->Multicast->Address->IPv4Address = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(venc_cfg->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
	venc_cfg->SessionTimeout = 10000; //TODO

	/*FIXME onlu support single stream now.*/
	if (audio_en && num == 0) {
		/*AudioEncoderConfiguration*/
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration =
		        aux_onvif_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));
		aenc_cfg = trt__GetProfileResponse->Profile->AudioEncoderConfiguration;
		aenc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(aenc_cfg->token, trt__GetProfileResponse->Profile->token);
		aenc_cfg->Name = aux_onvif_malloc(soap, sizeof(char) * MAX_STR_LEN);
		sprintf(aenc_cfg->Name, "AudioE_%s", trt__GetProfileResponse->Profile->token);
		aenc_cfg->UseCount = 2;
		if (audio_code == AGTX_AUDIO_CODEC_ULAW || audio_code == AGTX_AUDIO_CODEC_ALAW) {
			aenc_cfg->Encoding = tt__AudioEncoding__G711;
		} else if (audio_code == AGTX_AUDIO_CODEC_G726) {
			aenc_cfg->Encoding = tt__AudioEncoding__G726;
		} else {
			ONVIF_TRACE("onvif doesn't support audio format %d\n", audio_code);
			return soap_sender_fault(soap, "Wrong audio format ", NULL);
		}
		aenc_cfg->Encoding = tt__AudioEncoding__G711;
		aenc_cfg->Bitrate = audio_freq * audio_bits;
		aenc_cfg->SampleRate = audio_freq;
		aenc_cfg->Multicast = aux_onvif_malloc(soap, sizeof(struct tt__MulticastConfiguration));
		aenc_cfg->Multicast->Address = aux_onvif_malloc(soap, sizeof(struct tt__IPAddress));
		aenc_cfg->Multicast->Address->Type = tt__IPType__IPv4;
		aenc_cfg->Multicast->Address->IPv4Address = aux_onvif_malloc(soap, sizeof(char) * MAX_STR_LEN);
		strcpy(aenc_cfg->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
		aenc_cfg->SessionTimeout = 10000; //TODO
	}

	/*VideoAnalyticsConfiguration*/
	if (num == 0) {
		if (aux_get_iva_setting(soap, &trt__GetProfileResponse->Profile->VideoAnalyticsConfiguration) !=
		    SOAP_OK)
			return SOAP_FAULT;
	}

	ONVIF_TRACE("__trt__GetProfile\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetProfiles' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles,
                                             struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
	int i = 0;
	int tmpWidth = 0;
	int tmpHeight = 0;
	int tmpFps = 0;
	int tmpGov = 0;
	int tmpQuality = 0;
	int tmpRcMode __attribute__((unused)) = 0;
	int tmpVbrBitrate = 0;
	int tmpBitrate = 0;
	int stream_num = 0;
	int audio_en = 0;
	int audio_bits = 0;
	int audio_freq = 0;
	AGTX_AUDIO_CODEC_E audio_code = 0;
	struct tt__VideoEncoderConfiguration *venc_cfg = NULL;
	struct tt__VideoSourceConfiguration *vsc_cfg = NULL;
	struct tt__AudioSourceConfiguration *audio_cfg = NULL;
	struct tt__AudioEncoderConfiguration *aenc_cfg = NULL;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_AUDIO_CONF_S audio = { 0 };

	ONVIF_TRACE("__trt__GetProfiles\n");

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
	audio_bits = audio.sampling_bit;
	audio_freq = audio.sampling_frequency;
	audio_code = audio.codec;
	stream_num = strm.video_strm_cnt;

	trt__GetProfilesResponse->Profiles_ = aux_onvif_malloc(soap, sizeof(struct tt__Profile) * stream_num);
	trt__GetProfilesResponse->__sizeProfiles = stream_num;

	for (i = 0; i < stream_num; i++) {
		if (!strm.video_strm[i].strm_en) {
			continue;
		}

		tmpWidth = strm.video_strm[i].width;
		tmpHeight = strm.video_strm[i].height;
		tmpFps = strm.video_strm[i].output_fps;
		tmpGov = strm.video_strm[i].gop_size;
		tmpQuality = strm.video_strm[i].vbr_quality_level_index;
		tmpRcMode = strm.video_strm[i].rc_mode;
		tmpVbrBitrate = strm.video_strm[i].vbr_max_bit_rate;
		tmpBitrate = strm.video_strm[i].bit_rate;

		trt__GetProfilesResponse->Profiles_[i].fixed = aux_onvif_malloc(soap, sizeof(enum xsd__boolean));
		*(trt__GetProfilesResponse->Profiles_[i].fixed) = xsd__boolean__true_;

		trt__GetProfilesResponse->Profiles_[i].token = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(trt__GetProfilesResponse->Profiles_[i].token, "0%02d", i);

		trt__GetProfilesResponse->Profiles_[i].Name = (char *)aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(trt__GetProfilesResponse->Profiles_[i].Name, "Profile_%s",
		        trt__GetProfilesResponse->Profiles_[i].token);

		/*VideoSourceConfiguration*/
		trt__GetProfilesResponse->Profiles_[i].VideoSourceConfiguration =
		        aux_onvif_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));

		vsc_cfg = trt__GetProfilesResponse->Profiles_[i].VideoSourceConfiguration;
		vsc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(vsc_cfg->token, "000");

		vsc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(vsc_cfg->Name, "VideoS_000");

		vsc_cfg->UseCount = 3; //stream_num;

		vsc_cfg->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(vsc_cfg->SourceToken, vsc_cfg->token);

		vsc_cfg->Bounds = aux_onvif_malloc(soap, sizeof(struct tt__IntRectangle));
		vsc_cfg->Bounds->x = 0;
		vsc_cfg->Bounds->y = 0;
		vsc_cfg->Bounds->width = dev.input_path[0].width;
		;
		vsc_cfg->Bounds->height = dev.input_path[0].height;

		/*FIXME onlu support single stream now.*/
		if (audio_en && i == 0) {
			/*AudioSourceConfiguration*/
			trt__GetProfilesResponse->Profiles_[i].AudioSourceConfiguration =
			        aux_onvif_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
			audio_cfg = trt__GetProfilesResponse->Profiles_[i].AudioSourceConfiguration;

			audio_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy(audio_cfg->token, "000");
			audio_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(audio_cfg->Name, "Audio_%s", audio_cfg->token);
			audio_cfg->UseCount = 2;
			audio_cfg->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy(audio_cfg->SourceToken, audio_cfg->token);
		}

		/*VideoEncoderConfiguration*/
		trt__GetProfilesResponse->Profiles_[i].VideoEncoderConfiguration =
		        aux_onvif_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));

		venc_cfg = trt__GetProfilesResponse->Profiles_[i].VideoEncoderConfiguration;
		venc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(venc_cfg->token, trt__GetProfilesResponse->Profiles_[i].token);
		venc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(venc_cfg->Name, "VideoE_%s", trt__GetProfilesResponse->Profiles_[i].token);
		venc_cfg->UseCount = 1;
		venc_cfg->Encoding = tt__VideoEncoding__H264;
		venc_cfg->Resolution = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution));

		if (dev.stitch_en) {
			venc_cfg->Resolution->Height =
			        (tmpWidth == 3840 && tmpHeight == 1080) ?
			                2160 :
			                (tmpHeight == 720) ?
			                1440 :
			                (tmpHeight == 544) ?
			                1080 :
			                (tmpHeight == 272) ? 540 : (tmpHeight == 184) ? 360 : tmpHeight;
		} else {
			venc_cfg->Resolution->Height = tmpHeight;
		}
		venc_cfg->Resolution->Width = tmpWidth;
		venc_cfg->Quality = tmpQuality;
		venc_cfg->RateControl = aux_onvif_malloc(soap, sizeof(struct tt__VideoRateControl));
		venc_cfg->RateControl->FrameRateLimit = tmpFps;
		venc_cfg->RateControl->EncodingInterval = 1;

		if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_VBR) {
			venc_cfg->RateControl->BitrateLimit = tmpVbrBitrate;
		} else if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_CBR) {
			venc_cfg->RateControl->BitrateLimit = tmpBitrate;
		} else if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_SBR) {
			venc_cfg->RateControl->BitrateLimit = tmpBitrate;
		} else if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_CQP) {
			venc_cfg->RateControl->BitrateLimit = INT_MAX;
		} else {
			venc_cfg->RateControl->BitrateLimit = tmpBitrate;
		}

		venc_cfg->H264 = aux_onvif_malloc(soap, sizeof(struct tt__H264Configuration));
		venc_cfg->H264->GovLength = tmpGov;
		venc_cfg->H264->H264Profile = tt__H264Profile__Baseline;
		venc_cfg->Multicast = aux_onvif_malloc(soap, sizeof(struct tt__MulticastConfiguration));
		venc_cfg->Multicast->Address = aux_onvif_malloc(soap, sizeof(struct tt__IPAddress));
		venc_cfg->Multicast->Address->Type = tt__IPType__IPv4;
		venc_cfg->Multicast->Address->IPv4Address = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(venc_cfg->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
		venc_cfg->SessionTimeout = 10000; //TODO

		/*FIXME onlu support single stream now.*/
		if (audio_en && i == 0) {
			/*AudioEncoderConfiguration*/
			trt__GetProfilesResponse->Profiles_[i].AudioEncoderConfiguration =
			        aux_onvif_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));

			aenc_cfg = trt__GetProfilesResponse->Profiles_[i].AudioEncoderConfiguration;
			aenc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy(aenc_cfg->token, trt__GetProfilesResponse->Profiles_[i].token);
			aenc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
			sprintf(aenc_cfg->Name, "AudioE_%s", trt__GetProfilesResponse->Profiles_[i].token);
			aenc_cfg->UseCount = 2;
			if (audio_code == AGTX_AUDIO_CODEC_ULAW || audio_code == AGTX_AUDIO_CODEC_ALAW) {
				aenc_cfg->Encoding = tt__AudioEncoding__G711;
			} else if (audio_code == AGTX_AUDIO_CODEC_G726) {
				aenc_cfg->Encoding = tt__AudioEncoding__G726;
			} else {
				ONVIF_TRACE("onvif doesn't support audio format %d\n", audio_code);
				return soap_sender_fault(soap, "Wrong audio format ", NULL);
			}
			aenc_cfg->Bitrate = audio_freq * audio_bits;
			aenc_cfg->SampleRate = audio_freq;

			aenc_cfg->Multicast = aux_onvif_malloc(soap, sizeof(struct tt__MulticastConfiguration));
			aenc_cfg->Multicast->Address = aux_onvif_malloc(soap, sizeof(struct tt__IPAddress));
			aenc_cfg->Multicast->Address->Type = tt__IPType__IPv4;
			aenc_cfg->Multicast->Address->IPv4Address = aux_onvif_malloc(soap, MAX_STR_LEN);
			strcpy(aenc_cfg->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
			aenc_cfg->SessionTimeout = 10000; //TODO
		}

		/*VideoAnalyticsConfiguration*/
		if (aux_get_iva_setting(soap, &trt__GetProfilesResponse->Profiles_[i].VideoAnalyticsConfiguration) !=
		    SOAP_OK) {
			return SOAP_FAULT;
		}
	}
	return SOAP_OK;
};
/** Web service operation '__trt__AddVideoEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(
        struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration,
        struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddVideoEncoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__AddVideoSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(
        struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration,
        struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddVideoSourceConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__AddAudioEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(
        struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration,
        struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
	AGTX_AUDIO_CONF_S audio = { 0 };

	ONVIF_TRACE("__trt__AddAudioEncoderConfiguration %s %s\n",
	            trt__AddAudioEncoderConfiguration->ConfigurationToken,
	            trt__AddAudioEncoderConfiguration->ProfileToken);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	audio.enabled = 1;

	if (aux_set_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
};
/** Web service operation '__trt__AddAudioSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(
        struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration,
        struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddAudioSourceConfiguration %s %s\n", trt__AddAudioSourceConfiguration->ConfigurationToken,
	            trt__AddAudioSourceConfiguration->ProfileToken);
	return SOAP_OK;
};
/** Web service operation '__trt__AddPTZConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration,
                           struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddPTZConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__AddVideoAnalyticsConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(
        struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration,
        struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddVideoAnalyticsConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__AddMetadataConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration,
                                struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddMetadataConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__AddAudioOutputConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(
        struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration,
        struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddAudioOutputConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__AddAudioDecoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(
        struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration,
        struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__AddAudioDecoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveVideoEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(
        struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration,
        struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveVideoEncoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveVideoSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(
        struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration,
        struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveVideoSourceConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveAudioEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(
        struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration,
        struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
	AGTX_AUDIO_CONF_S audio = { 0 };

	ONVIF_TRACE("__trt__RemoveAudioEncoderConfiguration %s\n", trt__RemoveAudioEncoderConfiguration->ProfileToken);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	audio.enabled = 0;

	if (aux_set_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
};
/** Web service operation '__trt__RemoveAudioSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(
        struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration,
        struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveAudioSourceConfiguration %s\n", trt__RemoveAudioSourceConfiguration->ProfileToken);

	return SOAP_OK;
};
/** Web service operation '__trt__RemovePTZConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration,
                              struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemovePTZConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveVideoAnalyticsConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(
        struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration,
        struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveVideoAnalyticsConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveMetadataConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(
        struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration,
        struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveMetadataConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveAudioOutputConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(
        struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration,
        struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveAudioOutputConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__RemoveAudioDecoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(
        struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration,
        struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__RemoveAudioDecoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__DeleteProfile' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile,
                                               struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
	ONVIF_TRACE("__trt__DeleteProfile TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoSourceConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(
        struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations,
        struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
	int numOfConfs = 0;
	AGTX_DEV_CONF_S dev = { 0 };
	struct tt__VideoSourceConfiguration *vsc_cfg = NULL;

	ONVIF_TRACE("__trt__GetVideoSourceConfigurations \n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}

	numOfConfs = dev.input_path_cnt;

	trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = numOfConfs;

	trt__GetVideoSourceConfigurationsResponse->Configurations_ =
	        aux_onvif_malloc(soap, sizeof(struct tt__VideoSourceConfiguration) * numOfConfs);

	for (int i = 0; i < numOfConfs; i++) {
		vsc_cfg = &trt__GetVideoSourceConfigurationsResponse->Configurations_[i];

		vsc_cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(trt__GetVideoSourceConfigurationsResponse->Configurations_[i].token, "00%d", i);

		vsc_cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(vsc_cfg->Name, "VideoS_00%d", i);

		vsc_cfg->UseCount = 3; //TODO
		vsc_cfg->SourceToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(vsc_cfg->SourceToken, "00%d", i);

		vsc_cfg->Bounds = aux_onvif_malloc(soap, sizeof(struct tt__IntRectangle));
		vsc_cfg->Bounds->x = 0;
		vsc_cfg->Bounds->y = 0;
		vsc_cfg->Bounds->width = dev.input_path[i].width;
		vsc_cfg->Bounds->height = dev.input_path[i].height;
	}

	ONVIF_TRACE("__trt__GetVideoSourceConfigurations: Done \n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoEncoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(
        struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations,
        struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
	int i = 0;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };
	struct tt__VideoEncoderConfiguration *cfg = NULL;
	int tmpWid, tmpHeight;

	ONVIF_TRACE("__trt__GetVideoEncoderConfigurations\n");
	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}
	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}
	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_USER_ERROR;
	}

	trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = strm.video_strm_cnt;
	trt__GetVideoEncoderConfigurationsResponse->Configurations_ =
	        aux_onvif_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration) * strm.video_strm_cnt);

	for (i = 0; (unsigned)i < strm.video_strm_cnt; i++) {
		if (!strm.video_strm[i].strm_en) {
			continue;
		}

		cfg = &trt__GetVideoEncoderConfigurationsResponse->Configurations_[i];
		cfg->token = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(cfg->token, "0%02d", i);
		cfg->Name = aux_onvif_malloc(soap, MAX_STR_LEN);
		sprintf(cfg->Name, "VideoE_%s", cfg->token);
		cfg->UseCount = 1;
		cfg->Encoding = tt__VideoEncoding__H264;
		cfg->Resolution = aux_onvif_malloc(soap, sizeof(struct tt__VideoResolution));
		tmpWid = strm.video_strm[i].width;
		tmpHeight = strm.video_strm[i].height;
		cfg->Resolution->Width = (tmpWid >= 0) ? tmpWid : 0;

		if (dev.stitch_en) {
			cfg->Resolution->Height =
			        (tmpWid == 3840 && tmpHeight == 1080) ?
			                2160 :
			                (tmpHeight == 720) ?
			                1440 :
			                (tmpHeight == 544) ?
			                1080 :
			                (tmpHeight == 272) ? 540 : (tmpHeight == 184) ? 360 : tmpHeight;
		} else {
			cfg->Resolution->Height = tmpHeight;
		}

		cfg->Quality = strm.video_strm[i].vbr_quality_level_index;
		cfg->RateControl = aux_onvif_malloc(soap, sizeof(struct tt__VideoRateControl));
		cfg->RateControl->FrameRateLimit = strm.video_strm[i].output_fps;
		cfg->RateControl->EncodingInterval = 1;

		if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_VBR) {
			cfg->RateControl->BitrateLimit = strm.video_strm[i].vbr_max_bit_rate;
		} else if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_CBR) {
			cfg->RateControl->BitrateLimit = strm.video_strm[i].bit_rate;
		} else if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_SBR) {
			cfg->RateControl->BitrateLimit = strm.video_strm[i].bit_rate;
		} else if (strm.video_strm[i].rc_mode == AGTX_RC_MODE_CQP) {
			cfg->RateControl->BitrateLimit = INT_MAX;
		} else {
			cfg->RateControl->BitrateLimit = strm.video_strm[i].bit_rate;
			ONVIF_TRACE("Unknown rc mode %d\n", strm.video_strm[i].rc_mode);
			return SOAP_USER_ERROR;
		}

		cfg->H264 = aux_onvif_malloc(soap, sizeof(struct tt__H264Configuration));
		cfg->H264->GovLength = strm.video_strm[i].gop_size;
		cfg->H264->H264Profile = strm.video_strm[i].venc_profile == AGTX_PRFL_BASELINE ?
		                                 tt__H264Profile__Baseline :
		                                 strm.video_strm[i].venc_profile == AGTX_PRFL_MAIN ?
		                                 tt__H264Profile__Main :
		                                 strm.video_strm[i].venc_profile == AGTX_PRFL_HIGH ?
		                                 tt__H264Profile__High :
		                                 tt__H264Profile__High;
		cfg->Multicast = aux_onvif_malloc(soap, sizeof(struct tt__MulticastConfiguration));
		cfg->Multicast->Address = aux_onvif_malloc(soap, sizeof(struct tt__IPAddress));
		cfg->Multicast->Address->Type = tt__IPType__IPv4;
		cfg->Multicast->Address->IPv4Address = aux_onvif_malloc(soap, sizeof(char) * MAX_STR_LEN);
		strcpy(cfg->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
		cfg->SessionTimeout = 10; //TODO
	}

	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioSourceConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(
        struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations,
        struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
	struct tt__AudioSourceConfiguration *audio_conifg = NULL;
	char token[] = { "000" };

	ONVIF_TRACE("__trt__GetAudioSourceConfigurations TODO\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	/*AudioSourceConfiguration*/
	trt__GetAudioSourceConfigurationsResponse->__sizeConfigurations = 1;
	trt__GetAudioSourceConfigurationsResponse->Configurations_ =
	        soap_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
	memset(trt__GetAudioSourceConfigurationsResponse->Configurations_, 0,
	       sizeof(struct tt__AudioSourceConfiguration));
	audio_conifg = trt__GetAudioSourceConfigurationsResponse->Configurations_;
	audio_conifg->token = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_conifg->token, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_conifg->token, token);
	audio_conifg->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_conifg->Name, 0x00, sizeof(char) * MAX_STR_LEN);
	sprintf(audio_conifg->Name, "Audio_%s", token);
	audio_conifg->UseCount = 2;
	audio_conifg->SourceToken = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_conifg->SourceToken, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_conifg->SourceToken, token);

	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioEncoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(
        struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations,
        struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
	int sample_bit = 0;
	int sample_freq = 0;
	AGTX_AUDIO_CODEC_E code_type = 0;
	AGTX_AUDIO_CONF_S audio = { 0 };

	struct tt__AudioEncoderConfiguration *audio_enc;

	ONVIF_TRACE("__trt__GetAudioEncoderConfigurations TODO\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	code_type = audio.codec;
	sample_bit = audio.sampling_bit;
	sample_freq = audio.sampling_frequency;

	trt__GetAudioEncoderConfigurationsResponse->__sizeConfigurations = 1;
	trt__GetAudioEncoderConfigurationsResponse->Configurations_ =
	        soap_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));
	memset(trt__GetAudioEncoderConfigurationsResponse->Configurations_, 0x00,
	       sizeof(struct tt__AudioEncoderConfiguration));
	audio_enc = trt__GetAudioEncoderConfigurationsResponse->Configurations_;
	audio_enc->token = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_enc->token, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_enc->token, "000");
	audio_enc->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_enc->Name, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_enc->Name, "AudioE_000");

	if (code_type == AGTX_AUDIO_CODEC_ULAW || code_type == AGTX_AUDIO_CODEC_ALAW) {
		audio_enc->Encoding = tt__AudioEncoding__G711;
	} else if (code_type == AGTX_AUDIO_CODEC_G726) {
		audio_enc->Encoding = tt__AudioEncoding__G726;
	} else {
		ONVIF_TRACE("onvif doesn't support audio format %d\n", code_type);
		return soap_sender_fault(soap, "Wrong audio format ", NULL);
	}
	audio_enc->Bitrate = (sample_bit * sample_freq) / 1000;
	audio_enc->SampleRate = sample_freq / 1000;
	audio_enc->UseCount = 2;

	audio_enc->Multicast = soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	memset(audio_enc->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
	audio_enc->Multicast->Address = soap_malloc(soap, sizeof(struct tt__IPAddress));
	memset(audio_enc->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
	audio_enc->Multicast->Address->Type = tt__IPType__IPv4;
	audio_enc->Multicast->Address->IPv4Address = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_enc->Multicast->Address->IPv4Address, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_enc->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
	audio_enc->SessionTimeout = 10000; //TODO

	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoAnalyticsConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(
        struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations,
        struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
	int ret = 0;

	ONVIF_TRACE("__trt__GetVideoAnalyticsConfigurations\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	trt__GetVideoAnalyticsConfigurationsResponse->__sizeConfigurations = 1;
	ret = aux_get_iva_setting(soap, &trt__GetVideoAnalyticsConfigurationsResponse->Configurations_);

	return ret;
};
/** Web service operation '__trt__GetMetadataConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(
        struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations,
        struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetMetadataConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioOutputConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(
        struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations,
        struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetAudioOutputConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioDecoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(
        struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations,
        struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetAudioDecoderConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(
        struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration,
        struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
	int num = 0;
	AGTX_DEV_CONF_S dev = { 0 };

	ONVIF_TRACE("__trt__GetVideoSourceConfiguration\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (trt__GetVideoSourceConfiguration->ConfigurationToken) {
		num = atoi(trt__GetVideoSourceConfiguration->ConfigurationToken);
	} else {
		return SOAP_ERR;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}

	trt__GetVideoSourceConfigurationResponse->Configuration =
	        soap_malloc(soap, sizeof(struct tt__VideoSourceConfiguration));
	memset(trt__GetVideoSourceConfigurationResponse->Configuration, 0, sizeof(struct tt__VideoSourceConfiguration));

	trt__GetVideoSourceConfigurationResponse->Configuration->token = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetVideoSourceConfigurationResponse->Configuration->token, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->token,
	       trt__GetVideoSourceConfiguration->ConfigurationToken);

	trt__GetVideoSourceConfigurationResponse->Configuration->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetVideoSourceConfigurationResponse->Configuration->Name, 0x00, sizeof(char) * MAX_STR_LEN);
	sprintf(trt__GetVideoSourceConfigurationResponse->Configuration->Name, "VideoS_%s",
	        trt__GetVideoSourceConfiguration->ConfigurationToken);

	trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = 3; //TODO

	trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken,
	       trt__GetVideoSourceConfiguration->ConfigurationToken);

	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds =
	        soap_malloc(soap, sizeof(struct tt__IntRectangle));
	memset(trt__GetVideoSourceConfigurationResponse->Configuration->Bounds, 0, sizeof(struct tt__IntRectangle));
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x = 0;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y = 0;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width = dev.input_path[num].width;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height = dev.input_path[num].height;

	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(
        struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration,
        struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
	int width = 0;
	int height = 0;
	int num = 0;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (trt__GetVideoEncoderConfiguration->ConfigurationToken) {
		//ONVIF_TRACE (">>>> %s \n",trt__GetVideoEncoderConfiguration->ConfigurationToken);
		num = atoi(trt__GetVideoEncoderConfiguration->ConfigurationToken);
	} else {
		return SOAP_ERR;
	}

	if (num >= (int)strm.video_strm_cnt) {
		return soap_sender_fault(soap, "Unknown Resource", NULL);
	}

	trt__GetVideoEncoderConfigurationResponse->Configuration =
	        soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration));
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration, 0x00,
	       sizeof(struct tt__VideoEncoderConfiguration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->token = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->token, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->token,
	       trt__GetVideoEncoderConfiguration->ConfigurationToken);
	trt__GetVideoEncoderConfigurationResponse->Configuration->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, 0x00, sizeof(char) * MAX_STR_LEN);
	sprintf(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, "VideoE_%s",
	        trt__GetVideoEncoderConfiguration->ConfigurationToken);
	trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = 1;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = tt__VideoEncoding__H264; //TODO
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution =
	        soap_malloc(soap, sizeof(struct tt__VideoResolution));
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution, 0x00,
	       sizeof(struct tt__VideoResolution));

	width = strm.video_strm[num].width;
	height = strm.video_strm[num].height;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width = width;

	if (dev.stitch_en) {
		trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height =
		        (width == 3840 && height == 1080) ?
		                2160 :
		                (height == 720) ?
		                1440 :
		                (height == 544) ? 1080 : (height == 272) ? 540 : (height == 184) ? 360 : height;
	} else {
		trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height = height;
	}

	trt__GetVideoEncoderConfigurationResponse->Configuration->Quality =
	        strm.video_strm[num].vbr_quality_level_index;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl =
	        soap_malloc(soap, sizeof(struct tt__VideoRateControl));
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl, 0x00,
	       sizeof(struct tt__VideoRateControl));

	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit =
	        strm.video_strm[num].output_fps;

	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = 0; //1;//TODO

	if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_VBR) {
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit =
		        strm.video_strm[num].vbr_max_bit_rate;
	} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_CBR) {
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit =
		        strm.video_strm[num].bit_rate;
	} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_SBR) {
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit =
		        strm.video_strm[num].bit_rate;
	} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_CQP) {
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit = INT_MAX;
	} else {
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit =
		        strm.video_strm[num].bit_rate;
	}

	trt__GetVideoEncoderConfigurationResponse->Configuration->H264 =
	        soap_malloc(soap, sizeof(struct tt__H264Configuration));
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->H264, 0x00,
	       sizeof(struct tt__H264Configuration));

	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength = strm.video_strm[num].gop_size;
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = tt__H264Profile__Baseline;

	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast =
	        soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast, 0x00,
	       sizeof(struct tt__MulticastConfiguration));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address =
	        soap_malloc(soap, sizeof(struct tt__IPAddress));
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address, 0x00,
	       sizeof(struct tt__IPAddress));
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = tt__IPType__IPv4;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 0x00,
	       sizeof(char) * MAX_STR_LEN);
	strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address,
	       "224.1.2.3"); //TODO
	trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = 10; //TODO

	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(
        struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration,
        struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
	//int num = 0;

	ONVIF_TRACE("__trt__GetAudioSourceConfiguration TODO\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	trt__GetAudioSourceConfigurationResponse->Configuration =
	        soap_malloc(soap, sizeof(struct tt__AudioSourceConfiguration));
	memset(trt__GetAudioSourceConfigurationResponse->Configuration, 0x00,
	       sizeof(struct tt__AudioSourceConfiguration));

	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(
        struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration,
        struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
	int num = 0;
	int sample_bit = 0;
	int sample_freq = 0;
	AGTX_AUDIO_CODEC_E code_type = 0;
	AGTX_AUDIO_CONF_S audio = { 0 };
	struct tt__AudioEncoderConfiguration *audio_enc;

	if (trt__GetAudioEncoderConfiguration->ConfigurationToken) {
		num = atoi(trt__GetAudioEncoderConfiguration->ConfigurationToken);
	} else {
		return SOAP_ERR;
	}

	ONVIF_TRACE("__trt__GetAudioEncoderConfiguration %d\n", num);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	code_type = audio.codec;
	sample_bit = audio.sampling_bit;
	sample_freq = audio.sampling_frequency;

	trt__GetAudioEncoderConfigurationResponse->Configuration =
	        soap_malloc(soap, sizeof(struct tt__AudioEncoderConfiguration));
	memset(trt__GetAudioEncoderConfigurationResponse->Configuration, 0x00,
	       sizeof(struct tt__AudioEncoderConfiguration));
	audio_enc = trt__GetAudioEncoderConfigurationResponse->Configuration;
	audio_enc->token = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_enc->token, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_enc->token, "000");
	audio_enc->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_enc->Name, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_enc->Name, "AudioE_000");

	if (code_type == AGTX_AUDIO_CODEC_ULAW || code_type == AGTX_AUDIO_CODEC_ALAW) {
		audio_enc->Encoding = tt__AudioEncoding__G711;
	} else if (code_type == AGTX_AUDIO_CODEC_G726) {
		audio_enc->Encoding = tt__AudioEncoding__G726;
	} else {
		ONVIF_TRACE("onvif doesn't support audio format %d\n", code_type);
		return soap_sender_fault(soap, "Wrong audio format ", NULL);
	}
	audio_enc->Bitrate = (sample_bit * sample_freq) / 1000;
	audio_enc->SampleRate = sample_freq / 1000;
	audio_enc->UseCount = 2;

	audio_enc->Multicast = soap_malloc(soap, sizeof(struct tt__MulticastConfiguration));
	memset(audio_enc->Multicast, 0x00, sizeof(struct tt__MulticastConfiguration));
	audio_enc->Multicast->Address = soap_malloc(soap, sizeof(struct tt__IPAddress));
	memset(audio_enc->Multicast->Address, 0x00, sizeof(struct tt__IPAddress));
	audio_enc->Multicast->Address->Type = tt__IPType__IPv4;
	audio_enc->Multicast->Address->IPv4Address = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(audio_enc->Multicast->Address->IPv4Address, 0x00, sizeof(char) * MAX_STR_LEN);
	strcpy(audio_enc->Multicast->Address->IPv4Address, "224.1.2.3"); //TODO
	audio_enc->SessionTimeout = 10000; //TODO

	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoAnalyticsConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(
        struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration,
        struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
	ONVIF_TRACE("__trt__GetVideoAnalyticsConfiguration TODO\n");
	return SOAP_FAULT;
};
/** Web service operation '__trt__GetMetadataConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration,
                                struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
	ONVIF_TRACE("__trt__GetMetadataConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioOutputConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(
        struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration,
        struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
	ONVIF_TRACE("__trt__GetAudioOutputConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioDecoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(
        struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration,
        struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__GetAudioDecoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleVideoEncoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations,
        struct _trt__GetCompatibleVideoEncoderConfigurationsResponse
                *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleVideoEncoderConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleVideoSourceConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations,
        struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleVideoSourceConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleAudioEncoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations,
        struct _trt__GetCompatibleAudioEncoderConfigurationsResponse
                *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleAudioEncoderConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleAudioSourceConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations,
        struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleAudioSourceConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleVideoAnalyticsConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations,
        struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse
                *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleVideoAnalyticsConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleMetadataConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(
        struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations,
        struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleMetadataConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleAudioOutputConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations,
        struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleAudioOutputConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetCompatibleAudioDecoderConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(
        struct soap *soap,
        struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations,
        struct _trt__GetCompatibleAudioDecoderConfigurationsResponse
                *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
	ONVIF_TRACE("__trt__GetCompatibleAudioDecoderConfigurations TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetVideoSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(
        struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration,
        struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetVideoSourceConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetVideoEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(
        struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration,
        struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
	int num = 0;
	int height = 0;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_RES_OPTION_S *devResOptions = aux_get_res();

	struct tt__VideoEncoderConfiguration *enc = NULL;

	ONVIF_TRACE("__trt__SetVideoEncoderConfiguration\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_USER_ERROR;
	}

	enc = trt__SetVideoEncoderConfiguration->Configuration;

	if (enc->token) {
		//ONVIF_TRACE (">>>> trt__SetVideoEncoderConfiguration->Configuration->token num = %s \n", trt__SetVideoEncoderConfiguration->Configuration->token);
		num = atoi(enc->token);
	} else {
		return SOAP_ERR;
	}
	//validate FPS if it exists in the frame_rate_list
	if (enc->RateControl) {
		int tmpFrameRateLimit = (int)(enc->RateControl->FrameRateLimit);
		for (int i = 0; i < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; i++) {
			for (int idx = 0; idx < 60; idx++) {
				if (tmpFrameRateLimit == devResOptions->strm[num].res[i].frame_rate_list[idx]) {
					idx = 60;
					tmpFrameRateLimit = -1;
				}
			}
		}
		if (tmpFrameRateLimit != -1)
			return SOAP_TYPE;
	}
	//End of Validation

	if (enc->H264) {
		//sprintf(jsonCmd,"%s,'venc_profile':%d",jsonCmd,(int) (trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile));
		strm.video_strm[num].venc_profile =
		        (enc->H264->H264Profile == tt__H264Profile__Baseline) ?
		                AGTX_PRFL_BASELINE :
		                (enc->H264->H264Profile == tt__H264Profile__Main) ? AGTX_PRFL_MAIN : AGTX_PRFL_HIGH;
		strm.video_strm[num].gop_size = (int)(enc->H264->GovLength);
	}

	if (enc->RateControl) {
		if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_VBR) {
			strm.video_strm[num].vbr_max_bit_rate = enc->RateControl->BitrateLimit;
		} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_CBR) {
			strm.video_strm[num].bit_rate = enc->RateControl->BitrateLimit;
		} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_SBR) {
			strm.video_strm[num].bit_rate = enc->RateControl->BitrateLimit;
		} else if (strm.video_strm[num].rc_mode == AGTX_RC_MODE_CQP) {
			strm.video_strm[num].bit_rate = enc->RateControl->BitrateLimit;
		} else {
			ONVIF_TRACE("Unknown rc mode %d\n", strm.video_strm[num].rc_mode);
			strm.video_strm[num].bit_rate = enc->RateControl->BitrateLimit;
		}
		strm.video_strm[num].output_fps = enc->RateControl->FrameRateLimit;
	}

	if (enc->Resolution) {
		height = enc->Resolution->Height;
		if (dev.stitch_en) {
			if ((height == 1440) || (height == 2160) || (height == 540) || (height == 360)) {
				height = (((height / 2) + 7) >> 3) << 3;
			}
		}
		strm.video_strm[num].height = height;
		strm.video_strm[num].width = enc->Resolution->Width;
	}

	strm.video_strm[num].vbr_quality_level_index = enc->Quality;

	if (aux_set_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	return SOAP_OK;
};
/** Web service operation '__trt__SetAudioSourceConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(
        struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration,
        struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetAudioSourceConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetAudioEncoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(
        struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration,
        struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetAudioEncoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetVideoAnalyticsConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(
        struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration,
        struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetVideoAnalyticsConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetMetadataConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration,
                                struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetMetadataConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetAudioOutputConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(
        struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration,
        struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetAudioOutputConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetAudioDecoderConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(
        struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration,
        struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
	ONVIF_TRACE("__trt__SetAudioDecoderConfiguration TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoSourceConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(
        struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions,
        struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
	ONVIF_TRACE("__trt__GetVideoSourceConfigurationOptions TODO\n");

	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoEncoderConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(
        struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions,
        struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
	int num = 0;
	int h264profiles_nums = 1, availRes = 0;
	int idx = 0, fpsRange = 0, govMax = 0;
	AGTX_DEV_CONF_S dev = { 0 };
	AGTX_RES_OPTION_S *devResOptions = aux_get_res();
	AGTX_VENC_OPTION_S *devVencOptions = aux_get_venc();

	if (aux_get_cc_config(AGTX_CMD_VIDEO_DEV_CONF, &dev) < 0) {
		return SOAP_FAULT;
	}

	if (trt__GetVideoEncoderConfigurationOptions->ConfigurationToken) {
		num = atoi(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);
	} else {
		return SOAP_NO_DATA;
	}
	ONVIF_TRACE("__trt__GetVideoEncoderConfigurationOptions num %d\n", num);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	govMax = devVencOptions->strm[num].venc[0].max_gop_size;

	//TODO GET_OPTIONS FORM XXXX
	trt__GetVideoEncoderConfigurationOptionsResponse->Options =
	        soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options, 0x00,
	       sizeof(struct tt__VideoEncoderConfigurationOptions));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange =
	        soap_malloc(soap, sizeof(struct tt__VideoEncoderConfigurationOptions));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange, 0x00,
	       sizeof(struct tt__VideoEncoderConfigurationOptions));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 1;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = 6;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 =
	        soap_malloc(soap, sizeof(struct tt__H264Options));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264, 0x00, sizeof(struct tt__H264Options));

	//#ifdef MT801
	if (dev.stitch_en) {
		if (num == 0) {
			availRes = 6;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable =
			        availRes; //resolutions_available;

			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_ =
			        soap_malloc(soap,
			                    sizeof(struct tt__VideoResolution) * availRes); //resolutions_available);
			memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_,
			       0x00, sizeof(struct tt__VideoResolution) * availRes);
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[0].Width =
			        3840;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[0]
			        .Height = 2160;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[1].Width =
			        3840;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[1]
			        .Height = 1080;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[2].Width =
			        2560;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[2]
			        .Height = 1440;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[3].Width =
			        2560;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[3]
			        .Height = 720;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[4].Width =
			        1920;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[4]
			        .Height = 1080;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[5].Width =
			        1920;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[5]
			        .Height = 544;
		} else if (num == 1) {
			availRes = 4;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable =
			        availRes; //resolutions_available;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_ =
			        soap_malloc(soap,
			                    sizeof(struct tt__VideoResolution) * availRes); //resolutions_available);
			memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_,
			       0x00, sizeof(struct tt__VideoResolution) * availRes);
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[0].Width =
			        960;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[0]
			        .Height = 540;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[1].Width =
			        960;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[1]
			        .Height = 272;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[2].Width =
			        640;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[2]
			        .Height = 360;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[3].Width =
			        640;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[3]
			        .Height = 184;
		} else {
		}
	} else {
		//#if 1
		//#else
		availRes = 0;
		for (idx = 0; idx < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; idx++) {
			if (devResOptions->strm[num].res[idx].width != 0)
				availRes = availRes + 1;
		}
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable =
		        availRes; //resolutions_available;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_ =
		        soap_malloc(soap,
		                    sizeof(struct tt__VideoResolution) * availRes); //resolutions_available);
		memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_, 0x00,
		       sizeof(struct tt__VideoResolution) * availRes);
		for (idx = 0; idx < availRes; idx++) {
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[idx]
			        .Width = devResOptions->strm[num].res[idx].width; //2592;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable_[idx]
			        .Height = devResOptions->strm[num].res[idx].height; //1944;
			fprintf(stderr, "No of aval resol %d ::::idx:%d res= %dX%d \n", availRes, idx,
			        devResOptions->strm[num].res[idx].width, devResOptions->strm[num].res[idx].height);
		}
	}

	//#endif
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange, 0x00,
	       sizeof(struct tt__IntRange));
	for (int idx2 = 0; idx2 < availRes; idx2++) {
		for (idx = 0; idx < MAX_AGTX_RES_ENTITY_S_FRAME_RATE_LIST_SIZE; idx++) {
			if (devResOptions->strm[num].res[idx2].frame_rate_list[idx] >= fpsRange)
				fpsRange = devResOptions->strm[num].res[idx2].frame_rate_list[idx];
		}
	}
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max = fpsRange; //num?10:20;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min = 1;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported =
	        h264profiles_nums;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported_ =
	        soap_malloc(soap, sizeof(enum tt__H264Profile) * h264profiles_nums);
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported_, 0x00,
	       (sizeof(enum tt__H264Profile) * h264profiles_nums));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported_[0] =
	        tt__H264Profile__Baseline;
	//trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported_[1] = tt__H264Profile__Main;
	//trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported_[2] = tt__H264Profile__High;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange, 0x00,
	       sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max = govMax;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min = 1;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange, 0x00,
	       sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max = 2;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min = 0;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension =
	        soap_malloc(soap, sizeof(struct tt__VideoEncoderOptionsExtension));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension, 0x00,
	       sizeof(struct tt__VideoEncoderOptionsExtension));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264 =
	        soap_malloc(soap, sizeof(struct tt__H264Options2));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264, 0x00,
	       sizeof(struct tt__H264Options2));

	if (1) {
		int idx;
		availRes = 0;
		for (idx = 0; idx < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; idx++) {
			if (devResOptions->strm[num].res[idx].width != 0)
				availRes = availRes + 1;
		}
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeResolutionsAvailable =
		        availRes; //resolutions_available;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable_ =
		        soap_malloc(soap, sizeof(struct tt__VideoResolution) * availRes); //resolutions_available);
		memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->ResolutionsAvailable_,
		       0x00, sizeof(struct tt__VideoResolution) * availRes);
		for (idx = 0; idx < availRes; idx++) {
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264
			        ->ResolutionsAvailable_[idx]
			        .Width = devResOptions->strm[num].res[idx].width; //2592;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264
			        ->ResolutionsAvailable_[idx]
			        .Height = devResOptions->strm[num].res[idx].height; //1944;
		}
	} else {
	}

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange, 0x00,
	       sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Max = fpsRange;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->FrameRateRange->Min = 1;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->__sizeH264ProfilesSupported =
	        h264profiles_nums;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported_ =
	        soap_malloc(soap, sizeof(enum tt__H264Profile) * h264profiles_nums);
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported_[0] =
	        tt__H264Profile__Baseline;
	//trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported_[1] = tt__H264Profile__Main;
	//trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->H264ProfilesSupported_[2] = tt__H264Profile__High;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange, 0x00,
	       sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Max = govMax;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->GovLengthRange->Min = 1;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Max = 2;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->EncodingIntervalRange->Min = 0;

	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange =
	        soap_malloc(soap, sizeof(struct tt__IntRange));
	memset(trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange, 0x00,
	       sizeof(struct tt__IntRange));
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Min =
	        (devVencOptions->strm[num].venc[0].min_bit_rate);
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264->BitrateRange->Max =
	        (devVencOptions->strm[num].venc[0].max_bit_rate);

	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioSourceConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(
        struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions,
        struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
	ONVIF_TRACE("__trt__GetAudioSourceConfigurationOptions TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioEncoderConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(
        struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions,
        struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
	int codec_num = 2;
	struct tt__AudioEncoderConfigurationOption *opt = NULL;

	ONVIF_TRACE("__trt__GetAudioEncoderConfigurationOptions\n");

	trt__GetAudioEncoderConfigurationOptionsResponse->Options =
	        aux_onvif_malloc(soap, sizeof(struct tt__AudioEncoderConfigurationOptions));
	trt__GetAudioEncoderConfigurationOptionsResponse->Options->__sizeOptions = codec_num;
	trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options_ =
	        aux_onvif_malloc(soap, sizeof(struct tt__AudioEncoderConfigurationOption) * codec_num);
	opt = &trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options_[0];
	opt->Encoding = tt__AudioEncoding__G711;
	opt->BitrateList = aux_onvif_malloc(soap, sizeof(struct tt__IntList));
	opt->BitrateList->__sizeItems = 1;
	opt->BitrateList->Items_ = aux_onvif_malloc(soap, sizeof(int) * opt->BitrateList->__sizeItems);
	opt->BitrateList->Items_[0] = 64;
	opt->SampleRateList = aux_onvif_malloc(soap, sizeof(struct tt__IntList));
	opt->SampleRateList->__sizeItems = 1;
	opt->SampleRateList->Items_ = aux_onvif_malloc(soap, sizeof(int) * opt->SampleRateList->__sizeItems);
	opt->SampleRateList->Items_[0] = 8;

	opt = &trt__GetAudioEncoderConfigurationOptionsResponse->Options->Options_[1];
	opt->Encoding = tt__AudioEncoding__G726;
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
};
/** Web service operation '__trt__GetMetadataConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(
        struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions,
        struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
	ONVIF_TRACE("__trt__GetMetadataConfigurationOptions TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioOutputConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(
        struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions,
        struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
	ONVIF_TRACE("__trt__GetAudioOutputConfigurationOptions TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetAudioDecoderConfigurationOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(
        struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions,
        struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
	ONVIF_TRACE("__trt__GetAudioDecoderConfigurationOptions TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetGuaranteedNumberOfVideoEncoderInstances' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(
        struct soap *soap,
        struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances,
        struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse
                *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
	ONVIF_TRACE("__trt__GetGuaranteedNumberOfVideoEncoderInstances TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetStreamUri' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri,
                                              struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
	int stream_num = 0;
	int audio_en = 0;
	char ip_buf[MAX_STR_LEN] = { 0 };
	AGTX_AUDIO_CONF_S audio = { 0 };

	if (trt__GetStreamUri->ProfileToken) {
		stream_num = atoi(trt__GetStreamUri->ProfileToken);
		printf("stream_num %d trt__GetStreamUri->ProfileToken %s\n", stream_num,
		       trt__GetStreamUri->ProfileToken);
	} else {
		return SOAP_NO_DATA;
	}

	ONVIF_TRACE("__trt__GetStreamUri stream_num %d\n", stream_num);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (aux_get_cc_config(AGTX_CMD_AUDIO_CONF, &audio) < 0) {
		return SOAP_FAULT;
	}

	audio_en = audio.enabled;

	trt__GetStreamUriResponse->MediaUri = soap_malloc(soap, sizeof(struct tt__MediaUri));
	memset(trt__GetStreamUriResponse->MediaUri, 0x00, sizeof(struct tt__MediaUri));
	trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__false_;
	trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot = xsd__boolean__false_;
	trt__GetStreamUriResponse->MediaUri->Uri = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(trt__GetStreamUriResponse->MediaUri->Uri, 0x00, sizeof(char) * MAX_STR_LEN);
	//strcpy(trt__GetStreamUriResponse->MediaUri->Uri,"rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov");
	
	// currently only 3 port numbers are defined
	assert(stream_num >= 0 && stream_num < 3);

	if (audio_en) {
#if AUDIO_EN
		sprintf(trt__GetStreamUriResponse->MediaUri->Uri, "rtsp://%s:%d/liveaudio/%d",
		        SYS_Getipaddr("eth0", ip_buf), portNumber[stream_num], stream_num);
#else
		sprintf(trt__GetStreamUriResponse->MediaUri->Uri, "rtsp://%s:%d/live/%d", SYS_Getipaddr("eth0", ip_buf),
		        portNumber[stream_num], stream_num);
#endif
	} else {
		sprintf(trt__GetStreamUriResponse->MediaUri->Uri, "rtsp://%s:%d/live/%d", SYS_Getipaddr("eth0", ip_buf),
		        portNumber[stream_num], stream_num);
	}

	trt__GetStreamUriResponse->MediaUri->Timeout = 10000;

	return SOAP_OK;
};
/** Web service operation '__trt__StartMulticastStreaming' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming,
                               struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
	ONVIF_TRACE("__trt__StartMulticastStreaming TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__StopMulticastStreaming' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming,
                              struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
	ONVIF_TRACE("__trt__StopMulticastStreaming TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetSynchronizationPoint' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint,
                               struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
	// TODO:  DaHua NVR needs a Send iframe on demand: for video stream
	ONVIF_TRACE("__trt__SetSynchronizationPoint TODO : Token: trt__SetSynchronizationPoint->ProfileToken: %s \n",
	            trt__SetSynchronizationPoint->ProfileToken);
	return SOAP_OK;
};
/** Web service operation '__trt__GetSnapshotUri' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri,
                                                struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
	int num = 0;
	char ip_buf[MAX_STR_LEN] = { 0 };
	struct tt__MediaUri *mediaUri = NULL;

	ONVIF_TRACE("__trt__GetSnapshotUri\n");

	if (trt__GetSnapshotUri->ProfileToken) {
		num = atoi(trt__GetSnapshotUri->ProfileToken);
	} else {
		return soap_sender_fault(soap, "Unknown token", NULL);
	}

	trt__GetSnapshotUriResponse->MediaUri = aux_onvif_malloc(soap, sizeof(struct tt__MediaUri));
	mediaUri = trt__GetSnapshotUriResponse->MediaUri;
	mediaUri->InvalidAfterConnect = xsd__boolean__false_;
	mediaUri->InvalidAfterReboot = xsd__boolean__false_;
	mediaUri->Timeout = 0;
	mediaUri->Uri = aux_onvif_malloc(soap, MAX_STR_LEN);
	sprintf(mediaUri->Uri, "http://%s:8899/%s%02d", SYS_Getipaddr("eth0", ip_buf), SNAPSHOT_URI, num);

	return SOAP_OK;
};
/** Web service operation '__trt__GetVideoSourceModes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__GetVideoSourceModes(struct soap *soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes,
                           struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse)
{
	ONVIF_TRACE("__trt__GetVideoSourceModes TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__SetVideoSourceMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__trt__SetVideoSourceMode(struct soap *soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode,
                          struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse)
{
	ONVIF_TRACE("__trt__SetVideoSourceMode TODO\n");
	return SOAP_OK;
};
/** Web service operation '__trt__GetOSDs' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs(struct soap *soap, struct _trt__GetOSDs *trt__GetOSDs,
                                         struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
	int i = 0;
	int chn = 0;
	int idx = 0;
	int osd_num = 0;
	int osd_idx[MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE] = { 0 };
	AGTX_OSD_CONF_S osd_conf;
	struct tt__OSDConfiguration *osd = NULL;

	ONVIF_TRACE("__trt__GetOSDs \n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

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

	trt__GetOSDsResponse->__sizeOSDs = osd_num;
	trt__GetOSDsResponse->OSDs_ = aux_onvif_malloc(soap, sizeof(struct tt__OSDConfiguration) * osd_num);

	for (i = 0; i < osd_num; i++) {
		idx = osd_idx[i];
		osd = &trt__GetOSDsResponse->OSDs_[i];

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
};
/** Web service operation '__trt__GetOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD(struct soap *soap, struct _trt__GetOSD *trt__GetOSD,
                                        struct _trt__GetOSDResponse *trt__GetOSDResponse)
{
	ONVIF_TRACE("__trt__GetOSD TODO\n");
	return SOAP_OK;
};

/** Web service operation '__trt__GetOSDOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions(struct soap *soap, struct _trt__GetOSDOptions *trt__GetOSDOptions,
                                               struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse)
{
	int i = 0;
	int type_num = 1;
	int pos_num = 1;
	int osd_num = 5;
	int osd_type_num = 5;
	int date_format_num = 3;
	int time_format_num = 4;
	struct tt__OSDConfigurationOptions *osd_opt = NULL;

	ONVIF_TRACE("__trt__GetOSDOptions\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	trt__GetOSDOptionsResponse->OSDOptions = aux_onvif_malloc(soap, sizeof(struct tt__OSDConfigurationOptions));
	osd_opt = trt__GetOSDOptionsResponse->OSDOptions;
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

	for (i = 0; i < 3; i++) {
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
	strcpy(osd_opt->TextOption->TimeFormat_[1], "hh:mm:ss tt");
	strcpy(osd_opt->TextOption->TimeFormat_[2], "H:mm:ss");
	strcpy(osd_opt->TextOption->TimeFormat_[3], "HH:mm:ss");

	osd_opt->__sizePositionOption = pos_num;
	osd_opt->PositionOption_ = aux_onvif_malloc(soap, sizeof(char *) * pos_num);
	osd_opt->PositionOption_[0] = aux_onvif_malloc(soap, MAX_STR_LEN);
	strcpy(osd_opt->PositionOption_[0], "Custom");

	return SOAP_OK;
};

/** Web service operation '__trt__SetOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD(struct soap *soap, struct _trt__SetOSD *trt__SetOSD,
                                        struct _trt__SetOSDResponse *trt__SetOSDResponse)
{
	int i = 0;
	int chn = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_OSD_CONF_S osd_conf;
	struct tt__OSDConfiguration *osd;

	if (!trt__SetOSD->OSD || !trt__SetOSD->OSD->token) {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__trt__SetOSD\n");

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	osd = trt__SetOSD->OSD;

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
};
/** Web service operation '__trt__CreateOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD(struct soap *soap, struct _trt__CreateOSD *trt__CreateOSD,
                                           struct _trt__CreateOSDResponse *trt__CreateOSDResponse)
{
	int i = 0;
	int chn = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_OSD_CONF_S osd_conf;
	struct tt__OSDConfiguration *osd;

	if (!trt__CreateOSD->OSD || !trt__CreateOSD->OSD->token) {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__tr2__CreateOSD\n");

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	osd = trt__CreateOSD->OSD;

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
		trt__CreateOSDResponse->OSDToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(trt__CreateOSDResponse->OSDToken, "osdtoken_txt");

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
		trt__CreateOSDResponse->OSDToken = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(trt__CreateOSDResponse->OSDToken, "osdtoken_info");
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
};
/** Web service operation '__trt__DeleteOSD' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD(struct soap *soap, struct _trt__DeleteOSD *trt__DeleteOSD,
                                           struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse)
{
	int i = 0;
	int chn = 0;
	AGTX_STRM_CONF_S strm = { 0 };
	AGTX_OSD_CONF_S osd_conf;

	if (!trt__DeleteOSD->OSDToken) {
		return SOAP_FAULT;
	}

	ONVIF_TRACE("__trt__DeleteOSD \n");

	if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
		return SOAP_FAULT;
	}

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, &osd_conf) < 0) {
		return SOAP_FAULT;
	}

	if (strcmp(trt__DeleteOSD->OSDToken, "osdtoken_txt") == 0) {
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
	} else if (strcmp(trt__DeleteOSD->OSDToken, "osdtoken_info") == 0) {
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
	} else if (strcmp(trt__DeleteOSD->OSDToken, "osdtoken_002") == 0) {
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
};
