#ifndef AUGENTIX_H_
#define AUGENTIX_H_

#include <json.h>
#include "sha1.h"
#include "string.h"
#include <sys/time.h>
#include <sys/un.h>
#include "server.h"
#include "soapStub.h"
#include "stdsoap2.h"
#include "soapH.h"
#include "tz_upd.h"
#include <pthread.h>
#include "agtx_cmd.h"
#include "agtx_common.h"
#include "agtx_iva.h"
#include "agtx_video.h"
#include "agtx_audio.h"
#include "agtx_cmd.h"
#include "agtx_osd.h"
#include "agtx_color_conf.h"
#include "agtx_event_conf.h"
#include "agtx_res_option.h"
#include "agtx_venc_option.h"
#include "cm_video_dev_conf.h"
#include "cm_video_strm_conf.h"
#include "cm_audio_conf.h"
#include "cm_img_pref.h"
#include "cm_iva_td_conf.h"
#include "cm_iva_md_conf.h"
#include "cm_osd_conf.h"
#include "cm_product_option.h"

#define AUDIO_EN 1
#define MAX_STR_LEN 256
#define HOSTNAME_LEN 1024
#define JSON_STR_LEN 8192
#define MAX_USER 4
#define MAX_USER_LEN 32

#define USER_PATH "/usrdata/active_setting/onvif_user"
#define SNAPSHOT_URI "snapshot/"

#define CC_SOCKET_NAME "/tmp/ccUnxSkt"

#define ONVIF_DEBUG
#ifdef ONVIF_DEBUG
#define ONVIF_TRACE(format, args...) printf("[%s:%d] " format, __FILE__, __LINE__, ##args)
#else
#define ONVIF_TRACE(args...)
#endif

//Product Type
//#define   MT800_2 1
#ifdef HC1892_EM1
#define MT801 1
#elif defined HC1782_EM1
#define MT801 1
#elif defined MT801_1
#define MT801 1
#elif defined MT800_1
#define MT800_1 1
#elif defined MT800_2
#define MT800_2 1
#elif defined GT804_1 //GT804_1 is same as MT800_2
#define MT800_2 1
#elif defined GT804_2 //GT804_2 is same as MT800_2
#define MT800_2 1
#endif

extern char tmpOSDString[128];
//extern _AGTX_PRODUCT_OPTION_S *devOptions;
//extern char productOptionsJsonString[10240];
extern const int portNumber[3];

extern unsigned int numOfStreams;
extern unsigned int numOfMainRes;
extern unsigned int numOfsecondRes;
extern unsigned int numOfthirdRes;

int SYS_Gethostname(char *str);
int SYS_Getgateway(unsigned int *p);
char *SYS_Getipaddr(char *name, char *str);
char *SYS_Getmacaddr(char *name, char *str);
int SYS_Getadminsettings(char *name, unsigned char *autoneg, unsigned short *speed, unsigned char *duplex);

int SYS_Sethostname(char *str);
int SYS_NumNetworkIntf();
char *SYS_NetworkInfName(int idx);
int SYS_SetDefaultGateway(char *gwAddr);
int SYS_GetIPNetmask(char *name);
//JSON
void aux_delay(float num_sec);
int getCCReturn(int fd, char *str);
int aux_json_validation(char *buffer, int strLen);
int aux_json_get_int(char *buffer, char *dKey, int strLen);
double aux_json_get_double(char *buffer, char *dKey, int strLen);
char *aux_json_get_string(char *buffer, char *dKey, int strLen, char *retStr);
int aux_json_get_bool(char *buffer, char *dKey, int strLen); //0=false,1=true,-99=not a bool typ
//resolution,bitrate,fps, quality, gop
int aux_json_get_videores(char *ccRetStr, char *dKey, int strLen, int devNum);
int aux_json_get_from_array(char *buffer, char *arrKey, char *subKey, int arrIdx, int strLen);
char *aux_json_get_str_from_array(char *buffer, char *arrKey, char *subKey, int arrIdx, int strLen);
int aux_get_cc_config(int cmdId, void *data);
int aux_set_cc_config(int cmdId, void *data);
int AG_connect_to_cc();
int aux_init_venc_res(void);
AGTX_RES_OPTION_S *aux_get_res(void);
AGTX_VENC_OPTION_S *aux_get_venc(void);

void *aux_onvif_malloc(struct soap *soap, int size);
int aux_get_iva_setting(struct soap *soap, struct tt__VideoAnalyticsConfiguration **VideoAnalyticsConfiguration);
int aux_parse_iso8061_duration(char *str);
int aux_get_device_info(char *key, char *value, int max_size);
int aux_get_user_pwd(char user[MAX_USER][MAX_USER_LEN], char pwd[MAX_USER][MAX_USER_LEN]);
int aux_set_user_pwd(char user[MAX_USER][MAX_USER_LEN], char pwd[MAX_USER][MAX_USER_LEN]);
int aux_check_auth(struct soap *soap);
int aux_check_http_auth(struct soap *soap);
int aux_auth_fault(struct soap *soap);
char *SYS_NetworkPrefixToMask(int preFix);

//OSD  //TODO improve
int aux_json_osd_get_key(char *buffer /*jsonCmd*/, char *subKey /*subKey search key*/, int regionIdx /*StreamIdx*/,
                         int osdIdx, char *Rval);
void parse_res_option(AGTX_RES_OPTION_S *data, struct json_object *cmd_obj);
void parse_venc_option(AGTX_VENC_OPTION_S *data, struct json_object *cmd_obj);
#endif
