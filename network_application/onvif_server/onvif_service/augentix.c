#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "httpda.h"

#include <linux/sockios.h>
#include <linux/ethtool.h>

#include "augentix.h"
#include <sys/param.h>
#include <net/route.h>

#include <ctype.h>

const char authrealm[128] = { "augentix.com" };
char tmpOSDString[128] = { 0 };
unsigned int numOfStreams = 0;
unsigned int numOfMainRes = 0;
unsigned int numOfsecondRes = 0;
unsigned int numOfthirdRes = 0;

AGTX_RES_OPTION_S devResOptions;
AGTX_VENC_OPTION_S devVencOptions;

int SYS_Gethostname(char *str)
{
	return gethostname(str, HOSTNAME_LEN);
}

int SYS_Getgateway(unsigned int *p)
{
	FILE *fp;
	char buf[256];
	char iface[16];
	unsigned long dest_addr, gate_addr;

	*p = INADDR_NONE;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL) {
		return -1;
	}

	/* Skip title line */
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp)) {
		if (sscanf(buf, "%s\t%lX\t%lX", iface, &dest_addr, &gate_addr) != 3 || dest_addr != 0)
			continue;
		*p = gate_addr;
		break;
	}

	fclose(fp);

	return 0;
}

char *SYS_Getipaddr(char *name, char *str)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFADDR, &ifr)) {
		printf("Get SIOCGIFADDR fail\n");
		close(fd);
		return str;
	}

	close(fd);

	/* display result */
	//printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	strcpy(str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	return str;
}

char *SYS_Getmacaddr(char *name, char *str)
{
	int fd;
	char *mac;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr)) {
		printf("Get SIOCGIFHWADDR fail\n");
		close(fd);
		return str;
	}

	close(fd);

	mac = ((struct sockaddr *)&ifr.ifr_addr)->sa_data;
	//sprintf(str,"%x:%x:%x:%x:%x:%x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	memcpy(str, mac, 6);

	return str;
}

int SYS_Getadminsettings(char *name, unsigned char *autoneg, unsigned short *speed, unsigned char *duplex)
{
	int fd;
	struct ethtool_cmd cmd = { 0 };
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	ifr.ifr_data = (void *)&cmd;
	cmd.cmd = ETHTOOL_GSET; /* "Get settings" */
	if (ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
		/* Unknown */
		printf("Get SIOCETHTOOL fail %d(%m)\n", errno);
		speed = 0;
		duplex = 0;
		autoneg = 0;
		close(fd);
	} else {
		*autoneg = cmd.autoneg; //AutoNegotiation
		*speed = ethtool_cmd_speed(&cmd);
		*duplex = cmd.duplex;
	}

	close(fd);

	return 0;
}

//--- Set
int SYS_Sethostname(char *str)
{
	//return sethostname(str, HOSTNAME_LEN);
	return sethostname(str, strlen(str));
}

int SYS_NumNetworkIntf()
{
	struct if_nameindex *if_nidxs, *intf;
	int count;

	if_nidxs = if_nameindex(); //syscall to get interface list
	count = 0;
	if (if_nidxs != NULL) {
		for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
			printf("%s\n", intf->if_name);
			count++;
		}
	}
	return (count - 1); //remove loopback interface
}
char *SYS_NetworkInfName(int idx)
{
	struct if_nameindex *if_nidxs, *intf;
	int count;

	if_nidxs = if_nameindex(); //syscall to get interface list
	count = 0;
	if (if_nidxs != NULL) {
		for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
			//fprintf(stderr,"%s\n", intf->if_name);
			if (idx == count) {
				return intf->if_name;
			}
			count++;
		}
	}
	return NULL;
}
static unsigned int prefix2mask(int prefix)
{
	struct in_addr mask;
	memset(&mask, 0, sizeof(mask));
	if (prefix) {
		return htonl(~((1 << (32 - prefix)) - 1));
	} else {
		return htonl(0);
	}
}
char *SYS_NetworkPrefixToMask(int preFix)
{
	//prefix to mask = 255.255.255.255 4 octates
	// num = (prefix)%8 //number of 255's
	// rem = (prefix -(num*8)) // remaining prefix
	static char netMask[16] = { 0 };
	int num, remMask;
	int count = 1;
	num = preFix % 8;
	remMask = (preFix - (num * 8));
	for (int i = 0; i < num; i++) {
		sprintf(netMask, "255.");
		count += 1;
	}
	sprintf(netMask, "%u", prefix2mask(remMask));
	while (count < 5) {
		sprintf(netMask, ".0");
	}
	return netMask;
}
int SYS_SetDefaultGateway(char *gwAddr)
{
	int sockfd;
	struct rtentry route;
	struct sockaddr_in *addr;
	int err = 0;
	if (((sockfd = socket(AF_INET, SOCK_DGRAM, 0))) < 0) {
		fprintf(stderr, "set default gw: socket ERROR \n ");
		return -1;
	}
	memset(&route, 0, sizeof(route));
	addr = (struct sockaddr_in *)&route.rt_gateway;
	addr->sin_family = AF_INET;
	//addr->sin_addr.s_addr = inet_addr("192.168.2.1");
	addr->sin_addr.s_addr = inet_addr(gwAddr);
	addr = (struct sockaddr_in *)&route.rt_dst;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("0.0.0.0");
	addr = (struct sockaddr_in *)&route.rt_genmask;
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("0.0.0.0");
	route.rt_flags = RTF_UP | RTF_GATEWAY;
	route.rt_metric = 0;
	if ((err = ioctl(sockfd, SIOCADDRT, &route)) != 0) {
		perror("SetDef GW: SIOCADDRT failed \n");
		return -1;
	}
	return 0;
}

static int bit_count(unsigned int i)
{
	int c = 0;
	unsigned int seen_one = 0;

	while (i > 0) {
		if (i & 1) {
			seen_one = 1;
			c++;
		} else {
			if (seen_one) {
				return -1;
			}
		}
		i >>= 1;
	}
	return c;
}
static int mask2prefix(struct in_addr mask)
{
	return bit_count(ntohl(mask.s_addr));
}
static int ip_to_mask_to_int(const char *prefix)
{
	int ret;
	struct in_addr in;
	ret = inet_pton(AF_INET, prefix, &in);
	if (ret == 0)
		return -1;

	return mask2prefix(in);
}
int SYS_GetIPNetmask(char *name)
{
	int fd;
	struct ifreq ifr;
	int i = 0;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFNETMASK, &ifr)) {
		printf("Get SIOCGIFADDR fail\n");
		close(fd);
		return -1;
	}
	close(fd);

	/* display result */
	//printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	//inet_pton(AF_INET,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), &n);
	i = ip_to_mask_to_int(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	//printf(">>>>> Netmask %d\n", i);

	//while (n > 0) {
	//	n = n >> 1;
	//	i++;
	//}
	return i;
}
//  CC and Json
void aux_delay(float num_sec)
{
	//convert time to millisec
	int millSec = 1000 * num_sec;
	//start time
	clock_t startTime = clock();
	//loop till required time
	while (clock() < (startTime + millSec))
		;
}
// Read the response from Cc // TODO Improvise
int getCCReturn(int fd, char *str)
{
	int ret = 0;
	while (1) {
		//if (recv(cmdTransFD,str,256,0) <= 0) {
		if ((ret = read(fd, str, JSON_STR_LEN)) <= 0) {
			printf("read error %d(%m)\n", errno);
			return -1;
		} else {
			//fprintf(stderr, "CC Response: %s\n", str);
			return 0;
		}
	}
	return -1;
}

int aux_init_venc_res(void)
{
	memset(&devResOptions, 0, sizeof(AGTX_CMD_RES_OPTION));
	memset(&devVencOptions, 0, sizeof(AGTX_CMD_VENC_OPTION));

	if (aux_get_cc_config(AGTX_CMD_RES_OPTION, (void *)&devResOptions) < 0) {
		return -1;
	}

	if (aux_get_cc_config(AGTX_CMD_VENC_OPTION, (void *)&devVencOptions) < 0) {
		return -1;
	}
	return 0;
}

AGTX_RES_OPTION_S *aux_get_res(void)
{
	return &devResOptions;
}

AGTX_VENC_OPTION_S *aux_get_venc(void)
{
	return &devVencOptions;
}

//validate the cc read string is in Json format
int aux_json_validation(char *buffer, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;
	int rval = 0;

	if (strLen > 0) {
		bzero(json_buf, 0);
		strcpy(json_buf, (char *)buffer);
		json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	} else {
		json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_continue) {
		json_obj = json_tokener_parse_ex(tok, " ", -1);
		if (json_tokener_get_error(tok) != json_tokener_success)
			json_obj = NULL;
	} else if (jerr != json_tokener_success) {
		fprintf(stderr, " JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		rval = -1;
	} else if (jerr == json_tokener_success) {
		rval = 0;
	}
	if (json_obj != NULL)
		json_object_put(json_obj); //Decrement the ref count and free if it reaches zero

	json_tokener_free(tok);
	return rval;
}
//JSON: Get int val for given Key
int aux_json_get_int(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	int rval;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);

	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
			//fprintf(stderr,"[%s : %d] \n",dKey, rval);
		} else {
			rval = -99; //since -ve return  are possible we select -99 to avoid conflict with spec
		}
	} else { //the key may not exist
		rval = -99;
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);

	json_tokener_free(tok);
	return rval;
}
//JSON: Get float/double  val for given Key
double aux_json_get_double(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	double rval = 0;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_double) {
			rval = json_object_get_double(tmp_obj);
			//fprintf(stderr," [%s : %d] \n",dKey, rval);
		} else {
			rval = -99.0; //since -ve return  are possible we select -99 to avoid conflict with spec
		}
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);

	json_tokener_free(tok);
	return rval;
}
//JSON: return  String for a given key  // NOTE: Caller need to free(retStr)
char *aux_json_get_string(char *buffer, char *dKey, int strLen, char *retStr)
{
	char json_buf[JSON_STR_LEN] = { 0 };
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	char rval[128] = { 0 };

	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);

	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_string) {
			retStr = malloc(128);
			sprintf(rval, (char *)json_object_get_string(tmp_obj));
			strcpy(retStr, rval);

			if (json_obj != NULL)
				json_object_put(json_obj);
			if (tmp_obj != NULL)
				json_object_put(tmp_obj);

			json_tokener_free(tok);
			return retStr;
			//return (char *)json_object_get_string(tmp_obj);
		} else {
			if (json_obj != NULL)
				json_object_put(json_obj);
			if (tmp_obj != NULL)
				json_object_put(tmp_obj);

			json_tokener_free(tok);
			return NULL;
		}
	}

	if (json_obj != NULL)
		json_object_put(json_obj);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);

	json_tokener_free(tok);
	return NULL;
}
//JSON: Get Bool for a given key
int aux_json_get_bool(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	int rval;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);

	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_boolean) {
			if (json_object_get_boolean(tmp_obj)) {
				if (json_obj != NULL)
					json_object_put(json_obj);
				if (tmp_obj != NULL)
					json_object_put(tmp_obj);

				json_tokener_free(tok);
				return 1; //true
			} else {
				if (json_obj != NULL)
					json_object_put(json_obj);
				if (tmp_obj != NULL)
					json_object_put(tmp_obj);

				json_tokener_free(tok);
				return 0; //false
			}
			//fprintf(stderr," [%s : %d] \n",dKey, rval);
		} else {
			rval = -99.0; //since -ve return  are possible we select -99 to avoid conflict with spec
		}
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);

	json_tokener_free(tok);
	return rval;
}
//Video  TODO : find a better function to get dev, channel, resol, bitrate, gop, encoder related params.
int aux_json_get_videores(char *ccRetStr, char *dKey, int strLen, int devNum)
{
	return 0;
}
// {'a';1,'arrKey':[{'subKey': 123, 'subKey2': 1123},{'subKey': 123, 'subKey2': 1123}] }
int aux_json_get_from_array(char *buffer, char *arrKey, char *subKey, int arrIdx, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj = NULL, *tmp_obj = NULL;
	json_object *json_obj1 = NULL, *tmp_obj1 = NULL;
	struct json_tokener *tok = json_tokener_new();
	//struct json_tokener *tok1 = json_tokener_new();
	int rval;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, arrKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
			//fprintf(stderr,"[%s : %d] \n",arrKey, rval);
		} else if (json_object_get_type(tmp_obj) == json_type_array) {
			rval = json_object_array_length(tmp_obj);
			//printf("req key is a array  of length %d : type %d \n", rval, json_object_get_type(tmp_obj));
			//json_obj1 = json_object_get_type(tmp_obj);
			json_obj1 = json_object_array_get_idx(tmp_obj, arrIdx);
			if (json_object_object_get_ex(json_obj1, subKey, &tmp_obj1)) {
				if (json_object_get_type(tmp_obj1) == json_type_int) {
					rval = json_object_get_int(tmp_obj1);
					//fprintf(stderr,">>>>>>>>>>>>>>>>>>>   [%s : %d] \n",subKey, rval);
				} else {
					rval = -99;
				}
			} else {
				rval = -99;
			}
		} else {
			rval = -99;
		}
	} else { //the key may not exist
		rval = -99;
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (json_obj1 != NULL)
		json_object_put(json_obj1);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);
	if (tmp_obj1 != NULL)
		json_object_put(tmp_obj1);

	json_tokener_free(tok);
	return rval;
}

char *aux_json_get_str_from_array(char *buffer, char *arrKey, char *subKey, int arrIdx, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj = NULL, *tmp_obj = NULL;
	json_object *json_obj1 = NULL, *tmp_obj1 = NULL;
	struct json_tokener *tok = json_tokener_new();
	//struct json_tokener *tok1 = json_tokener_new();
	char *retStr = NULL;
	char rval[128] = { 0 };

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, arrKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_string) {
			retStr = malloc(128);
			sprintf(rval, (char *)json_object_get_string(tmp_obj));
			strcpy(retStr, rval);
			//fprintf(stderr,"[%s : %d] \n",arrKey, rval);
		} else if (json_object_get_type(tmp_obj) == json_type_array) {
			//printf("req key is a array  of length %d : type %d \n", array_len, json_object_get_type(tmp_obj));
			//json_obj1 = json_object_get_type(tmp_obj);
			json_obj1 = json_object_array_get_idx(tmp_obj, arrIdx);
			if (json_object_object_get_ex(json_obj1, subKey, &tmp_obj1)) {
				if (json_object_get_type(tmp_obj1) == json_type_string) {
					retStr = malloc(128);
					sprintf(rval, (char *)json_object_get_string(tmp_obj1));
					strcpy(retStr, rval);
					//fprintf(stderr,">>>>>>>>>>>>>>>>>>>   [%s : %d] \n",subKey, rval);
				} else {
					if (json_obj != NULL)
						json_object_put(json_obj);
					if (json_obj1 != NULL)
						json_object_put(json_obj1);
					if (tmp_obj != NULL)
						json_object_put(tmp_obj);
					if (tmp_obj1 != NULL)
						json_object_put(tmp_obj1);

					json_tokener_free(tok);
					return NULL;
				}
			} else {
				if (json_obj != NULL)
					json_object_put(json_obj);
				if (json_obj1 != NULL)
					json_object_put(json_obj1);
				if (tmp_obj != NULL)
					json_object_put(tmp_obj);
				if (tmp_obj1 != NULL)
					json_object_put(tmp_obj1);

				json_tokener_free(tok);
				return NULL;
			}
		} else {
			if (json_obj != NULL)
				json_object_put(json_obj);
			if (json_obj1 != NULL)
				json_object_put(json_obj1);
			if (tmp_obj != NULL)
				json_object_put(tmp_obj);
			if (tmp_obj1 != NULL)
				json_object_put(tmp_obj1);

			json_tokener_free(tok);
			return NULL;
		}
	} else { //the key may not exist
		if (json_obj != NULL)
			json_object_put(json_obj);
		if (json_obj1 != NULL)
			json_object_put(json_obj1);
		if (tmp_obj != NULL)
			json_object_put(tmp_obj);
		if (tmp_obj1 != NULL)
			json_object_put(tmp_obj1);

		json_tokener_free(tok);
		return NULL;
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (json_obj1 != NULL)
		json_object_put(json_obj1);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);
	if (tmp_obj1 != NULL)
		json_object_put(tmp_obj1);

	json_tokener_free(tok);
	return retStr;
}

//OSD
// aux_json_osd_get_key : 1. if request key is int returns the value
// 						  2. if request key is string returns -9
int aux_json_osd_get_key(char *buffer /*jsonCmd*/, char *subKey /*subKey*/, int regionIdx /*StreamIdx*/, int osdIdx,
                         char *Rval)
{
	char json_buf[4096] = { 0 };
	json_object *json_obj = NULL, *tmp_obj = NULL;
	json_object *json_obj1 = NULL, *tmp_obj1 = NULL;
	struct json_tokener *tok = json_tokener_new();
	//struct json_tokener *tok1 = json_tokener_new();
	int rval = 0, strLen;
	//char *resultStr;
	strcpy(json_buf, buffer);
	strLen = strlen(json_buf);
	//tokenize the string
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	//First Key(strm) : val
	if (json_object_object_get_ex(json_obj, "strm", &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_array) {
			rval = json_object_array_length(tmp_obj);

			//Get the First array element (should be json obj)
			json_obj1 = json_object_array_get_idx(
			        tmp_obj, regionIdx); //Gets the first array element of strm json object
			//json_obj1 = json_object_array_get_idx(tmp_obj,0); //Gets the first array element of strm json object
			if (json_object_object_get_ex(
			            json_obj1, "region",
			            &tmp_obj1)) { //in tmp_obj1 key is region with val as array of jsonObjs
				if (json_object_get_type(tmp_obj1) == json_type_array) {
					rval = json_object_array_length(tmp_obj1);
					//fprintf(stderr,"%d:::[Found 'region' array Object] of length = %d \n",__LINE__,rval);//,subKey, resultStr);
					for (int i = 0; i < rval; i++) {
						//parse the json object array as they all have same key: value types
						if (i == osdIdx) {
							json_object *pval = NULL;
							if (json_object_object_get_ex(
							            json_object_array_get_idx(tmp_obj1, i), subKey,
							            &pval)) {
								if (json_object_get_type(pval) == json_type_string) {
									strcpy(Rval,
									       (char *)json_object_get_string(pval));
									if (pval != NULL)
										json_object_put(pval);

									rval = -9;
								} else if (json_object_get_type(pval) ==
								           json_type_int) {
									rval = json_object_get_int(pval);
									if (pval != NULL)
										json_object_put(pval);

									//fprintf(stderr,":::Found val in subjson obj idx <int> :%s = [%d] \n",subKey,rval );
									//Rval = (char *) malloc(4);
									strcpy(tmpOSDString, "null");
									//retInt = 0;
								} else {
									rval = -99;
								}
							}
						}
					}
				}
			}
		}
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (json_obj1 != NULL)
		json_object_put(json_obj1);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);
	if (tmp_obj1 != NULL)
		json_object_put(tmp_obj1);

	//	free(Rval);

	json_tokener_free(tok);
	return rval;
}

int aux_parse_cc_config(int cmdId, void *data, struct json_object *json_obj)
{
	if (cmdId == AGTX_CMD_VIDEO_STRM_CONF) {
		parse_video_strm_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VIDEO_DEV_CONF) {
		parse_video_dev_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_MD_CONF) {
		parse_iva_md_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_TD_CONF) {
		parse_iva_td_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_AUDIO_CONF) {
		parse_audio_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_OSD_CONF) {
		parse_osd_conf(data, json_obj);
		// } else if (cmdId == AGTX_CMD_PRODUCT_OPTION) {
		// 	//useless
		// 	parse_product_option(data, json_obj);
	} else if (cmdId == AGTX_CMD_RES_OPTION) {
		parse_res_option(data, json_obj);
	} else if (cmdId == AGTX_CMD_VENC_OPTION) {
		parse_venc_option(data, json_obj);
	} else if (cmdId == AGTX_CMD_IMG_PREF) {
		parse_img_pref(data, json_obj);
	} else {
		ONVIF_TRACE("Unknown cmdId %d\n", cmdId);
	}

	return 0;
}

int aux_comp_cc_config(int cmdId, void *data, struct json_object *json_obj)
{
	if (cmdId == AGTX_CMD_VIDEO_STRM_CONF) {
		comp_video_strm_conf(json_obj, data);
		//	} else if (cmdId == AGTX_CMD_VIDEO_DEV_CONF) {
		//		parse_video_dev_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_MD_CONF) {
		comp_iva_md_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_TD_CONF) {
		comp_iva_td_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_OSD_CONF) {
		comp_osd_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_AUDIO_CONF) {
		comp_audio_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_IMG_PREF) {
		comp_img_pref(json_obj, data);
	} else {
		ONVIF_TRACE("Unknown cmdId %d\n", cmdId);
	}

	return 0;
}

int aux_get_cc_config(int cmdId, void *data)
{
	int fd = cmdTransFD;
	int len = 0;
	int rval = 0;
	int jsonRetLen = 0;
	char json_buf[JSON_STR_LEN] = { 0 };
	char ccRetStr[JSON_STR_LEN] = { 0 };
	enum json_tokener_error jerr;
	struct json_object *json_obj = NULL;
	struct json_tokener *tok = NULL;

	sprintf(json_buf, "{'master_id': %d, 'cmd_id': %d ,'cmd_type': 'get'}\n", master_id, cmdId);

	//CC Send
	//	ONVIF_TRACE("Get CC command %s \n", json_buf);
	if (write(fd, json_buf, strlen(json_buf)) < 0) {
		ONVIF_TRACE("failed to send message to command Translator \n");
		return -1;
	} else {
		bzero(ccRetStr, JSON_STR_LEN);
	}

	//CC recv
	if (getCCReturn(fd, ccRetStr) != 0) {
		ONVIF_TRACE("Get cc config failed no data\n");
		return -1;
	}

	tok = json_tokener_new();

	len = strlen(ccRetStr);
	if (len > 0) {
		bzero(json_buf, 0);
		strcpy(json_buf, (char *)ccRetStr);
		json_obj = json_tokener_parse_ex(tok, json_buf, len);
	} else {
		json_obj = json_tokener_parse_ex(tok, json_buf, len);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_success) {
		if (aux_parse_cc_config(cmdId, data, json_obj) < 0) {
			// Need to do something?
		}
		if (json_obj != NULL) {
			json_object_put(json_obj); //Decrement the ref count and free if it reaches zero
		}

		//cc check return val to process furthe
		rval = aux_json_get_int(ccRetStr, "rval", strlen(ccRetStr));
		if (rval < 0) {
			ONVIF_TRACE(" CC response: FAIL  Return = %d \n", rval);
			jsonRetLen = -1;
		} else { // cc return success
			jsonRetLen = strlen(ccRetStr);
		}
	} else {
		ONVIF_TRACE(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		ONVIF_TRACE(" CC response: Invalid Json Str : rval = %d \n", rval);
		jsonRetLen = -1;
	}

	json_tokener_free(tok);

	return jsonRetLen;
}

int aux_set_cc_config(int cmdId, void *data)
{
	int fd = cmdTransFD;
	char jsonCmd[JSON_STR_LEN] = { 0 };
	char ccRetStr[JSON_STR_LEN] = { 0 };
	struct json_object *json_obj = NULL;
	struct json_object *tmp_obj = NULL;
	json_obj = json_object_new_object();
	if (!json_obj) {
		ONVIF_TRACE("Cannot create object\n");
		return -1;
	}
	if (aux_comp_cc_config(cmdId, data, json_obj) < 0) {
		// Need to do something?
	}

	tmp_obj = json_object_new_int(cmdId);
	if (tmp_obj) {
		json_object_object_add(json_obj, "cmd_id", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cmd_id");
	}
	tmp_obj = json_object_new_int(master_id);
	if (tmp_obj) {
		json_object_object_add(json_obj, "master_id", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "master_id");
	}
	tmp_obj = json_object_new_string("set");
	if (tmp_obj) {
		json_object_object_add(json_obj, "cmd_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cmd_type");
	}

	sprintf(jsonCmd, "%s", json_object_to_json_string(json_obj));
	printf("jsonCmd %s\n", jsonCmd);
	if (json_obj) {
		json_object_put(json_obj);
	}

	if (write(fd, jsonCmd, strlen(jsonCmd)) < 0) {
		fprintf(stderr, "failed to send message to command Translator \n");
	}

	if (getCCReturn(fd, ccRetStr) != 0) {
		printf("ccRetStr %s\n", ccRetStr);
		return -1;
	}

	return 0;
}

int AG_register_to_cc(int sockfd)
{
	int ret;
	char buf[128] = { 0 };
	char reg_buf[128] = { 0 };
	char ret_cmd[128] = { 0 };

	sprintf(reg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"ONVIF\"}",
	        AGTX_CMD_REG_CLIENT);
	sprintf(ret_cmd, "{ \"master_id\": 0, \"cmd_id\": %d, \"cmd_type\": \"reply\", \"rval\": 0 }",
	        AGTX_CMD_REG_CLIENT);

	/*Send register information*/
	if (write(sockfd, &reg_buf, strlen(reg_buf)) < 0) {
		ONVIF_TRACE("write socket error %d(%m)\n", errno);
		return -1;
	}

	ret = read(sockfd, buf, strlen(ret_cmd));
	if (ret != (int)strlen(ret_cmd)) {
		ONVIF_TRACE("read socket error %d(%m)\n", errno);
		return -1;
	}

	if (strncmp(buf, ret_cmd, strlen(ret_cmd))) {
		ONVIF_TRACE("Registere to CC Failed %s\n", buf);
		return -1;
	}

	/*start session*/
	sprintf(reg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\"}", AGTX_CMD_SESS_START);

	if (write(sockfd, &reg_buf, strlen(reg_buf)) < 0) {
		ONVIF_TRACE("write socket error %d(%m)\n", errno);
		return -1;
	}

	memset(buf, 0x00, sizeof(buf));

	ret = read(sockfd, buf, sizeof(ret_cmd));
	if (ret <= 0) {
		ONVIF_TRACE("read socket error %d(%m)\n", errno);
		return -1;
	}

	if (aux_json_get_int(buf, "rval", strlen(buf)) < 0) {
		ONVIF_TRACE("Register to CC Failed %s\n", buf);
		return -1;
	} else {
		if (strstr(buf, "master_id")) {
			master_id = aux_json_get_int(buf, "master_id", strlen(buf));
			printf("master_id %d\n", master_id);
		} else {
			ONVIF_TRACE("Register to CC Failed %s\n", buf);
			return -1;
		}
	}

	return 0;
}

int AG_connect_to_cc()
{
	int sockfd = 0;
	int servlen;
	struct sockaddr_un serv_addr;
	struct timeval tv;
	tv.tv_sec = 10; // 30 Secs Timeout
	tv.tv_usec = 0; // Not init'ing this can cause strange errors

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, CC_SOCKET_NAME);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		ONVIF_TRACE("Create sockfd failed %d(%m)\n", errno);
		return -1;
	}

	//Set sockopts for time out
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval));

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		ONVIF_TRACE("Connecting to server failed %d(%m)\n", errno);
		close(sockfd);
		return -1;
	}

	AG_register_to_cc(sockfd);

	return sockfd;
}

void *aux_onvif_malloc(struct soap *soap, int size)
{
	char *ptr = NULL;

	ptr = soap_malloc(soap, size);
	if (ptr) {
		memset(ptr, 0x00, size);
	} else {
		ONVIF_TRACE("soap_malloc failed No memory\n");
		return NULL;
	}

	return ptr;
}

int aux_get_user_pwd(char user[MAX_USER][MAX_USER_LEN], char pwd[MAX_USER][MAX_USER_LEN])
{
/*Need to change %31s when length changed*/
#if (31 + 1) != MAX_USER_LEN
#error
#endif
	int i = 0;
	int cnt = 0;
	FILE *fp = NULL;

	fp = fopen(USER_PATH, "r+");
	if (fp == NULL) {
		ONVIF_TRACE("%s not found, create default file\n", USER_PATH);
		fp = fopen(USER_PATH, "wb");
		if (fp == NULL) {
			ONVIF_TRACE("Create %s failed\n", USER_PATH);
			return -1;
		}
		/*No user file found, Create default file*/
		fprintf(fp, "user_count=0\n");
		fflush(fp);
		fseek(fp, 0, SEEK_SET);
	}

	fscanf(fp, "user_count=%d\n", &cnt);

	if (cnt > MAX_USER) {
		cnt = MAX_USER;
	}

	for (i = 0; i < cnt; i++) {
		//		fscanf(fp, "%31s %31s\n", user[i], pwd[i]);
		fscanf(fp, "%31s", user[i]);
		fseek(fp, 1, SEEK_CUR); /*skip white space*/
		fscanf(fp, "%31[^\n]\n", pwd[i]);
		//		printf("cnt %d user %s pwd %s len %d\n", cnt, user[i], pwd[i], strlen(pwd[i]));
	}

	fclose(fp);

	return cnt;
}

int aux_set_user_pwd(char user[MAX_USER][MAX_USER_LEN], char pwd[MAX_USER][MAX_USER_LEN])
{
	int i = 0;
	int cnt = 0;
	FILE *fp = NULL;

	fp = fopen(USER_PATH, "wb");
	if (fp == NULL) {
		ONVIF_TRACE("open %s failed\n", USER_PATH);
		return 0;
	}

	/*calculate how many users*/
	for (i = 0; i < MAX_USER; i++) {
		if (user[i][0]) {
			cnt++;
		}
	}

	fprintf(fp, "user_count=%d\n", cnt);

	if (cnt > MAX_USER) {
		cnt = MAX_USER;
	}

	for (i = 0; i < MAX_USER; i++) {
		if (!user[i][0]) {
			continue;
		}
		fprintf(fp, "%s %s\n", user[i], pwd[i]);
		printf("cnt %d user %s pwd %s\n", cnt, user[i], pwd[i]);
	}

	fflush(fp);

	fclose(fp);

	return cnt;
}

int aux_auth_fault(struct soap *soap)
{
	soap_sender_fault_subcode(soap, "ter:NotAuthorized", "Sender not authorized",
	                          "The action requested requires authorization and the sender is not authorized");
	return 400;
}

int aux_check_auth(struct soap *soap)
{
	int i = 0;
	int cnt = 0;
	int len = 0;
	int offset = 0;
	char user[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char pwd[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char tmp[512] = { 0 };
	char str[512] = { 0 };
	struct _wsse__UsernameToken *token = NULL;

	cnt = aux_get_user_pwd(user, pwd);

	if (!cnt) { /*No authorization*/
		return SOAP_OK;
	}

	if (soap->header && soap->header->wsse__Security && soap->header->wsse__Security->UsernameToken) {
		token = soap->header->wsse__Security->UsernameToken;

		/*Must include nonce and creation timestamps*/
		if (!token->Username || !token->Nonce || !token->Nonce->__item || !token->wsu__Created ||
		    !token->Password || !token->Password->__item) {
			return SOAP_FAULT;
		}

		//		ONVIF_TRACE("Username %s\n", token->Username);

		for (i = 0; i < cnt; i++) {
			if (!strcmp(user[i], token->Username)) {
				break;
			}
		}

		if (i >= cnt) {
			return SOAP_FAULT;
		}

		//		ONVIF_TRACE("Nonce %s\n", token->Nonce->__item);
		soap_base642s(soap, token->Nonce->__item, str, strlen(token->Nonce->__item), &len);
		memcpy(tmp + offset, str, len);
		offset += len;

		//		ONVIF_TRACE("wsu__Created %s\n", token->wsu__Created);
		len = strlen(token->wsu__Created);
		memcpy(tmp + offset, token->wsu__Created, strlen(token->wsu__Created));
		offset += len;

		len = strlen(pwd[i]);
		memcpy(tmp + offset, pwd[i], len);
		offset += len;
		SHA1_Enc(str, tmp, offset);
#if 0 /*for debug sha1*/
		int i = 0;
		printf("sha1 : ");
		for (i = 0; i < 20; i++) {
			printf("%02x", str[i]);
		}
		printf("\n", str[i]);
#endif
		soap_s2base64(soap, (const unsigned char *)str, tmp, 20);
		//		ONVIF_TRACE("Digest Password : %s\n", tmp);

		if (strcmp(tmp, token->Password->__item)) {
			return SOAP_FAULT;
		}
	} else {
		return SOAP_FAULT;
	}

	return SOAP_OK;
}

int aux_check_http_auth(struct soap *soap)
{
	int i = 0;
	int cnt = 0;
	char user[MAX_USER][MAX_USER_LEN] = { { 0 } };
	char pwd[MAX_USER][MAX_USER_LEN] = { { 0 } };

	cnt = aux_get_user_pwd(user, pwd);

	if (cnt <= 0) {
		return SOAP_OK;
	}

	if (soap->userid && soap->passwd) { // Basic authentication
		for (i = 0; i < cnt; i++) {
			if (!strcmp(soap->userid, user[i]) && !strcmp(soap->passwd, pwd[i])) {
				// can also check soap->authrealm
				// process request and produce response
				return SOAP_OK;
			}
		}
	} else if (soap->authrealm && soap->userid) { // Digest authentication
		printf("cnt %d\n", cnt);
		for (i = 0; i < cnt; i++) {
			printf("user %s pwd %s\n", user[i], pwd[i]);
			if (!strcmp(soap->authrealm, authrealm) && !strcmp(soap->userid, user[i])) {
				if (!http_da_verify_get(soap, pwd[i])) // HTTP GET DA verification
				{
					// process request and produce response
					return SOAP_OK;
				}
			}
		}
	}

	soap->authrealm = authrealm;
	return 401;
}

int aux_get_iva_setting(struct soap *soap, struct tt__VideoAnalyticsConfiguration **VideoAnalyticsConfiguration)
{
	char *active_cell = NULL;
	int i = 0;
	int td_en = 0;
	int md_sensitivity = 0;
	int td_sensitivity = 0;
	int va_num = 2;
	int va_index = 0;
	int polygon_num = 4;
	int item_num = 0;
	struct tt__CellLayout *layout = NULL;
	struct tt__Config *va_module = NULL;
	struct tt__Config *rule = NULL;
	struct tt__VideoAnalyticsConfiguration *va = NULL;
	struct tt__PolygonConfiguration *polygon = NULL;
	AGTX_IVA_MD_CONF_S md_conf;
	AGTX_IVA_TD_CONF_S td_conf = { 0 };

	/*Get MD config*/
	if (aux_get_cc_config(AGTX_CMD_MD_CONF, &md_conf) < 0) {
		return SOAP_NO_DATA;
	}

	/*Get TD config*/
	if (aux_get_cc_config(AGTX_CMD_TD_CONF, &td_conf) < 0) {
		return SOAP_NO_DATA;
	}

	md_sensitivity = md_conf.sens;
	active_cell = (char *)md_conf.active_cell;
	td_sensitivity = td_conf.sensitivity;
	td_en = td_conf.enabled;

	/*VideoAnalyticsConfiguration*/
	*VideoAnalyticsConfiguration = soap_malloc(soap, sizeof(struct tt__VideoAnalyticsConfiguration));
	va = *VideoAnalyticsConfiguration;
	memset(va, 0x00, sizeof(struct tt__VideoAnalyticsConfiguration));
	va->UseCount = 2;
	va->__size = 1;
	va->token = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va->token, "VideoAnalyticsToken");
	va->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va->Name, "VideoAnalyticsName");
	va->AnalyticsEngineConfiguration = soap_malloc(soap, sizeof(struct tt__AnalyticsEngineConfiguration));
	memset(va->AnalyticsEngineConfiguration, 0x00, sizeof(struct tt__AnalyticsEngineConfiguration));
	va->AnalyticsEngineConfiguration->__sizeAnalyticsModule = va_num;
	va->AnalyticsEngineConfiguration->AnalyticsModule_ = soap_malloc(soap, sizeof(struct tt__Config) * va_num);
	memset(va->AnalyticsEngineConfiguration->AnalyticsModule_, 0x00, sizeof(struct tt__Config) * va_num);
	va->RuleEngineConfiguration = soap_malloc(soap, sizeof(struct tt__RuleEngineConfiguration));
	memset(va->RuleEngineConfiguration, 0x00, sizeof(struct tt__RuleEngineConfiguration));
	va->RuleEngineConfiguration->__sizeRule = va_num;
	va->RuleEngineConfiguration->Rule_ = soap_malloc(soap, sizeof(struct tt__Config) * va_num);
	memset(va->RuleEngineConfiguration->Rule_, 0x00, sizeof(struct tt__Config) * va_num);

	/*motion module*/
	va_module = &va->AnalyticsEngineConfiguration->AnalyticsModule_[va_index];
	memset(va_module, 0x00, sizeof(struct tt__Config));
	va_module->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Name, "MyCellMotionModule");
	va_module->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Type, "tt:CellMotionEngine");
	va_module->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(va_module->Parameters, 0x00, sizeof(struct tt__ItemList));
	va_module->Parameters->__sizeElementItem = 1;
	va_module->Parameters->__sizeSimpleItem = 1;
	va_module->Parameters->SimpleItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
	memset(va_module->Parameters->SimpleItem_, 0x00, sizeof(struct _tt__ItemList_SimpleItem));
	va_module->Parameters->SimpleItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->SimpleItem_->Name, "Sensitivity");
	va_module->Parameters->SimpleItem_->Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	sprintf(va_module->Parameters->SimpleItem_->Value, "%d", md_sensitivity);
	va_module->Parameters->ElementItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
	memset(va_module->Parameters->ElementItem_, 0x00, sizeof(struct _tt__ItemList_ElementItem));
	va_module->Parameters->ElementItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->ElementItem_->Name, "Layout");
	va_module->Parameters->ElementItem_->CellLayout = soap_malloc(soap, sizeof(struct tt__CellLayout));
	layout = (struct tt__CellLayout *)va_module->Parameters->ElementItem_->CellLayout;
	memset(layout, 0, sizeof(struct tt__CellLayout));
	layout->Columns = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(layout->Columns, "16"); //22
	layout->Rows = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(layout->Rows, "12"); //15
	layout->Transformation = soap_malloc(soap, sizeof(struct tt__Transformation));
	memset(layout->Transformation, 0, sizeof(struct tt__Transformation));
	layout->Transformation->Translate = soap_malloc(soap, sizeof(struct tt__Vector));
	layout->Transformation->Translate->x = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Translate->x = -1.000000;
	layout->Transformation->Translate->y = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Translate->y = -1.000000;
	layout->Transformation->Scale = soap_malloc(soap, sizeof(struct tt__Vector));
	layout->Transformation->Scale->x = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Scale->x = 0.006250; //0.090909;
	layout->Transformation->Scale->y = soap_malloc(soap, sizeof(float));
	*layout->Transformation->Scale->y = 0.008340; //0.133333;
	va_index++;

	//	/*tamper module*/
	va_module = &va->AnalyticsEngineConfiguration->AnalyticsModule_[va_index];
	memset(va_module, 0x00, sizeof(struct tt__Config));
	va_module->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Name, "MyTamperDetecModule");
	va_module->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Type, "hikxsd:TamperEngine");
	va_module->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(va_module->Parameters, 0x00, sizeof(struct tt__ItemList));
	va_module->Parameters->__sizeElementItem = 1;
	va_module->Parameters->__sizeSimpleItem = 1;
	va_module->Parameters->SimpleItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_SimpleItem));
	memset(va_module->Parameters->SimpleItem_, 0x00, sizeof(struct _tt__ItemList_SimpleItem));
	va_module->Parameters->SimpleItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->SimpleItem_->Name, "Sensitivity");
	va_module->Parameters->SimpleItem_->Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	/*Dislabe TD by td_sensitivity equal zero*/
	sprintf(va_module->Parameters->SimpleItem_->Value, "%d", td_en ? td_sensitivity : 0);
	va_module->Parameters->ElementItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
	memset(va_module->Parameters->ElementItem_, 0x00, sizeof(struct _tt__ItemList_ElementItem));
	va_module->Parameters->__sizeElementItem = 2;
	va_module->Parameters->ElementItem_ =
	        soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem) * va_module->Parameters->__sizeElementItem);
	memset(va_module->Parameters->ElementItem_, 0x00,
	       sizeof(struct _tt__ItemList_ElementItem) * va_module->Parameters->__sizeElementItem);
	va_module->Parameters->ElementItem_[0].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->ElementItem_[0].Name, "Transformation");
	va_module->Parameters->ElementItem_[0].Transformation = soap_malloc(soap, sizeof(struct tt__Transformation));
	memset(va_module->Parameters->ElementItem_[0].Transformation, 0, sizeof(struct tt__Transformation));
	va_module->Parameters->ElementItem_[0].Transformation->Translate = soap_malloc(soap, sizeof(struct tt__Vector));
	va_module->Parameters->ElementItem_[0].Transformation->Translate->x = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Translate->x = -1.000000;
	va_module->Parameters->ElementItem_[0].Transformation->Translate->y = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Translate->y = -1.000000;
	va_module->Parameters->ElementItem_[0].Transformation->Scale = soap_malloc(soap, sizeof(struct tt__Vector));
	va_module->Parameters->ElementItem_[0].Transformation->Scale->x = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Scale->x = 0.002841;
	va_module->Parameters->ElementItem_[0].Transformation->Scale->y = soap_malloc(soap, sizeof(float));
	*va_module->Parameters->ElementItem_[0].Transformation->Scale->y = 0.004167;

	va_module->Parameters->ElementItem_[1].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(va_module->Parameters->ElementItem_[1].Name, "Field");
	va_module->Parameters->ElementItem_[1].PolygonConfiguration =
	        soap_malloc(soap, sizeof(struct tt__PolygonConfiguration));
	polygon = (struct tt__PolygonConfiguration *)va_module->Parameters->ElementItem_[1].PolygonConfiguration;
	memset(polygon, 0, sizeof(struct tt__PolygonConfiguration));
	polygon->Polygon = soap_malloc(soap, sizeof(struct tt__Polygon));
	memset(polygon->Polygon, 0x00, sizeof(struct tt__Polygon));
	polygon->Polygon->__sizePoint = polygon_num;
	polygon->Polygon->Point_ = soap_malloc(soap, sizeof(struct tt__Vector) * polygon_num);
	memset(polygon->Polygon->Point_, 0x00, sizeof(struct tt__Vector) * polygon_num);
	for (i = 0; i < polygon_num; i++) {
		polygon->Polygon->Point_[i].x = soap_malloc(soap, sizeof(float));
		polygon->Polygon->Point_[i].y = soap_malloc(soap, sizeof(float));
		*polygon->Polygon->Point_[i].x = ((i == 2) || (i == 3)) ? 704 : 0;
		*polygon->Polygon->Point_[i].y = ((i == 1) || (i == 2)) ? 480 : 0;
	}

	va_index = 0;
	/*motion rule*/
	rule = &va->RuleEngineConfiguration->Rule_[va_index];
	rule->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Name, "MyMotionDetectorRule");
	rule->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Type, "tt:CellMotionDetector");
	rule->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(rule->Parameters, 0x00, sizeof(struct tt__ItemList));
	item_num = 4;
	rule->Parameters->__sizeSimpleItem = item_num;
	rule->Parameters->SimpleItem_ = soap_malloc(soap, sizeof(struct tt__ItemList) * item_num);
	memset(rule->Parameters->SimpleItem_, 0x00, (sizeof(struct tt__ItemList) * item_num));
	rule->Parameters->SimpleItem_[0].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[0].Name, "MinCount");
	rule->Parameters->SimpleItem_[0].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[0].Value, "5");
	rule->Parameters->SimpleItem_[1].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[1].Name, "AlarmOnDelay");
	rule->Parameters->SimpleItem_[1].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[1].Value, "1000");
	rule->Parameters->SimpleItem_[2].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[2].Name, "AlarmOffDelay");
	rule->Parameters->SimpleItem_[2].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[2].Value, "1000");
	rule->Parameters->SimpleItem_[3].Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[3].Name, "ActiveCells");
	rule->Parameters->SimpleItem_[3].Value = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->SimpleItem_[3].Value, active_cell);
	va_index++;

	/*tamper rule*/
	rule = &va->RuleEngineConfiguration->Rule_[va_index];
	rule->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Name, "MyTamperDetectorRule");
	rule->Type = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Type, "hikxsd:TamperDetector");
	rule->Parameters = soap_malloc(soap, sizeof(struct tt__ItemList));
	memset(rule->Parameters, 0x00, sizeof(struct tt__ItemList));
	rule->Parameters->__sizeElementItem = 1;
	rule->Parameters->ElementItem_ = soap_malloc(soap, sizeof(struct _tt__ItemList_ElementItem));
	memset(rule->Parameters->ElementItem_, 0x00, sizeof(struct _tt__ItemList_ElementItem));
	rule->Parameters->ElementItem_->Name = soap_malloc(soap, sizeof(char) * MAX_STR_LEN);
	strcpy(rule->Parameters->ElementItem_->Name, "Field");
	rule->Parameters->ElementItem_->PolygonConfiguration =
	        soap_malloc(soap, sizeof(struct tt__PolygonConfiguration));
	polygon = (struct tt__PolygonConfiguration *)rule->Parameters->ElementItem_->PolygonConfiguration;
	memset(polygon, 0, sizeof(struct tt__PolygonConfiguration));
	polygon->Polygon = soap_malloc(soap, sizeof(struct tt__Polygon));
	memset(polygon->Polygon, 0x00, sizeof(struct tt__Polygon));
	polygon->Polygon->__sizePoint = polygon_num;
	polygon->Polygon->Point_ = soap_malloc(soap, sizeof(struct tt__Vector) * polygon_num);
	memset(polygon->Polygon->Point_, 0x00, sizeof(struct tt__Vector) * polygon_num);
	for (i = 0; i < polygon_num; i++) {
		polygon->Polygon->Point_[i].x = soap_malloc(soap, sizeof(float));
		polygon->Polygon->Point_[i].y = soap_malloc(soap, sizeof(float));
		*polygon->Polygon->Point_[i].x = 0;
		*polygon->Polygon->Point_[i].y = 0;
	}

	return SOAP_OK;
}

int aux_parse_iso8061_duration(char *str)
{
	int time_designator = 0;
	char *token = NULL;
	char *pre_ptr = NULL;
	char tmp[16] = { 0 };
	char designator[] = { "YMWDTHMS" };
	int duration = 0;

	if (!str || str[0] != 'P') {
		ONVIF_TRACE("It's not duration\n");
		return 0;
	}

	pre_ptr = &str[1];
	token = strpbrk(&str[1], designator);

	while (token != NULL) {
		strncpy(tmp, pre_ptr, token - pre_ptr);
		//		printf("tmp %s size %d\n", tmp, token - pre_ptr);
		if (*token == 'Y') {
			duration += atoi(tmp) * 365 * 24 * 3600;
		} else if (*token == 'W') {
			duration += atoi(tmp) * 7 * 24 * 3600;
		} else if (*token == 'D') {
			duration += atoi(tmp) * 24 * 3600;
		} else if (*token == 'H') {
			duration += atoi(tmp) * 3600;
		} else if (*token == 'M') {
			if (time_designator) {
				duration += atoi(tmp) * 60;
			} else {
				duration += atoi(tmp) * 30 * 24 * 3600;
			}
		} else if (*token == 'S') {
			duration += atoi(tmp);
		} else if (*token == 'T') {
			time_designator = 1;
		}
		//				printf("key %c\n", *token);
		//				printf("%s\n", token);
		pre_ptr = token + 1;
		token = strpbrk(token + 1, designator);
	}

	return duration;
}

int get_fw_env(const char *key, char *val, int buffersize)
{
#define GET_FW_ENV_STR_LEN (128)
	char tmp[GET_FW_ENV_STR_LEN] = { 0 };
	char buf[GET_FW_ENV_STR_LEN] = { 0 };
	char *ptr = NULL;
	FILE *fp = NULL;

	if (!key || buffersize < 1 || buffersize > GET_FW_ENV_STR_LEN) { /* bad user input */
		printf("get_fw_env failed key %p size %d / %d\n", key, buffersize, GET_FW_ENV_STR_LEN);
		return -1;
	}
	sprintf(tmp, "fw_printenv %s", key);
	if ((fp = popen(tmp, "r")) == NULL) { /* error opening pipe */
		return -1;
	}

	if (fgets(buf, GET_FW_ENV_STR_LEN, fp) != NULL) {
		strtok(buf, "\n");
	}

	if (pclose(fp)) { /* error closing pipe */
		return -1;
	}

	ptr = strchr(buf, '=');
	strncpy(val, ptr + 1, buffersize - 1);
	val[buffersize - 1] = '\0';

	return 0;
}

int get_fw_version(const char *file_path, char *val, int buffersize)
{
	FILE *fp = NULL;

	if (!file_path) {
		return -1;
	}

	if ((fp = fopen(file_path, "r")) == NULL) {
		return -1;
	}

	if (fgets(val, buffersize, fp) != NULL) {
		strtok(val, "\n");
	}

	if (fclose(fp)) {
		return -1;
	}

	return 0;
}

int aux_get_device_info(char *key, char *value, int max_size)
{
	//const char *key[NUM_OF_KEYS] = {"manufacturer", "model", "firmware_version", "serial_number", "hardware_id"};
	char fw_ver_path[] = "/etc/sw-version";

	/* Firmware version is stored on file system */
	if (strcmp(key, "firmware_version") == 0) {
		if (get_fw_version(fw_ver_path, value, max_size)) {
			printf("Failed to get file info %s\n", key);
			return -1;
		}
	} else { /* Other info can be accessed via fw_printenv utility */
		if (get_fw_env(key, value, max_size)) {
			printf("Failed to get uboot env %s\n", key);
			return -1;
		}
	}
	//	printf("%s = %s\n", key, value);
	return 0;
}
//------------------------
