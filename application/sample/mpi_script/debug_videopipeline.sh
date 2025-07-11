#! /bin/sh
#
# debug.sh
# Copyright (C) 2023 Augentix Inc.
#
# Distributed under terms of the MIT license.
#

# If some of the programs are not put under $PATH (e.g. /system/bin/)
# please set "EXTERN_PROGRAM_PATH" variable to the path where these programs are stored.
# Programs/scripts required: busybox, cmdsender, csr, ddr2pgm, dip_dump, dump, dump_csr, mpi_snapshot
PROGRAM_PATH="$EXTERN_PROGRAM_PATH"

if [ "$PROGRAM_PATH" ]; then
	CMDSENDER="${PROGRAM_PATH}/cmdsender"
else
	CMDSENDER="cmdsender"
fi


if [ "$(command -v ${CMDSENDER})" -a -f "$(command -v ${CMDSENDER})" ]; then :
else
	$ECHO "Error: Cannot find cmdsender program!"
	exit 1
fi


# for loop to check all cmds
echo 'Start to get video pipeline attributes!'

dev_max=0
path_max=1
chn_max=3
enc_max=3
win_max=8

for i in $(seq 0 $dev_max)
do
	echo '---get device :' $i '---'
	$CMDSENDER --vb $i
	$CMDSENDER --dev $i
	for j in $(seq 0 $path_max)
	do
		echo '---get path:' $i $j '---'
		$CMDSENDER --roi $i $j
	done
	for k in $(seq 0 $chn_max)
	do
		echo '---get channel:' $i $k '---'
		$CMDSENDER --chn $i $k
		$CMDSENDER --chn_layout $i $k
		for m in $(seq 0 $win_max)
		do
			echo '---get window:' $i $k $m '---'
			$CMDSENDER --win $i $k $m
			$CMDSENDER --winroi $i $k $m
		done
	done
	for l in $(seq 0 $enc_max)
	do
		echo '---get encoder:' $i $l'---'
		$CMDSENDER --venc --get_vattr --enc_idx=$l
	done
done

echo "-----[VBS driver info]-----"
cat /dev/vbs

echo "-----[SENIF driver info]-----"
cat /dev/senif

echo "-----[IS driver info]-----"
cat /dev/is

echo "-----[ISP driver info]-----"
cat /dev/isp

echo "-----[ENC driver info]-----"
cat /dev/enc


echo 'All Done!'