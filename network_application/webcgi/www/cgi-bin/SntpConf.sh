#! /bin/sh

if [ $1 == '--help' ]; then
	echo "#==================================#"
	echo "#== Set/Get Sntp config ==#"
	echo "#==================================#"
	echo "# NOTE: This script is used to set or get Sntp config."
	echo ""
	echo "Usage"
	echo " sh /system/www/cgi-bin/SntpConf.sh [MODE] [IP] [hr]"
	echo ""
	echo "OPTION:"
	echo " [MODE]: set, getip, gethr"
	echo " [IP]: Time server IP"
	echo " [hr]: Interval update time"
	echo ""
	echo "Example:"
	echo " sh /system/www/cgi-bin/SntpConf.sh set 0.pool.ntp.org 3"
	echo " sh /system/www/cgi-bin/SntpConf.sh getip"
	echo " sh /system/www/cgi-bin/SntpConf.sh gethr"
fi

#sntp config
sntp_conf="/usrdata/active_setting/sntp.conf"

MODE=$1

if [ $MODE == 'set' ]; then
	sed -i '/NTP_server/d' $sntp_conf
	sed -i '/Interval/d' $sntp_conf
	echo "NTP_server="$2" " >> "$sntp_conf"
	echo "Interval="$3" " >> "$sntp_conf"

elif [ $MODE == 'getip' ]; then
	source "$sntp_conf"
	echo $NTP_server
	return "$NTP_server"

elif [ $MODE == 'gethr' ]; then
	source "$sntp_conf"
	echo $Interval
	return "$Interval"
fi
