#! /bin/sh

if [ $1 == '--help' ]; then
	echo "#==================================#"
	echo "#== Set DST and timeMode config ==#"
	echo "#==================================#"
	echo ""
	echo " Usage_DST"
	echo " sh /system/www/cgi-bin/DSTConf.sh set GMT[offset]DST[offset],M[m].[w].[d]/[hr],M[m2].[w2].[d2]/[hr2]"
	echo ""
	echo " The day d must be between 0 (Sunday) and 6. The week w must be between 1 and 5. The month m should be between 1 and 12."
	echo " Week 1 is the first week in which day d occurs, and week 5 specifies the last d day in the month."
	echo ""
	echo " Usage_Enabled"
	echo " sh /system/www/cgi-bin/DSTConf.sh setEnabled [Manual_enabled] [DST_enabled] [Manual_SyncWithPC_enabled]"
	echo ""
	echo " Example:"
	echo " sh /system/www/cgi-bin/SntpConf.sh set GMT+5DST,M3.2.0/2,M11.1.0/2"
	echo " sh /system/www/cgi-bin/SntpConf.sh set GMT+8"
	echo " sh /system/www/cgi-bin/SntpConf.sh setEnabled 0 0 1"
fi

#TZ config
TZ_conf="/etc/TZ"
timeMode_conf="/usrdata/active_setting/timeMode.conf"

MODE=$1

if [ $MODE == 'set' ]; then
	echo -n "$2" > "$TZ_conf"
	[ $? = 0 ] && echo "OK" || echo "FAIL"

elif [ $MODE == 'setEnabled' ]; then
	sed -i '/=/d' "$timeMode_conf"
	echo "Manual_enabled=$2" >> "$timeMode_conf"
	echo "DST_enabled=$3" >> "$timeMode_conf"
	echo "SyncWithPC_enabled=$4" >> "$timeMode_conf"

fi
