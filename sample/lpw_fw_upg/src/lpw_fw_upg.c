#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "lpw_fw_upg.h"
#include "log.h"

//#define DEBUG

/* default setting */
#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief call libLPW API to upgrade the firmware of the Wi-Fi module
 * @details
 * @param[in] *new_fw_path path of new firmware
 * @return ret
 * @retval 0     upgrade success
 * @retval not 0 upgrade failure
 * @see
 */
int LPW_appUpgFw(unsigned char *new_fw_path)
{
	int ret = 0;
	lpw_handle hd;
	lpw_fw_ver_t fw_ver_before_upg;

	/* check new fw upgrade file is not null */
	if (new_fw_path == NULL) {
		log_err("new firmware path is null.\n");
		return -ENOENT;
	}

	/* initialize LPW handler */
	hd = lpw_open();
	if (hd == (lpw_handle)NULL) {
		log_err("open LPW device fail.\n");
		return -EPERM;
	}

	/* get the current version of firmware */
	lpw_module_get_version(hd, &fw_ver_before_upg);
	log_info("current firmware version : [%u.%u.%u]\n", fw_ver_before_upg.ver[0], fw_ver_before_upg.ver[1],
	         fw_ver_before_upg.ver[2]);

	/* call libLPW API to start the firmware upgrade */
	ret = lpw_fw_upg(hd, new_fw_path);
	if (ret != 0) {
		log_err("firmware upgrade fail. Error no. = %d\n", ret);
		ret = -EPERM;
		goto end;
	}
end:
	lpw_close(hd);

	if (ret == 0) { // upgrade success
		system("ifdown wlan0");

		/* wait for Wi-Fi module proceed */
		sleep(5);
		printf("Wireless module FW upgrade success, please reboot!\n");

		system("killall lpw_controller");
#ifdef CONFIG_LPW_HI3861L
		system("rmmod hichannel");
#endif
	}

	return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
