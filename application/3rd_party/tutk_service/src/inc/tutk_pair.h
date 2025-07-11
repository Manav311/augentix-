#ifndef TUTK_WIFI_PAIR_H_
#define TUTK_WIFI_PAIR_H_

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tutk_define.h"

int TUTK_pairingWiFi(char *gudid);
int TUTK_disableWiFi();

int Device_LAN_WIFI_Config(const char *udid, const char *psk, const char *secret_id,
                           const NebulaJsonObject *profile_pbj, unsigned int *abort_flag);

#endif /* TUTK_WIFI_PAIR_H_ */