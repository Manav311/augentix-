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

FSIZEW=$1
FSIZEH=$2
FPHASE=$3
BIT=$4
FNAME=$5
USBPATH=$6

### get address ###
TFWADDR=$(cat /dev/bmgr | sed -n '2p' | awk '{print $4}' | sed "s/(//g")

### dump pgm ###
ddr2pgm -r"$FSIZEW"x"$FSIZEH" -p$FPHASE -b$BIT $TFWADDR $USBPATH/$FNAME.pgm

### it's a hint for calibration tool ###
echo "Dump PGM Finished"