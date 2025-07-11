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
#include <dirent.h>

void fcgiGetUploadFile(char *name)
{
	FILE *fp;
	char key[100];
	char buf[2048];
	int size, idx, keylen, check_len, match_flag;

	if ((fp = fopen(name, "wb")) != NULL) {
		key[0] = '\r';
		key[1] = '\n';
		match_flag = 0;
		fgets(&key[2], 100, stdin);
		// fprintf(fp,"key:%s\n",key);
		keylen = strlen(key) - 4;
		key[keylen] = 0;

		do {
			fgets(buf, 2048, stdin);
			size = strlen(buf);
			// fprintf(fp,"pass:%s size:%d",buf,size);
		} while (size > 2); // bypass all the header

		size = fread(buf, 1, keylen - 1, stdin);

		if (size < keylen - 1) {
			return;
		}

		check_len = 2049 - keylen;

		while ((size = fread(&buf[keylen - 1], 1, check_len, stdin)) > 0) {
			check_len = size;
			idx = 0;

			for (idx = 0; idx < check_len; idx++) {
				int i;
				int str_flag = 1;

				for (i = 0; i < keylen; i++) {
					str_flag = (buf[idx + i] == key[i]);

					if (!str_flag) {
						break;
					}
				}

				if (str_flag) {
					match_flag = 1;
				}

				if (match_flag) {
					break;
				}
			}
			if (match_flag) {
				if (idx > 0) {
					// fprintf(fp,"idx:%d\n",idx);
					fwrite(buf, 1, idx, fp);
				}

				break;
			} else {
				// fprintf(fp,"check_len:%d\n",check_len);
				fwrite(buf, 1, check_len, fp);

				for (int i = 0; i < keylen; i++) {
					buf[i] = buf[check_len + i];
				}
			}
		}

		fclose(fp);
	}

	printf("Content-type: text/html\r\n\r\n");
}

void getTz(void)
{
	FILE *fp_tz;
	char buffer[128];

	fp_tz = fopen(TZ_FILE_PATH, "r");
	if (fp_tz) {
		fgets(buffer, 128, fp_tz);
		fclose(fp_tz);
		setenv("TZ", buffer, 1);
		tzset();
	}
}

void fcgiUpdateCA(void)
{
	char cmd[200];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(cmd, "/system/www/cgi-bin/updateCA.sh");
	status = system(cmd);
	printf("{\"rval\":%d}", status);
}

void fcgiResetCA(void)
{
	char cmd[200];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(cmd, "/system/www/cgi-bin/resetCA.sh");
	status = system(cmd);
	printf("{\"rval\":%d}", status);
}

void fcgiGetTime(void)
{
	time_t rawtime;
	struct tm *timeinfo;
	printf("Content-type: text/html\r\n\r\n");
	getTz();
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("%04d.%02d.%02d-%02d:%02d:%02d\n", 1900 + timeinfo->tm_year, (timeinfo->tm_mon) + 1, timeinfo->tm_mday,
	       timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void fcgiGetSntpConf(void)
{
	printf("Content-type: text/html\r\n\r\n");
	char ip[100];
	char hr[100];
	FILE *fp;
	if ((fp = fopen("/usrdata/active_setting/sntp.conf", "rb")) != NULL) {
		fgets(ip, 100, fp);
		fgets(hr, 100, fp);
	}
	fclose(fp);
	if ((sscanf(ip, "NTP_server=%s", ip) != EOF) && (sscanf(hr, "Interval=%s", hr) != EOF)) {
		printf("{\"NTPserver\":\"%s\",\"Interval\":\"%s\"}", ip, hr);
	} else {
		printf("{\"NTPserver\":\"\",\"Interval\":\"\"}");
	}
}

void fcgiGetEnabledConf(void)
{
	printf("Content-type: text/html\r\n\r\n");
	char Manual_enabled[20];
	char DST_enabled[20];
	char SyncWithPC_enabled[20];
	int Manual_en;
	int DST_en;
	int Sync_en;
	FILE *fp;
	if ((fp = fopen("/usrdata/active_setting/timeMode.conf", "rb")) != NULL) {
		fgets(Manual_enabled, 100, fp);
		fgets(DST_enabled, 100, fp);
		fgets(SyncWithPC_enabled, 100, fp);
	}
	fclose(fp);
	if ((sscanf(Manual_enabled, "Manual_enabled=%s", Manual_enabled) != EOF) &&
	    (sscanf(DST_enabled, "DST_enabled=%s", DST_enabled) != EOF) &&
	    (sscanf(SyncWithPC_enabled, "SyncWithPC_enabled=%s", SyncWithPC_enabled) != EOF)) {
		Manual_en = atoi(Manual_enabled);
		DST_en = atoi(DST_enabled);
		Sync_en = atoi(SyncWithPC_enabled);
		printf("{\"Manual_enabled\":%d,\"DST_enabled\":%d,\"SyncWithPC_"
		       "enabled\":%d}",
		       Manual_en, DST_en, Sync_en);
	} else {
		printf("{\"Manual_enabled\":\"\",\"DST_enabled\":\"\",\"SyncWithPC_"
		       "enabled\":\"\"}");
	}
}

void fcgiGetDSTConf(void)
{
	printf("Content-type: text/html\r\n\r\n");
	char DSTConf[100];
	char s[5] = ",", s1[5] = "DST", s2[5] = ":", s3[5] = ".", s4[5] = "/";
	char *TZ;
	char *DSTStart;
	char *DSTEnd;
	char *SGMT;
	char *SGMThr;
	char *SGMTmin;
	char *SMonthStart;
	char *SPriorityStart;
	char *SWeekStart;
	char *SHourStart;
	char *SMonthEnd;
	char *SPriorityEnd;
	char *SWeekEnd;
	char *SHourEnd;
	char *SBiasSet;
	char *SBiasHr;
	char *SBiasMin;
	int BiasHr = 0;
	int BiasMin = 0;
	int GMT = 0;
	int GMThr = 0;
	int GMTmin = 0;
	int MonthStart = 0;
	int PriorityStart = 0;
	int WeekStart = 0;
	int HourStart = 0;
	int MonthEnd = 0;
	int PriorityEnd = 0;
	int WeekEnd = 0;
	int HourEnd = 0;
	int BiasSet = 0;
	FILE *fp;
	if ((fp = fopen("/etc/TZ", "rb")) != NULL) {
		fgets(DSTConf, 100, fp);
	}
	fclose(fp);
	// s = ","
	TZ = strtok(DSTConf, s);
	DSTStart = strtok(NULL, s);
	DSTEnd = strtok(NULL, s);

	// s1 = "DST"
	strtok(TZ, s1);
	SGMT = strtok(NULL, s1);
	SBiasSet = strtok(NULL, s1);
	SGMThr = strtok(SGMT, s2);
	SGMTmin = strtok(NULL, s2);
	GMThr = atoi(SGMThr);
	GMTmin = atoi(SGMTmin);
	if (GMThr > 0) {
		GMT = GMThr * 60 + GMTmin;
	} else {
		GMT = GMThr * 60 - GMTmin;
	}

	// s2 = ":"
	SBiasHr = strtok(SBiasSet, s2);
	SBiasMin = strtok(NULL, s2);
	BiasHr = atoi(SBiasHr);
	BiasMin = atoi(SBiasMin);
	if (BiasHr > 0) {
		BiasHr = BiasHr * 60 + BiasMin;
	} else {
		BiasHr = BiasHr * 60 - BiasMin;
	}
	BiasHr = GMT - BiasHr;

	// s3 = "."
	SMonthStart = strtok(DSTStart, s3);
	SPriorityStart = strtok(NULL, s3);
	DSTStart = strtok(NULL, s3);
	sscanf(SMonthStart, "M%s", SMonthStart);

	SMonthEnd = strtok(DSTEnd, s3);
	SPriorityEnd = strtok(NULL, s3);
	DSTEnd = strtok(NULL, s3);
	sscanf(SMonthEnd, "M%s", SMonthEnd);

	// s4 = "/"
	SWeekStart = strtok(DSTStart, s4);
	SHourStart = strtok(NULL, s4);

	SWeekEnd = strtok(DSTEnd, s4);
	SHourEnd = strtok(NULL, s4);

	MonthStart = atoi(SMonthStart);
	PriorityStart = atoi(SPriorityStart);
	WeekStart = atoi(SWeekStart);
	HourStart = atoi(SHourStart);
	MonthEnd = atoi(SMonthEnd);
	PriorityEnd = atoi(SPriorityEnd);
	WeekEnd = atoi(SWeekEnd);
	HourEnd = atoi(SHourEnd);
	BiasSet = BiasHr;

	printf("{\"GMT\":%d,\"MonthStart\":%d,\"PriorityStart\":%d,\"WeekStart\":"
	       "%d,\"HourStart\":%d,\"MonthEnd\":%d,\"PriorityEnd\":%d,"
	       "\"WeekEnd\":%d,\"HourEnd\":%d,\"BiasSet\":%d}",
	       GMT, MonthStart, PriorityStart, WeekStart, HourStart, MonthEnd, PriorityEnd, WeekEnd, HourEnd, BiasSet);
}
void fcgiGetTZ(void)
{
	char TZConf[20];
	char s[5] = "GMT", s1[5] = ":";
	char *TZ;
	char *TZ_hr;
	char *TZ_min;
	FILE *fp;
	int GMT = 0;
	printf("Content-type: text/html\r\n\r\n");
	if ((fp = fopen("/etc/TZ", "rb")) != NULL) {
		fgets(TZConf, 100, fp);
		fclose(fp);
	}
	TZ = strtok(TZConf, s);
	TZ_hr = strtok(TZ, s1);
	TZ_min = strtok(NULL, s1);
	if (TZ_min == NULL) {
		TZ_min = "0";
	}
	GMT = atoi(TZ_hr);

	if (GMT < 0) {
		GMT = GMT * 60 - atoi(TZ_min);
	} else {
		GMT = GMT * 60 + atoi(TZ_min);
	}
	printf("{\"GMT\":%d,\"MonthStart\":4,\"PriorityStart\":1,\"WeekStart\":0,"
	       "\"HourStart\":2,\"MonthEnd\":10,\"PriorityEnd\":2,\"WeekEnd\":0,"
	       "\"HourEnd\":2,\"BiasSet\":60}",
	       GMT);
}

void fcgiGetTimeSwitch(void)
{
	char Switch_enabled[100];
	char SwitchStart_hr[100];
	char SwitchStart_min[100];
	char SwitchEnd_hr[100];
	char SwitchEnd_min[100];
	printf("Content-type: text/html\r\n\r\n");
	int Switch_en = 0;
	FILE *fp;
	if ((fp = fopen("/usrdata/active_setting/TimeSwitch.conf", "rb")) != NULL) {
		fgets(Switch_enabled, 100, fp);
		fgets(SwitchStart_hr, 100, fp);
		fgets(SwitchStart_min, 100, fp);
		fgets(SwitchEnd_hr, 100, fp);
		fgets(SwitchEnd_min, 100, fp);
	}
	fclose(fp);

	if ((sscanf(Switch_enabled, "TimeSwitch_enabled=%s", Switch_enabled) != EOF) &&
	    (sscanf(SwitchStart_hr, "TimeStartHr=%s", SwitchStart_hr) != EOF) &&
	    (sscanf(SwitchStart_min, "TimeStartMin=%s", SwitchStart_min) != EOF) &&
	    (sscanf(SwitchEnd_hr, "TimeEndHr=%s", SwitchEnd_hr) != EOF) &&
	    (sscanf(SwitchEnd_min, "TimeEndMin=%s", SwitchEnd_min) != EOF)) {
		Switch_en = atoi(Switch_enabled);
		printf("{\"TimeSwitch_enabled\":%d,\"time_start\":\"%s:%s\",\"time_"
		       "end\":\"%s:%s\"}",
		       Switch_en, SwitchStart_hr, SwitchStart_min, SwitchEnd_hr, SwitchEnd_min);
	} else {
		printf("{\"TimeSwitch_enabled\":\"\",\"time_start\":\"\",\"time_end\":"

		       "\"\"}");
	}
}

void fcgiExportSetting(void)
{
	FILE *fp;
	size_t bytes;
	char buf[2048];
	printf("Content-Type:application/octet-stream; name = \"Export.dat\"\r\n");
	printf("Content-Disposition: attachment; filename = \"Export.dat\"\r\n\n");
	system("/system/www/cgi-bin/exportSetting.sh");

	if ((fp = fopen("/tmp/Export.dat", "rb")) != NULL) {
		while (0 < (bytes = fread(buf, 1, sizeof(buf), fp))) {
			fwrite(buf, 1, bytes, FCGI_stdout);
		}

		fclose(fp);
	}
}

void fcgiFirmwareUpload(void)
{
	char buf1[2048];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(buf1, "mv /tmp/00000* /tmp/update.swu");
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);

#ifdef SECURE_WEB_PAGE
	int bootpart_to_write = 0;
	char img_collection[50];

	FILE *fp = popen("fw_printenv slot_b_active -n", "r");
	if (fp == NULL) {
		perror("Failed to run fw_printenv command");
		return;
	}

	char output[10];
	if (fgets(output, sizeof(output), fp) != NULL) {
		output[strcspn(output, "\n")] = 0; // Remove newline
		if (strcmp(output, "1") == 0) {
			bootpart_to_write = 1;
		} else {
			bootpart_to_write = 2;
		}
	} else {
		pclose(fp);
		return;
	}
	pclose(fp);

	snprintf(img_collection, sizeof(img_collection), "release,copy_%d", bootpart_to_write);
	fprintf(stderr, "img_collection : %s\n", img_collection);

	// Construct swupdate command
	snprintf(buf1, sizeof(buf1),
	         "swupdate -v -c -b \"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19\" "
	         "-e \"%s\" -i /tmp/update.swu -k /usr/share/misc/sysupd_pub_key.pem > /tmp/swupdate.log 2>&1",
	         img_collection);

	// Execute swupdate command
	fprintf(stderr, "swupdate command : %s\n", buf1);
	status = system(buf1);
	fprintf(stderr, "status = %d\n", status);

	if (status != 0) {
		perror("Failed to verify the firmware image or the image is illeagal\n");
		sprintf(buf1, "rm -rf /tmp/update.swu /tmp/sw-description*");
		system(buf1);
		return;
	}

	snprintf(buf1, sizeof(buf1), "chmod 400 /tmp/update.swu");
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);
#endif

	return;
}

void fcgiGetHostname(void)
{
	char hostname[128];
	printf("Content-type: text/html\r\n\r\n");
	gethostname(hostname, 127);
	printf("%s", hostname);
}

void fcgiSetToDefault(void)
{
	printf("Content-type: text/html\r\n\r\n");
#ifdef SECURE_WEB_PAGE
	system("/system/www/cgi-bin/setToDefault_secure.sh");
#else
	system("/system/www/cgi-bin/setToDefault.sh");
#endif
	printf("{\"rval\":0}");
}

void fcgiReboot(void)
{
	printf("Content-type: text/html\r\n\r\n");
	system("/sbin/reboot");
	printf("{\"rval\":0}");
}

void fcgiStopStream(char *MachineMode)
{
	char buf1[2048];
	int status;

	printf("Content-type: text/html\r\n\r\n");
#ifdef SECURE_WEB_PAGE
	system("export LD_LIBRARY_PATH=/system/lib");
	snprintf(buf1, sizeof(buf1), "sh /etc/init.d/develop/S95flv_agt705-4 stop");
	fprintf(stderr, "stop flv_server : %s\n", buf1);
	status = system(buf1);
	fprintf(stderr, "status = %d\n", status);

	snprintf(buf1, sizeof(buf1), "sh /etc/init.d/develop/S26av_main2_agt705-4 stop");
	fprintf(stderr, "stop av_main2 : %s\n", buf1);
	status = system(buf1);
	fprintf(stderr, "status = %d\n", status);

	snprintf(buf1, sizeof(buf1), "sh /etc/init.d/develop/S22ccserver stop");
	fprintf(stderr, "stop S22ccserver : %s\n", buf1);
	status = system(buf1);
	fprintf(stderr, "status = %d\n", status);

	snprintf(buf1, sizeof(buf1), "sh /etc/init.d/develop/S21db stop");
	fprintf(stderr, "stop S21db : %s\n", buf1);
	status = system(buf1);
	fprintf(stderr, "status = %d\n", status);

	snprintf(buf1, sizeof(buf1), "sh /system/mpp/script/load_mpp.sh -e");
	fprintf(stderr, "stop mpp : %s\n", buf1);
	status = system(buf1);
	fprintf(stderr, "status = %d\n", status);
#else
	(void)(status);
#endif
	sprintf(buf1, "/etc/init.d/%s/S95rtmp stop", MachineMode);
	system(buf1);
	sprintf(buf1, "/etc/init.d/%s/S26av_main stop", MachineMode);
	system(buf1);
	system("/system/mpp/script/load_mpp.sh -e");
	printf("{\"rval\":0}");
}

void fcgiSysupdOS(void)
{
	int status;
	printf("Content-type: text/html\r\n\r\n");
	status = system("/usr/sbin/sysupd > /tmp/sysupd_terminal.log");
	printf("{\"rval\":%d}", status);
}

void fcgiSwitch2SysupdOS(void)
{
	printf("Content-type: text/html\r\n\r\n");
	system("/usr/sbin/sysupd-recover");
	printf("{\"rval\":0}");
}

void fcgiChangePass(char *buf)
{
	int exists, exists1;
	enum json_type type;
	char cmdstr[200];
	char name[100];
	char pass[100];
	json_object *obj_name, *obj_pass;
	json_object *jobj;
	cmdstr[0] = 0;

	printf("Content-type: text/html\r\n\r\n");

	jobj = json_tokener_parse(buf);
	exists = json_object_object_get_ex(jobj, "name", &obj_name);

	if (exists) {
		type = json_object_get_type(obj_name);

		if (type == json_type_string) {
			strncpy(name, json_object_get_string(obj_name), 100);
			// printf("<p>name:%s\n</p>",hostname);
#ifdef SECURE_WEB_PAGE
			strcat(cmdstr, "/system/www/cgi-bin/passwd_secure.sh ");
#else
			strcat(cmdstr, "/system/www/cgi-bin/passwd.sh ");
#endif
			strcat(cmdstr, name);

			exists1 = json_object_object_get_ex(jobj, "pass", &obj_pass);

			if (exists1) {
				type = json_object_get_type(obj_pass);

				if (type == json_type_string) {
					strncpy(pass, json_object_get_string(obj_pass), 100);
					// printf("<p>pass:%s\n</p>",pass);
					strcat(cmdstr, " ");
					strcat(cmdstr, "'");
					strcat(cmdstr, pass);
					strcat(cmdstr, "'");
					system(cmdstr);
					printf("{\"rval\":0}");
				}
			}
		}
	}

	json_object_put(jobj);
}

void fcgiAssignIP(char *MachineMode, char *buf)
{
	int exists, exists1;
	int status;
	enum json_type type;
	char cmdstr[200];
	char dec_buf[2048];
	char buf1[2048];
	char hostname[128];
	char IPAddress[20];
	char Netmask[20];
	char Gateway[20];
	char DNS1[20];
	char DNS2[20];
	int dhcp_status;
	json_object *obj_IPAddress, *obj_Netmask, *obj_Gateway, *obj_DNS1, *obj_DNS2, *obj_dhcp_status, *obj_hostname,
	        *jobj;

	decode(buf, dec_buf);
	jobj = json_tokener_parse(dec_buf);
	exists = json_object_object_get_ex(jobj, "Hostname", &obj_hostname);
	cmdstr[0] = 0;
	printf("Content-type: text/html\r\n\r\n");
	cmdstr[0] = 0;

	if (exists) {
		type = json_object_get_type(obj_hostname);

		if (type == json_type_string) {
			strncpy(hostname, json_object_get_string(obj_hostname), 20);
			printf("<p>Hostname:%s\n</p>", hostname);
			strcat(cmdstr, " -n ");
			strcat(cmdstr, hostname);
		}
	}

	exists = json_object_object_get_ex(jobj, "IPAddress", &obj_IPAddress);

	if (exists) {
		type = json_object_get_type(obj_IPAddress);

		if (type == json_type_string) {
			strncpy(IPAddress, json_object_get_string(obj_IPAddress), 20);
			printf("<p>IPAddress:%s\n</p>", IPAddress);
			strcat(cmdstr, " -i ");
			strcat(cmdstr, IPAddress);
		}
	}

	exists = json_object_object_get_ex(jobj, "Netmask", &obj_Netmask);

	if (exists) {
		type = json_object_get_type(obj_Netmask);

		if (type == json_type_string) {
			strncpy(Netmask, json_object_get_string(obj_Netmask), 20);
			printf("<p>Netmask:%s\n</p>", Netmask);
			strcat(cmdstr, " -m ");
			strcat(cmdstr, Netmask);
		}
	}

	exists = json_object_object_get_ex(jobj, "Gateway", &obj_Gateway);

	if (exists) {
		type = json_object_get_type(obj_Gateway);

		if (type == json_type_string) {
			strncpy(Gateway, json_object_get_string(obj_Gateway), 20);
			printf("<p>Gateway:%s\n</p>", Gateway);
			strcat(cmdstr, " -g ");
			strcat(cmdstr, Gateway);
		}
	}

	exists = json_object_object_get_ex(jobj, "dhcp", &obj_dhcp_status);

	if (exists) {
		type = json_object_get_type(obj_dhcp_status);

		if (type == json_type_int) {
			dhcp_status = json_object_get_int(obj_dhcp_status);
			printf("<p>Dhcp_status:%d\n</p>", dhcp_status);
			strcat(cmdstr, " -s ");

			if (dhcp_status) {
				strcat(cmdstr, "dhcp");
			} else {
				strcat(cmdstr, "static");
			}
		}
	}

	exists = json_object_object_get_ex(jobj, "DNS1", &obj_DNS1);

	if (exists) {
		type = json_object_get_type(obj_DNS1);

		if (type == json_type_string) {
			strncpy(DNS1, json_object_get_string(obj_DNS1), 20);
			printf("<p>DNS1:%s\n</p>", DNS1);
			strcat(cmdstr, " -d \"");
			strcat(cmdstr, DNS1);
			// If DNS1 exists , consider DNS2
			exists1 = json_object_object_get_ex(jobj, "DNS2", &obj_DNS2);

			if (exists1) {
				type = json_object_get_type(obj_DNS2);

				if (type == json_type_string) {
					strncpy(DNS2, json_object_get_string(obj_DNS2), 20);
					printf("<p>DNS2:%s\n</p>", DNS2);
					strcat(cmdstr, " ");
					strcat(cmdstr, DNS2);
				}

				strcat(cmdstr, "\" ");
			}
		}
	}
	json_object_put(jobj);
	sprintf(buf1, "/system/bin/ip_assign %s", cmdstr);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);
	sprintf(buf1, "/etc/init.d/%s/S40network restart", MachineMode);
	printf("<p>%s\n</p>", buf1);
	// status=system(buf1);
	printf("<p>status(echo)=%d</p>", status);
}

void fcgiNetInfo(void)
{
	char buf[100];
	char buf1[50];
	char hostname[128];
	char IPAddress[20];
	char Netmask[20];
	char Gateway[20];
	char DNS1[20];
	char DNS2[20];
	char MAC[20];
	int dhcp_status;

	printf("Content-type: text/html\r\n\r\n");
	IPAddress[0] = 0;
	Netmask[0] = 0;
	DNS1[0] = 0;
	DNS2[0] = 0;
	MAC[0] = 0;
	Gateway[0] = 0;
	dhcp_status = 0;

	gethostname(hostname, 127);
	FILE *fp = fopen("/etc/network/interfaces", "rb");

	fgets(buf, sizeof(buf), fp);
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (sscanf(buf, "iface eth0 inet %s", buf1) == 1) {
			if (strcmp(buf1, "static") == 0) {
				dhcp_status = 0;
			} else {
				dhcp_status = 1;
			}
		} else if (sscanf(buf, "address %s", IPAddress) == 1) {
		} else if (sscanf(buf, "netmask %s", Netmask) == 1) {
		} else if (sscanf(buf, "gateway %s", Gateway) == 1) {
		} else if (sscanf(buf, "dns-nameservers %s %s", DNS1, DNS2) > 1) {
		}
	}

	fclose(fp);
	fp = fopen("/sys/class/net/eth0/address", "rb");
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%s", MAC);
	fclose(fp);
	printf("{\"Hostname\":\"%s\",\"IPAddress\":\"%s\",\"Netmask\":\"%s\","
	       "\"Gateway\":\"%s\",\"DNS1\":\"%s\",\"DNS2\":\"%s\",\"MAC\":\"%"
	       "s\",\"dhcp\":%d}",
	       hostname, IPAddress, Netmask, Gateway, DNS1, DNS2, MAC, dhcp_status);
}

void fcgiUpTime(void)
{
	struct sysinfo sys_info;
	int days, hours, mins;

	printf("Content-type: text/html\r\n\r\n");

	if (sysinfo(&sys_info) != 0) {
		perror("sysinfo");
	}

	days = sys_info.uptime / 86400;
	hours = (sys_info.uptime / 3600) - (days * 24);
	mins = (sys_info.uptime / 60) - (days * 1440) - (hours * 60);

	printf("%ddays, %dhours, %dminutes, %ldseconds", days, hours, mins, sys_info.uptime % 60);
}

void fcgiGetMAC(void)
{
	char buf[50];
	printf("Content-type: text/html\r\n\r\n");
	FILE *fp = fopen("/sys/class/net/eth0/address", "rb");
	fgets(buf, 50, fp);
	fclose(fp);
	printf("%s\n", buf);
}

void fcgiGetFirmwareVersion(void)
{
	char buf[100];
	FILE *fp;
	printf("Content-type: text/html\r\n\r\n");

	if ((fp = fopen("/etc/sw-version", "rb")) != NULL) {
		fgets(buf, 100, fp);
		fclose(fp);
		printf("%s\n", buf);
	} else {
		printf("0.0.1\n");
	}
}

void fcgiGetIPAddress(void)
{
	int fd;
	struct ifreq ifr;

	printf("Content-type: text/html\r\n\r\n");
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;
	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	/* display result */
	printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void fcgiGetNetmask(void)
{
	int fd;
	struct ifreq ifr;

	printf("Content-type: text/html\r\n\r\n");
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;
	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFNETMASK, &ifr);
	close(fd);
	/* display result */
	printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void fcgiGetGateway(void)
{
	FILE *fp;
	unsigned int p;
	char buf[256];
	char iface[16];
	unsigned long dest_addr, gate_addr;
	struct in_addr addr;

	printf("Content-type: text/html\r\n\r\n");
	p = (unsigned int)INADDR_NONE;

	fp = fopen("/proc/net/route", "r");

	if (fp == NULL) {
		// break;
		return;
	}

	/* Skip title line */
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp)) {
		if (sscanf(buf, "%s\t%lX\t%lX", iface, &dest_addr, &gate_addr) != 3 || dest_addr != 0) {
			continue;
		}

		p = (unsigned int)gate_addr;
		break;
	}

	fclose(fp);
	addr.s_addr = (in_addr_t)p;
	printf("%s\n", inet_ntoa(addr));
}

void fcgiGetDNS(void)
{
	FILE *fp;
	char dns1[32];
	char dns2[32];
	char buf[2048];

	printf("Content-type: text/html\r\n\r\n");
	fp = fopen("/etc/resolv.conf", "r");

	if (fp == NULL) {
		return;
	}

	/* Skip title line */
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp)) {
		if (sscanf(buf, "nameserver %s", dns1) != 1) {
			continue;
		}

		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "nameserver %s", dns2);
		break;
	}

	fclose(fp);
	printf("{\"dns1\":\"%s\",\"dns2\":\"%s\"}\n", dns1, dns2);
}

void fcgiSetIP(char *buf)
{
	pid_t pid;
	int status;
	printf("Content-type: text/html\r\n\r\n");
	printf("arg:%s", buf);
	pid = fork();

	if (pid == 0) {
		execl("/system/www/cgi-bin/setIP.sh", "setIP.sh", buf, (char *)NULL);
	} else {
		wait(&status);
	}
}

void fcgiSetMask(char *buf)
{
	pid_t pid;
	int status;
	printf("Content-type: text/html\r\n\r\n");
	printf("arg:%s", buf);
	pid = fork();

	if (pid == 0) {
		execl("/system/www/cgi-bin/setMask.sh", "setMask.sh", buf, (char *)NULL);
	} else {
		wait(&status);
	}
}

void fcgiSetGateway(char *buf)
{
	pid_t pid;
	int status;
	printf("Content-type: text/html\r\n\r\n");
	printf("arg:%s", buf);
	pid = fork();

	if (pid == 0) {
		execl("/system/www/cgi-bin/setGateway.sh", "setGateway.sh", buf, (char *)NULL);
	} else {
		wait(&status);
	}
}

void fcgiSetDNS(char *buf)
{
	char primary[20] = "";
	char secondary[20] = "";
	char str[3] = " ";
	char str1[3] = "\"";
	pid_t pid;
	int status;
	printf("Content-type: text/html\r\n\r\n");
	printf("arg:%s", buf);
	sscanf(buf, "%[^&]&%s", primary, secondary);
	strcat(str1, primary);
	strcat(str1, str);
	strcat(str1, secondary);
	pid = fork();

	if (pid == 0) {
		execl("/system/www/cgi-bin/setDNS.sh.", "setDNS.sh", str1, (char *)NULL);
	} else {
		wait(&status);
	}
}

void fcgiSetTime(char *MachineMode, char *buf)
{
	char buf1[2048];
	pid_t pid;
	int status;
	printf("Content-type: text/html\r\n\r\n");
	printf("arg:%s", buf);
	pid = fork();

	if (pid == 0) {
		execl("/system/www/cgi-bin/time.sh", "time.sh", buf, (char *)NULL);
	} else {
		wait(&status);
	}

	sprintf(buf1, "/etc/init.d/%s/S50sntp stop", MachineMode);
	status = system(buf1);
	printf("<p>status(echo)=%d</p>", status);
	sprintf(buf1, "/etc/init.d/%s/S99cron restart", MachineMode);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status(echo)=%d</p>", status);
}

void fcgiSetSntpConf(char *MachineMode, char *buf)
{
	int exists, exists1;
	enum json_type type;
	char cmdstr[200];
	char buf1[2048];
	int status;
	char NTPserver[20];
	char Interval[20];
	json_object *obj_NTPserver, *obj_Interval;
	json_object *jobj = json_tokener_parse(buf);

	printf("Content-type: text/html\r\n\r\n");
	cmdstr[0] = 0;

	exists = json_object_object_get_ex(jobj, "NTPserver", &obj_NTPserver);

	if (exists) {
		type = json_object_get_type(obj_NTPserver);

		if (type == json_type_string) {
			strncpy(NTPserver, json_object_get_string(obj_NTPserver), 20);
			strcat(cmdstr, NTPserver);

			exists1 = json_object_object_get_ex(jobj, "Interval", &obj_Interval);

			if (exists1) {
				type = json_object_get_type(obj_Interval);

				if (type == json_type_string) {
					strncpy(Interval, json_object_get_string(obj_Interval), 20);
					strcat(cmdstr, " ");
					strcat(cmdstr, Interval);
				}
			}
		}
	}

	json_object_put(jobj);
	sprintf(buf1, "/system/www/cgi-bin/SntpConf.sh set %s", cmdstr);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);
	sprintf(buf1, "/etc/init.d/%s/S50sntp restart", MachineMode);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status(echo)=%d</p>", status);
	sprintf(buf1, "/etc/init.d/%s/S99cron restart", MachineMode);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status(echo)=%d</p>", status);
}

void fcgiDSTSet(char *buf)
{
	int exists;
	int BiasHr = 0;
	enum json_type type;
	char cmdstr[200];
	char buf1[2048];
	int status;
	int GMT = 0;
	int GMT_hr = 0;
	int GMT_min = 0;
	int MonthStart = 0;
	int PriorityStart = 0;
	int WeekStart = 0;
	int HourStart = 0;
	int MonthEnd = 0;
	int PriorityEnd = 0;
	int WeekEnd = 0;
	int HourEnd = 0;
	int BiasSet = 0;
	json_object *obj_GMT, *obj_MonthStart, *obj_PriorityStart, *obj_WeekStart, *obj_HourStart, *obj_MonthEnd,
	        *obj_PriorityEnd, *obj_WeekEnd, *obj_HourEnd, *obj_BiasSet;
	json_object *jobj = json_tokener_parse(buf);
	printf("Content-type: text/html\r\n\r\n");
	cmdstr[0] = 0;

	exists = json_object_object_get_ex(jobj, "GMT", &obj_GMT);

	if (exists) {
		type = json_object_get_type(obj_GMT);

		if (type == json_type_int) {
			GMT = json_object_get_int(obj_GMT);
		}
	}

	exists = json_object_object_get_ex(jobj, "MonthStart", &obj_MonthStart);

	if (exists) {
		type = json_object_get_type(obj_MonthStart);

		if (type == json_type_int) {
			MonthStart = json_object_get_int(obj_MonthStart);
		}
	}

	exists = json_object_object_get_ex(jobj, "PriorityStart", &obj_PriorityStart);

	if (exists) {
		type = json_object_get_type(obj_PriorityStart);

		if (type == json_type_int) {
			PriorityStart = json_object_get_int(obj_PriorityStart);
		}
	}

	exists = json_object_object_get_ex(jobj, "WeekStart", &obj_WeekStart);

	if (exists) {
		type = json_object_get_type(obj_WeekStart);

		if (type == json_type_int) {
			WeekStart = json_object_get_int(obj_WeekStart);
		}
	}

	exists = json_object_object_get_ex(jobj, "HourStart", &obj_HourStart);

	if (exists) {
		type = json_object_get_type(obj_HourStart);
		if (exists) {
			type = json_object_get_type(obj_HourStart);

			if (type == json_type_int) {
				HourStart = json_object_get_int(obj_HourStart);
			}
		}

		exists = json_object_object_get_ex(jobj, "MonthEnd", &obj_MonthEnd);

		if (exists) {
			type = json_object_get_type(obj_MonthEnd);

			if (type == json_type_int) {
				MonthEnd = json_object_get_int(obj_MonthEnd);
			}
		}

		exists = json_object_object_get_ex(jobj, "PriorityEnd", &obj_PriorityEnd);

		if (exists) {
			type = json_object_get_type(obj_PriorityEnd);

			if (type == json_type_int) {
				PriorityEnd = json_object_get_int(obj_PriorityEnd);
			}
		}

		exists = json_object_object_get_ex(jobj, "WeekEnd", &obj_WeekEnd);

		if (exists) {
			type = json_object_get_type(obj_WeekEnd);

			if (type == json_type_int) {
				WeekEnd = json_object_get_int(obj_WeekEnd);
			}
		}

		exists = json_object_object_get_ex(jobj, "HourEnd", &obj_HourEnd);

		if (exists) {
			type = json_object_get_type(obj_HourEnd);

			if (type == json_type_int) {
				HourEnd = json_object_get_int(obj_HourEnd);
			}
		}

		exists = json_object_object_get_ex(jobj, "BiasSet", &obj_BiasSet);

		if (exists) {
			type = json_object_get_type(obj_BiasSet);

			if (type == json_type_int) {
				BiasSet = json_object_get_int(obj_BiasSet);
			}
		}

		BiasSet = GMT - BiasSet;
		BiasHr = BiasSet / 60;
		BiasSet = BiasSet - BiasHr * 60;
		if (BiasSet < 0) {
			BiasSet = -BiasSet;
		}

		GMT_hr = GMT / 60;
		GMT_min = GMT - GMT_hr * 60;
		if (GMT_min < 0) {
			GMT_min = -GMT_min;
		}

		json_object_put(jobj);

		sprintf(cmdstr, "GMT%d:%dDST%d:%d,M%d.%d.%d/%d,M%d.%d.%d/%d", GMT_hr, GMT_min, BiasHr, BiasSet,
		        MonthStart, PriorityStart, WeekStart, HourStart, MonthEnd, PriorityEnd, WeekEnd, HourEnd);

		sprintf(buf1, "/system/www/cgi-bin/DSTConf.sh set %s", cmdstr);
		printf("<p>%s\n</p>", buf1);
		status = system(buf1);
		printf("<p>status=%d</p>", status);
	}
}

void fcgiTZSet(char *buf)
{
	int exists;
	enum json_type type;
	char cmdstr[200];
	char buf1[2048];
	int status;
	int GMT = 0;
	int GMT_hr = 0;
	int GMT_min = 0;
	json_object *obj_GMT;
	json_object *jobj = json_tokener_parse(buf);
	printf("Content-type: text/html\r\n\r\n");
	cmdstr[0] = 0;

	exists = json_object_object_get_ex(jobj, "GMT", &obj_GMT);

	if (exists) {
		type = json_object_get_type(obj_GMT);

		if (type == json_type_int) {
			GMT = json_object_get_int(obj_GMT);
		}
	}

	GMT_hr = GMT / 60;
	GMT_min = GMT - GMT_hr * 60;
	if (GMT_min < 0) {
		GMT_min = -GMT_min;
	}

	json_object_put(jobj);
	sprintf(cmdstr, "GMT%d:%d", GMT_hr, GMT_min);
	sprintf(buf1, "/system/www/cgi-bin/DSTConf.sh set %s", cmdstr);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);
}

void fcgiEnabledSet(char *buf)
{
	int exists;
	int Manual_en = 0;
	int DST_en = 0;
	int Sync_en = 0;
	enum json_type type;
	char cmdstr[200];
	char buf1[2048];
	int status;
	json_object *obj_Manual, *obj_DST, *obj_Sync;
	json_object *jobj = json_tokener_parse(buf);
	printf("Content-type: text/html\r\n\r\n");
	cmdstr[0] = 0;

	exists = json_object_object_get_ex(jobj, "Manual_enabled", &obj_Manual);

	if (exists) {
		type = json_object_get_type(obj_Manual);

		if (type == json_type_boolean) {
			Manual_en = json_object_get_int(obj_Manual);
		}
	}

	exists = json_object_object_get_ex(jobj, "DST_enabled", &obj_DST);

	if (exists) {
		type = json_object_get_type(obj_DST);

		if (type == json_type_boolean) {
			DST_en = json_object_get_int(obj_DST);
		}
	}

	exists = json_object_object_get_ex(jobj, "SyncWithPC_enabled", &obj_Sync);

	if (exists) {
		type = json_object_get_type(obj_Sync);

		if (type == json_type_boolean) {
			Sync_en = json_object_get_int(obj_Sync);
		}
	}

	json_object_put(jobj);
	sprintf(cmdstr, "%d %d %d", Manual_en, DST_en, Sync_en);
	sprintf(buf1, "/system/www/cgi-bin/DSTConf.sh setEnabled %s", cmdstr);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);
}

void fcgiTimeSwitchSet(char *MachineMode, char *buf)
{
	int exists;
	char TimeStart[20];
	char TimeEnd[20];
	char s[5] = ":";
	char *TimeStart_hr;
	char *TimeStart_min;
	char *TimeEnd_hr;
	char *TimeEnd_min;
	int timeStart_hr = 0;
	int timeStart_min = 0;
	int timeEnd_hr = 0;
	int timeEnd_min = 0;
	int TimeSwitch_en = 0;
	int timeNow_hr = 0;
	int timeNow_min = 0;
	enum json_type type;
	char cmdstr[200];
	char buf1[2048];
	int status;
	json_object *obj_TimeStart, *obj_TimeEnd, *obj_TimeSwitch_enabled, *obj_TimeNowHr, *obj_TimeNowMin;
	json_object *jobj = json_tokener_parse(buf);
	printf("Content-type: text/html\r\n\r\n");
	cmdstr[0] = 0;

	exists = json_object_object_get_ex(jobj, "TimeSwitch_enabled", &obj_TimeSwitch_enabled);

	if (exists) {
		type = json_object_get_type(obj_TimeSwitch_enabled);

		if (type == json_type_boolean) {
			TimeSwitch_en = json_object_get_int(obj_TimeSwitch_enabled);
		}
	}

	exists = json_object_object_get_ex(jobj, "time_start", &obj_TimeStart);

	if (exists) {
		type = json_object_get_type(obj_TimeStart);

		if (type == json_type_string) {
			strncpy(TimeStart, json_object_get_string(obj_TimeStart), 20);
		}
	}

	exists = json_object_object_get_ex(jobj, "time_end", &obj_TimeEnd);

	if (exists) {
		type = json_object_get_type(obj_TimeEnd);

		if (type == json_type_string) {
			strncpy(TimeEnd, json_object_get_string(obj_TimeEnd), 20);
		}
	}

	exists = json_object_object_get_ex(jobj, "time_now_hr", &obj_TimeNowHr);

	if (exists) {
		type = json_object_get_type(obj_TimeNowHr);

		if (type == json_type_int) {
			timeNow_hr = json_object_get_int(obj_TimeNowHr);
		}
	}

	exists = json_object_object_get_ex(jobj, "time_now_min", &obj_TimeNowMin);

	if (exists) {
		type = json_object_get_type(obj_TimeNowMin);

		if (type == json_type_int) {
			timeNow_min = json_object_get_int(obj_TimeNowMin);
		}
	}

	TimeStart_hr = strtok(TimeStart, s);
	TimeStart_min = strtok(NULL, s);

	TimeEnd_hr = strtok(TimeEnd, s);
	TimeEnd_min = strtok(NULL, s);

	timeStart_hr = atoi(TimeStart_hr);
	timeStart_min = atoi(TimeStart_min);
	timeEnd_hr = atoi(TimeEnd_hr);
	timeEnd_min = atoi(TimeEnd_min);

	json_object_put(jobj);

	if (timeEnd_hr >= timeStart_hr) {
		if (timeEnd_hr == timeNow_hr && timeNow_hr > timeStart_hr) {
			if (timeNow_min >= timeEnd_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			} else if (timeNow_min < timeEnd_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			}
		} else if (timeEnd_hr > timeNow_hr && timeNow_hr == timeStart_hr) {
			if (timeNow_min >= timeStart_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			} else if (timeNow_min < timeStart_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			}
		} else if (timeEnd_hr > timeNow_hr && timeNow_hr > timeStart_hr) {
			sprintf(buf1, "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
			printf("<p>%s\n</p>", buf1);
			status = system(buf1);
			printf("<p>status=%d</p>", status);
		} else if (timeEnd_hr == timeNow_hr && timeNow_hr == timeStart_hr) {
			if (timeEnd_min > timeStart_min) {
				if (timeEnd_min > timeNow_min && timeNow_min >= timeStart_min) {
					sprintf(buf1,
					        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
					printf("<p>%s\n</p>", buf1);
					status = system(buf1);
					printf("<p>status=%d</p>", status);
				} else {
					sprintf(buf1,
					        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
					printf("<p>%s\n</p>", buf1);
					status = system(buf1);
					printf("<p>status=%d</p>", status);
				}
			} else if (timeEnd_min < timeStart_min) {
				if (timeStart_min > timeNow_min && timeNow_min >= timeEnd_min) {
					sprintf(buf1,
					        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
					printf("<p>%s\n</p>", buf1);
					status = system(buf1);
					printf("<p>status=%d</p>", status);
				} else {
					sprintf(buf1,
					        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
					printf("<p>%s\n</p>", buf1);
					status = system(buf1);
					printf("<p>status=%d</p>", status);
				}
			}
		} else {
			sprintf(buf1, "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
			printf("<p>%s\n</p>", buf1);
			status = system(buf1);
			printf("<p>status=%d</p>", status);
		}
	} else if (timeStart_hr > timeEnd_hr) {
		if (timeStart_hr == timeNow_hr && timeNow_hr > timeEnd_hr) {
			if (timeNow_min >= timeStart_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			} else if (timeNow_min < timeStart_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			}
		} else if (timeStart_hr > timeNow_hr && timeNow_hr == timeEnd_hr) {
			if (timeNow_min >= timeEnd_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			} else if (timeNow_min < timeEnd_min) {
				sprintf(buf1,
				        "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
				printf("<p>%s\n</p>", buf1);
				status = system(buf1);
				printf("<p>status=%d</p>", status);
			}
		} else if (timeEnd_hr > timeNow_hr && timeNow_hr > timeStart_hr) {
			sprintf(buf1, "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"ON\"}'");
			printf("<p>%s\n</p>", buf1);
			status = system(buf1);
			printf("<p>status=%d</p>", status);
		} else {
			sprintf(buf1, "/system/bin/ccclient -c AGTX_CMD_ADV_IMG_PREF -s '{\"night_mode\":\"OFF\"}'");
			printf("<p>%s\n</p>", buf1);
			status = system(buf1);
			printf("<p>status=%d</p>", status);
		}
	}

	sprintf(cmdstr, "%d %02d %02d %02d %02d", TimeSwitch_en, timeStart_hr, timeStart_min, timeEnd_hr, timeEnd_min);
	sprintf(buf1, "/system/www/cgi-bin/TimeSwitch.sh setTimeSwitch %s", cmdstr);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status=%d</p>", status);
	sprintf(buf1, "/etc/init.d/%s/S55timeSwitch restart", MachineMode);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status(echo)=%d</p>", status);
	sprintf(buf1, "/etc/init.d/%s/S99cron restart", MachineMode);
	printf("<p>%s\n</p>", buf1);
	status = system(buf1);
	printf("<p>status(echo)=%d</p>", status);
}

void fcgiGetFaceModelList(void)
{
	DIR *dir;
	struct dirent *ent;
	char *FaceModelPath = "/usrdata/eaif/facereco/faces";
	int count = 0;

	printf("Content-type: text/html\r\n\r\n");
	printf("[");
	if ((dir = opendir(FaceModelPath)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (count <= 1) {
				// omit directory . & ..
			} else if (count == 2) {
				printf("\"%s\"", ent->d_name);
			} else {
				printf(",\"%s\"", ent->d_name);
			}
			count++;
		}
		closedir(dir);
	} else {
		/*   could not open directory */
		perror("");
	}
	printf("]");
}

void fcgiRemoveFile(char *buf)
{
	char cmd[200];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(cmd, "/system/www/cgi-bin/removeFile.sh \"%s\"", buf);
	status = system(cmd);
	printf("{\"rval\":%d}", status);
}

void fcgiValidateFaceModel(char *buf)
{
	char cmd[200];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(cmd, "/system/www/cgi-bin/validateFaceModel.sh \"%s\"", buf);
	status = system(cmd);
	printf("{\"rval\":%d}", status);
}

void fcgiRegisterFaceModel(char *buf)
{
	char cmd[200];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(cmd, "/system/www/cgi-bin/registerFaceModel.sh \"%s\"", buf);
	status = system(cmd);
	printf("{\"rval\":%d}", status);
}

void fcgiUnregisterFaceModel(char *buf)
{
	char cmd[200];
	int status;
	printf("Content-type: text/html\r\n\r\n");
	sprintf(cmd, "/system/www/cgi-bin/unregisterFaceModel.sh \"%s\"", buf);
	status = system(cmd);
	printf("{\"rval\":%d}", status);
}
void fcgiSetWifi(char *buf)
{
	int exists, exists1;
	int status;
	enum json_type type;
	char cmdstr[200];
	char ssid[100];
	char psk[100];
	json_object *obj_ssid, *obj_psk;
	json_object *jobj;
	cmdstr[0] = 0;
	printf("Content-type: text/html\r\n\r\n");
	jobj = json_tokener_parse(buf);
	exists = json_object_object_get_ex(jobj, "ssid", &obj_ssid);
	if (exists) {
		type = json_object_get_type(obj_ssid);
		if (type == json_type_string) {
			strncpy(ssid, json_object_get_string(obj_ssid), 100);
			printf("<p>ssid:%s\n</p>", ssid);
			strcat(cmdstr, "/system/www/cgi-bin/setWPA.sh");
			strcat(cmdstr, " '");
			strcat(cmdstr, ssid);
			strcat(cmdstr, "'");
			exists1 = json_object_object_get_ex(jobj, "psk", &obj_psk);
			if (exists1) {
				type = json_object_get_type(obj_psk);

				if (type == json_type_string) {
					strncpy(psk, json_object_get_string(obj_psk), 100);
					printf("<p>psk:%s\n</p>", psk);
					strcat(cmdstr, " '");
					strcat(cmdstr, psk);
					strcat(cmdstr, "'");
					//printf("{\"rval\":0}");
				}
			}
		}
	}
	json_object_put(jobj);
	printf("<p>%s\n</p>", cmdstr);
	status = system(cmdstr); // Edit /etc/wpa_supplicant.conf
	printf("<p>status=%d\n</p>", status);
	status = system("/system/script/wifi_on.sh /etc/wpa_supplicant.conf");
	printf("<p>status(echo)=%d\n</p>", status);
}

void fcgiDisconnectWifi()
{
	printf("Content-type: text/html\r\n\r\n");
	system("/system/script/wifi_off.sh");
}

void fcgiGetSSID()
{
	char *tmp;
	int i = 0, j = 0;
	char str[128];
	char buffer[128];
	char SSID[128][128];
	char list[1024];
	char number[8];
	list[0] = 0;
	printf("Content-type: text/html\r\n\r\n");
	system("iw dev wlan0 scan | grep SSID: > /tmp/SSID");
	FILE *fp = fopen("/tmp/SSID", "r");
	while (fgets(buffer, sizeof(buffer), fp)) {
		tmp = strstr(buffer, "SSID:");
		sscanf(tmp, "%*s %[^\t\n]", str);
		memset(buffer, sizeof(buffer), 0);
		strcpy(SSID[i], str);
		i++;
	}
	fclose(fp);
	strcat(list, "{\"ssidnumber\":");
	sprintf(number, "%d", i);
	strcat(list, number);
	strcat(list, ",\"SSID\":[");
	for (j = 0; j < i; j++) {
		strcat(list, "\"");
		strcat(list, SSID[j]);
		if (j < i - 1) {
			strcat(list, "\",");
		} else {
			strcat(list, "\"]}");
		}
	}
	printf(list);
}

void fcgiGetWPASSID()
{
	char *tmp;
	char str[128];
	char ssid[128];
	char psk[128];
	printf("Content-type: text/html\r\n\r\n");
	FILE *fp = fopen("/etc/wpa_supplicant.conf", "r");
	while (fgets(str, sizeof(str), fp)) {
		if ((tmp = strstr(str, "ssid"))) {
			sscanf(tmp, "%*[^\"]\"%[^\"]", ssid);
		}
		if ((tmp = strstr(str, "psk"))) {
			sscanf(tmp, "%*[^\"]\"%[^\"]", psk);
		}
	}
	fclose(fp);
	printf("{\"ssid\":\"%s\",\"psk\":\"%s\"}", ssid, psk);
}

void fcgiGetWlanInfo()
{
	char *tmp;
	char str[128];
	char str2[128];
	char str3[128];
	char ssid[128];
	char straddr[128];
	char buffer[128];
	printf("Content-type: text/html\r\n\r\n");
	FILE *fp = popen("iwgetid", "r");
	while (fgets(str, sizeof(str), fp)) {
		if ((tmp = strstr(str, "SSID"))) {
			sscanf(tmp, "%*[^\"]\"%[^\"]", ssid);
		}
	}
	fclose(fp);
	FILE *fp2 = popen("ifconfig wlan0", "r");
	while (fgets(buffer, sizeof(buffer), fp2)) {
		if ((tmp = strstr(buffer, "addr:"))) {
			sscanf(tmp, "%s%s%s", str, str2, str3);
			if (strlen(str) > 14) {
				sscanf(str, "%*[^:]:%s", straddr);
			}
		}
	}
	fclose(fp2);
	printf("{\"ssid\":\"%s\",\"wifiIP\":\"%s\"}", ssid, straddr);
}

void fcgiPackNginxLog(void)
{
	int status = 0;
	printf("Content-type: text/html\r\n\r\n");
	system("/system/www/cgi-bin/packNginxLog.sh");
	printf("{\"rval\":%d}", status);
}

