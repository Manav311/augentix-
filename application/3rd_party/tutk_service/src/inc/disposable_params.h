#ifndef _CLI_PLUGIN_H_
#define _CLI_PLUGIN_H_

#define DISPOSABLE_PARAMS_PATH "/usrdata/active_setting/disposable_params"
#define DISPOSABLE_PIN_CODE_LENGTH 4
#define DISPOSABLE_SECRET_ID_LENGTH 8
#define DISPOSABLE_PSK_LENGTH 8

int loadDisposableParams(char *pin_code, char *secret_id, char *psk);
void getRandomStr(char *buf, int length);

#endif /* _CLI_PLUGIN_H_ */