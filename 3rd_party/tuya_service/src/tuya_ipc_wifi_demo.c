/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/*********************************************************************************
  *Copyright(C),2015-2020,  www.tuya.comm
  *FileName: tuya_linux_wifi_demo.c
  *
  * File Description:
  * 1. WIFI operation API
  *
  * Developer work
  * 1. Connect to WIFI based on SSID and PASSWD.
  * 2. Grab the image and identify the QR code.
  * 3. Specific WIFI chip adaptation.
  *
**********************************************************************************/

/*
 * Caution:
 *   Include mpi_base_types.h in the very first one.
 *   In order to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_system_control_demo.h"
#include "tuya_ipc_motion_detect_demo.h"
#include "tuya_ipc_api.h"
#include "tuya_utils.h"
#include "tuya_ipc_common_demo.h"

#include "cJSON.h"
#include "wifi_hwl.h"
#include "errno.h"

#include "mpi_dev.h"
#include "mpi_sys.h"
#include "agtx_types.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/reboot.h>

#include "iwlib.h"

#define ZBAR_SUPPORT
#ifdef ZBAR_SUPPORT
#include "zbar.h"
#endif /* !ZBAR_SUPPORT */

#define QR_DEBUG_DEF
#ifdef QR_DEBUG_DEF
#define QR_DEBUG(fmt, args...) PR_DEBUG(fmt, ##args)
#else
#define QR_DEBUG(fmt, args...)
#endif /* QR_DEBUG_DEF */
//variable to know if IP is obtained in station Mode
int wfGotIP = 0;
int g_weak_wifi = 0;
unsigned int WIFI_OPERATIONAL_MODE;
char WLAN_DEV_MAC[6] = { 0 };

unsigned int  reboot_func_thread = 0;
pthread_t noWlan0_inf_th;

#ifdef ZBAR_SUPPORT
int qrcode_parse_from_buffer(void *y8data, int w, int h, char *pqrcode)
{
	int n = -1;
	zbar_image_scanner_t *scanner = NULL;
	scanner = zbar_image_scanner_create();

	zbar_image_t *image = zbar_image_create();
	zbar_image_set_userdata(image, NULL);
	zbar_image_set_format(image, *(int *)"Y800");
	zbar_image_set_size(image, w, h);
	zbar_image_set_data(image, y8data, w * h, (void *)zbar_image_get_userdata);

	n = zbar_scan_image(scanner, image);
	QR_DEBUG("[%s:%d]n=%d\n", __func__, __LINE__, n);
	const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
	if (symbol == NULL) {
		return -1;
	}
	for (; symbol; symbol = zbar_symbol_next(symbol)) {
		/* do something useful with results */
		zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
		const char *data = zbar_symbol_get_data(symbol);
		QR_DEBUG("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(typ), data);
		sprintf(pqrcode, "%s", data);
		PR_INFO("QR code scanned success!\n");
	}
	zbar_image_destroy(image);
	zbar_image_scanner_destroy(scanner);
	return n;
}
#endif /* !ZBAR_SUPPORT */

static int check_wifi_associated(void)
{
#define MAX_DATA_SIZE_BYTES 64
	FILE *fp;
	char str[MAX_DATA_SIZE_BYTES] = {0};
	char cmd[128] = {0};

	sprintf(cmd, "iwconfig wlan0 | grep ESSID");
	fp = popen(cmd, "r");
	if (fp == NULL){
		pclose(fp);
		return -1;
	}

	if (fgets(str, MAX_DATA_SIZE_BYTES, fp) != NULL) {
		pclose(fp);
		if (strstr(str, "off")) {
			puts(str);
			return -1;
		}
		return 0;
	}

	pclose(fp);
	return -1;
}

static void *notify_led(void *data __attribute__((unused)))
{
	char tmpWpaSupp[256];
	char tmpdhcpSupp[256];
	//char stopWlan0[128];
	//char enableWlan0[128];

	int fd;
	struct ifreq ifr;
	int count = 50;
	int count_ssid = 10;
	int count_mqtt = 10;

	//struct timeval startTime, curTime;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, WLAN_DEV, IFNAMSIZ - 1);
	//Wait for AP mode OFF
	usleep(10000*1000);
	do {
		/*finding IP : to make sure udhcpc worked to fetch IP */
		if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
			fprintf(stderr,"Fail to get wifi ip: Get SIOCGIFADDR fail\n");
			wfGotIP = 0;
			usleep(1000 * 1000);
		} else {
			printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
			wfGotIP = 1;
			goto next_;
		}
		count = count - 1;
	} while (count > 0);

	close(fd);
	//setLEDInform("Connecting_Fail", 1);
	printf("Time out 50 sec: Unable to get IP from AccessPoint\n");
	return (void *)0;

next_:
	while (1) {
		if (check_wifi_associated()) {
			if (count_ssid > 0) {
				--count_ssid;
				usleep(1000 * 1000);
				continue;
			}

			system("killall -2 wpa_supplicant");
			system("killall -2 udhcpc");
			usleep(1000 * 1000);
			system("ifdown wlan0");
			usleep(1000 * 1000);
			system("ifup wlan0");
			usleep(1000 * 1000);
			sprintf(tmpWpaSupp, "wpa_supplicant -B -i %s -c %s", WLAN_DEV, WPA_SUPPLICANT_CONF_FILE);
			system(tmpWpaSupp);
			usleep(1000 * 1000);
			sprintf(tmpdhcpSupp, "udhcpc -n -i %s -R -t 20 -T 2 -p /var/run/udhcpc.pid -S", WLAN_DEV);
			system(tmpdhcpSupp);
			count_ssid = 10;
			count_mqtt = 10;
		} else {
			//One case that iwconfig gets the ESSID, however, udhcpc is
			//NOT able to set the ip, so we still need to exam mqtt status!
			//finding IP : to make sure udhcpc worked to fetch IP

			if (count_mqtt > 0 && ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
				fprintf(stderr, "Fail to get wifi ip: Get SIOCGIFADDR fail\n");
				usleep(3000 * 1000);
				--count_mqtt;
				count_ssid = 10;
			} else if (count_mqtt == 0 && ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
				system("killall -2 wpa_supplicant");
				system("killall -2 udhcpc");
				usleep(1000 * 1000);
				system("ifdown wlan0");
				usleep(1000 * 1000);
				system("ifup wlan0");
				usleep(1000 * 1000);
				//system("ip addr flush dev wlan0");
				//usleep(1000*1000);
				//system("ip link set dev wlan0 up");
				//usleep(1000*1000);
				sprintf(tmpWpaSupp, "wpa_supplicant -B -i %s -c %s", WLAN_DEV,
				        WPA_SUPPLICANT_CONF_FILE);
				system(tmpWpaSupp);
				usleep(1000 * 1000);
				sprintf(tmpdhcpSupp, "udhcpc -n -i %s -R -t 20 -T 2 -p /var/run/udhcpc.pid -S",
				        WLAN_DEV);
				system(tmpdhcpSupp);
				count_mqtt = 10;
				count_ssid = 10;
			} else {
				count_mqtt = 10;
				usleep(3000 * 1000);
			}
		}
	}

	close(fd);
    return (void *)0;
}

/*
Interface that the user needs to implement
hwl_wf_station_connect
__tuya_linux_get_snap_qrcode
Other interfaces can be used directly in theory, if a user has their own implementation, they can be replaced.
*/
static int chkForSpecialChars(char *ch)
{
	int i = 0;
	for (i = 0; (unsigned long)i < strlen(ch); ++i) {
		if ((ch[i] >= ' ' && ch[i] <= '~'))
			continue;
		else
			return 1;
	}
	return 0;
}

static int chkForHexDigit(char *ch)
{
	int i = 0;
	for (i = 0; (unsigned long)i < strlen(ch); ++i) {
		if ((ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F') || (ch[i] >= '0' && ch[i] <= '9'))
			continue;
		else
			return 1;
	}
	return 0;
}

static BOOL_T sniffer_set_done = FALSE;
static	pthread_t led_notify_th;

OPERATE_RET hwl_wf_station_connect(IN CONST CHAR_T *ssid, IN CONST CHAR_T *passwd)
{
	if (sniffer_set_done) {
		sniffer_set_done = FALSE;
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_REV_WIFI_CFG);
		usleep(1000 * 1000);
	}

	IPC_APP_Notify_LED_Sound_Status_CB(IPC_CONNECTING_WIFI);

	if (NULL == ssid) {
		//get bind info from ethernet network
		PR_DEBUG("get bind info ...\n");
	} else {
		//get bind info from ap / wifi-smart / qrcode
		PR_DEBUG("get wifi info ...\n");
	}

	//Add a blocking operation for the wifi connection here.
	PR_NOTICE("----------0-0-Start Wifi Init-0-0--------------\n");
	int status;
	char tmpWpaSupp[512] = "";
	char tmpWepSupp[256] = "";
	char tmpOpenAuth[256] = "";
	char *name = "ledNotify";
	int ordr = 3; //Number of priority in wpa_supplicant.conf file
	int tmpPasswdLen = strlen(passwd);

	//PR_NOTICE("DBG:Construct wpa_supplicant.conf , passed wifi passwd:len =%s:%d\n",
	//         passwd,tmpPasswdLen);

	if ((tmpPasswdLen > 63) || (tmpPasswdLen < 8 && tmpPasswdLen != 5) || chkForSpecialChars((char *)passwd)) {
		PR_ERR("[WIFI] Connecting fail: %s\n", strerror(errno));
		while (1) {
			usleep(1000 * 3000);
		}
	}


	if ((tmpPasswdLen == 10) || (tmpPasswdLen == 26) || (tmpPasswdLen == 32) || (tmpPasswdLen == 64)) {
		ordr = ordr - chkForHexDigit((char *)passwd);
	} else if ((tmpPasswdLen == 5) || (tmpPasswdLen == 13) || (tmpPasswdLen == 16)) { //wep ascii
		// Do nothing
	} else if ((tmpPasswdLen <= 0) || (passwd == NULL)) { //No Enc Key
		ordr = 1;
	} else {
		ordr = 2;
	}

	int offset;
	int i;

	char tmpSsid[64] = "";
	char tmpPasswd[128] = "";

	offset = 0;
	i = 0;
	for (i = 0; (unsigned long)i < strlen(ssid); i++) {
		if (ssid[i] != '\\' && ssid[i] != '\"' && ssid[i] != '$') {
			tmpSsid[i + offset] = ssid[i];
		} else {
			tmpSsid[i + offset++] = '\\';
			tmpSsid[i + offset] = ssid[i];
		}
	}

	offset = 0;
	i = 0;
	for (i = 0; (unsigned long)i < strlen(passwd); i++) {
		if (passwd[i] != '\\' && passwd[i] != '\"' && passwd[i] != '$') {
			tmpPasswd[i + offset] = passwd[i];
		} else {
			tmpPasswd[i + offset++] = '\\';
			tmpPasswd[i + offset] = passwd[i];
		}
	}
	//printf("tmpSsid --->%s<---\n", tmpSsid);
	//printf("tmpPasswd --->%s<---\n", tmpPasswd);

	if (ordr >= 2) {
		if ( tmpPasswdLen >= 8) {
			sprintf(tmpWpaSupp, "echo \"ctrl_interface=/var/run/wpa_supplicant \n update_config=1 \n  \
					\n network={\n \
					\n  ssid=\\\"%s\\\" \
					\n  key_mgmt=WPA-PSK \
					\n  pairwise=CCMP TKIP  \
					\n  group=CCMP TKIP  \
					\n  proto=WPA RSN  \
					\n  psk=\\\"%s\\\"  \
					\n  scan_ssid=1 \
					\n  priority=%d \n} \n\" > %s",
			        tmpSsid, tmpPasswd, ordr, WPA_SUPPLICANT_CONF_FILE);
			//printf("%s\n", tmpWpaSupp);
			system(tmpWpaSupp);
			ordr = ordr - 1;
		} else {
			ordr = ordr - 1;
		}

	}
	//printf("@@@ tmpWpaSupp @@@\n");
	//system("cat /tmp/wpa_supplicant.conf");
	if ( ordr == 2) { // ie passwd has no special characters
		if ((tmpPasswdLen == 5 ) || (tmpPasswdLen == 13 )||(tmpPasswdLen == 16 )) {
			// ASCII string
			sprintf(tmpWepSupp, "echo \"\n network={ \
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
			sprintf(tmpWepSupp, "echo \"\n network={ \
					\n  ssid=\\\"%s\\\"  \
					\n  key_mgmt=NONE  \
					\n  wep_key0=%s  \
					\n  wep_tx_keyidx=0 \
					\n  scan_ssid=1 \
					\n  priority=%d  \
					\n} \n\" >> %s",
			        tmpSsid, tmpPasswd, ordr, WPA_SUPPLICANT_CONF_FILE);
		}
		system(tmpWepSupp);
	}
	//printf("@@@ tmpWepSupp @@@\n");
	//system("cat /tmp/wpa_supplicant.conf");
	sprintf(tmpOpenAuth, "echo \"\n network={ \
			\n  ssid=\\\"%s\\\"  \
			\n  key_mgmt=NONE  \
			\n  scan_ssid=1 \
			\n  priority=1  \
			\n} \n\" >> %s",
	        tmpSsid, WPA_SUPPLICANT_CONF_FILE);
	system(tmpOpenAuth);
	//printf("@@@ tmpOpenAuth @@@\n");
	//system("cat /tmp/wpa_supplicant.conf");
	pthread_create(&led_notify_th, NULL, notify_led, name);

	sprintf(tmpWpaSupp, "/system/script/wifi_on.sh %s 3", WPA_SUPPLICANT_CONF_FILE);
	status = system(tmpWpaSupp);
	if (status < 0) {
		PR_ERR("[WIFI] error: %s\n", strerror(errno));
	}

	if (WIFEXITED(status)) {
		PR_INFO("normal termination, exit status = %d\n", WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
		PR_INFO("abnormal termination,signal number =%d\n", WTERMSIG(status));
	} else if (WIFSTOPPED(status)) {
		PR_INFO("process stopped, signal number =%d\n", WSTOPSIG(status));
	}

	PR_NOTICE("-----------------0-0-Done-0-0------------------\n");

	//sleep(2);
	//wfGotIP = 1; // We need to update this in real time. to make re-entry into wifi zone work

	pthread_detach(led_notify_th);

	int check_idx = 0;
	do {
		usleep(1000 * 3000);
		check_idx++;
		if ((check_idx % 20) == 0) {
			int wait_mins = (check_idx / 20);
			PR_NOTICE("wait %d minutes to connect wifi AP\n", wait_mins);
			if (wait_mins == 5) {
				printf("Failed to connect Wifi AP, reboot.\n");
				reboot(RB_AUTOBOOT);
			}
		}
	} while (wfGotIP == 0);

	return OPRT_OK;
}

/* Tune this value if based on the wifi network env around  */
#define MAX_AP_SEARCH 20 //Tune this value if based on the wifi network env around
static void exec_cmd(char *pCmd)
{
	PR_DEBUG("Exec Cmd:%s \n", pCmd);
	system(pCmd);
}

/* Example scan result from iwlist
wlan0    Scan completed :
    Cell 01 - Address: XX:XX:XX:XX:XX:XX
    Channel:1
    Frequency:2.412 GHz (Channel 1)
*/
OPERATE_RET hwl_wf_all_ap_scan(OUT AP_IF_S **ap_ary, OUT UINT_T *num)
{
	if (NULL == ap_ary || NULL == num) {
		return OPRT_INVALID_PARM;
	}

	static AP_IF_S s_aps[MAX_AP_SEARCH];

	memset(s_aps, 0, sizeof(s_aps));
	*ap_ary = s_aps;
	*num = 0;

	FILE *pp = popen("/usr/bin/iwlist wlan0 scan", "r");
	if (pp == NULL) {
		PR_ERR("popen fails\n");
		return OPRT_COM_ERROR;
	}

	char tmp[256] = { 0 };
	memset(tmp, 0, sizeof(tmp));

	int recordIdx = -1;
	while (fgets(tmp, sizeof(tmp), pp) != NULL) {
		/* First look for BSSID as a benchmark */
		char *pBSSIDStart = strstr(tmp, " - Address: ");
		if (pBSSIDStart != NULL) {
			recordIdx++;
			if (recordIdx >= MAX_AP_SEARCH) {
				PR_DEBUG(" Reach Max Record \n");
				recordIdx--;
				break;
			}

			BYTE_T *pTmp = s_aps[recordIdx].bssid;
			int x1, x2, x3, x4, x5, x6;

			sscanf(pBSSIDStart + strlen(" - Address: "), "%x:%x:%x:%x:%x:%x", &x1, &x2, &x3, &x4, &x5, &x6);
			pTmp[0] = x1 & 0xFF;
			pTmp[1] = x2 & 0xFF;
			pTmp[2] = x3 & 0xFF;
			pTmp[3] = x4 & 0xFF;
			pTmp[4] = x5 & 0xFF;
			pTmp[5] = x6 & 0xFF;

			goto ReadNext;
		} else {
			if (recordIdx < 0) { /* find the first BSSID to continue reading */
				goto ReadNext;
			}
		}

		{
			/* find the signal  */
			char *pSIGNALStart = strstr(tmp, "Quality=");
			if (pSIGNALStart != NULL) {
				int x = 0;
				int y = 0;
				sscanf(pSIGNALStart + strlen("Quality="), "%d/%d ", &x, &y);
				s_aps[recordIdx].rssi = x * 100 / (y + 1);
				goto ReadNext;
			}
		}

		{
			/* find the channel	*/
			char *pCHANNELStart = strstr(tmp, "(Channel ");
			if (pCHANNELStart != NULL) {
				int x = 0;
				sscanf(pCHANNELStart + strlen("(Channel "), "%d)", &x);
				s_aps[recordIdx].channel = x;
				goto ReadNext;
			}
		}

		{
			/* find the ssid  */
			char *pSSIDStart = strstr(tmp, "ESSID:\"");
			if (pSSIDStart != NULL) {
				//sscanf(pSSIDStart + strlen("ESSID:\""), "%s", s_aps[recordIdx].ssid);
				//s_aps[recordIdx].s_len = strlen((const char *)s_aps[recordIdx].ssid);
				char delim = '\"';
				char *p = (char *)pSSIDStart, *sp = NULL, *ep = NULL;
				size_t i = 0;
				for (; *p; p++) {
					if (!sp && *p == delim)
						sp = p, sp++;
					else if (!ep && *p == delim)
						ep = p;
					if (sp && ep) {
						char substr[ep - sp + 1];
						for (i = 0, p = sp; p < ep; p++)
							substr[i++] = *p;
						substr[ep - sp] = 0;
						//sprintf(s_aps[recordIdx].ssid, "%s", substr);
						sprintf((char  *)s_aps[recordIdx].ssid, "%s", (char *)substr);
						s_aps[recordIdx].s_len = strlen((const char *)s_aps[recordIdx].ssid);
						sp = ep = NULL;
					}
				}
				//PR_INFO("Augentix Debug : scanned SSID:str Length =  %s %d <========= \n ",
				//s_aps[recordIdx].ssid,s_aps[recordIdx].s_len);
				if (s_aps[recordIdx].s_len != 0) {
					s_aps[recordIdx].ssid[s_aps[recordIdx].s_len - 1] = 0;
					s_aps[recordIdx].s_len--;
				}
				goto ReadNext;
			}
		}

	ReadNext:
		memset(tmp, 0, sizeof(tmp));
	}

	pclose(pp);
	*num = recordIdx + 1;

	PR_NOTICE("WIFI Scan AP Begin\n");
	int index = 0;
	for (index = 0; (UINT_T)index < *num; index++) {
		PR_INFO("index:%d bssid:%02X-%02X-%02X-%02X-%02X-%02X RSSI:%d SSID:%s\n", index, s_aps[index].bssid[0],
		       s_aps[index].bssid[1], s_aps[index].bssid[2], s_aps[index].bssid[3], s_aps[index].bssid[4],
		       s_aps[index].bssid[5], s_aps[index].rssi, s_aps[index].ssid);
	}
	PR_INFO("WIFI Scan AP End\n");

	return OPRT_OK;
}

OPERATE_RET hwl_wf_assign_ap_scan(IN CONST CHAR_T *ssid, OUT AP_IF_S **ap)
{
	if (NULL == ssid || NULL == ap) {
		return OPRT_INVALID_PARM;
	}

	/*
   scan all ap and search
    */
	AP_IF_S *pTotalAp = NULL;
	UINT_T tatalNum = 0;
	int index = 0;
	hwl_wf_all_ap_scan(&pTotalAp, &tatalNum);

	*ap = NULL;

	for (index = 0; (UINT_T)index < tatalNum; index++) {
		if (memcmp(pTotalAp[index].ssid, ssid, pTotalAp[index].s_len) == 0) {
			*ap = pTotalAp + index;
			break;
		}
	}

	return OPRT_OK;
}

OPERATE_RET hwl_wf_release_ap(IN AP_IF_S *ap __attribute__((unused)))
{ //Static variables, no need to free
	return OPRT_OK;
}

static int s_curr_channel = 1;
OPERATE_RET hwl_wf_set_cur_channel(IN CONST BYTE_T chan)
{
	char tmpCmd[100] = { 0 };
	snprintf(tmpCmd, 100, "iwconfig %s channel %d", WLAN_DEV, chan);
	exec_cmd(tmpCmd);
	s_curr_channel = chan;

	PR_INFO("WIFI Set Channel:%d \n", chan);

	return OPRT_OK;
}

/*
wlp3s0    13 channels in total; available frequencies :
          Channel 01 : 2.412 GHz
          Channel 13 : 2.472 GHz
          Current Frequency:2.452 GHz (Channel 9)
*/
OPERATE_RET hwl_wf_get_cur_channel(OUT BYTE_T *chan)
{
	if (NULL == chan) {
		return OPRT_INVALID_PARM;
	}

	FILE *pp = popen("/usr/bin/iwlist " WLAN_DEV " channel", "r");

	if (pp == NULL) {
		return OPRT_COM_ERROR;
	}

	char tmp[128] = { 0 };
	memset(tmp, 0, sizeof(tmp));
	while (fgets(tmp, sizeof(tmp), pp) != NULL) {
		char *pIPStart = strstr(tmp, " (Channel ");
		if (pIPStart != NULL) {
			break;
		}
	}

	/* find the channel	*/
	char *pCHANNELStart = strstr(tmp, " (Channel ");
	if (pCHANNELStart != NULL) {
		int x = 0;
		sscanf(pCHANNELStart + strlen(" (Channel "), "%d", &x);
		*chan = x;
	} else {
		*chan = s_curr_channel;
	}

	pclose(pp);

	PR_INFO("WIFI Get Curr Channel:%d \n", *chan);

	return OPRT_OK;
}

#pragma pack(1)
/*
http://www.radiotap.org/
*/
typedef struct {
	/**
     * @it_version: radiotap version, always 0
     */
	BYTE_T it_version;

	/**
     * @it_pad: padding (or alignment)
     */
	BYTE_T it_pad;

	/**
     * @it_len: overall radiotap header length
     */
	USHORT_T it_len;

	/**
     * @it_present: (first) present word
     */
	UINT_T it_present;
} ieee80211_radiotap_header;
#pragma pack()

static volatile SNIFFER_CALLBACK s_pSnifferCall = NULL;
static volatile BOOL_T s_enable_sniffer = FALSE;

char tmpStr[128];
STATIC CHAR_T *__tuya_linux_get_snap_qrcode1(VOID)
{
	//Developers need to parse QR code information from cameras
	//a typical string is {"s":"ssidxxxx","p":"password","t":"token frome tuya cloud"}
	//PR_INFO("--------------->::%s:%d %s \n",__FILE__,__LINE__,__func__);
	MPI_VIDEO_FRAME_INFO_S info;
	UINT32 error;
	int n;
	int dev_idx = 0;
	int chn_idx = 0;
	int win_idx = 0;
	MPI_WIN idx = MPI_VIDEO_WIN(dev_idx, chn_idx, win_idx);
	PR_INFO("Taking snapshot of video window (d:c:w=%d:%d:%d)\n", idx.dev, idx.chn, idx.win);
	MPI_SYS_init();
	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = MPI_VIDEO_CHN(dev_idx, chn_idx);
	error = MPI_DEV_queryChnState(chn, &chn_stat);
	if (error != MPI_SUCCESS) {
		PR_ERR("Query channel state on channel %d on device %d failed\n", idx.chn, idx.dev);
		goto err;
	}

	error = MPI_DEV_getWinFrame(idx, &info, -1);
	if (error != MPI_SUCCESS) {
		PR_ERR("Failed to take snapshot.\n");
		goto err;
	}

#ifdef ZBAR_SUPPORT
	QR_DEBUG("image info: w:%d, h:%d, size:%d\n", info.width, info.height, info.size);
	n = qrcode_parse_from_buffer(info.uaddr, info.width, info.height, tmpStr);
	if (n <= 0) {
		goto release_memory;
	}
#endif

	MPI_DEV_releaseWinFrame(idx, &info);
	MPI_SYS_exit();
	return tmpStr;

release_memory:
	MPI_DEV_releaseWinFrame(idx, &info);
err:
	MPI_SYS_exit();
	return NULL;
}

int s_enable_qrcode1 = 1;
void *thread_qrcode1(void *data)
{
	AGTX_UNUSED(data);
	PR_NOTICE("Augentix: Qrcode Thread start\n");
	while (s_enable_qrcode1) {
		usleep(100 * 1000);
		char *pStr = __tuya_linux_get_snap_qrcode1();
		if (pStr) {
			PR_DEBUG("get string from qrcode %s\n", pStr);
			OPERATE_RET ret = tuya_ipc_direct_connect(pStr, TUYA_IPC_DIRECT_CONNECT_QRCODE);
			if (ret == OPRT_OK) {
				PR_DEBUG("register to tuya cloud via qrcode success\n");
				break;
			}
		}
	}

	PR_NOTICE("Qrcode Proc Finish\n");
	//disable Sniffer mode if qr paring is success
	s_enable_sniffer = FALSE;
	sleep(5);
	return (void *)0;
}

static void *func_Sniffer(void *data)
{
	AGTX_UNUSED(data);
	PR_NOTICE("Sniffer Thread Create\n");

	int sock = socket(PF_PACKET, SOCK_RAW, htons(0x03)); //ETH_P_ALL
	if (sock < 0) {
		PR_ERR("Sniffer Socket Alloc Fails %d \n", sock);
		return (void *)0;
	}

	{ /* Force binding to wlan0, can be considered to remove */
		struct ifreq ifr;
		memset(&ifr, 0x00, sizeof(ifr));
		strncpy(ifr.ifr_name, WLAN_DEV, sizeof(ifr.ifr_name) - 1);
		setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
	}

#define MAX_REV_BUFFER 512
	BYTE_T rev_buffer[MAX_REV_BUFFER];

	int skipLen = 26; /* Radiotap default length is 26 */

	while ((s_pSnifferCall != NULL) && (TRUE == s_enable_sniffer)) {
		int rev_num = recvfrom(sock, rev_buffer, MAX_REV_BUFFER, 0, NULL, NULL);
		ieee80211_radiotap_header *pHeader = (ieee80211_radiotap_header *)rev_buffer;
		skipLen = pHeader->it_len;

		if (skipLen >= MAX_REV_BUFFER) { /* Package is lost directly when its length greater than maximum*/
			continue;
		}

		if (0) {
			PR_INFO("skipLen:%d ", skipLen);
			int index = 0;
			for (index = 0; index < 180; index++) {
				PR_INFO("%02X-", rev_buffer[index]);
			}
			PR_INFO("\n");
		}
		if (rev_num > skipLen) {
			s_pSnifferCall(rev_buffer + skipLen, rev_num - skipLen);
		}
	}

	s_pSnifferCall = NULL;

	close(sock);

	PR_NOTICE("Sniffer Proc Finish\n");
	s_enable_qrcode1 = 0; //disable QR pairing thread if Sniff mode pass
	return (void *)0;
}

static pthread_t sniffer_thId; // ID of capture thread
static pthread_t qrc_thId; // ID of capture thread

//Prevent duplication calling
OPERATE_RET hwl_wf_sniffer_set(IN CONST BOOL_T en, IN CONST SNIFFER_CALLBACK cb)
{
	char *name = "sniffer"; // max. length is 16

	if (en == s_enable_sniffer) {
		PR_WARN("Already in status %d\n", en);
		return OPRT_OK;
	}
	s_enable_sniffer = en;
	if (en == TRUE) {
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_START_WIFI_CFG);

		/*Augentix: wifi interface to work in Monitor mode*/
		if (AGTX_TUYA_PAIR_SNIFF_MODE == 1) {
			PR_DEBUG("Enable Sniffer\n");
			hwl_wf_wk_mode_set(WWM_SNIFFER);
		} else {
			hwl_wf_wk_mode_set(WWM_STATION);
		}

		s_pSnifferCall = cb;

		/*Augentix Enable both Wifi Monitor mode and QR mode threads if we wish to
         * work the device in both modes. */
		if (AGTX_TUYA_PAIR_SNIFF_MODE == 1) {
			pthread_create(&sniffer_thId, NULL, func_Sniffer, name);
		}

		if (AGTX_TUYA_PAIR_QR_MODE == 1) {
			PR_DEBUG("Enable Qrcode \n");
			char *mpi_stream_argv[] = { "mpi_stream", "-d", "/system/mpp/case_config/case_config_FEN105_QR",
				                    NULL };
			forkIndependentProc("/system/bin/mpi_stream", mpi_stream_argv);
			sleep(5);
			name = "qrcode";
			pthread_create(&qrc_thId, NULL, thread_qrcode1, name);
			s_enable_qrcode1 = 1;
		} else {
			usleep(100 * 1000); //required for iwlist scan to fetch ap list
		}
	} else {
		if (AGTX_TUYA_PAIR_SNIFF_MODE == 1) {
			pthread_join(sniffer_thId, NULL);
			PR_DEBUG("===> Disable Sniffer <===\n");
		}
		if (AGTX_TUYA_PAIR_QR_MODE == 1) {
			pthread_join(qrc_thId, NULL);
			PR_DEBUG("Disable Qrcode\n");
		}

		hwl_wf_wk_mode_set(WWM_STATION);

		system("killall -2 mpi_stream");
		sleep(5);

		s_enable_qrcode1 = 0;

		sniffer_set_done = TRUE;
	}

	return OPRT_OK;
}

void reboot_func(void) {
    unsigned int reboot_timer = 0;
    unsigned int eventList_ctr = 0;
    unsigned int tmpCntr;
    unsigned int tmpval;
    eventDetectCntr = 1; // Start the eventDetectcntr
    tmpCntr = 1;
    do {
        printf("reboot_func :=====> chk time:events= %u:%u \n",eventList_ctr,tmpCntr);
        tmpval = eventDetectCntr;
        //check eventlist
        if (tmpCntr == tmpval) {            // no event in last 1 sec
            eventList_ctr += 1;             // increment by 1 sec pooling freq time
            if( eventList_ctr >= 30) {      // no event for past 30 sec or greater  =  reboot
                sync();
                reboot(RB_AUTOBOOT);
            }
        } else {
            eventList_ctr = 0; //reset eventList_ctr
        }
        tmpCntr = tmpval;
        usleep(1000 * 1000); //poll every 1 sec
        reboot_timer =  reboot_timer + 1;

    } while (reboot_timer < 300);

    sync();
    reboot(RB_AUTOBOOT);

}

static OPERATE_RET hwl_get_local_ip_info(char *interface, OUT NW_IP_S *ip)
{
	//char tmp[256];
	int fd;
	struct ifreq ifr;

	{ /*finding IP*/
		fd = socket(AF_INET, SOCK_DGRAM, 0);

		ifr.ifr_addr.sa_family = AF_INET;
		strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
		if (ioctl(fd, SIOCGIFADDR, &ifr)) {
			PR_ERR("%s: Get SIOCGIFADDR fail\n", interface);
			struct if_nameindex *if_nidxs, *intf;
			if_nidxs = if_nameindex();
			unsigned int intf_no = 0;
			if (if_nidxs != NULL) {
				for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
					if (strstr(intf->if_name, WLAN_DEV) != NULL) {
						intf_no += 1;
					}
				}
				if_freenameindex(if_nidxs);
			}
			if (intf_no == 0) {
				PR_ERR("read fail: NO Wifi interface\n");
				//start thread if  not started
				do { //cleanup and reboot
					if (reboot_func_thread == 0) {
						pthread_create(&noWlan0_inf_th, NULL, (void *)reboot_func, NULL);
						reboot_func_thread = 1;
						pthread_detach(noWlan0_inf_th);
					}
					usleep(1000 * 1000);
				} while (1);
			}
		}
		//else { PR_INFO("got %s ipaddress = %s",interface, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));}
		strcpy(ip->ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	}

    { /* finding gw  */
        if (ioctl(fd, SIOCGIFBRDADDR, &ifr)) {
            PR_ERR("%s: Get SIOCGIFBRDADDR fail\n", interface);
        }
        //else { PR_INFO("got %s broadcast = %s",interface, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));}
        strcpy(ip->gw, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr));

    }
    {
        if (ioctl(fd, SIOCGIFNETMASK, &ifr)) {
            PR_ERR("%s: Get SIOCGIFNETMASK fail\n", interface);
        }
        //else { PR_INFO("got %s netmask  = %s",interface, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));}
        strcpy(ip->mask, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr));
    }

    close(fd);

	return OPRT_OK;
}

/*
wlan0	  Link encap:Ethernet  HWaddr 08:57:00:88:5c:16
          inet addr:192.168.22.3  Bcast:192.168.23.255	Mask:255.255.252.0
*/
OPERATE_RET hwl_wf_get_ip(IN CONST WF_IF_E wf, OUT NW_IP_S *ip)
{
	if (NULL == ip) {
		return OPRT_INVALID_PARM;
	}

	if (wf == WF_AP) { /* Simple Processing in AP Mode */
		memcpy(ip->ip, "192.168.0.1", strlen("192.168.0.1"));
		memcpy(ip->gw, "192.168.0.1", strlen("192.168.0.1"));
		memcpy(ip->mask, "255.255.255.0", strlen("255.255.255.0"));
	}

	if (wf == WF_STATION) {
		//get the ip of ethernet
		//hwl_get_local_ip_info(NET_DEV, ip);

		NW_IP_S tmp;
		memset(&tmp, 0, sizeof(NW_IP_S));
		//get the ip of wifi
		hwl_get_local_ip_info(WLAN_DEV, &tmp);
		if (strlen(tmp.ip)) {
			//replace ip
			memcpy(ip, &tmp, sizeof(NW_IP_S));
		}
	}

	//PR_INFO("WIFI[%d] Get IP:%s\n", wf, ip->ip);
	return OPRT_OK;
}

/*
wlp3s0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.3.14  netmask 255.255.255.0  broadcast 192.168.3.255
        ether a4:02:b9:b1:99:50  txqueuelen 1000  (Ethernet)
*/
OPERATE_RET hwl_wf_get_mac(IN CONST WF_IF_E wf __attribute__((unused)), INOUT NW_MAC_S *mac)
{
	int fd;
	//char *tmpmac;
	unsigned char *tmpmac;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, WLAN_DEV, IFNAMSIZ - 1);
	if (ioctl(fd, SIOCGIFHWADDR, &ifr)) {
		PR_ERR("Get SIOCGIFHWADDR fail\n");
		close(fd);
		return OPRT_INVALID_PARM;
	}
	close(fd);
	tmpmac = (unsigned char *)ifr.ifr_addr.sa_data;
	memcpy(mac, tmpmac, 6);
	sprintf(WLAN_DEV_MAC, "%.2X%.2X%.2X", tmpmac[3], tmpmac[4], tmpmac[5]);
	//sprintf(WLAN_DEV_MAC,"%.2X%.2X%.2X%.2X%.2X%.2X",tmpmac[0],tmpmac[1],tmpmac[2],tmpmac[3],tmpmac[4],tmpmac[5]);

#if 0 //diasable Tuya mac read function
	if (NULL == mac) {
		return OPRT_INVALID_PARM;
	}

	FILE *pp = popen("/sbin/ifconfig " WLAN_DEV, "r");
	if (pp == NULL) {
		return OPRT_COM_ERROR;
	}

	char tmp[256];
	memset(tmp, 0, sizeof(tmp));
	while (fgets(tmp, sizeof(tmp), pp) != NULL) {
		char *pMACStart = strstr(tmp, "ether ");
		if (pMACStart != NULL) {
			int x1, x2, x3, x4, x5, x6;
			sscanf(pMACStart + strlen("ether "), "%x:%x:%x:%x:%x:%x", &x1, &x2, &x3, &x4, &x5, &x6);
			mac->mac[0] = x1 & 0xFF;
			mac->mac[1] = x2 & 0xFF;
			mac->mac[2] = x3 & 0xFF;
			mac->mac[3] = x4 & 0xFF;
			mac->mac[4] = x5 & 0xFF;
			mac->mac[5] = x6 & 0xFF;

			break;
		}
	}
	pclose(pp);

	PR_INFO("WIFI Get MAC %02X-%02X-%02X-%02X-%02X-%02X\n", mac->mac[0], mac->mac[1], mac->mac[2], mac->mac[3],
	       mac->mac[4], mac->mac[5]);
#endif
	return OPRT_OK;
}

OPERATE_RET hwl_wf_set_mac(IN CONST WF_IF_E wf __attribute__((unused)), IN CONST NW_MAC_S *mac)
{
	if (NULL == mac) {
		return OPRT_INVALID_PARM;
	}
	PR_DEBUG("WIFI Set MAC\n");

	return OPRT_OK;
}

OPERATE_RET hwl_wf_wk_mode_set(IN CONST WF_WK_MD_E mode)
{
/*Augentix: Dont let tuya set the network modes */
/*TODO: Enable this features once driver is working in monitor and Master modes */
#if 1
	char tmpCmd[100] = { 0 };

	snprintf(tmpCmd, 100, "ifconfig %s up", WLAN_DEV);
	exec_cmd(tmpCmd);

	switch (mode) {
	case WWM_LOWPOWER: {
		//Linux system does not care about low power
		break;
	}
	case WWM_SNIFFER: {
		snprintf(tmpCmd, 100, "iwconfig %s mode Monitor", WLAN_DEV);
		exec_cmd(tmpCmd);
		break;
	}
	case WWM_STATION: {
		//snprintf(tmpCmd, 100, "iwconfig %s mode Managed", WLAN_DEV);
		//exec_cmd(tmpCmd);
		snprintf(tmpCmd, 100, "killall hostapd dhcpd"); //forcekill
		exec_cmd(tmpCmd);
		break;
	}
	case WWM_SOFTAP: {
		//snprintf(tmpCmd, 100, "iwconfig %s mode Master", WLAN_DEV);
		//exec_cmd(tmpCmd);
		//snprintf(tmpCmd, 100, "killall -9 hostapd"); //forcekill
		//exec_cmd(tmpCmd);

		break;
	}
	case WWM_STATIONAP: {
		break;
	}
	default: {
		break;
	}
	}
	PR_DEBUG("WIFI Set Mode %d\n", mode);
#endif
	return OPRT_OK;
}


static inline int iw_get_ext_modes( int fd, /* Socket to the kernel */
            const char *     ifname,     /* Device name */
            int          request,    /* WE ID */
            struct iwreq *   pwrq)       /* Fixed part of the request */
{
    /* Set device name */
    strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
    /* Do the request */
    return(ioctl(fd, request, pwrq));
}
static void iw_get_basic_config_mode(int fd, const char *    ifname,
                wireless_config *   info) {
    struct iwreq      wrq;
    memset((char *) info, 0, sizeof(struct wireless_config));
    if(iw_get_ext_modes(fd, ifname, SIOCGIWMODE, &wrq) >= 0) {
        info->has_mode = 1;
        if(wrq.u.mode < IW_NUM_OPER_MODE) {
            info->mode = wrq.u.mode;
            WIFI_OPERATIONAL_MODE = wrq.u.mode;
        } else {
            info->mode = IW_NUM_OPER_MODE;
            WIFI_OPERATIONAL_MODE = IW_NUM_OPER_MODE;
        }
    }
}
static void getinfo(int fd, char *name, struct wireless_info *info) {
    memset((char *) info, 0, sizeof(struct wireless_info));
    iw_get_basic_config_mode(fd, name, &(info->b));
}

/*Augentix Get Wlan Operational mode using ioctl*/
static unsigned int getWlanOperationalMode() {
    int fd;
    struct wireless_info info;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    getinfo(fd, "wlan0", &info);
    close(fd);
    //if ( WIFI_OPERATIONAL_MODE != IW_NUM_OPER_MODE)
    //    PR_INFO("Wifi Mode = %d\n",WIFI_OPERATIONAL_MODE);
    //else
    //    PR_INFO("failed to get Wifi Mode  = %d\n",WIFI_OPERATIONAL_MODE);
    return WIFI_OPERATIONAL_MODE;
}

/*
wlp3s0    IEEE 802.11  ESSID:"H"
          Mode:Managed  Frequency:2.452 GHz  Access Point: 58:7F:66:04:73:A8
          Bit Rate=130 Mb/s   Tx-Power=22 dBm
*/
OPERATE_RET hwl_wf_wk_mode_get(OUT WF_WK_MD_E *mode)
{
	//Augentix  below 2 lines == we work only in station mode
	//*mode = WWM_STATION;
	//return OPRT_OK;
	//
	if (NULL == mode) {
		return OPRT_INVALID_PARM;
	}
#if 0 // replace iwconfig with relavent ioctl cmd
	//FILE *pp = popen("iwconfig "WLAN_DEV, "r"); //getram
	FILE *pp = popen("/usr/bin/iwconfig wlan0 ", "r");
	if (pp == NULL) {
		PR_INFO("%s %d(%m)\n", WLAN_DEV, errno);
		PR_INFO("WIFI Get Mode Fail. Force Set Station \n");
		*mode = WWM_STATION;
		return OPRT_OK;
	}

	char scan_mode[10] = { 0 };
	char tmp[256] = { 0 };
	while (fgets(tmp, sizeof(tmp), pp) != NULL) {
		char *pModeStart = strstr(tmp, "Mode:");
		if (pModeStart != NULL) {
			//int x1, x2, x3, x4, x5, x6;
			sscanf(pModeStart + strlen("Mode:"), "%s ", scan_mode);
			break;
		}
	}
	pclose(pp);
#endif
    //*mode = getWlanOperationalMode();
    switch (getWlanOperationalMode()) {
        case WWM_STATION:
            *mode = WWM_STATION;
	        //PR_INFO("DBG:::::    STATION MODE  \n");
            break;
        case WWM_SNIFFER:
            *mode = WWM_SNIFFER;
	        PR_DEBUG("DBG:::::    MONITOR MODE  \n");
            break;
        case WWM_SOFTAP:
            *mode = WWM_SOFTAP;
	        PR_DEBUG("DBG:::::    SOFTAP  \n");
            break;
        case WWM_LOWPOWER:
        case WWM_STATIONAP:
        default:
            *mode = WWM_STATION; //default to station mode
            break;
    }
#if 0
	if (strncasecmp(scan_mode, "Managed", strlen("Managed")) == 0) {
		*mode = WWM_STATION;
	}

	if (strncasecmp(scan_mode, "Master", strlen("Master")) == 0) {
		PR_INFO("DBG:::::    SOFTAP  \n");
		*mode = WWM_SOFTAP;
	}

	if (strncasecmp(scan_mode, "Monitor", strlen("Monitor")) == 0) {
		*mode = WWM_SNIFFER;
	}
	PR_INFO("WIFI Get Mode [%s] %d\n", scan_mode, *mode);
#endif
	return OPRT_OK;
}

/***********************************************************
*  Function: wf_station_disconnect
*  Input: none
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET hwl_wf_station_disconnect(VOID)
{
	PR_DEBUG("STA Disconn AP\n");
	//Reserved

	return OPRT_OK;
}

/*
wlp3s0    IEEE 802.11  ESSID:"Augentix Guest "
          Link Quality=70/70  Signal level=-6 dBm
*/
OPERATE_RET hwl_wf_station_get_conn_ap_rssi(OUT SCHAR_T *rssi)
{
	char tmp[128];
	FILE *fp;

	if (rssi == NULL) {
		return OPRT_INVALID_PARM;
	}
	*rssi = 0;

	fp = popen(IWCONFIG_CMD " " WLAN_DEV, "r");
	if (fp == NULL) {
		g_weak_wifi = 1;
		return OPRT_COM_ERROR;
	}

	while (fgets(tmp, sizeof(tmp), fp) != NULL) {
		char *p = strstr(tmp, "Quality=");
		if (p != NULL) {
			int x = 0;
			int y = 0;
			sscanf(p + strlen("Quality="), "%d/%d", &x, &y);
			*rssi = x * 100 / (y + 1);
			break;
		}
	}

	pclose(fp);

	if (*rssi < WEAK_WIFI_ASSERT_TH) {
		g_weak_wifi = 1;
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_WEAK_WIFI_TRUE);
	} else if (*rssi > WEAK_WIFI_DEASSERT_TH) {
		g_weak_wifi = 0;
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_WEAK_WIFI_FALSE);
	} else {
		/* keep current value */
	}

	return OPRT_OK;
}

//note High frequency interface
OPERATE_RET hwl_wf_station_stat_get(OUT WF_STATION_STAT_E *stat)
{
	if (NULL == stat) {
		return OPRT_INVALID_PARM;
	}
	if (wfGotIP == 0) {
		*stat = WSS_CONNECTING;
	} else {
		*stat = WSS_GOT_IP; //Be sure to return in real time
	} //Reserved
	return OPRT_OK;
}

OPERATE_RET hwl_wf_ap_start(IN CONST WF_AP_CFG_IF_S *cfg)
{
	if (NULL == cfg) {
		return OPRT_INVALID_PARM;
	}

	IPC_APP_Notify_LED_Sound_Status_CB(IPC_START_WIFI_CFG);

	PR_INFO("Start AP SSID:%s \n", cfg->ssid);
	//Copy the hostapd conf to tmp
	//system("cp -f /system/script/rtl_hostapd_2G.conf /tmp/augentix_ap.conf");
	system("cp -f /usrdata/augentix_ap.conf /tmp/augentix_ap.conf");
	//Assign the SSID and passwd (Tuya SDK to hostapd conf)
	char tmpSSID[64];
	//sprintf(tmpSSID, "echo ssid=%s >> /tmp/augentix_ap.conf", cfg->ssid);
	sprintf(tmpSSID, "echo ssid=%s%s >> /tmp/augentix_ap.conf", CUSTOMER_SSID_PREFIX, WLAN_DEV_MAC);
	PR_INFO("\nDBG HotSpot AP Essid::: %s\n", tmpSSID);
	system(tmpSSID);
	//sprintf(tmpPASSWD,"echo wpa_passphrase=%s >> /tmp/augentix_ap.conf",cfg->passwd);
	//PR_INFO("\nDBG HotSpot AP Essid::: %s\n",tmpPASSWD);
	//system(tmpPASSWD);

	//Reserved
	PR_INFO("----------0-0-Start SoftAP Init-0-0--------------\n");
	system("touch /var/lib/dhcp/dhcpd.leases");
	system("/usr/sbin/dhcpd -cf /etc/dhcp/dhcpd.conf wlan0 &");
	system("/system/script/ap_on.sh");
	PR_INFO("-----------------0-0-SoftAP Start Done-0-0------------------\n");
	sleep(1);
	return OPRT_OK;
}

OPERATE_RET hwl_wf_ap_stop(VOID)
{
	PR_DEBUG("Stop AP \n");
	//Reserved
	return OPRT_OK;
}

OPERATE_RET hwl_wf_set_country_code(IN CONST CHAR_T *p_country_code)
{
	PR_DEBUG("Set Country Code:%s \n", p_country_code);

	return OPRT_OK;
}
