/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include "tange_cloud_config.h"
#include "inifile.h"

/**
 * Definition
*/
typedef struct {
	int cmd;
	int length;
	unsigned int crc32;
} cfg_cmdhdr;

/**
 * Variables
*/

/**
 * Static Variables
*/
static char config_path[128] = { 0 };
static char testcase_path[128] = { 0 };
static char *_features = NULL;
static char *_settings = NULL;
static char *_states = NULL;

/**
 * Static Function Prototype
*/
static const char *__getConfig(const char *kvs, const char *key);
static const char *_getConfig(const char *section, const char *key);

/**
 * Functions
*/
void TGC_loadConfig(const char *cfg_dir)
{
	sprintf(config_path, "%s/config.dat", cfg_dir);
	sprintf(testcase_path, "%s/testcase.ini", cfg_dir);
	if (!_features && !_settings && !_states) {
		_features = GetProfileSection(testcase_path, "features");
		_settings = GetProfileSection(testcase_path, "setting");
		_states = GetProfileSection(testcase_path, "state");
	}
}

void TGC_releaseConfig(void)
{
	if (_features) {
		My_free(_features);
		_features = NULL;
	}
	if (_settings) {
		My_free(_settings);
		_settings = NULL;
	}
	if (_states) {
		My_free(_states);
		_states = NULL;
	}
}

void TGC_resetSetting(void)
{
	remove(config_path);
	remove("/usrdata/wpa_supplicant.conf");
}

int TGC_loadSetting(int cmd, void *buff, int size)
{
	FILE *fp = fopen(config_path, "rb");
	if (fp) {
		int rlen;
		cfg_cmdhdr hdr;
		while ((rlen = fread(&hdr, 1, sizeof(hdr), fp)) == sizeof(hdr)) {
			if (hdr.cmd == cmd) {
				rlen = hdr.length < size ? hdr.length : size;
				fread(buff, 1, rlen, fp);
				fclose(fp);
				return rlen;
			} else
				fseek(fp, hdr.length, SEEK_CUR);
		}
		fclose(fp);
	}
	return 0;
}

int TGC_saveSetting(int cmd, const void *buf, int length)
{
	FILE *fp = fopen(config_path, "r+b");
	cfg_cmdhdr hdr;

	if (fp) {
		int rlen;
		while (1) {
			if ((rlen = fread(&hdr, 1, sizeof(hdr), fp)) == sizeof(hdr)) {
				if (hdr.cmd == cmd) {
					if (hdr.length != length) {
						hdr.cmd = 0;
						fseek(fp, -(sizeof(hdr)), SEEK_CUR);
						fwrite(&hdr, sizeof(hdr), 1, fp);
					} else {
						fwrite(buf, 1, length, fp);
						fclose(fp);
						return 0;
					}
				}
				fseek(fp, hdr.length, SEEK_CUR);
			} else {
				if (rlen > 0)
					fseek(fp, -rlen, SEEK_CUR);
				break;
			}
		}
	} else
		fp = fopen(config_path, "wb");

	if (fp) {
		hdr.cmd = cmd;
		hdr.length = length;
		hdr.crc32 = 0;
		fwrite(&hdr, sizeof(hdr), 1, fp);
		fwrite(buf, 1, length, fp);
		fclose(fp);
		return 0;
	}

	return -1;
}

const char *TGC_getOption(const char *section, const char *key)
{
	return _getConfig(section, key);
}

const char *TGC_getFeature(const char *key)
{
	return _getConfig("features", key);
}

const char *TGC_getState(const char *key)
{
	return _getConfig("state", key);
}

const char *TGC_getSetting(const char *key)
{
	return _getConfig("setting", key);
}

int TGC_getInt(const char *key)
{
	const char *val = TGC_getSetting(key);
	if (val)
		return atoi(val);
	return 0;
}

/**
 * Static Functions
*/
static const char *__getConfig(const char *kvs, const char *key)
{
	if (!kvs)
		return NULL;
	while (*kvs) {
		int klen = strlen(key);
		if (strncmp(key, kvs, klen) == 0 && kvs[klen] == '=')
			return kvs + klen + 1;
		kvs += strlen(kvs) + 1;
	}
	return NULL;
}

static const char *_getConfig(const char *section, const char *key)
{
	if (strcmp(section, "features") == 0)
		return __getConfig(_features, key);
	if (strcmp(section, "setting") == 0)
		return __getConfig(_settings, key);
	if (strcmp(section, "state") == 0)
		return __getConfig(_states, key);
	return NULL;
}
