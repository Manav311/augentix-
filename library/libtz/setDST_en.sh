#! /bin/sh

if [ $1 == '--help' ]; then
		echo "#==================================#"
		echo "#== Set DST Enabled config ==#"
		echo "#==================================#"
		echo ""
		echo " Usage_Enabled"
		echo " sh /system/bin/setDST_en.sh setDST [DST_enabled]"
		echo " DST_enabled, 1 is enable, 0 is disable."
		echo ""
		echo " Example:"
		echo " sh /system/bin/setDST_en.sh setDST 1"
fi

#Time mode config
timeMode_conf="/usrdata/active_setting/timeMode.conf"

MODE=$1

if [ $MODE == 'setDST' ]; then
	source "$timeMode_conf"
	if [ $DST_enabled = 0 ]; then
		sed -i "s/DST_enabled=0/DST_enabled=$2/" "$timeMode_conf"
		[ $? = 0 ] && echo "OK" || echo "FAIL"
	elif [ $DST_enabled = 1 ]; then
		sed -i "s/DST_enabled=1/DST_enabled=$2/" "$timeMode_conf"
		[ $? = 0 ] && echo "OK" || echo "FAIL"
	fi
fi
