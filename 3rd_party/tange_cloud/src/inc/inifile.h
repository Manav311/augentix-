#ifndef __inifile_h__
#define __inifile_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "platforms.h"

/*处理 ini 文件。 section 可以为NULL(平面文件)*/
SA_BOOL GetProfileString(const char *iniFile, const char *section, const char *szKey, char *value, int len);
int GetProfileInt(const char *iniFile, const char *section, const char *key, int def);
char *GetProfileSection(const char *iniFile, const char *section);
SA_BOOL SetProfileSection(const char *iniFile, const char *section, char *sectionValue);
SA_BOOL SetProfileString(const char *iniFile, const char *section, const char *szKey, const char *value);
SA_BOOL SetProfileInt(const char *iniFile, const char *section, const char *szKey, int val);

#ifdef __cplusplus
}
#endif

#endif
