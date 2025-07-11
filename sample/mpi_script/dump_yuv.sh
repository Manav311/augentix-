#!/bin/sh

if [ $# != 6 ]; then
	echo "input reg: width, height, phase, bit, name, path"
	echo "		width: image width"
	echo "		height: image height"
	echo "		phase:[0,3]"
	echo "		bit: 8 or 10 bit"
    echo "		name: image name"
	echo "		path: /mnt/nfs/usbnfs"
	exit 1
else
	echo "File width: $1"
	echo "File height: $2"
	echo "File phase: $3"
	echo "File bit: $4"
	echo "File name: $5"
    echo "File path: $6"
fi

#USBPATH=/mnt/nfs/usbnfs
FSIZEWO=$1
FSIZEHO=$2
FPHASE=$3
BIT=$4
FNAME=$5_
USBPATH=$6

#### NRW BUFFER ########################################
DUMP_CHANNEL=0
DUMP_YUV=1
Y_ONLY=0
if [ "$BIT" == 8 ]; then
	MSB_ONLY=1
else
	MSB_ONLY=0
fi

### dump image for result ###

NRWADDR=$(cat /dev/bmgr | sed -n '6p' | awk '{print $4}' | sed "s/(//g")
nrw2yuv -w$FSIZEWO -h$FSIZEHO -c$DUMP_CHANNEL -d$DUMP_YUV -y$Y_ONLY -m$MSB_ONLY $NRWADDR $USBPATH/$FNAME

### it's a hint for calibration tool ###
echo "Dump YUV Finished"
