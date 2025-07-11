#ifndef AUGENTIX_H_
#define AUGENTIX_H_

#define MAX_STR_LEN 256
#define HOSTNAME_LEN 1024

#define ONVIF_DEBUG
#ifdef ONVIF_DEBUG
#define ONVIF_TRACE(format, args...) printf("[%s:%d] " format, __FILE__, __LINE__, ##args)
#else
#define ONVIF_TRACE(args...)
#endif

int SYS_Gethostname(char *str);
int SYS_Getgateway(unsigned int *p);
char *SYS_Getipaddr(char *name, char *str);
char *SYS_Getmacaddr(char *name, char *str);
int SYS_Getadminsettings(char *name, unsigned char *autoneg, unsigned short *speed, unsigned char *duplex);

#endif
