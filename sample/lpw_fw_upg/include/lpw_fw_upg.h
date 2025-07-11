#ifndef LPW_FW_UPG_H_
#define LPW_FW_UPG_H_

#include <agtx_lpw.h>
#include <stdint.h>

/**
 * @brief call libLPW API to upgrade the firmware of the Wi-Fi module
 * @details
 * @param[in] *new_fw buffer of new fw
 * @return ret
 * @retval 0     read success
 * @retval not 0 read failure
 * @see
 */
int LPW_appUpgFw(unsigned char *new_fw_path);

#endif /* LPW_FW_UPG_H_ */
