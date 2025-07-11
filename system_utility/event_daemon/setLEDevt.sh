#! /bin/sh

if [ $1 == '--help' ]; then
		echo "#==================================#"
		echo "#== Set LED Event Daemon ==#"
		echo "#==================================#"
		echo ""
		echo " Usage_Enabled"
		echo " sh /system/bin/setLEDevt.sh [LED_Client] [LED_enabled]"
		echo " LED_enabled, 1 is enable, 0 is disable."
		echo ""
		echo " Example:"
		echo " sh /system/bin/setLEDevt.sh Wifi_Pairing 1"
		echo ""
		echo " Client list"
		echo " Wifi_Pairing"
		echo " Wifi_Connecting"
		echo " Wifi_Connected"
		echo " Motion_Detected"
		echo " Live_view"
		echo " Low_Signal"
		echo " Disconnected"
		echo " OTA"
		echo " Critical_Error"
		echo " Card_Upgrade"
		echo " DEBUG_MODE"
		echo " DEBUG_INFO_DUMP"
		echo " Reset_INFO_Slow"
		echo " Reset_INFO_Fast"
fi

#LED event config
ledapp_path="/system/bin/ledapp"

usage(){
	echo "Usage:"
	echo "[Set LED Event] [LED_Client] [LED_enabled]"
	echo ""
	echo "Use --help to get more detail instruction"
}

usage_exit(){
	usage
	exit 0
}

if [ $# != 2 ]; then usage_exit; fi

$ledapp_path $1 $2