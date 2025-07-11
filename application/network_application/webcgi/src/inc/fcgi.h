#ifndef _FCGI_H_
#define _FCGI_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcgi_stdio.h>

#define TZ_FILE_PATH "/etc/TZ"
#define Machine_Mode "/usrdata/mode"

void getTz(void);

void fcgiGetTime(void);

void fcgiGetSntpConf(void);

void fcgiGetEnabledConf(void);

void fcgiGetDSTConf(void);

void fcgiGetTZ(void);

void fcgiGetTimeSwitch(void);

void fcgiExportSetting(void);

void fcgiFirmwareUpload(void);

void fcgiImportSetting(void);

void fcgiGetHostname(void);

void fcgiSetToDefault(void);

void fcgiReboot(void);

void fcgiStopStream(char *MachineMode);

void fcgiSysupdOS(void);

void fcgiSwitch2SysupdOS(void);

void fcgiSetPort4530(void);

void fcgiChangePass(char *buf);

void fcgiAssignIP(char *MachineMode, char *buf);

void fcgiNetInfo(void);

void fcgiUpTime(void);

void fcgiGetMAC(void);

void fcgiGetFirmwareVersion(void);

void fcgiGetIPAddress(void);

void fcgiGetNetmask(void);

void fcgiGetGateway(void);

void fcgiGetDNS(void);

void fcgiSetIP(char *buf);

void fcgiSetMask(char *buf);

void fcgiSetGateway(char *buf);

void fcgiSetDNS(char *buf);

void fcgiSetTime(char *MachineMode, char *buf);

void fcgiSetSntpConf(char *MachineMode, char *buf);

void fcgiDSTSet(char *buf);

void fcgiTZSet(char *buf);

void fcgiEnabledSet(char *buf);

void fcgiTimeSwitchSet(char *MachineMode, char *buf);

void fcgiGetUploadFile(char *name);

void fcgiUpdateCA(void);

void fcgiResetCA(void);

void fcgiGetFaceModelList(void);

void fcgiRemoveFile(char *name);

void fcgiValidateFaceModel(char *name);

void fcgiRegisterFaceModel(char *name);

void fcgiUnregisterFaceModel(char *name);

void fcgiSetWifi(char *buf);

void fcgiDisconnectWifi(void);

void fcgiGetSSID(void);

void fcgiGetWPASSID(void);

void fcgiGetWlanInfo(void);

void fcgiPackNginxLog(void);
#endif /* _FCGI_H_ */
