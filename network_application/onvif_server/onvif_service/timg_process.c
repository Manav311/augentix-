#include "stdio.h"
#include "stdlib.h"

#include "soapStub.h"
#include "stdsoap2.h"
#include "augentix.h"

/** Web service operation '__timg__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__timg__GetServiceCapabilities(struct soap *soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities,
                               struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse)
{
	return SOAP_OK;
};

/** Web service operation '__timg__GetImagingSettings' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__timg__GetImagingSettings(struct soap *soap, struct _timg__GetImagingSettings *timg__GetImagingSettings,
                           struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
	AGTX_IMG_PREF_S img = { 0 };

	ONVIF_TRACE("__timg__GetImagingSettings\n");

	if (aux_get_cc_config(AGTX_CMD_IMG_PREF, &img) < 0) {
		return SOAP_NO_DATA;
	}

	timg__GetImagingSettingsResponse->ImagingSettings =
	        aux_onvif_malloc(soap, sizeof(struct tt__ImagingSettings20));
	memset(timg__GetImagingSettingsResponse->ImagingSettings, 0x00, sizeof(struct tt__ImagingSettings20));

	timg__GetImagingSettingsResponse->ImagingSettings->Brightness = aux_onvif_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Brightness = img.brightness;

	timg__GetImagingSettingsResponse->ImagingSettings->Contrast = aux_onvif_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Contrast = img.contrast;

	timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = aux_onvif_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = img.saturation;

	timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = aux_onvif_malloc(soap, sizeof(float));
	*timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = img.sharpness;

	return SOAP_OK;
};
/** Web service operation '__timg__SetImagingSettings' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__timg__SetImagingSettings(struct soap *soap, struct _timg__SetImagingSettings *timg__SetImagingSettings,
                           struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
	AGTX_IMG_PREF_S img = { 0 };

	ONVIF_TRACE("__timg__SetImagingSettings\n");

	if (aux_get_cc_config(AGTX_CMD_IMG_PREF, &img) < 0) {
		return SOAP_NO_DATA;
	}

	if (timg__SetImagingSettings->ImagingSettings->Brightness != NULL) {
		img.brightness = (int)*timg__SetImagingSettings->ImagingSettings->Brightness;
	}
	if (timg__SetImagingSettings->ImagingSettings->ColorSaturation != NULL) {
		img.saturation = (int)*timg__SetImagingSettings->ImagingSettings->ColorSaturation;
	}
	if (timg__SetImagingSettings->ImagingSettings->Contrast != NULL) {
		img.contrast = (int)*timg__SetImagingSettings->ImagingSettings->Contrast;
	}
	if (timg__SetImagingSettings->ImagingSettings->Sharpness != NULL) {
		img.sharpness = (int)*timg__SetImagingSettings->ImagingSettings->Sharpness;
	}

	if (aux_set_cc_config(AGTX_CMD_IMG_PREF, &img) < 0) {
		return SOAP_NO_DATA;
	}

	return SOAP_OK;
};
/** Web service operation '__timg__GetOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap *soap, struct _timg__GetOptions *timg__GetOptions,
                                             struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
	//int num = atoi(timg__GetOptions->VideoSourceToken);

	ONVIF_TRACE("__timg__GetOptions TODO\n");

	timg__GetOptionsResponse->ImagingOptions = soap_malloc(soap, sizeof(struct tt__ImagingOptions20));
	memset(timg__GetOptionsResponse->ImagingOptions, 0x00, sizeof(struct tt__ImagingOptions20));
	timg__GetOptionsResponse->ImagingOptions->Brightness = soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->Brightness->Max = 100; //255;//100;
	timg__GetOptionsResponse->ImagingOptions->Brightness->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->Contrast = soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->Contrast->Max = 100; //255;//;
	timg__GetOptionsResponse->ImagingOptions->Contrast->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation = soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Max = 100; //255;//100;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->Sharpness = soap_malloc(soap, sizeof(struct tt__FloatRange));
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Max = 100; //255;//100;
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Min = 0;

	return SOAP_OK;
};
/** Web service operation '__timg__Move' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap *soap, struct _timg__Move *timg__Move,
                                       struct _timg__MoveResponse *timg__MoveResponse)
{
	return SOAP_OK;
};
/** Web service operation '__timg__Stop' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap *soap, struct _timg__Stop *timg__Stop,
                                       struct _timg__StopResponse *timg__StopResponse)
{
	return SOAP_OK;
};
/** Web service operation '__timg__GetStatus' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap *soap, struct _timg__GetStatus *timg__GetStatus,
                                            struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
	return SOAP_OK;
};
/** Web service operation '__timg__GetMoveOptions' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap *soap, struct _timg__GetMoveOptions *timg__GetMoveOptions,
                                                 struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
	ONVIF_TRACE("__timg__GetMoveOptions TODO\n");
	return SOAP_OK;
};
/** Web service operation '__timg__GetPresets' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __timg__GetPresets(struct soap *soap, struct _timg__GetPresets *timg__GetPresets,
                                             struct _timg__GetPresetsResponse *timg__GetPresetsResponse)
{
	return SOAP_OK;
};
/** Web service operation '__timg__GetCurrentPreset' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__timg__GetCurrentPreset(struct soap *soap, struct _timg__GetCurrentPreset *timg__GetCurrentPreset,
                         struct _timg__GetCurrentPresetResponse *timg__GetCurrentPresetResponse)
{
	return SOAP_OK;
};
/** Web service operation '__timg__SetCurrentPreset' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__timg__SetCurrentPreset(struct soap *soap, struct _timg__SetCurrentPreset *timg__SetCurrentPreset,
                         struct _timg__SetCurrentPresetResponse *timg__SetCurrentPresetResponse)
{
	return SOAP_OK;
};
