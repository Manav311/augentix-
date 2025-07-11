#include "tutk_pair.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <asm-generic/errno-base.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "tutk_define.h"
#include "disposable_params.h"
#include "log_define.h"

#include "agtx_lpw.h"

extern char gUdid[MAX_UDID_LENGTH + 1];
extern char gPinCode[MAX_PIN_CODE_LENGTH + 1];
extern char gSecretId[MAX_NEBULA_SECRETID_LENGTH + 1];

extern unsigned int gBindAbort;
extern char gUserIdentitiesFilePath[];
extern char gProfilePath[128];
extern NebulaJsonObject *gProfileJsonObj;

typedef struct {
	char ssid[32];
	char type[12];
	int dbm;
} WiFi_config;

extern lpw_handle gWifihd;

int disable_wifi_STA()
{
	/** disable already WiFi*/
	char tmpWpaSupp[512] = "";
	sprintf(tmpWpaSupp, "killall /usr/sbin/lpw_supplicant \n");
	TUTK_runSystemCmdWithRetry(tmpWpaSupp);

	return 0;
}

int startSoftAP()
{
	printf("----------0-0-Start SoftAP Init-0-0--------------\n");

	gWifihd = lpw_open();
	if (gWifihd == (lpw_handle)NULL) {
		printf("open lpw device fail\n");
	}

	lpw_wifi_ap_config_t ap_conf = {
		.channel_num = 10,
		.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
		.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
		.ssid = "AGT300-27-AP",
		.key = "12345678",
	};

	do {
		int result = lpw_wifi_set_ap_mode(gWifihd, &ap_conf);
		printf("[liblpw] run lpw_wifi_set_ap_mode(hd, config), ret = %d", result);
		if (result != 0) {
			printf("... Fail!\n");
		} else {
			printf("... OK!\n");
		}
	} while (0);

	if (gWifihd != (lpw_handle)NULL) {
		printf("For lpw module workaround\n");
		lpw_close(gWifihd);
	}

	gWifihd = lpw_open();
	if (gWifihd == (lpw_handle)NULL) {
		printf("open lpw device fail\n");
	}

	do {
		int result = lpw_wifi_set_ap_mode(gWifihd, &ap_conf);
		printf("[liblpw] workaround.. run lpw_wifi_set_ap_mode(hd, config), ret = %d", result);
		if (result != 0) {
			printf("... Fail!\n");
		} else {
			printf("... OK!\n");
		}
	} while (0);

	printf("---------------0-0-SoftAP Done-0-0---------------\n");

	return 0;
}

#define MAX_WIFI_RETRY_TIMES (5)
int stopSoftAPandConnectByLpw(const char *ssid, const char *pwd, const int enctype)
{
	(void)(enctype);

	printf("----------0-0-Stop SoftAP Init-0-0--------------\n");
	int ret = 0;
	/* wifi status */
	lpw_wifi_state_t st;

	ret = lpw_wifi_get_status(gWifihd, &st);
	if (ret == 0) {
		printf("ip %d:%d:%d:%d, gw:%d:%d:%d:%d, conn: %d\n", st.ip[0], st.ip[1], st.ip[2], st.ip[3], st.gw[0],
		       st.gw[1], st.gw[2], st.gw[3], st.conn);
	}

	do {
		int result = lpw_wifi_set_sta_mode(gWifihd);
		printf("[liblpw] run lpw_wifi_set_sta_mode(hd), ret = %d", result);
		if (result != 0) {
			printf("... Fail!\n");
		} else {
			printf("... OK!\n");
		}
	} while (0);

	ret = lpw_wifi_get_status(gWifihd, &st);
	if (ret == 0) {
		printf("ip %d:%d:%d:%d, gw:%d:%d:%d:%d, conn: %d\n", st.ip[0], st.ip[1], st.ip[2], st.ip[3], st.gw[0],
		       st.gw[1], st.gw[2], st.gw[3], st.conn);
	}

	printf("----------0-0-Start STA mode Init-0-0--------------\n");

	/* scan target and result */
	lpw_wifi_scan_t sc;
	memset(&sc, 0, sizeof(lpw_wifi_scan_t));
	sc.ssid = calloc(WIFI_MAX_SSID_LEN + 1, sizeof(char));
	lpw_wifi_scan_result_t result;

	/* connect info */
	lpw_wifi_conn_t conn;
	memset(&conn, 0, sizeof(lpw_wifi_conn_t));
	conn.ssid = calloc(WIFI_MAX_SSID_LEN + 1, sizeof(char));
	conn.key = calloc(WIFI_MAX_KEY_LEN + 1, sizeof(char));

	strcpy(sc.ssid, ssid);
	sc.scan_type = WIFI_SSID_SCAN;

	int sc_ret = -1;
	int retry_times = 0;
	while (sc_ret != 0) {
		sc_ret = lpw_wifi_scan(gWifihd, &sc, &result);
		if (sc_ret == 0) {
			printf("%s ...present\n", sc.ssid);

			strcpy(conn.ssid, ssid);
			strncpy(conn.key, pwd, WIFI_MAX_KEY_LEN);
			conn.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX;
			conn.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX;
			memset(&conn.bssid, 0, WIFI_MAC_LEN);
		} else {
			printf("...doesn't exist\n");
		}

		if (retry_times > MAX_WIFI_RETRY_TIMES) {
			free(conn.ssid);
			free(conn.key);
			free(sc.ssid);
			return -EINVAL;
		}
		retry_times++;
		tutkservice_log_info("scan retry: %d\n", retry_times);
	}

	retry_times = 0;
	while (1) {
		int result = lpw_wifi_connect_to(gWifihd, &conn);
		printf("[liblpw] run lpw_wifi_connect_to(hd, config), ret = %d", result);
		if (result != 0) {
			printf("... Fail!\n");
		} else {
			printf("... OK!\n");
			break;
		}

		if (retry_times > MAX_WIFI_RETRY_TIMES) {
			free(conn.ssid);
			free(conn.key);
			free(sc.ssid);
			return -EINVAL;
		}
		retry_times++;
		tutkservice_log_info("TCP connect retry: %d\n", retry_times);
	}

	ret = lpw_wifi_get_status(gWifihd, &st);
	if (ret == 0) {
		printf("ip %d:%d:%d:%d, gw:%d:%d:%d:%d, conn: %d\n", st.ip[0], st.ip[1], st.ip[2], st.ip[3], st.gw[0],
		       st.gw[1], st.gw[2], st.gw[3], st.conn);
	}

	free(conn.ssid);
	free(conn.key);
	free(sc.ssid);

	printf("---------------0-0-STA mode Done-0-0---------------\n");

	return 0;
}

int TUTK_pairingWiFi(char *udid)
{
	/* TODO: TUTK_pairingWiFi*/
	if (access(BIND_FLAG, F_OK) != 0) {
		tutkservice_log_info("device LOCAL_BIND\n");
		disable_wifi_STA();
		char admin_psk[MAX_NEBULA_PSK_LENGTH + 1] = { 0 };
		int ret = GetPskFromFile("admin", gUserIdentitiesFilePath, admin_psk, MAX_NEBULA_PSK_LENGTH);
		if (ret != 200) {
			AppendPskToFile("admin", gUserIdentitiesFilePath, admin_psk, MAX_NEBULA_PSK_LENGTH);
		}

		FILE *fp = fopen(gProfilePath, "r");
		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char *profile_buf = calloc(1, file_size + 1);
		fread(profile_buf, 1, file_size, fp);
		fclose(fp);
		profile_buf[file_size] = '\0';

		ret = Nebula_Json_Obj_Create_From_String(profile_buf, &gProfileJsonObj);
		if (ret != IOTC_ER_NoERROR) {
			tutkservice_log_err("profile format error,  exit...!!\n");
			return -1;
		}
		Nebula_Json_Obj_To_String(gProfileJsonObj);

		startSoftAP();

		ret = loadDisposableParams(gPinCode, gSecretId, NULL);
		if (ret < 0) {
			tutkservice_log_err("loadDisposableParams()=%d,exit...!!\n", ret);
			return -1;
		}

		free(profile_buf);

		Device_LAN_WIFI_Config(udid, admin_psk, gSecretId, gProfileJsonObj, &gBindAbort);

	} else {
		/*wifi on*/
		tutkservice_log_info("has already connect to WiFi\n");
	}

	return 0;
}

int TUTK_disableWiFi()
{
	lpw_close(gWifihd);
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "NebulaWiFiConfig.h"
#include "NebulaLANAPIs.h"
#include "NebulaJsonAPIs.h"

#include "log_define.h"
#include "tutk_define.h"

#define COUNTRY_CODE_SCRIPT "/system/script/load_wifi.sh"
#define WIFI_SCAN_LIST "/tmp/wifi_list"
#define WIFI_AP_SCAN_CMD "iwlist wlan0 scan"
#define WIFI_KEY_WORD "ESSID"

/*
=====Initialize=====
1.Init device net status
2.Peripheral broadcast BLE Beacon & wait connect
3.Start notfy thread & method handler
4.Send BLE service & characterist of this Devcie(Peripheral)

=====Exchange IOCtrl Message=====
(1) Central WRITE TUTK IOCtrl Message to Peripheral.
(2) Nebula_Restore_IOCtrl_From_BLE() will restore this message.
(3) if restore TUTK IOCtrl message suuccess, this callback handle message of Central.
(4) identify message type (customer defined , TUTK defined or unknow message).
(5) handle IOCTRL_XXX_REQ & generate TUTK IOCtrl message of IOCTRL_XXX_RESP.
    (5-1) handle IOCTRL_UDID_REQ & generate IOCTRL_UDID_RESP example.
    (5-2) handle IOCTRL_SSIDLIST_REQ & generate IOCTRL_SSIDLIST_RESP example.
    (5-3) handle IOCTRL_SETWIFI_REQ & generate IOCTRL_SETWIFI_RESP example.
(6) Central READ TUTK IOCtrl Message From Peripheral.

=====Notify of Net Status=====
Send NOTIFY after net status changed
*/

#define BUF_LEN 4096
#define LAN_SEARCH_UDID_LENGTH 20
#define HAUSETOPIA_LOCAL_BIND_PWD "888888ii"
#define HAUSETOPIA_LOCAL_BIND_PIN "0000"

static char gUDID[MAX_UDID_LENGTH + 1] = { 0 }; //unique ID of this device
static char *gDeviceName = NULL;
static char *gDeviceFwVer = NULL;
static char gPsk[MAX_NEBULA_PSK_LENGTH + 1] = { 0 };
static bool gDemoDone = false;

/**************************************************
 * Fake WiFi Functions (implement by customer)
***************************************************/

static int chkForHexDigit(char *ch)
{
	unsigned int i = 0;
	for (i = 0; i < strlen(ch); ++i) {
		if ((ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F') || (ch[i] >= '0' && ch[i] <= '9'))
			continue;
		else
			return 1;
	}
	return 0;
}

static int chkForSpecialChars(char *ch)
{
	unsigned int i = 0;
	for (i = 0; i < strlen(ch); ++i) {
		if (!(ch[i] >= ' ' && ch[i] <= '~'))
			return 1;
	}
	return 0;
}

static int setWpaSupplicant(const char *ssid, const char *pwd)
{
	char cmd[512] = "";
	unsigned int tmpPasswdLen = strlen(pwd);
	int ordr = 1; //Number of priority in wpa_supplicant.conf file

	if ((tmpPasswdLen > 63) || (tmpPasswdLen < 8 && tmpPasswdLen != 5 && tmpPasswdLen != 0) ||
	    chkForSpecialChars((char *)pwd)) {
		tutkservice_log_err("The password is NOT qualified...!!!\n");
		return -1;
	}

	if ((tmpPasswdLen == 10) || (tmpPasswdLen == 26) || (tmpPasswdLen == 32) || (tmpPasswdLen == 64)) {
		ordr = ordr - chkForHexDigit((char *)pwd);
	} else if ((tmpPasswdLen == 5) || (tmpPasswdLen == 13) || (tmpPasswdLen == 16)) { //wep ascii
		// Do nothing
	} else if ((tmpPasswdLen <= 0) || (pwd == NULL)) { //No Enc Key
		ordr = 1;
	} else {
		ordr = 2;
	}

	int offset = 0;
	char tmpSsid[64] = "";
	char tmpPasswd[128] = "";

	for (unsigned int i = 0; i < strlen(ssid); i++) {
		if (ssid[i] != '`' && ssid[i] != '\"' && ssid[i] != '$') {
			tmpSsid[i + offset] = ssid[i];
		} else {
			tmpSsid[i + offset++] = '\\';
			tmpSsid[i + offset] = ssid[i];
		}
	}

	offset = 0;
	for (unsigned int i = 0; i < strlen(pwd); i++) {
		if (pwd[i] != '\\' && pwd[i] != '\"' && pwd[i] != '$') {
			tmpPasswd[i + offset] = pwd[i];
		} else {
			tmpPasswd[i + offset++] = '\\';
			tmpPasswd[i + offset] = pwd[i];
		}
	}

	sprintf(cmd, "echo \"ctrl_interface=/var/run/wpa_supplicant \nupdate_config=1\n\n\" > %s",
	        WPA_SUPPLICANT_CONF_FILE);
	TUTK_exeSystemCmd(cmd);

	if (ordr >= 2) {
		if (tmpPasswdLen >= 8) {
			sprintf(cmd, "echo \"\n network={ \
					\n  ssid=\\\"%s\\\" \
					\n  key_mgmt=WPA-PSK \
					\n  pairwise=CCMP TKIP  \
					\n  group=CCMP TKIP  \
					\n  proto=WPA RSN  \
					\n  psk=\\\"%s\\\"  \
					\n  scan_ssid=1 \
					\n  priority=%d \n} \n\" >> %s",
			        tmpSsid, tmpPasswd, ordr, WPA_SUPPLICANT_CONF_FILE);
			TUTK_exeSystemCmd(cmd);
			ordr = ordr - 1;
		} else {
			ordr = ordr - 1;
		}
	}

	if (ordr == 2) { // ie passwd has no special characters
		if ((tmpPasswdLen == 5) || (tmpPasswdLen == 13) || (tmpPasswdLen == 16)) {
			// ASCII string
			sprintf(cmd, "echo \"\n network={ \
					\n  ssid=\\\"%s\\\"  \
					\n  key_mgmt=NONE  \
					\n  wep_key0=\\\"%s\\\"  \
					\n  wep_tx_keyidx=0 \
					\n  scan_ssid=1 \
					\n  priority=%d  \
					\n} \n\" >> %s",
			        tmpSsid, tmpPasswd, ordr, WPA_SUPPLICANT_CONF_FILE);
		} else {
			// hex string
			sprintf(cmd, "echo \"\n network={ \
					\n  ssid=\\\"%s\\\"  \
					\n  key_mgmt=NONE  \
					\n  wep_key0=%s  \
					\n  wep_tx_keyidx=0 \
					\n  scan_ssid=1 \
					\n  priority=%d  \
					\n} \n\" >> %s",
			        tmpSsid, tmpPasswd, ordr, WPA_SUPPLICANT_CONF_FILE);
		}
		TUTK_exeSystemCmd(cmd);
	} else { // For open network ONLY
		sprintf(cmd, "echo \"\n network={ \
					\n  ssid=\\\"%s\\\" \
					\n  key_mgmt=WPA-PSK \
					\n  pairwise=CCMP TKIP  \
					\n  group=CCMP TKIP  \
					\n  proto=WPA RSN  \
					\n  psk=\\\"%s\\\"  \
					\n  scan_ssid=1 \
					\n  priority=%d \n} \n\" >> %s",
		        tmpSsid, tmpPasswd, ordr, WPA_SUPPLICANT_CONF_FILE);
		TUTK_exeSystemCmd(cmd);
	}

	return 0;
}

WiFi_config gBest5WiFiAP[MAX_WIFI_LIST_NUM];
int UpdateWifiList()
{
	/** fake data*/
	gBest5WiFiAP[0].dbm = -46;
	strcpy(gBest5WiFiAP[0].ssid, "WiFiSSID1");
	strcpy(gBest5WiFiAP[0].type, "WPA2PSKAES");

	gBest5WiFiAP[1].dbm = -53;
	strcpy(gBest5WiFiAP[1].ssid, "WiFiSSID2");
	strcpy(gBest5WiFiAP[1].type, "WPA2PSKTKIP");

	gBest5WiFiAP[2].dbm = -59;
	strcpy(gBest5WiFiAP[2].ssid, "WiFiSSID3");
	strcpy(gBest5WiFiAP[2].type, "WPAAES");

	gBest5WiFiAP[3].dbm = -63;
	strcpy(gBest5WiFiAP[3].ssid, "WiFiSSID4");
	strcpy(gBest5WiFiAP[3].type, "WPA2PSKTKIP");

	gBest5WiFiAP[4].dbm = -68;
	strcpy(gBest5WiFiAP[4].ssid, "WiFiSSID5");
	strcpy(gBest5WiFiAP[4].type, "WPA2PSKAES");

	return 5;
}

int ConnectToWiFiInfo(char *ssid, char *pwd, int enctype)
{
	(void)(enctype);
	printf("=====Connect to Wifi SUCCESS=====\n");
	printf("\tSSID=%s\n", ssid);
	printf("\tPassowrd=%s\n", pwd);
	printf("=================================\n");
	return 0;
}

/**************************************************
 * Nebula Example functions
***************************************************/

int Respond_NotSupport(uint16_t type)
{
	NebulaIOCtrlNotSupportMsg tmp_not_support_msg;
	tmp_not_support_msg.type = type;
	int ret = Nebula_Send_IOCtrl_On_LAN(IOCTRL_NOT_SUPPORT_MSG, (char *)&tmp_not_support_msg,
	                                    sizeof(tmp_not_support_msg));
	if (ret < NEBULA_ER_NoERROR)
		tutkservice_log_err("[%s():%d] send message error[%d]\n", __FUNCTION__, __LINE__, ret);
	return ret;
}

void Handle_IOCTRL_Cmd(int type, char *ioctrl_buf, int ioctrl_len)
{
	int ret = 0;

	if (type >= IOCTRL_USER_DEFINED_START) {
		if (type == IOCTRL_USER_DEFINED_START || type == IOCTRL_USER_DEFINED_END) {
			tutkservice_log_err(
			        "[%s():%d] type IOCTRL_USER_DEFINED_START & IOCTRL_USER_DEFINED_END are valid msg\n",
			        __FUNCTION__, __LINE__);
		}

		tutkservice_log_info("###USER DEFINE MESSAGE###\n");
		tutkservice_log_info("\ttype=%u\n", type);
		//handle by customer
	} else if (type <= IOCTRL_RESERVE ||
	           (type >= IOCTRL_MSG_MAX_COUNT && type < IOCTRL_NOT_SUPPORT_MSG)) //not tutk defined msg
	{
		tutkservice_log_info("[%s():%d] get unknow message type=%d, ioctrl_len=%d\n", __FUNCTION__, __LINE__,
		                     type, ioctrl_len);
		Respond_NotSupport(type);
	} else //TUTK defined IOCtrl Message
	{
		switch (type) {
		case IOCTRL_UDID_REQ: {
			tutkservice_log_info("Handle CMD: IOCTRL_UDID_REQ\n");

			NebulaIOCtrlMsgUDIDResp udid_resp;
			memset(&udid_resp, 0, sizeof(udid_resp));

			strcpy(udid_resp.pin_code, HAUSETOPIA_LOCAL_BIND_PIN);
			memcpy(udid_resp.udid, gUDID, MAX_PUBLIC_UDID_LENGTH);

			ret = Nebula_Send_IOCtrl_On_LAN(IOCTRL_UDID_RESP, (char *)&udid_resp, sizeof(udid_resp));

			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
				gDemoDone = true;
			}
			break;
		}
		case IOCTRL_SETREGION_REQ: {
			tutkservice_log_info("Handle CMD: IOCTRL_SETREGION_REQ\n");

			NebulaIOCtrlMsgSetRegionResp region_resp;
			NebulaIOCtrlMsgSetRegionReq *region_req = (NebulaIOCtrlMsgSetRegionReq *)ioctrl_buf;
			memset(&region_resp, 0, sizeof(region_resp));
			tutkservice_log_info("Set region %d\n", region_req->tutk_region);

			region_resp.result = CONFIG_SET_SUCCESS;

			ret = Nebula_Send_IOCtrl_On_LAN(IOCTRL_SETREGION_RESP, (char *)&region_resp,
			                                sizeof(region_resp));
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
				gDemoDone = true;
			}
			break;
		}
		case IOCTRL_SSIDLIST_REQ: //@@@@@(5-2) handle IOCTRL_SSIDLIST_REQ & generate IOCTRL_SSIDLIST_RESP example.@@@@@
		{
			tutkservice_log_info("Handle CMD: IOCTRL_SSIDLIST_REQ\n");
			NebulaIOCtrlMsgSSIDListReq *ssid_list_req;
			NebulaIOCtrlMsgSSIDListResp ssid_list_resp;
			memset(&ssid_list_resp, 0, sizeof(ssid_list_resp));
			if (ioctrl_len != sizeof(NebulaIOCtrlMsgSSIDListReq)) {
				tutkservice_log_err("!!!INVALID IOCTRL_SSIDLIST_REQ MESSAGE!!!\n");
				gDemoDone = true;
				break;
			}
			ssid_list_req = (NebulaIOCtrlMsgSSIDListReq *)ioctrl_buf;

			//Get WiFi SSID list & put configure to struct NebulaIOCtrlMsgSSIDListResp
			int max_count = UpdateWifiList();
			max_count = (max_count < ssid_list_req->max_ap_count) ? max_count : ssid_list_req->max_ap_count;

			int buf_index = 0;
			char list_buf[BUF_LEN] = { 0 };
			for (int i = 0; i < max_count; i++) {
				strcpy(ssid_list_resp.ssid, gBest5WiFiAP[i].ssid);
				if (strcmp(gBest5WiFiAP[i].type, "WPA2PSKAES") == 0) {
					ssid_list_resp.enctype = WIFIAPENC_WPA2_PSK_AES;
				} else if (strcmp(gBest5WiFiAP[i].type, "WPA2PSKTKIP") == 0) {
					ssid_list_resp.enctype = WIFIAPENC_WPA2_PSK_TKIP;
				} else if (strcmp(gBest5WiFiAP[i].type, "WPAAES") == 0) {
					ssid_list_resp.enctype = WIFIAPENC_WPA_AES;
				}
				memcpy(list_buf + buf_index, (uint8_t *)&ssid_list_resp,
				       sizeof(NebulaIOCtrlMsgSSIDListResp));

				buf_index += sizeof(NebulaIOCtrlMsgSSIDListResp);

				if (i == 4)
					break;
			}

			ret = Nebula_Send_IOCtrl_On_LAN(IOCTRL_SSIDLIST_RESP, list_buf, buf_index);
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
				gDemoDone = true;
			}
			break;
		}
		case IOCTRL_SETWIFI_REQ: //@@@@@(5-3) handle IOCTRL_SETWIFI_REQ & generate IOCTRL_SETWIFI_RESP example.@@@@@
		{
			tutkservice_log_info("Handle CMD: IOCTRL_SETWIFI_REQ\n");
			NebulaIOCtrlMsgSetWifiReq *set_wifi_req;
			NebulaIOCtrlMsgSetWifiResp set_wifi_resp;
			memset(&set_wifi_resp, 0, sizeof(set_wifi_resp));
			if (ioctrl_len != sizeof(NebulaIOCtrlMsgSetWifiReq)) {
				tutkservice_log_err("!!!INVALID IOCTRL_SETWIFI_REQ MESSAGE!!!\n");
				gDemoDone = true;
				break;
			}
			set_wifi_req = (NebulaIOCtrlMsgSetWifiReq *)ioctrl_buf;

			//Try to connect WiFi AP & put result to struct NebulaIOCtrlMsgSetWifiResp
			strcpy(set_wifi_resp.ssid, set_wifi_req->ssid);
			ret = stopSoftAPandConnectByLpw(set_wifi_req->ssid, set_wifi_req->password,
			                                set_wifi_req->enctype);

			if (ret == 0) {
				set_wifi_resp.result = WIFICONN_SUCCESS;

				/*this function only log info*/
				ConnectToWiFiInfo(set_wifi_req->ssid, set_wifi_req->password, set_wifi_req->enctype);
				setWpaSupplicant(&set_wifi_req->ssid[0], set_wifi_req->password);
				gDemoDone = true;
			} else {
				set_wifi_resp.result = WIFICONN_FAIL;
				gDemoDone = false;

				tutkservice_log_info("restart AP mode\n");
				startSoftAP();
			}

			ret = Nebula_Send_IOCtrl_On_LAN(IOCTRL_SETWIFI_RESP, (char *)&set_wifi_resp,
			                                sizeof(NebulaIOCtrlMsgSetWifiResp));
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
			}

			break;
		}
		case IOCTRL_NEBULA_BIND_REQ: //@@@@@(5-4) handle IOCTRL_NEBULA_BIND_REQ & generate IOCTRL_NEBULA_BIND_RESP example.@@@@@
		{
			tutkservice_log_info("Handle CMD: IOCTRL_NEBULA_BIND_REQ\n");
			char *nebula_bind_response = NULL;
			if (ioctrl_len != sizeof(NebulaIOCtrlMsgNebulaBindReq)) {
				tutkservice_log_err("!!!INVALID IOCTRL_NEBULA_BIND_REQ MESSAGE!!!\n");
				gDemoDone = true;
				break;
			}
			//Generating device's information for client which used to create client's context.
			Nebula_Device_New_Credential(gUDID, "admin", gPsk, gSecretId, &nebula_bind_response);
			tutkservice_log_info("Create Nebula Device info string ,size[%d]\n",
			                     strlen(nebula_bind_response));
			tutkservice_log_info("info string: [%s]\n", nebula_bind_response);

			ret = Nebula_Send_IOCtrl_On_LAN(IOCTRL_NEBULA_BIND_RESP, nebula_bind_response,
			                                strlen(nebula_bind_response));
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
			}
			free(nebula_bind_response);

			break;
		}
		case IOTYPE_SETTIMEZONE_REQ: //@@@@@(5-5) handle IOTYPE_SETTIMEZONE_REQ & generate IOTYPE_SETTIMEZONE_RESP example.@@@@@
		{
			tutkservice_log_info("Handle CMD: IOTYPE_SETTIMEZONE_REQ\n");
			NebulaIOCtrlMsgTimeZoneReq *timezone_req = (NebulaIOCtrlMsgTimeZoneReq *)ioctrl_buf;
			NebulaIOCtrlMsgTimeZoneResp timezone_resp;
			tutkservice_log_info("call set timezone function or process (%s)\n",
			                     timezone_req->timezone_str);
			timezone_resp.result = CONFIG_SET_SUCCESS;
			ret = Nebula_Send_IOCtrl_On_LAN(IOTYPE_SETTIMEZONE_RESP, (char *)&timezone_resp,
			                                sizeof(timezone_resp));
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
			}

			break;
		}
		case IOTYPE_GETFWVERSION_REQ: //@@@@@(5-6) handle IOTYPE_GETFWVERSION_REQ & generate IOTYPE_GETFWVERSION_RESP example.@@@@@
		{
			tutkservice_log_info("Handle CMD: IOTYPE_GETFWVERSION_REQ\n");
			char fw_version_response[64] = { 0 };
			if (ioctrl_len != sizeof(NebulaIOCtrlMsgGetFWVersionReq)) {
				tutkservice_log_err("!!!INVALID IOTYPE_GETFWVERSION_REQ MESSAGE!!!\n");
				gDemoDone = true;
				break;
			}

			if (gDeviceFwVer != NULL) {
				strcpy(fw_version_response, gDeviceFwVer);
			} else {
				tutkservice_log_info("call get fw version function or process\n");
				strcpy(fw_version_response, "test-2.0.15A");
			}

			ret = Nebula_Send_IOCtrl_On_LAN(IOTYPE_GETFWVERSION_RESP, fw_version_response,
			                                strlen(fw_version_response) + 1);
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
			}
			break;
		}
		case IOTYPE_LAN_RESTART_REQ: {
			tutkservice_log_info("Handle CMD: IOTYPE_LAN_RESTART\n");
			tutkservice_log_info("Nebula LAN demo done, restart for next connection.\n");
			NebulaIOCtrlMsgLanRestartResp restart_resp;
			restart_resp.result = CONFIG_SET_SUCCESS;
			ret = Nebula_Send_IOCtrl_On_LAN(IOTYPE_LAN_RESTART_RESP, (char *)&restart_resp,
			                                sizeof(restart_resp));
			if (ret < 0) {
				tutkservice_log_err("@%d Nebula_Send_IOCtrl_On_LAN fail[%d]\n", __LINE__, ret);
			}
			break;
		}
		default: {
			tutkservice_log_info("[%s():%d] get unknow message type=%d, ioctrl_len=%d\n", __FUNCTION__,
			                     __LINE__, type, ioctrl_len);
			Respond_NotSupport(type);
		}
		}

		if (ret < NEBULA_ER_NoERROR) {
			tutkservice_log_err("!!!ERROR Nebula_Gen_IOCtrl_For_BLE() return %d !!![%d]\n", ret, __LINE__);
			return;
		}
	}
}

/**************************************************
 * Main functions
***************************************************/
int Device_LAN_WIFI_Config(const char *udid, const char *psk, const char *secret_id,
                           const NebulaJsonObject *profile_obj, unsigned int *abort_flag)
{
	int ret = 0;
	NebulaIOCtrlType recvType;
	char ioCtrlBuf[2000] = { 0 };
	const char *tmp = NULL;

	tutkservice_log_info("udid[%s]\n", udid);
	strcpy(gUDID, udid);
	strcpy(gPsk, psk);
	strcpy(gSecretId, secret_id);

	ret = Nebula_Json_Obj_Get_Sub_Obj_String(profile_obj, "name", &tmp);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("Error profile: lack name[%d]\n", ret);
		return -1;
	}
	gDeviceName = strdup(tmp);
	tutkservice_log_info("gDeviceName[%s]\n", gDeviceName);

	ret = Nebula_Json_Obj_Get_Sub_Obj_String(profile_obj, "fwVer", &tmp);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("Error profile: lack fwVer[%d]\n", ret);
		free(gDeviceName);
		return -1;
	}
	gDeviceFwVer = strdup(tmp);
	tutkservice_log_info("gDeviceFwVer[%s]\n", gDeviceFwVer);

	while (1) {
	start_paring:
		gDemoDone = false;

		tutkservice_log_info("WiFi Setup Start\n");

		ret = Nebula_WiFi_Setup_Start_On_LAN(DEVICE, 1);
		if (ret < 0) {
			tutkservice_log_err("WiFi Setup Start fail[%d]\n", ret);
			break;
		}

		tutkservice_log_info("UDID:%s\n", gUDID);
		tutkservice_log_info("PWD:%s\n", HAUSETOPIA_LOCAL_BIND_PWD);
		tutkservice_log_info("Name:%s\n", gDeviceName);
		ret = Nebula_Device_Listen_On_LAN(gUDID, HAUSETOPIA_LOCAL_BIND_PWD, gDeviceName, 60000);
		if (ret < 0) {
			tutkservice_log_err("WiFi Setup listen fail[%d]\n", ret);
			Nebula_WiFi_Setup_Stop_On_LAN();
			continue;
		}

		tutkservice_log_info("wait for recv IOCtrl\n");
		while (!gDemoDone) {
			/*at least 3 min, or input and then retry for */
			ret = Nebula_Recv_IOCtrl_From_LAN(&recvType, (char *)ioCtrlBuf, 2000, 1800000 /*sec timeout*/);
			if (ret > 0) {
				Handle_IOCTRL_Cmd(recvType, ioCtrlBuf, ret);

				if (recvType == IOCTRL_SETWIFI_REQ) {
					tutkservice_log_info("Device LAN Wifi Configure End\n");
				}
				if (recvType == IOTYPE_LAN_RESTART_REQ) {
					tutkservice_log_info("Device LAN Wifi Configure Restart\n");
					continue;
				}

			} else if (ret == NEBULA_ER_TIMEOUT) {
				tutkservice_log_err("Nebula_Recv_IOCtrl_From_LAN error, code[%d], NEBULA_ER_TIMEOUT\n",
				                    ret);
				TUTK_exeSystemCmd("reboot -f");
			} else if (ret != NEBULA_ER_TIMEOUT) {
				tutkservice_log_err("Nebula_Recv_IOCtrl_From_LAN error, code[%d]\n", ret);
				/*Nebula_WiFi_Setup_Start_On_LAN restart*/
				tutkservice_log_info("wait for recv IOCtrl\n");
				/*any other failed case will restart wifi pairing loop*/
				goto start_paring;
			}
		}


		if (abort_flag != NULL && *abort_flag != 0)
			break;

		if (recvType == IOCTRL_SETWIFI_REQ)
			break;

		if (recvType == IOTYPE_LAN_RESTART_REQ)
			continue;
	}

	Nebula_WiFi_Setup_Stop_On_LAN();

	free(gDeviceName);
	free(gDeviceFwVer);

	return 0;
}
