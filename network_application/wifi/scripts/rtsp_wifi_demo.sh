#!/bin/sh
#
# Manually select SSID and PSK
#

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/root/bin:/system/bin

WPA_CONF_TMP=/tmp/wpa_supplicant.conf
WPA_CONF=/etc/wpa_supplicant.conf

case "$1" in
	start)
		if [ ! -e $WPA_CONF_TMP ]; then
			if [ -e $WPA_CONF ]; then
				cp $WPA_CONF $WPA_CONF_TMP
			else
				echo File $WPA_CONF does not exist
				exit 1
			fi
			echo -n "Please input SSID: "
			read SSID
			echo -n "Please input password: "
			read PSK
			sed -i "s/ssid=.*/ssid=\"${SSID}\"/" ${WPA_CONF_TMP}
			sed -i "s/psk=.*/psk=\"${PSK}\"/" ${WPA_CONF_TMP}
		fi

		sh /system/script/wifi_on.sh $WPA_CONF_TMP
		;;
	stop)
		sh /system/script/wifi_off.sh
		;;
	*)
		echo "Usage: $0 {start|stop}"
		exit 1
esac
