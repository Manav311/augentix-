#!/bin/sh
tuya_db_path="/usrdata/active_setting/tuya"

#OTA/Card system upgrade flag
system_upgrade_flag="/tmp/SystemUpgradeFlag"

if [ ! -e $system_upgrade_flag ]; then
    rm -rf $tuya_db_path
    echo "Tuya DB reset. Rebooting system..."
    reboot -f
else
    /system/bin/setLEDevt.sh LED_OFF 0
fi