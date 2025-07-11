#ifndef TGC_CONFIG_H_
#define TGC_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

void TGC_loadConfig(const char *cfg_dir);
void TGC_releaseConfig(void);
void TGC_resetSetting(void);
int TGC_loadSetting(int cmd, void *buff, int size);
int TGC_saveSetting(int cmd, const void *buf, int length);

const char *TGC_getOption(const char *section, const char *key);
const char *TGC_getFeature(const char *key);
const char *TGC_getState(const char *key);
const char *TGC_getSetting(const char *key);
int TGC_getInt(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* TGC_CONFIG_H_ */