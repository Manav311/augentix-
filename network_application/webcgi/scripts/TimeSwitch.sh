#! /bin/sh

if [ $1 == '--help' ]; then
	echo "#========================================#"
	echo "#== Set Time Switch and Enabled config ==#"
	echo "#========================================#"
	echo ""
	echo " Usage_TimeSwitch"
	echo " sh /system/www/cgi-bin/TimeSwitch.sh setTimeSwitch [TimeSwitch_enabled] [TimeStartHr] [TimeStartMin] [TimeEndHr] [TimeEndMin]"
	echo ""
	echo " The Hr must be between 0 and 24. The Min must be between 0 and 60."
	echo ""
	echo " Example:"
	echo " sh /system/www/cgi-bin/TimeSwitch.sh setTimeSwitch 1 06 30 18 30"
fi

#TimeSwitch config
TimeSwitch_conf="/usrdata/active_setting/TimeSwitch.conf"

MODE=$1

if [ $MODE == 'setTimeSwitch' ]; then
	sed -i '/=/d' "$TimeSwitch_conf"
	echo "TimeSwitch_enabled=$2" >> "$TimeSwitch_conf"
	echo "TimeStartHr=$3" >> "$TimeSwitch_conf"
	echo "TimeStartMin=$4" >> "$TimeSwitch_conf"
	echo "TimeEndHr=$5" >> "$TimeSwitch_conf"
	echo "TimeEndMin=$6" >> "$TimeSwitch_conf"

fi
