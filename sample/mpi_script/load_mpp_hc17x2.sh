#!/bin/sh
# Usage: source load_mpp.sh [-i]
#

####################Variables Definition##########################

UTRC_DRV=/system/lib/utrc.ko
VB_DRV=/system/lib/vb.ko
SR_DRV=/system/lib/sr.ko
VDA_DRV=/system/lib/vda.ko
IS_DRV=/system/lib/is.ko
ISP_DRV=/system/lib/isp.ko
ENC_DRV=/system/lib/enc.ko
RC_DRV=/system/lib/rc.ko
SENIF_DRV=/system/lib/senif.ko
OSD_DRV=/system/lib/osd.ko
POWER_ON_SCRIPT_PATH=/system/mpp/script/

##################################################################

report_error()
{
        echo "==== Error: There's something wrong, please check! ===="
        exit 1
}

load_usage()
{
	echo "Usage:  source load_mpp.sh [-option]"
	echo "options:"
	echo "    -i  initialize MPP"
#	echo "    -e  exit MPP"
#	echo "    -a  exit MPP first, then initialize MPP"
	echo "    -h  help information"
	echo -e "example: source load_mpp.sh -i\n"
}

power_on()
{
	SENSOR_POWER_ON=`find $POWER_ON_SCRIPT_PATH -iname "sensor_power_on*.sh"`

	if [ -z "$SENSOR_POWER_ON" ]; then
		echo "Cannot find sensor power on script!"
		exit 1
	fi

	for script in $SENSOR_POWER_ON
	do
		sh $script
	done
}

insert_ko()
{
        # install modules
        if [ -f $UTRC_DRV ]; then
                insmod $UTRC_DRV
        else
                echo "Cannot find $UTRC_DRV!"
                exit 1
        fi

        if [ -f $VB_DRV ]; then
                insmod $VB_DRV
        else
                echo "Cannot find $VB_DRV!"
                exit 1
        fi

        if [ -f $SR_DRV ]; then
                insmod $SR_DRV
        else
                echo "Cannot find $SR_DRV!"
                exit 1
        fi

        if [ -f $VDA_DRV ]; then
                insmod $VDA_DRV
        else
                echo "Cannot find $VDA_DRV!"
                exit 1
        fi

        if [ -f $IS_DRV ]; then
                insmod $IS_DRV
        else
                echo "Cannot find $IS_DRV!"
                exit 1
        fi

        if [ -f $ISP_DRV ]; then
                insmod $ISP_DRV
        else
                echo "Cannot find $ISP_DRV!"
                exit 1
        fi

        if [ -f $OSD_DRV ]; then
                insmod $OSD_DRV
        else
                echo "Cannot find $OSD_DRV!"
                exit 1
        fi

		if [ -f $RC_DRV ]; then
                insmod $RC_DRV
        else
                echo "Cannot find $RC_DRV!"
                exit 1
        fi

        if [ -f $ENC_DRV ]; then
                insmod $ENC_DRV
        else
                echo "Cannot find $ENC_DRV!"
                exit 1
		fi

        if [ -f $SENIF_DRV ]; then
                insmod $SENIF_DRV
        else
                echo "Cannot find $SENIF_DRV!"
                exit 1
        fi
}

remove_ko()
{
	#Remove modules
        rmmod senif.ko
        rmmod enc.ko
	rmmod rc.ko
	rmmod osd.ko
        rmmod isp.ko
        rmmod is.ko
	rmmod vda.ko
	rmmod sr.ko
        rmmod vb.ko
	rmmod utrc.ko
}

init_mpp()
{
        #Insert modules
        insert_ko

        #Power sensor
	power_on
}

exit_mpp()
{
        #Remove modules
        remove_ko
}

######################parse arg###################################

export INIT_FLAG

b_arg_init=0
b_arg_exit=0

for arg in $@
do
	case $arg in
	"-i")
		b_arg_init=1;
		;;
	"-e")
		b_arg_exit=1;
		;;
	"-a")
		b_arg_init=1;
		b_arg_exit=1;
		;;
	"-h")
		load_usage;
		;;
		*)
	echo "Invalid Parameter"
	;;
	esac
done

#######################parse arg end########################


#######################Action###############################

if [ $# -lt 1 ]; then
    load_usage;
    exit 0;
fi

if [ "${INIT_FLAG}" != "" ] && [ $b_arg_exit == 1 ]; then
	export INIT_FLAG="";
	echo "Exit $INIT_FLAG"
#	exit_mpp;
else
	echo "Nothing happen since initialization script has not yet been run!"
fi

if [ "${INIT_FLAG}"  == "" ] && [ $b_arg_init == 1 ]; then
	export INIT_FLAG=$(($INIT_FLAG + 1));
	init_mpp;
else
	echo "Initialization script can only be run once!"
fi

