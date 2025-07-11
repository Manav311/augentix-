/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * inc_parser.h - config file parser
 * Copyright (C) 2019 ShihChieh Lin, Augentix Inc. <shihchieh.lin@augentix.com>
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 */
#ifndef INI_PARSER_H
#define INI_PARSER_H

#include "iniparser.h"
typedef struct config_t {
	dictionary *ini;
	const char * ssid;
	const char * passwd;
	const char * ipaddr;
	const char * netmask;
	const char * interface;
	const char * usb_eth_ipaddr;
	const char * pc_ipaddr;
} MpttCfg;

int init_cfg(MpttCfg * cfg);
void release_cfg(MpttCfg * cfg);
void dump_cfg(MpttCfg *cfg);

int parse_ini_file(const char * file, MpttCfg * cfg);

#endif /* INI_PARSER_H */
