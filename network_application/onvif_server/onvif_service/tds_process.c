#include "stdio.h"
#include "stdlib.h"

#include "soapStub.h"
#include "stdsoap2.h"
#include "augentix.h"
#include "time.h"
//
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/reboot.h>
#include <signal.h>

#define RESET_DB "/usrdata/dbrst"
#define RESET_ALL_DEFAULT "/usrdata/reset_file"
#define NETWORK_SCRIPT_CMD "/etc/init.d/factory/S40network restart"

/** Web service operation 'SOAP_ENV__Fault' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor,
                                          struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code,
                                          struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node,
                                          char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	ONVIF_TRACE("SOAP_ENV__Fault TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetServices' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap *soap, struct _tds__GetServices *tds__GetServices,
                                             struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
	char ip_buf[MAX_STR_LEN] = { 0 };
	int num_services = 6;

	ONVIF_TRACE("__tds__GetServices %d \n", tds__GetServices->IncludeCapability);
	if (tds__GetServices->IncludeCapability) {
		ONVIF_TRACE("__tds__GetServices \n");
	}

	//if (tds__GetServices->IncludeCapability == 1) {
	if (1) {
		//Services
		//"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		//"http://%s:8899/onvif/Events",ip_buf)"
		//"http://%s:8899/onvif/imaging",ip_buf);"
		//"http://%s:8899/onvif/Media",ip_buf);"
		//"http://%s:8899/onvif/Media2",ip_buf);"
		//"http://%s:8899/onvif/PTZ",ip_buf);"
		//memset(tds__GetCapabilitiesResponse->Capabilities,0,sizeof(struct tt__SystemDateTime));
		/*Device*/
		tds__GetServicesResponse->Service_ = soap_malloc(soap, sizeof(struct tds__Service) * num_services);
		memset(tds__GetServicesResponse->Service_, 0x0, sizeof(struct tds__Service) * num_services);
		tds__GetServicesResponse->__sizeService = num_services; //1;

		tds__GetServicesResponse->Service_[0].Namespace = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[0].Namespace, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[0].Namespace,
		        "http://www.onvif.org/ver10/device/wsdl"); //"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		tds__GetServicesResponse->Service_[0].XAddr = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[0].XAddr, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[0].XAddr, "http://%s:8899/onvif/device_service",
		        SYS_Getipaddr("eth0", ip_buf));
		tds__GetServicesResponse->Service_[0].Version = soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		memset(tds__GetServicesResponse->Service_[0].Version, 0x0, sizeof(struct tt__OnvifVersion));
		tds__GetServicesResponse->Service_[0].Version->Major = 17;
		tds__GetServicesResponse->Service_[0].Version->Minor = 12;
		//Enable need to enable tds__GetServices->IncludeCapability if there client sends in request
		//tds__GetServicesResponse->Service_[0].Capabilities = NULL; /*soap_malloc(soap,sizeof(struct _tds__Service_Capabilities));

		tds__GetServicesResponse->Service_[1].Namespace = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[1].Namespace, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[1].Namespace,
		        "http://www.onvif.org/ver10/events/wsdl"); //"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		tds__GetServicesResponse->Service_[1].XAddr = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[1].XAddr, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[1].XAddr, "http://%s:8899/onvif/Events",
		        SYS_Getipaddr("eth0", ip_buf));
		tds__GetServicesResponse->Service_[1].Version = soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		memset(tds__GetServicesResponse->Service_[1].Version, 0x0, sizeof(struct tt__OnvifVersion));
		tds__GetServicesResponse->Service_[1].Version->Major = 17;
		tds__GetServicesResponse->Service_[1].Version->Minor = 12;
		if (tds__GetServices->IncludeCapability == xsd__boolean__true_) {
			tds__GetServicesResponse->Service_[1].Capabilities =
			        aux_onvif_malloc(soap, sizeof(struct _tds__Service_Capabilities));
			tds__GetServicesResponse->Service_[1].Capabilities->__any = soap_malloc(soap, MAX_STR_LEN);
			strcpy(tds__GetServicesResponse->Service_[1].Capabilities->__any,
			       "<tev:Capabilities WSSubscriptionPolicySupport=\"true\" WSPullPointSupport=\"true\" "
			       "WSPausableSubscriptionManagerInterfaceSupport=\"false\" MaxNotificationProducers=\"3\""
			       " MaxPullPoints=\"3\"></tev:Capabilities>");
		}

		tds__GetServicesResponse->Service_[2].Namespace = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[2].Namespace, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[2].Namespace,
		        "http://www.onvif.org/ver20/imaging/wsdl"); //"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		tds__GetServicesResponse->Service_[2].XAddr = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[2].XAddr, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[2].XAddr, "http://%s:8899/onvif/Imaging",
		        SYS_Getipaddr("eth0", ip_buf));
		tds__GetServicesResponse->Service_[2].Version = soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		memset(tds__GetServicesResponse->Service_[2].Version, 0x0, sizeof(struct tt__OnvifVersion));
		tds__GetServicesResponse->Service_[2].Version->Major = 17;
		tds__GetServicesResponse->Service_[2].Version->Minor = 12;

		tds__GetServicesResponse->Service_[3].Namespace = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[3].Namespace, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[3].Namespace,
		        "http://www.onvif.org/ver10/media/wsdl"); //"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		tds__GetServicesResponse->Service_[3].XAddr = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[3].XAddr, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[3].XAddr, "http://%s:8899/onvif/Media",
		        SYS_Getipaddr("eth0", ip_buf));
		tds__GetServicesResponse->Service_[3].Version = soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		memset(tds__GetServicesResponse->Service_[3].Version, 0x0, sizeof(struct tt__OnvifVersion));
		tds__GetServicesResponse->Service_[3].Version->Major = 2;
		tds__GetServicesResponse->Service_[3].Version->Minor = 60;

		tds__GetServicesResponse->Service_[4].Namespace = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[4].Namespace, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[4].Namespace,
		        "http://www.onvif.org/ver20/media/wsdl"); //"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		tds__GetServicesResponse->Service_[4].XAddr = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[4].XAddr, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[4].XAddr, "http://%s:8899/onvif/Media2",
		        SYS_Getipaddr("eth0", ip_buf));
		tds__GetServicesResponse->Service_[4].Version = soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		memset(tds__GetServicesResponse->Service_[4].Version, 0x0, sizeof(struct tt__OnvifVersion));
		tds__GetServicesResponse->Service_[4].Version->Major = 17;
		tds__GetServicesResponse->Service_[4].Version->Minor = 12;

		tds__GetServicesResponse->Service_[5].Namespace = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[5].Namespace, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[5].Namespace,
		        "http://www.onvif.org/ver20/ptz/wsdl"); //"http://%s:8899/onvif/device_service",SYS_Getipaddr("eth0",ip_buf));
		tds__GetServicesResponse->Service_[5].XAddr = soap_malloc(soap, MAX_STR_LEN);
		memset(tds__GetServicesResponse->Service_[5].XAddr, 0x0, MAX_STR_LEN);
		sprintf(tds__GetServicesResponse->Service_[5].XAddr, "http://%s:8899/onvif/PTZ",
		        SYS_Getipaddr("eth0", ip_buf));
		tds__GetServicesResponse->Service_[5].Version = soap_malloc(soap, sizeof(struct tt__OnvifVersion));
		memset(tds__GetServicesResponse->Service_[5].Version, 0x0, sizeof(struct tt__OnvifVersion));
		tds__GetServicesResponse->Service_[5].Version->Major = 17;
		tds__GetServicesResponse->Service_[5].Version->Minor = 12;
	}

	return SOAP_OK;
}
/** Web service operation '__tds__GetServiceCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetServiceCapabilities(struct soap *soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities,
                              struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
	ONVIF_TRACE("__tds__GetServiceCapabilities TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDeviceInformation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetDeviceInformation(struct soap *soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation,
                            struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
//CC JSON
#ifdef CC_JSON
	char ccRetStr[256] = { 0 };
	char jsonCmd[256] =
	        "{'cmd_id':3145734,'cmd_type':'set','brightness':128,'saturation':128,'contrast':128,'sharpness':128,'anti_flicker':'50Hz', 'master_id': ";
	if (write(cmdTransFD, jsonCmd, strlen(jsonCmd)) < 0) {
		fprintf(stderr, "failed to send message to command Translator \n");
	} else {
		bzero(ccRetStr, 256);
	}

	if (getCCReturn(ccRetStr) != 0) {
		return SOAP_OK;
	}
#endif
	ONVIF_TRACE("__tds__GetDeviceInformation\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

#define DEVICE_INFO_STR_LEN (32)
	static char Manufacturer[DEVICE_INFO_STR_LEN] = { 0 };
	static char Model[DEVICE_INFO_STR_LEN] = { 0 };
	static char FirmwareVersion[DEVICE_INFO_STR_LEN] = { 0 };
	static char SerialNumber[DEVICE_INFO_STR_LEN] = { 0 };
	static char HardwareId[DEVICE_INFO_STR_LEN] = { 0 };

	if (Manufacturer[0] == 0) {
		aux_get_device_info("manufacturer", Manufacturer, DEVICE_INFO_STR_LEN);
	}
	if (Model[0] == 0) {
		aux_get_device_info("model", Model, DEVICE_INFO_STR_LEN);
	}
	if (FirmwareVersion[0] == 0) {
		aux_get_device_info("firmware_version", FirmwareVersion, DEVICE_INFO_STR_LEN);
	}
	if (SerialNumber[0] == 0) {
		aux_get_device_info("serial_number", SerialNumber, DEVICE_INFO_STR_LEN);
	}
	if (HardwareId[0] == 0) {
		aux_get_device_info("hardware_id", HardwareId, DEVICE_INFO_STR_LEN);
	}

	tds__GetDeviceInformationResponse->Manufacturer = Manufacturer;
	tds__GetDeviceInformationResponse->Model = Model;
	tds__GetDeviceInformationResponse->FirmwareVersion = FirmwareVersion;
	tds__GetDeviceInformationResponse->SerialNumber = SerialNumber;
	tds__GetDeviceInformationResponse->HardwareId = HardwareId;

	return SOAP_OK;
}
/** Web service operation '__tds__SetSystemDateAndTime' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetSystemDateAndTime(struct soap *soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime,
                            struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
	struct tm utc_tm = { 0 };
	struct timeval tm_set = { 0 };
	time_t utc = 0;

	ONVIF_TRACE("__tds__SetSystemDateAndTime\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	//	tds__SetSystemDateAndTime->DateTimeType;//TODO
	if (tds__SetSystemDateAndTime->TimeZone && tds__SetSystemDateAndTime->TimeZone->TZ) {
		setTimeinfo(tds__SetSystemDateAndTime->DaylightSavings, tds__SetSystemDateAndTime->TimeZone->TZ);
	}

	if (tds__SetSystemDateAndTime->UTCDateTime && tds__SetSystemDateAndTime->UTCDateTime->Time) {
		utc_tm.tm_hour = tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
		utc_tm.tm_min = tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
		utc_tm.tm_sec = tds__SetSystemDateAndTime->UTCDateTime->Time->Second;
	}

	if (tds__SetSystemDateAndTime->UTCDateTime && tds__SetSystemDateAndTime->UTCDateTime->Time) {
		utc_tm.tm_mday = tds__SetSystemDateAndTime->UTCDateTime->Date->Day;
		utc_tm.tm_mon = tds__SetSystemDateAndTime->UTCDateTime->Date->Month - 1;
		utc_tm.tm_year = tds__SetSystemDateAndTime->UTCDateTime->Date->Year - 1900;
	}

	utc = mktime(&utc_tm) - timezone; //timezone global variable in env

	tm_set.tv_sec = utc;
	settimeofday(&tm_set, 0);

	return SOAP_OK;
}
/** Web service operation '__tds__GetSystemDateAndTime' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetSystemDateAndTime(struct soap *soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime,
                            struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
	struct tm utc_tm = { 0 };
	struct tm local_tm = { 0 };
	time_t utc = time(0);
	char *tz = getenv("TZ");

	ONVIF_TRACE("__tds__GetSystemDateAndTime\n");

	gmtime_r(&utc, &utc_tm);
	localtime_r(&utc, &local_tm);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime = soap_malloc(soap, sizeof(struct tt__SystemDateTime));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime, 0x00, sizeof(struct tt__SystemDateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime =
	        soap_malloc(soap, sizeof(struct tt__DateTime));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime, 0x00, sizeof(struct tt__DateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date =
	        soap_malloc(soap, sizeof(struct tt__Date));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date, 0x00, sizeof(struct tt__Date));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time =
	        soap_malloc(soap, sizeof(struct tt__Time));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time, 0x00, sizeof(struct tt__Time));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime =
	        soap_malloc(soap, sizeof(struct tt__DateTime));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime, 0x00, sizeof(struct tt__DateTime));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date =
	        soap_malloc(soap, sizeof(struct tt__Date));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date, 0x00,
	       sizeof(struct tt__Date));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time =
	        soap_malloc(soap, sizeof(struct tt__Time));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time, 0x00,
	       sizeof(struct tt__Time));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone = soap_malloc(soap, sizeof(struct tt__TimeZone));
	memset(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone, 0x00, sizeof(struct tt__TimeZone));
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = soap_malloc(soap, MAX_STR_LEN);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings = (getDst() == 1) ? xsd__boolean__true_ :
	                                                                                          xsd__boolean__false_;

	sprintf(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, "%s", tz ? tz : "AWST-8WDT");

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour = utc_tm.tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute = utc_tm.tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second = utc_tm.tm_sec;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day = utc_tm.tm_mday;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month = utc_tm.tm_mon + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year = utc_tm.tm_year + 1900;

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour = local_tm.tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute = local_tm.tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second = local_tm.tm_sec;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day = local_tm.tm_mday;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month = local_tm.tm_mon + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year = local_tm.tm_year + 1900;

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->__anyAttribute = NULL;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->Extension = NULL;

	return SOAP_OK;
}
/** Web service operation '__tds__SetSystemFactoryDefault' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetSystemFactoryDefault(struct soap *soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault,
                               struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
	FILE *fp = NULL;

	ONVIF_TRACE("__tds__SetSystemFactoryDefault\n");

	if (tds__SetSystemFactoryDefault->FactoryDefault == tt__FactoryDefaultType__Soft) {
		fp = fopen(RESET_DB, "w+");
		if (fp == NULL) {
			return SOAP_FAULT;
		}
		fclose(fp);
	} else {
		fp = fopen(RESET_ALL_DEFAULT, "w+");
		if (fp == NULL) {
			return SOAP_FAULT;
		}
		fclose(fp);
	}
	sync();
	//reboot(RB_AUTOBOOT); note, this reboots the system, it's not as graceful as asking the init system to reboot.
	//reboot system,
	kill(1, SIGTERM);

	return SOAP_OK;
}
/** Web service operation '__tds__UpgradeSystemFirmware' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__UpgradeSystemFirmware(struct soap *soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware,
                             struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
	ONVIF_TRACE("__tds__UpgradeSystemFirmware TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SystemReboot' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap *soap, struct _tds__SystemReboot *tds__SystemReboot,
                                              struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
	char msg[32] = "Rebooting in 2 Sec";
	ONVIF_TRACE("__tds__SystemReboot :System Reboot in 2 Sec \n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	tds__SystemRebootResponse->Message = (char *)soap_malloc(soap, sizeof(char) * 32);
	strcpy(tds__SystemRebootResponse->Message, msg);

	system("reboot -d 2");

	return SOAP_OK;
}
/** Web service operation '__tds__RestoreSystem' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap *soap, struct _tds__RestoreSystem *tds__RestoreSystem,
                                               struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
	ONVIF_TRACE("__tds__RestoreSystem TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetSystemBackup' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap *soap, struct _tds__GetSystemBackup *tds__GetSystemBackup,
                                                 struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
	ONVIF_TRACE("__tds__GetSystemBackup TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetSystemLog' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap *soap, struct _tds__GetSystemLog *tds__GetSystemLog,
                                              struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
	ONVIF_TRACE("__tds__GetSystemLog TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetSystemSupportInformation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(
        struct soap *soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation,
        struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
	ONVIF_TRACE("__tds__GetSystemSupportInformation TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap *soap, struct _tds__GetScopes *tds__GetScopes,
                                           struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	int scope_num = 6;

	ONVIF_TRACE("__tds__GetScopes\n");
	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	tds__GetScopesResponse->__sizeScopes = scope_num;
	tds__GetScopesResponse->Scopes_ = soap_malloc(soap, sizeof(struct tt__Scope) * scope_num);
	memset(tds__GetScopesResponse->Scopes_, 0x00, sizeof(struct tt__Scope) * scope_num);
	tds__GetScopesResponse->Scopes_[0].ScopeDef = tt__ScopeDefinition__Fixed;
	tds__GetScopesResponse->Scopes_[0].ScopeItem = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	tds__GetScopesResponse->Scopes_[0].ScopeItem = "onvif://www.onvif.org/type/video_encoder";
	tds__GetScopesResponse->Scopes_[1].ScopeDef = tt__ScopeDefinition__Fixed;
	tds__GetScopesResponse->Scopes_[1].ScopeItem = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	tds__GetScopesResponse->Scopes_[1].ScopeItem = "onvif://www.onvif.org/type/audio_encoder";
	tds__GetScopesResponse->Scopes_[2].ScopeDef = tt__ScopeDefinition__Fixed;
	tds__GetScopesResponse->Scopes_[2].ScopeItem = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	tds__GetScopesResponse->Scopes_[2].ScopeItem = "onvif://www.onvif.org/hardware/IPC-model";
	tds__GetScopesResponse->Scopes_[3].ScopeDef = tt__ScopeDefinition__Fixed;
	tds__GetScopesResponse->Scopes_[3].ScopeItem = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	tds__GetScopesResponse->Scopes_[3].ScopeItem = "onvif://www.onvif.org/location/country/taipei";
	tds__GetScopesResponse->Scopes_[4].ScopeDef = tt__ScopeDefinition__Fixed;
	tds__GetScopesResponse->Scopes_[4].ScopeItem = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	tds__GetScopesResponse->Scopes_[4].ScopeItem = "onvif://www.onvif.org/name/HC1892";
	tds__GetScopesResponse->Scopes_[5].ScopeDef = tt__ScopeDefinition__Fixed;
	tds__GetScopesResponse->Scopes_[5].ScopeItem = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	tds__GetScopesResponse->Scopes_[5].ScopeItem = "onvif://www.onvif.org/Profile/Streaming";

	return SOAP_OK;
}
/** Web service operation '__tds__SetScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap *soap, struct _tds__SetScopes *tds__SetScopes,
                                           struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
	ONVIF_TRACE("__tds__SetScopes TODO\n");
	//fprintf(stderr,"__sizeScope = %d, Scopes_ = %s \n",(int *) (tds__SetScopes->__sizeScopes),(char *) tds__SetScopes->Scopes_[0]);

	return SOAP_OK;
}
/** Web service operation '__tds__AddScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap *soap, struct _tds__AddScopes *tds__AddScopes,
                                           struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
	ONVIF_TRACE("__tds__AddScopes TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__RemoveScopes' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap *soap, struct _tds__RemoveScopes *tds__RemoveScopes,
                                              struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
	ONVIF_TRACE("__tds__RemoveScopes TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap *soap,
                                                  struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode,
                                                  struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
	ONVIF_TRACE("__tds__GetDiscoveryMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap *soap,
                                                  struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode,
                                                  struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
	ONVIF_TRACE("__tds__SetDiscoveryMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetRemoteDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetRemoteDiscoveryMode(struct soap *soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode,
                              struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{
	ONVIF_TRACE("__tds__GetRemoteDiscoveryMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetRemoteDiscoveryMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetRemoteDiscoveryMode(struct soap *soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode,
                              struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
	ONVIF_TRACE("__tds__SetRemoteDiscoveryMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDPAddresses' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap *soap, struct _tds__GetDPAddresses *tds__GetDPAddresses,
                                                struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
	ONVIF_TRACE("__tds__GetDPAddresses TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetEndpointReference' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetEndpointReference(struct soap *soap, struct _tds__GetEndpointReference *tds__GetEndpointReference,
                            struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
	ONVIF_TRACE("__tds__GetEndpointReference TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetRemoteUser' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap *soap, struct _tds__GetRemoteUser *tds__GetRemoteUser,
                                               struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
	ONVIF_TRACE("__tds__GetRemoteUser TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetRemoteUser' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap *soap, struct _tds__SetRemoteUser *tds__SetRemoteUser,
                                               struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
	ONVIF_TRACE("__tds__SetRemoteUser TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetUsers' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap *soap, struct _tds__GetUsers *tds__GetUsers,
                                          struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
	int i = 0;
	int cnt = 0;
	char user[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char pwd[MAX_USER][MAX_USER_LEN] = { { 0 } };

	ONVIF_TRACE("__tds__GetUsers\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	cnt = aux_get_user_pwd(user, pwd);

	if (!cnt) { /*No user*/
		return SOAP_OK;
	}

	tds__GetUsersResponse->__sizeUser = cnt;
	tds__GetUsersResponse->User_ = aux_onvif_malloc(soap, sizeof(struct tt__User) * cnt);
	for (i = 0; i < cnt; i++) {
		tds__GetUsersResponse->User_[i].Username = aux_onvif_malloc(soap, MAX_STR_LEN);
		strcpy(tds__GetUsersResponse->User_[i].Username, user[i]);
		tds__GetUsersResponse->User_[i].UserLevel = tt__UserLevel__Administrator;
	}

	return SOAP_OK;
}
/** Web service operation '__tds__CreateUsers' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap *soap, struct _tds__CreateUsers *tds__CreateUsers,
                                             struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
	int i = 0;
	int j = 0;
	int cnt = 0;
	int index = 0;
	int user_size = 0;
	char user[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char pwd[MAX_USER][MAX_USER_LEN] = { { 0 } };
	struct tt__User *user_conf = NULL;

	ONVIF_TRACE("__tds__CreateUsers\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (!tds__CreateUsers->__sizeUser || !tds__CreateUsers->User_ || !tds__CreateUsers->User_->Username) {
		soap_sender_fault(soap, "No user set", NULL);
		return SOAP_FAULT;
	}

	cnt = aux_get_user_pwd(user, pwd);

	if ((cnt + tds__CreateUsers->__sizeUser) > MAX_USER) {
		soap_sender_fault(soap, "Too many users", NULL);
		return SOAP_FAULT;
	}

	user_size = tds__CreateUsers->__sizeUser;
	user_conf = tds__CreateUsers->User_;
	index = cnt;

	for (i = 0; i < user_size; i++) {
		if (strlen(user_conf[i].Username) >= MAX_USER_LEN) {
			soap_sender_fault(soap, "User string too long", NULL);
			return SOAP_FAULT;
		}

		if (strchr(user_conf[i].Username, ' ')) {
			soap_sender_fault(soap, "User name can't include whitespace", NULL);
			return SOAP_FAULT;
		}

		for (j = 0; j < cnt; j++) {
			if (!strcmp(user[j], user_conf[i].Username)) {
				soap_sender_fault(soap, "User name already exist.", NULL);
				return SOAP_FAULT;
			}
		}

		strcpy(user[index], user_conf[i].Username);

		if (user_conf[i].Password && strlen(user_conf[i].Password) >= MAX_USER_LEN) {
			soap_sender_fault(soap, "Password string too long", NULL);
			return SOAP_FAULT;
		}

		if (user_conf[i].Password) {
			strcpy(pwd[index], user_conf[i].Password);
		} else {
			pwd[index][0] = 0; //No password, set to empty string
		}
		index++;
	}

	cnt += user_size;

	aux_set_user_pwd(user, pwd);

	return SOAP_OK;
}
/** Web service operation '__tds__DeleteUsers' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap *soap, struct _tds__DeleteUsers *tds__DeleteUsers,
                                             struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
	int i = 0;
	int j = 0;
	int cnt = 0;
	int user_size = 0;
	char user[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char pwd[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char **user_name = NULL;

	ONVIF_TRACE("__tds__DeleteUsers\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (!tds__DeleteUsers->__sizeUsername || !tds__DeleteUsers->Username_) {
		soap_sender_fault(soap, "No user set", NULL);
		return SOAP_FAULT;
	}

	cnt = aux_get_user_pwd(user, pwd);

	if (!cnt) { /*No user*/
		return SOAP_OK;
	}

	user_size = tds__DeleteUsers->__sizeUsername;
	user_name = tds__DeleteUsers->Username_;

	for (i = 0; i < user_size; i++) {
		for (j = 0; j < cnt; j++) {
			if (!strcmp(user[j], user_name[i])) {
				user[j][0] = 0;
				break;
			}
		}
	}

	aux_set_user_pwd(user, pwd);

	return SOAP_OK;
}
/** Web service operation '__tds__SetUser' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap *soap, struct _tds__SetUser *tds__SetUser,
                                         struct _tds__SetUserResponse *tds__SetUserResponse)
{
	int i = 0;
	int j = 0;
	int cnt = 0;
	int user_size = 0;
	char user[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char pwd[MAX_USER][MAX_USER_LEN] = { { 0 } };
	struct tt__User *user_conf = NULL;

	ONVIF_TRACE("__tds__SetUser\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (!tds__SetUser->__sizeUser || !tds__SetUser->User_ || !tds__SetUser->User_->Username) {
		soap_sender_fault(soap, "No user set", NULL);
		return SOAP_FAULT;
	}

	cnt = aux_get_user_pwd(user, pwd);

	if (!cnt) { /*No user*/
		return SOAP_OK;
	}

	user_size = tds__SetUser->__sizeUser;
	user_conf = tds__SetUser->User_;

	for (i = 0; i < user_size; i++) {
		for (j = 0; j < cnt; j++) {
			if (!strcmp(user[j], user_conf[i].Username)) {
				if (user_conf[i].Password && strlen(user_conf[i].Password) >= MAX_USER_LEN) {
					soap_sender_fault(soap, "Password string too long", NULL);
					return SOAP_FAULT;
				}

				if (user_conf[i].Password) {
					strcpy(pwd[j], user_conf[i].Password);
				} else {
					pwd[j][0] = 0; //No password, set to empty string
				}
				break;
			}
		}
	}

	aux_set_user_pwd(user, pwd);

	return SOAP_OK;
}
/** Web service operation '__tds__GetWsdlUrl' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap *soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl,
                                            struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
	ONVIF_TRACE("__tds__GetWsdlUrl TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetCapabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap *soap, struct _tds__GetCapabilities *tds__GetCapabilities,
                                                 struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
	char ip_buf[MAX_STR_LEN] = { 0 };
#ifdef Config_To_Json
	char ccRetStr[256] = { 0 };
	char jsonCmd[256] =
	        "{'cmd_id':3145734,'cmd_type':'set','brightness':128,'saturation':128,'contrast':128,'sharpness':128,'anti_flicker':'50Hz', 'master_id': ";
	if (write(cmdTransFD, jsonCmd, strlen(jsonCmd)) < 0) {
		fprintf(stderr, "failed to send message to command Translator \n");
	} else {
		bzero(ccRetStr, 256);
	}

	if (getCCReturn(ccRetStr) != 0) {
		return SOAP_OK;
	}
#endif
	ONVIF_TRACE("__tds__GetCapabilities\n");

	tds__GetCapabilitiesResponse->Capabilities = soap_malloc(soap, sizeof(struct tt__SystemDateTime));
	memset(tds__GetCapabilitiesResponse->Capabilities, 0, sizeof(struct tt__SystemDateTime));
	/*Device*/
	tds__GetCapabilitiesResponse->Capabilities->Device = soap_malloc(soap, sizeof(struct tt__DeviceCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device, 0, sizeof(struct tt__DeviceCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Device->XAddr = soap_malloc(soap, MAX_STR_LEN);
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, 0x00, MAX_STR_LEN);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, "http://%s:8899/onvif/device_service",
	        SYS_Getipaddr("eth0", ip_buf));

	/*Network*/
	tds__GetCapabilitiesResponse->Capabilities->Device->Network =
	        soap_malloc(soap, sizeof(struct tt__NetworkCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->Network, 0, sizeof(struct tt__NetworkCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS =
	        soap_malloc(soap, sizeof(enum xsd__boolean));

	*tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = xsd__boolean__false_;
	*tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = xsd__boolean__false_;
	*tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = xsd__boolean__false_;
	*tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = xsd__boolean__false_;

	/*System*/
	tds__GetCapabilitiesResponse->Capabilities->Device->System =
	        soap_malloc(soap, sizeof(struct tt__SystemCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->System, 0, sizeof(struct tt__SystemCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions_ =
	        soap_malloc(soap, sizeof(struct tt__OnvifVersion));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions_, 0x00,
	       sizeof(struct tt__OnvifVersion));
	tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = 1;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension =
	        soap_malloc(soap, sizeof(struct tt__SystemCapabilitiesExtension));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension, 0,
	       sizeof(struct tt__SystemCapabilitiesExtension));
	tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation =
	        soap_malloc(soap, sizeof(enum xsd__boolean));

	tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions_->Major = 17; //TODO ONVIF VERSION
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions_->Minor = 12; //TODO
	*(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpFirmwareUpgrade) =
	        xsd__boolean__true_;
	*(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemBackup) =
	        xsd__boolean__false_;
	*(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSystemLogging) =
	        xsd__boolean__false_;
	*(tds__GetCapabilitiesResponse->Capabilities->Device->System->Extension->HttpSupportInformation) =
	        xsd__boolean__false_;

	/*IO*/
	tds__GetCapabilitiesResponse->Capabilities->Device->IO = soap_malloc(soap, sizeof(struct tt__IOCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->IO, 0, sizeof(struct tt__IOCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = soap_malloc(soap, sizeof(int));
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs = soap_malloc(soap, sizeof(int));
	*tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = 0;
	*tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension =
	        soap_malloc(soap, sizeof(struct tt__IOCapabilitiesExtension));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension, 0x00,
	       sizeof(struct tt__IOCapabilitiesExtension));
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->Auxiliary =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->AuxiliaryCommands_ =
	        soap_malloc(soap, sizeof(char *));
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->AuxiliaryCommands_[0] =
	        soap_malloc(soap, MAX_STR_LEN);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->AuxiliaryCommands_[0], "nothing");
	tds__GetCapabilitiesResponse->Capabilities->Device->IO->Extension->__sizeAuxiliaryCommands = 1;

	/*Security*/
	tds__GetCapabilitiesResponse->Capabilities->Device->Security =
	        (struct tt__SecurityCapabilities *)soap_malloc(soap, sizeof(struct tt__SecurityCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Device->Security, 0,
	       sizeof(struct tt__SecurityCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1 = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2 = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken = xsd__boolean__false_;

	/*Event*/
	tds__GetCapabilitiesResponse->Capabilities->Events = soap_malloc(soap, sizeof(struct tt__EventCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Events, 0x00, sizeof(struct tt__EventCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Events->XAddr = soap_malloc(soap, MAX_STR_LEN);
	memset(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, 0x00, MAX_STR_LEN);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, "http://%s:8899/onvif/Events", ip_buf);
	tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport = xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport = xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport =
	        xsd__boolean__false_;

	/*Imaging*/
	tds__GetCapabilitiesResponse->Capabilities->Imaging =
	        (struct tt__ImagingCapabilities *)soap_malloc(soap, sizeof(struct tt__ImagingCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Imaging, 0x00, sizeof(struct tt__ImagingCapabilities));

	tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, 0x00, sizeof(char) * MAX_STR_LEN);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, "http://%s:8899/onvif/imaging", ip_buf);

	/*Media*/
	tds__GetCapabilitiesResponse->Capabilities->Media =
	        (struct tt__MediaCapabilities *)soap_malloc(soap, sizeof(struct tt__MediaCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Media, 0x00, sizeof(struct tt__MediaCapabilities));

	tds__GetCapabilitiesResponse->Capabilities->Media->XAddr =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, 0x00, sizeof(char) * MAX_STR_LEN);
	//sprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, "http://%s:8899/onvif/Media",ip_buf);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, "http://%s:8899/onvif/Media", ip_buf);

	tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities =
	        (struct tt__RealTimeStreamingCapabilities *)soap_malloc(
	                soap, sizeof(struct tt__RealTimeStreamingCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities, 0,
	       sizeof(struct tt__RealTimeStreamingCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = xsd__boolean__false_;
	tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP =
	        soap_malloc(soap, sizeof(enum xsd__boolean));
	*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP =
	        xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Media->Extension =
	        soap_malloc(soap, sizeof(struct tt__MediaCapabilitiesExtension));
	memset(tds__GetCapabilitiesResponse->Capabilities->Media->Extension, 0x00,
	       sizeof(struct tt__MediaCapabilitiesExtension));
	tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities =
	        soap_malloc(soap, sizeof(struct tt__ProfileCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities, 0x00,
	       sizeof(struct tt__ProfileCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Media->Extension->ProfileCapabilities->MaximumNumberOfProfiles = 2;

	/*PTZ*/
	tds__GetCapabilitiesResponse->Capabilities->PTZ = soap_malloc(soap, sizeof(struct tt__PTZCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->PTZ, 0, sizeof(struct tt__PTZCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr = (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr, 0x00, sizeof(char) * MAX_STR_LEN);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr, "http://%s:8899/onvif/PTZ", ip_buf);

	tds__GetCapabilitiesResponse->Capabilities->Analytics =
	        soap_malloc(soap, sizeof(struct tt__AnalyticsCapabilities));
	memset(tds__GetCapabilitiesResponse->Capabilities->Analytics, 0, sizeof(struct tt__AnalyticsCapabilities));
	tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport = xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport = xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	sprintf(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, "http://%s:8899/onvif/Analytics", ip_buf);

	return SOAP_OK;
}
/** Web service operation '__tds__SetDPAddresses' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap *soap, struct _tds__SetDPAddresses *tds__SetDPAddresses,
                                                struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
	ONVIF_TRACE("__tds__SetDPAddresses TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetHostname' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap *soap, struct _tds__GetHostname *tds__GetHostname,
                                             struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
	char hostname[HOSTNAME_LEN] = { 0 };

	ONVIF_TRACE("__tds__GetHostname\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	SYS_Gethostname(hostname);
	tds__GetHostnameResponse->HostnameInformation = soap_malloc(soap, sizeof(struct tt__HostnameInformation));
	memset(tds__GetHostnameResponse->HostnameInformation, 0x00, sizeof(struct tt__HostnameInformation));
	tds__GetHostnameResponse->HostnameInformation->Name = (char *)soap_malloc(soap, sizeof(char) * HOSTNAME_LEN);
	strcpy(tds__GetHostnameResponse->HostnameInformation->Name, hostname);
	tds__GetHostnameResponse->HostnameInformation->FromDHCP = xsd__boolean__false_;

	return SOAP_OK;
}
/** Web service operation '__tds__SetHostname' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap *soap, struct _tds__SetHostname *tds__SetHostname,
                                             struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
	char hostname[HOSTNAME_LEN] = { 0 };
	char tmpCmd[16 + HOSTNAME_LEN] = { 0 };

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	if (tds__SetHostname->Name != NULL) {
		strcpy(hostname, tds__SetHostname->Name);
		ONVIF_TRACE("__tds__SetHostname Setting HostName to = %s \n", hostname);

		system("echo '127.0.0.1	localhost' > /etc/hosts");
		SYS_Sethostname(hostname);
		sprintf(tmpCmd, "echo  '127.0.0.1  %s ' >> /etc/hosts", hostname);
		//update /etc/hosts file
		system(tmpCmd);
	}

	return SOAP_OK;
}
/** Web service operation '__tds__SetHostnameFromDHCP' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetHostnameFromDHCP(struct soap *soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP,
                           struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
	/*----getram----
	 * Need to check if udhcp client (udhcpc) has some hook support to set hostname based on DHCP assignment
	 * tds__SetHostnameFromDHCP->FromDHCP = xsd__boolean__false_ or 0*/
	ONVIF_TRACE("__tds__SetHostnameFromDHCP TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap *soap, struct _tds__GetDNS *tds__GetDNS,
                                        struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
	ONVIF_TRACE("__tds__GetDNS\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	res_init();
#if 1 //getram
	tds__GetDNSResponse->DNSInformation = soap_malloc(soap, sizeof(struct tt__DNSInformation));
	memset(tds__GetDNSResponse->DNSInformation, 0x00, sizeof(struct tt__DNSInformation));
	tds__GetDNSResponse->DNSInformation->FromDHCP =
	        xsd__boolean__false_; //TODO This need to check if udhcpc supports DHCP assignments for host and other info
	tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 1;
	tds__GetDNSResponse->DNSInformation->SearchDomain_ = soap_malloc(soap, sizeof(char *));
	*tds__GetDNSResponse->DNSInformation->SearchDomain_ = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(*tds__GetDNSResponse->DNSInformation->SearchDomain_, (const char *)_res.dnsrch); //= "JufengInfo"; //TODO
	tds__GetDNSResponse->DNSInformation->DNSManual_ = soap_malloc(soap, sizeof(struct tt__IPAddress) * 2);
	memset(tds__GetDNSResponse->DNSInformation->DNSManual_, 0x00, sizeof(struct tt__IPAddress) * 2);

	res_init();

	if (_res.nscount >= 1) {
		tds__GetDNSResponse->DNSInformation->DNSManual_[0].IPv4Address =
		        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
		strcpy(tds__GetDNSResponse->DNSInformation->DNSManual_[0].IPv4Address,
		       inet_ntoa(_res.nsaddr_list[0].sin_addr)); //"192.168.1.1";
		tds__GetDNSResponse->DNSInformation->DNSManual_[0].Type = tt__IPType__IPv4;
		tds__GetDNSResponse->DNSInformation->DNSManual_[1].IPv4Address =
		        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
		strcpy(tds__GetDNSResponse->DNSInformation->DNSManual_[1].IPv4Address,
		       inet_ntoa(_res.nsaddr_list[1].sin_addr)); //"8.8.8.8";
		tds__GetDNSResponse->DNSInformation->DNSManual_[1].Type = tt__IPType__IPv4;
		tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 2;
		ONVIF_TRACE("__tds__GetDNS[0] %s \n", tds__GetDNSResponse->DNSInformation->DNSManual_[0].IPv4Address);
		ONVIF_TRACE("__tds__GetDNS[1] %s \n", tds__GetDNSResponse->DNSInformation->DNSManual_[1].IPv4Address);
		return SOAP_OK;
	} else if (_res.nscount == 1) {
		tds__GetDNSResponse->DNSInformation->DNSManual_[0].IPv4Address =
		        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
		tds__GetDNSResponse->DNSInformation->DNSManual_[0].IPv4Address =
		        inet_ntoa(_res.nsaddr_list[0].sin_addr); //"192.168.1.1";
		tds__GetDNSResponse->DNSInformation->DNSManual_[0].Type = tt__IPType__IPv4;
		tds__GetDNSResponse->DNSInformation->DNSManual_[1].IPv4Address =
		        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
		tds__GetDNSResponse->DNSInformation->DNSManual_[1].IPv4Address = "";
		tds__GetDNSResponse->DNSInformation->DNSManual_[1].Type = tt__IPType__IPv4;
		ONVIF_TRACE("__tds__GetDNS[0] %s \n", tds__GetDNSResponse->DNSInformation->DNSManual_[0].IPv4Address);
		return SOAP_OK;
	} else {
		return SOAP_OK;
	}
#endif
	return SOAP_OK;
}
/** Web service operation '__tds__SetDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap *soap, struct _tds__SetDNS *tds__SetDNS,
                                        struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
	int idx = tds__SetDNS->__sizeDNSManual;
	char dnsEntry[128] = "dns-nameservers ";
	char dnsCmd[256] = "sed -i 's/^dns-nameservers.*/";

	ONVIF_TRACE("__tds__SetDNS TODO\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	for (int i = 0; i < idx; i++) {
		//fprintf(stderr,"setDNS::: %d ::  %s \n",__LINE__,tds__SetDNS->DNSManual_[i].IPv4Address);
		sprintf(dnsEntry, "%s %s", dnsEntry, tds__SetDNS->DNSManual_[i].IPv4Address);
	}

	sprintf(dnsCmd, "%s%s/' /etc/network/interfaces", dnsCmd, dnsEntry);
	fprintf(stderr, "setDNS::: %d ::Sync Entry to /etc/network/interfaces file  %s \n", __LINE__, dnsCmd);
	system(dnsCmd);
	system(NETWORK_SCRIPT_CMD);

	return SOAP_OK;
}
/** Web service operation '__tds__GetNTP' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap *soap, struct _tds__GetNTP *tds__GetNTP,
                                        struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
	ONVIF_TRACE("__tds__GetNTP TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetNTP' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP,
                                        struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
	ONVIF_TRACE("__tds__SetNTP TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDynamicDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap *soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS,
                                               struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
	ONVIF_TRACE("__tds__GetDynamicDNS TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetDynamicDNS' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap *soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS,
                                               struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
	ONVIF_TRACE("__tds__SetDynamicDNS TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetNetworkInterfaces' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetNetworkInterfaces(struct soap *soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces,
                            struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
	//int i = 0;
	char ip_buf[256] = { 0 };
	char mac_addr[6] = { 0 };
	unsigned char autoneg = 0;
	unsigned short speed = 0;
	unsigned char duplex = 0;
	int net_num = 0;
#ifdef Config_To_Json
	char ccRetStr[256] = { 0 };
	char jsonCmd[256] =
	        "{'cmd_id':3145734,'cmd_type':'set','brightness':128,'saturation':128,'contrast':128,'sharpness':128,'anti_flicker':'50Hz', 'master_id': ";
	if (write(cmdTransFD, jsonCmd, strlen(jsonCmd)) < 0) {
		fprintf(stderr, "failed to send message to command Translator \n");
	} else {
		bzero(ccRetStr, 256);
	}

	if (getCCReturn(ccRetStr) != 0) {
		return SOAP_OK;
	}
#endif
	ONVIF_TRACE("__tds__GetNetworkInterfaces\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	net_num = SYS_NumNetworkIntf();
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_ =
	        soap_malloc(soap, sizeof(struct tt__NetworkInterface) * net_num);
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_, 0x00,
	       sizeof(struct tt__NetworkInterface) * net_num);
	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = net_num; //1
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Enabled = xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->token =
	        (char *)soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->token, 0x00, sizeof(char) * MAX_STR_LEN);
	//sprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->token,"eth%01d",i);//TODO
	sprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->token, "%s",
	        SYS_NetworkInfName(1)); //TODO : eth0 int num is 1
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info =
	        soap_malloc(soap, sizeof(struct tt__NetworkInterfaceInfo));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info, 0x00,
	       sizeof(struct tt__NetworkInterfaceInfo));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->Name =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->Name, 0x00, sizeof(char) * MAX_STR_LEN);
	//sprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->Name,"eth%01d",i);//TODO
	sprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->Name, "%s",
	        SYS_NetworkInfName(1)); //TODO: eth0 int num is 1
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->HwAddress =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->HwAddress, 0x00,
	       sizeof(char) * MAX_STR_LEN);
	SYS_Getmacaddr("eth0", mac_addr);
	sprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->HwAddress, "%x:%x:%x:%x:%x:%x",
	        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->MTU = soap_malloc(soap, sizeof(int));
	*tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Info->MTU = 1500; //TODO
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link =
	        soap_malloc(soap, sizeof(struct tt__NetworkInterfaceLink));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link, 0x00,
	       sizeof(struct tt__NetworkInterfaceLink));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->AdminSettings =
	        soap_malloc(soap, sizeof(struct tt__NetworkInterfaceConnectionSetting));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->AdminSettings, 0x00,
	       sizeof(struct tt__NetworkInterfaceConnectionSetting));
	SYS_Getadminsettings("eth0", &autoneg, &speed, &duplex);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->AdminSettings->AutoNegotiation =
	        autoneg ? xsd__boolean__true_ : xsd__boolean__false_; //TODO
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->AdminSettings->Duplex = duplex ? tt__Duplex__Half :
	                                                                                              tt__Duplex__Full;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->AdminSettings->Speed = speed;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->OperSettings =
	        soap_malloc(soap, sizeof(struct tt__NetworkInterfaceConnectionSetting));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->OperSettings, 0x00,
	       sizeof(struct tt__NetworkInterfaceConnectionSetting));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->OperSettings->AutoNegotiation = autoneg;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->OperSettings->Duplex = duplex;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->OperSettings->Speed = speed;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->Link->InterfaceType = 0; //TODO

	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4 =
	        soap_malloc(soap, sizeof(struct tt__IPv4NetworkInterface));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4, 0x00,
	       sizeof(struct tt__IPv4NetworkInterface));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Enabled = xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config =
	        soap_malloc(soap, sizeof(struct tt__IPv4Configuration));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config, 0x00,
	       sizeof(struct tt__IPv4Configuration));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->Manual_ =
	        soap_malloc(soap, sizeof(struct tt__PrefixedIPv4Address));
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->Manual_, 0x00,
	       sizeof(struct tt__PrefixedIPv4Address));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->Manual_->Address =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	memset(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->Manual_->Address, 0x00,
	       sizeof(char) * MAX_STR_LEN);
	sprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->Manual_->Address, "%s",
	        SYS_Getipaddr("eth0", ip_buf));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->Manual_->PrefixLength =
	        SYS_GetIPNetmask("eth0"); //24; //TODO
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->__sizeManual = 1;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces_->IPv4->Config->DHCP = xsd__boolean__false_;

	return SOAP_OK;
}
/** Web service operation '__tds__SetNetworkInterfaces' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetNetworkInterfaces(struct soap *soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces,
                            struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
	//char setIPEntry[128] = "address ";
	//char mask[36] = {0};
	//char setIPCmd[256] = "sed -i 's/^address.*/";
	ONVIF_TRACE("__tds__SetNetworkInterfaces ::  %s\n", tds__SetNetworkInterfaces->InterfaceToken);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	//Handle DHCP
	if (*(tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP) == 1) {
		system("sed -i 's/^iface eth0 inet static.*/iface eth0 inet dhcp/' /etc/network/interfaces");
		system(NETWORK_SCRIPT_CMD);
		return SOAP_OK;
	} else if (tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual_->Address !=
	           NULL) { //Static IP and mask change
		char tmpCmd[128] = { 0 };
		sprintf(tmpCmd, "sed -i 's/^address.*/address %s/' /etc/network/interfaces",
		        tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual_->Address);
		system(tmpCmd);
	} else if (tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual_->PrefixLength > 0) {
		char tmpCmd[128] = { 0 };
		int tmpPrefix = tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual_->PrefixLength;
		sprintf(tmpCmd, "sed -i 's/^netmask.*/netmask %s/' /etc/network/interfaces",
		        SYS_NetworkPrefixToMask(tmpPrefix));
		system(tmpCmd);
	} else {
		ONVIF_TRACE("__tds__SetNetworkInterfaces :: Did not match all conditions to change IP address");
		return SOAP_NO_DATA;
	}
	//apply changes
	system(NETWORK_SCRIPT_CMD);
	return SOAP_OK;
}
/** Web service operation '__tds__GetNetworkProtocols' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetNetworkProtocols(struct soap *soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols,
                           struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
	ONVIF_TRACE("__tds__GetNetworkProtocols TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetNetworkProtocols' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetNetworkProtocols(struct soap *soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols,
                           struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
	ONVIF_TRACE("__tds__SetNetworkProtocols TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetNetworkDefaultGateway' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetNetworkDefaultGateway(struct soap *soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway,
                                struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
	unsigned int gateway = 0;

	ONVIF_TRACE("__tds__GetNetworkDefaultGateway\n");

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	tds__GetNetworkDefaultGatewayResponse->NetworkGateway = soap_malloc(soap, sizeof(struct tt__NetworkGateway));
	memset(tds__GetNetworkDefaultGatewayResponse->NetworkGateway, 0x00, sizeof(struct tt__NetworkGateway));
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address = 1;
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address_ = soap_malloc(soap, sizeof(char *));
	*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address_ =
	        soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	SYS_Getgateway(&gateway);
	sprintf(*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address_, "%d.%d.%d.%d",
	        (gateway << 24) >> 24, (gateway << 16) >> 24, (gateway << 8) >> 24, gateway >> 24);

	return SOAP_OK;
}
/** Web service operation '__tds__SetNetworkDefaultGateway' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetNetworkDefaultGateway(struct soap *soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway,
                                struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{
	char gwEntry[128] = "gateway ";
	char gwCmd[256] = "sed -i 's/^gateway.*/";

	ONVIF_TRACE("__tds__SetNetworkDefaultGateway : %s \n", *tds__SetNetworkDefaultGateway->IPv4Address_);

	if (aux_check_auth(soap) != SOAP_OK) {
		return aux_auth_fault(soap);
	}

	sprintf(gwEntry, "%s %s", gwEntry, *tds__SetNetworkDefaultGateway->IPv4Address_);

	sprintf(gwCmd, "%s%s/' /etc/network/interfaces", gwCmd, gwEntry);
	fprintf(stderr, "setDNS::: %d ::Sync Entry to /etc/network/interfaces file  %s \n", __LINE__, gwCmd);
	system(gwCmd);
	system(NETWORK_SCRIPT_CMD);
	return SOAP_OK;
}
/** Web service operation '__tds__GetZeroConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetZeroConfiguration(struct soap *soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration,
                            struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
	ONVIF_TRACE("__tds__GetZeroConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetZeroConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetZeroConfiguration(struct soap *soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration,
                            struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
	ONVIF_TRACE("__tds__SetZeroConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetIPAddressFilter(struct soap *soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter,
                          struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
	ONVIF_TRACE("__tds__GetIPAddressFilter TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetIPAddressFilter(struct soap *soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter,
                          struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
	ONVIF_TRACE("__tds__SetIPAddressFilter TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__AddIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__AddIPAddressFilter(struct soap *soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter,
                          struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
	ONVIF_TRACE("__tds__AddIPAddressFilter TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__RemoveIPAddressFilter' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__RemoveIPAddressFilter(struct soap *soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter,
                             struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
	ONVIF_TRACE("__tds__RemoveIPAddressFilter TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetAccessPolicy' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap *soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy,
                                                 struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
	ONVIF_TRACE("__tds__GetAccessPolicy TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetAccessPolicy' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap *soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy,
                                                 struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
	ONVIF_TRACE("__tds__SetAccessPolicy TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__CreateCertificate' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__CreateCertificate(struct soap *soap, struct _tds__CreateCertificate *tds__CreateCertificate,
                         struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
	ONVIF_TRACE("__tds__CreateCertificate TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetCertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap *soap, struct _tds__GetCertificates *tds__GetCertificates,
                                                 struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
	ONVIF_TRACE("__tds__GetCertificates TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetCertificatesStatus' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetCertificatesStatus(struct soap *soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus,
                             struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
	ONVIF_TRACE("__tds__GetCertificatesStatus TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetCertificatesStatus' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetCertificatesStatus(struct soap *soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus,
                             struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
	ONVIF_TRACE("__tds__SetCertificatesStatus TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__DeleteCertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__DeleteCertificates(struct soap *soap, struct _tds__DeleteCertificates *tds__DeleteCertificates,
                          struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
	ONVIF_TRACE("__tds__DeleteCertificates TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetPkcs10Request' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap *soap,
                                                  struct _tds__GetPkcs10Request *tds__GetPkcs10Request,
                                                  struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
	ONVIF_TRACE("__tds__GetPkcs10Request TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__LoadCertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap *soap,
                                                  struct _tds__LoadCertificates *tds__LoadCertificates,
                                                  struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
	ONVIF_TRACE("__tds__LoadCertificates TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetClientCertificateMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetClientCertificateMode(struct soap *soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode,
                                struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
	ONVIF_TRACE("__tds__GetClientCertificateMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetClientCertificateMode' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetClientCertificateMode(struct soap *soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode,
                                struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
	ONVIF_TRACE("__tds__SetClientCertificateMode TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetRelayOutputs' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs,
                                                 struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	ONVIF_TRACE("__tds__GetRelayOutputs TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetRelayOutputSettings' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetRelayOutputSettings(struct soap *soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings,
                              struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
	ONVIF_TRACE("__tds__SetRelayOutputSettings TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetRelayOutputState' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState,
                           struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	ONVIF_TRACE("__tds__SetRelayOutputState TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SendAuxiliaryCommand' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SendAuxiliaryCommand(struct soap *soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand,
                            struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
	ONVIF_TRACE("__tds__SendAuxiliaryCommand TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetCACertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetCACertificates(struct soap *soap, struct _tds__GetCACertificates *tds__GetCACertificates,
                         struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
	ONVIF_TRACE("__tds__GetCACertificates TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__LoadCertificateWithPrivateKey' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(
        struct soap *soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey,
        struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
	ONVIF_TRACE("__tds__LoadCertificateWithPrivateKey TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetCertificateInformation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(
        struct soap *soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation,
        struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
	ONVIF_TRACE("__tds__GetCertificateInformation TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__LoadCACertificates' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__LoadCACertificates(struct soap *soap, struct _tds__LoadCACertificates *tds__LoadCACertificates,
                          struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
	ONVIF_TRACE("__tds__LoadCACertificates TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__CreateDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__CreateDot1XConfiguration(struct soap *soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration,
                                struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
	ONVIF_TRACE("__tds__CreateDot1XConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetDot1XConfiguration(struct soap *soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration,
                             struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
	ONVIF_TRACE("__tds__SetDot1XConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetDot1XConfiguration(struct soap *soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration,
                             struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
	ONVIF_TRACE("__tds__GetDot1XConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDot1XConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetDot1XConfigurations(struct soap *soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations,
                              struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
	ONVIF_TRACE("__tds__GetDot1XConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__DeleteDot1XConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__DeleteDot1XConfiguration(struct soap *soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration,
                                struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
	ONVIF_TRACE("__tds__DeleteDot1XConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDot11Capabilities' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetDot11Capabilities(struct soap *soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities,
                            struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
	ONVIF_TRACE("__tds__GetDot11Capabilities TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetDot11Status' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap *soap, struct _tds__GetDot11Status *tds__GetDot11Status,
                                                struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
	ONVIF_TRACE("__tds__GetDot11Status TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__ScanAvailableDot11Networks' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(
        struct soap *soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks,
        struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
	ONVIF_TRACE("__tds__ScanAvailableDot11Networks TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetSystemUris' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap *soap, struct _tds__GetSystemUris *tds__GetSystemUris,
                                               struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
	ONVIF_TRACE("__tds__GetSystemUris TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__StartFirmwareUpgrade' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__StartFirmwareUpgrade(struct soap *soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade,
                            struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
	ONVIF_TRACE("__tds__StartFirmwareUpgrade TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__StartSystemRestore' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__StartSystemRestore(struct soap *soap, struct _tds__StartSystemRestore *tds__StartSystemRestore,
                          struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
	ONVIF_TRACE("__tds__StartSystemRestore TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetStorageConfigurations' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetStorageConfigurations(struct soap *soap, struct _tds__GetStorageConfigurations *tds__GetStorageConfigurations,
                                struct _tds__GetStorageConfigurationsResponse *tds__GetStorageConfigurationsResponse)
{
	ONVIF_TRACE("__tds__GetStorageConfigurations TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__CreateStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateStorageConfiguration(
        struct soap *soap, struct _tds__CreateStorageConfiguration *tds__CreateStorageConfiguration,
        struct _tds__CreateStorageConfigurationResponse *tds__CreateStorageConfigurationResponse)
{
	ONVIF_TRACE("__tds__CreateStorageConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__GetStorageConfiguration(struct soap *soap, struct _tds__GetStorageConfiguration *tds__GetStorageConfiguration,
                               struct _tds__GetStorageConfigurationResponse *tds__GetStorageConfigurationResponse)
{
	ONVIF_TRACE("__tds__GetStorageConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__SetStorageConfiguration(struct soap *soap, struct _tds__SetStorageConfiguration *tds__SetStorageConfiguration,
                               struct _tds__SetStorageConfigurationResponse *tds__SetStorageConfigurationResponse)
{
	ONVIF_TRACE("__tds__SetStorageConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__DeleteStorageConfiguration' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteStorageConfiguration(
        struct soap *soap, struct _tds__DeleteStorageConfiguration *tds__DeleteStorageConfiguration,
        struct _tds__DeleteStorageConfigurationResponse *tds__DeleteStorageConfigurationResponse)
{
	ONVIF_TRACE("__tds__DeleteStorageConfiguration TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__GetGeoLocation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetGeoLocation(struct soap *soap, struct _tds__GetGeoLocation *tds__GetGeoLocation,
                                                struct _tds__GetGeoLocationResponse *tds__GetGeoLocationResponse)
{
	ONVIF_TRACE("__tds__GetGeoLocation TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__SetGeoLocation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6 __tds__SetGeoLocation(struct soap *soap, struct _tds__SetGeoLocation *tds__SetGeoLocation,
                                                struct _tds__SetGeoLocationResponse *tds__SetGeoLocationResponse)
{
	ONVIF_TRACE("__tds__SetGeoLocation TODO\n");
	return SOAP_OK;
}
/** Web service operation '__tds__DeleteGeoLocation' (returns SOAP_OK or error code) */
SOAP_FMAC5 int SOAP_FMAC6
__tds__DeleteGeoLocation(struct soap *soap, struct _tds__DeleteGeoLocation *tds__DeleteGeoLocation,
                         struct _tds__DeleteGeoLocationResponse *tds__DeleteGeoLocationResponse)
{
	ONVIF_TRACE("__tds__DeleteGeoLocation TODO\n");
	return SOAP_OK;
}
