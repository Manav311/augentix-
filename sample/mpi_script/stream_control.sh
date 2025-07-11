#!/bin/sh

if [ "${1}" == "-h" ]; then	
	echo "input reg: option, case_idx, nfs_path"
	echo "		option:START, STOP"
    echo "		case_idx:[1~13]"
    echo "		nfs_path: /mnt/nfs/usbnfs"
	sleep 1
	exit 1
elif [ $# != 3 ]; then
	echo "input reg: option, case_idx, nfs_path"
	echo "		option:START, STOP"
    echo "		case_idx:[1~13]"
    echo "		nfs_path: /mnt/nfs/usbnfs"
	sleep 1
	exit 1
else
	echo "option: $1"
    echo "case_idx: $2"
    echo "nfs_path: $3"
fi

OPTION=$1
CASEIDX=$2
NFSPATH=$3

if [ "${OPTION}" == "FIRST" ]; then
	source script/load_mpp.sh -i
elif [ "${OPTION}" == "START" ]; then
	### Kill existing stream program ###
    kill_demo() {
		kill -INT $(ps | grep 'mpi_stream' | grep -v 'grep' | awk '{print $1}')
	}
	kill_demo || true

    ### copy sensor.ini file form nfs ###
	#cp $NFSPATH/sensor.ini script/
	mpi_stream -d /system/mpp/case_config/case_config_$CASEIDX -p udpstream_enable=1 -p record_enable=0 > /dev/null &    
elif [ "${OPTION}" == "STOP" ]; then
	echo "Kill existing stream program"
	kill_demo() {
			kill -INT $(ps | grep 'mpi_stream' | grep -v 'grep' | awk '{print $1}')
	}
	kill_demo || true
fi 