#! /bin/sh

if [ $1 == '--help' ]; then
		echo "#======================================================#"
		echo "#== Set Debug Mode, Dump Debug Info and Reset Inform ==#"
		echo "#======================================================#"
		echo ""
		echo " Usage_Enabled"
		echo " sh /system/bin/debug_mode.sh [Mode]"
		echo " [Mode]"
		echo " Debug_Mode. If SD_dump_flag exist, change mode to debug mode."
		echo " Debug_Dump. If the status is in debug mode, dump debug info."
		echo " Debug_Dump. If the status isn't in debug mode, inform system reset status."
		echo ""
		echo " Example:"
		echo " sh /system/bin/debug_mode.sh Debug_Mode"
		echo " sh /system/bin/debug_mode.sh Debug_Dump"
		echo ""
		echo " Debug dump directory"
		echo " Debug_dump_dir = /mnt/sdcard/debug_dump"
		echo " SD_dump_flag = /mnt/sdcard/EnableDebugDump"
		echo ""
fi

#Debug dump directory.
Debug_dump_dir="/mnt/sdcard/debug_dump"
#Debug dump flag.
Debug_dump_flag="/usrdata/debug_dump_flag"
#SD card dump flag
SD_dump_flag="/mnt/sdcard/EnableDebugDump"
#OTA/Card system upgrade flag
system_upgrade_flag="/tmp/SystemUpgradeFlag"
#Augentix.log
log_file="/tmp/augentix.log"
#SD card upgrade flag.
card_upgrade_flag="/tmp/bt_update"

usage(){
	echo " Usage:"
	echo " [Set Debug Mode] $0 [Mode]"
	echo " Debug_dump_dir = /mnt/sdcard/debug_dump"
	echo " SD_dump_flag = /mnt/sdcard/EnableDebugDump"
	echo ""
	echo " Use --help to get more detail instruction"
}

usage_exit(){
	usage
	exit 0
}

if [ $# -gt 3 ]; then usage_exit; fi

Mode=$1
User_Mode=$(cat /usrdata/mode)
echo "User mode is $User_Mode"

if [ $Mode == 'Debug_Mode' ]; then
	if [ ! -e $system_upgrade_flag ]; then
		if [ $User_Mode == 'develop' ] && [ ! -e $Debug_dump_flag ] && [ -e $SD_dump_flag ]; then
			touch $Debug_dump_flag
			echo "User mode is develop mode, debug dump flag on"
			/system/bin/setLEDevt.sh DEBUG_MODE 1
		else
			rm -f $Debug_dump_flag
			echo "Debug dump flag off"
			/system/bin/setLEDevt.sh DEBUG_MODE 0
		fi
	fi

elif [ $Mode == 'Debug_Dump' ]; then
	if [ $User_Mode == 'develop' ] && [ -e $Debug_dump_flag ] && [ -e $SD_dump_flag ]; then
		/system/bin/setLEDevt.sh DEBUG_INFO_DUMP 1
		mkdir $Debug_dump_dir
			#Dump debug message
			cp $log_file $Debug_dump_dir
			[ $? = 0 ] && echo "Copy augentix.log OK" || echo "Copy augentix.log FAIL"
			/system/script/dump_flash.sh $Debug_dump_dir
			[ $? = 0 ] && echo "Dump flash OK" || echo "Dump flash FAIL"
			/system/mpp/script/debug.sh $Debug_dump_dir
			[ $? = 0 ] && echo "Debug dump OK" || echo "Debug dump FAIL"
			/system/bin/setLEDevt.sh DEBUG_MODE 0
			/system/bin/setLEDevt.sh DEBUG_INFO_DUMP 0
			/system/bin/setLEDevt.sh LED_OFF 0
		rm -f $Debug_dump_flag
	else
		if [ ! -e $system_upgrade_flag ]; then
			# When doing card upgrade in init stage, ignore factory reset.
			if [ ! -e $card_upgrade_flag ]; then
				touch /tmp/cuetone # To play a cue tone as a hint about factory reset was done
				touch /usrdata/reset_file
				reboot
			fi
		else
			/system/bin/setLEDevt.sh LED_OFF 0
		fi
	fi
fi