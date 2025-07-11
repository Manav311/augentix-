#ifndef TUTK_INIT_H_
#define TUTK_INIT_H_

#include "AVAPIs.h"

//########################################################
//# Callback functions for avServStartEx()
//########################################################
int ExTokenDeleteCallBackFn(int av_index, const char *identity);
int ExTokenRequestCallBackFn(int av_index, const char *identity, const char *identity_description, char *token,
                             unsigned int token_length);
void ExGetIdentityArrayCallBackFn(int avIndex, avServSendIdentityArray send_identity_array);
int ExChangePasswordCallBackFn(int av_index, const char *account, const char *old_password, const char *new_password,
                               const char *new_iotc_authkey);
void ExAbilityRequestFn(int av_index, avServSendAbility send_ability);

//########################################################
//# settings_change_handler callback function
//########################################################
int SettingsChangeHandle(NebulaDeviceCtx *device, const char *settings);
int DeviceLoginStateHandle(NebulaDeviceCtx *device, NebulaDeviceLoginState state);

//########################################################
//#Thread - Send live streaming
//########################################################
int ExJsonRequestFn(int av_index, const char *func, const NebulaJsonObject *json_args, NebulaJsonObject **response);

//########################################################
//# identity_handler callback function
//########################################################
void IdentityHandle(NebulaDeviceCtx *device, const char *identity, char *psk, unsigned int psk_size);

//########################################################
//#  Initialize / Deinitialize client list of AV server
//########################################################
void DeInitAVInfo();
void InitAVInfo();

//########################################################
//# command_handler callback function
//########################################################
int CommandHandle(NebulaDeviceCtx *device, const char *identity, const char *func, const NebulaJsonObject *json_args,
                  NebulaJsonObject **json_response);
int ShouldDeviceGoToSleep();
void ResetNoSessionTime();
void UpdateNoSessionTime();

//########################################################
//# Get Timestamp
//########################################################
unsigned int GetTimeStampMs();
unsigned int GetTimeStampSec();

//########################################################
//# Print error message
//########################################################
void PrintErrHandling(int nErr);

//########################################################
//# Print IOTC & AV version
//########################################################
void PrintVersion();
void PrintUsage();

//########################################################
//# Enable / Disable live stream to AV client
//########################################################
void *search_session_av_index(int av_index);
void RegEditClient(int sid, int av_index);
void RegEditClientToVideo(int sid, int av_index);
void UnRegEditClientFromVideo(int sid);
void RegEditClientToAudio(int sid, int av_index);
void UnRegEditClientFromAudio(int sid);
void RegEditClientStreamMode(int sid, int stream_mode);
int GetSidFromAvIndex(int av_index);

//########################################################
//# Start AV server and recv IOCtrl cmd for every new av idx
//########################################################
void PrepareWakeupDataBeforeSleep(NebulaDeviceCtx *device_ctx, NebulaSocketProtocol nebula_protocol,
                                  char *wakeup_pattern, NebulaWakeUpData **data, unsigned int *data_count);
void WaitForWakeupPatternWhenSleeping(NebulaSocketProtocol nebula_protocol, char *wakeup_pattern,
                                      NebulaWakeUpData *data, unsigned int data_count,
                                      unsigned int interpolation_level);

int TUTK_run(char *udid, char *profile_url);
#endif /* TUTK_INIT_H_ */