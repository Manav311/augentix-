#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "inc/fcgi.h"
#include "inc/utils.h"
#include <fcgi_stdio.h>
#include <json.h>

#define BUF_STR_SIZE 2048

int main(void)
{
	int count = 0, post_len = 0;
	char *cgi_name;
	char MachineMode[20];
	char buf[BUF_STR_SIZE];
	char *str_post_len = NULL;

	// Get machine mode
	FILE *fp_Mode;
	fp_Mode = fopen(Machine_Mode, "r");
	if (fp_Mode) {
		fgets(MachineMode, 20, fp_Mode);
	}
	sscanf(MachineMode, "%s", MachineMode);

	while (FCGI_Accept() >= 0) {
		cgi_name = getenv("SCRIPT_NAME");
		str_post_len = getenv("CONTENT_LENGTH");
		if ((str_post_len == NULL) || (sscanf(str_post_len, "%d", &post_len) != 1)) {
			post_len = 0;
		}

		if (strncmp(cgi_name, "/getTime.cgi", 20) == 0) {
			fcgiGetTime();
		} else if (strcmp(cgi_name, "/getSntpConf.cgi") == 0) {
			fcgiGetSntpConf();
		} else if (strcmp(cgi_name, "/getEnabledConf.cgi") == 0) {
			fcgiGetEnabledConf();
		} else if (strcmp(cgi_name, "/getDSTConf.cgi") == 0) {
			fcgiGetDSTConf();
		} else if (strcmp(cgi_name, "/getTZ.cgi") == 0) {
			fcgiGetTZ();
		} else if (strcmp(cgi_name, "/getTimeSwitch.cgi") == 0) {
			fcgiGetTimeSwitch();
		} else if (strcmp(cgi_name, "/exportSetting.cgi") == 0) {
			fcgiExportSetting();
		} else if (strcmp(cgi_name, "/firmwareUpload.cgi") == 0) {
			fcgiFirmwareUpload();
		} else if (strcmp(cgi_name, "/getHostname.cgi") == 0) {
			fcgiGetHostname();
		} else if (strcmp(cgi_name, "/setToDefault.cgi") == 0) {
			fcgiSetToDefault();
		} else if (strcmp(cgi_name, "/reboot.cgi") == 0) {
			fcgiReboot();
		} else if (strcmp(cgi_name, "/stopStream.cgi") == 0) {
			fcgiStopStream(MachineMode);
		} else if (strcmp(cgi_name, "/SysupdOS.cgi") == 0) {
			fcgiSysupdOS();
		} else if (strcmp(cgi_name, "/switch2SysupdOS.cgi") == 0) {
			fcgiSwitch2SysupdOS();
		} else if (strcmp(cgi_name, "/changePass.cgi") == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiChangePass(buf);
		} else if (strcmp(cgi_name, "/assignIP.cgi") == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiAssignIP(MachineMode, buf);
		} else if (strcmp(cgi_name, "/netInfo.cgi") == 0) {
			fcgiNetInfo();
		} else if (strcmp(cgi_name, "/upTime.cgi") == 0) {
			fcgiUpTime();
		} else if (strcmp(cgi_name, "/getMAC.cgi") == 0) {
			fcgiGetMAC();
		} else if (strcmp(cgi_name, "/getFirmwareVersion.cgi") == 0) {
			fcgiGetFirmwareVersion();
		} else if (strncmp(cgi_name, "/getIPAddress.cgi", 20) == 0) {
			fcgiGetIPAddress();
		} else if (strncmp(cgi_name, "/getNetmask.cgi", 20) == 0) {
			fcgiGetNetmask();
		} else if (strncmp(cgi_name, "/getGateway.cgi", 20) == 0) {
			fcgiGetGateway();
		} else if (strncmp(cgi_name, "/getDNS.cgi", 20) == 0) {
			fcgiGetDNS();
		} else if (strncmp(cgi_name, "/setIP.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetIP(buf);
		} else if (strncmp(cgi_name, "/setMask.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetMask(buf);
		} else if (strncmp(cgi_name, "/setGateway.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetGateway(buf);
		} else if (strncmp(cgi_name, "/setDNS.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetDNS(buf);
		} else if (strncmp(cgi_name, "/setTime.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetTime(MachineMode, buf);
		} else if (strncmp(cgi_name, "/setSntpConf.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetSntpConf(MachineMode, buf);
		} else if (strncmp(cgi_name, "/DSTSet.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiDSTSet(buf);
		} else if (strncmp(cgi_name, "/TZSet.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiTZSet(buf);
		} else if (strncmp(cgi_name, "/EnabledSet.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiEnabledSet(buf);
		} else if (strncmp(cgi_name, "/TimeSwitchSet.cgi", 20) == 0 && post_len > 0 &&
		           post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiTimeSwitchSet(MachineMode, buf);
		} else if (strncmp(cgi_name, "/uploadCert.cgi", 20) == 0) {
			fcgiGetUploadFile("/etc/nginx/ssl/cert.pem.upload");
		} else if (strncmp(cgi_name, "/uploadKey.cgi", 20) == 0) {
			fcgiGetUploadFile("/etc/nginx/ssl/key.pem.upload");
		} else if (strncmp(cgi_name, "/updateCA.cgi", 20) == 0) {
			fcgiUpdateCA();
		} else if (strncmp(cgi_name, "/resetCA.cgi", 20) == 0) {
			fcgiResetCA();
		} else if (strncmp(cgi_name, "/getFaceModelList.cgi", 20) == 0) {
			fcgiGetFaceModelList();
		} else if (strncmp(cgi_name, "/uploadFacePhoto.cgi", 20) == 0) {
			char dir[256] = "/usrdata/eaif/facereco/faces/";
			decode(getenv("QUERY_STRING"), buf);
			strcat(dir, buf);
			strrpc(dir, "%20", " ");
			fcgiGetUploadFile(dir);
		} else if (strncmp(cgi_name, "/removeFacePhoto.cgi", 20) == 0) {
			char dir[256] = "/usrdata/eaif/facereco/faces/";
			decode(getenv("QUERY_STRING"), buf);
			strcat(dir, buf);
			strrpc(dir, "%20", " ");
			fcgiRemoveFile(dir);
		} else if (strncmp(cgi_name, "/validateFaceModel.cgi", 20) == 0) {
			decode(getenv("QUERY_STRING"), buf);
			strrpc(buf, "%20", " ");
			fcgiValidateFaceModel(buf);
		} else if (strncmp(cgi_name, "/registerFaceModel.cgi", 20) == 0) {
			decode(getenv("QUERY_STRING"), buf);
			strrpc(buf, "%20", " ");
			fcgiRegisterFaceModel(buf);
		} else if (strncmp(cgi_name, "/unregisterFaceModel.cgi", 20) == 0) {
			decode(getenv("QUERY_STRING"), buf);
			strrpc(buf, "%20", " ");
			fcgiUnregisterFaceModel(buf);
		} else if (strncmp(cgi_name, "/setWifi.cgi", 20) == 0 && post_len > 0 && post_len < BUF_STR_SIZE) {
			readPostData(post_len, buf);
			fcgiSetWifi(buf);
		} else if (strncmp(cgi_name, "/disconnectWifi.cgi", 20) == 0) {
			fcgiDisconnectWifi();
		} else if (strncmp(cgi_name, "/getSSID.cgi", 20) == 0) {
			fcgiGetSSID();
		} else if (strncmp(cgi_name, "/getWPASSID.cgi", 20) == 0) {
			fcgiGetWPASSID();
		} else if (strncmp(cgi_name, "/getWlanInfo.cgi", 20) == 0) {
			fcgiGetWlanInfo();
		} else if (strncmp(cgi_name, "/packNginxLog.cgi", 20) == 0) {
			fcgiPackNginxLog();
		} else {
			printf("Content-type: text/html\r\n\r\n");
			printf("<title>FastCGI Hello!</title>"
			       "<h1>FastCGI Hello!</h1>"
			       "<div>Request number %d running on host : %s </div>\n"
			       "<div>QUERY_STRING : %s\n</div>"
			       "<div>REMOTE_ADDR : %s\n</div>"
			       "<div>REMOTE_PORT : %s\n</div>"
			       "<div>REQUEST_METHOD : %s\n</div>"
			       "<div>CONTENT_TYPE : %s\n</div>"
			       "<div>CONTENT_LENGTH : %s\n</div>"
			       "<div>SERVER_PROTOCOL : %s\n</div>"
			       "<div>REQUEST_URI : %s\n</div>"
			       "<div>SERVER_SOFTWARE : %s\n</div>"
			       "<div>SCRIPT_NAME : %s\n</div>",
			       ++count, getenv("SERVER_NAME"), getenv("QUERY_STRING"), getenv("REMOTE_ADDR"),
			       getenv("REMOTE_PORT"), getenv("REQUEST_METHOD"), getenv("CONTENT_TYPE"),
			       getenv("CONTENT_LENGTH"), getenv("REQUEST_URI"), getenv("SERVER_PROTOCOL"),
			       getenv("SERVER_SOFTWARE"), cgi_name);
			gets(buf);
			printf("Content:%s", buf);
		}
	}

	return 0;
}
