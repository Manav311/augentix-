#! /bin/sh
#
# debug.sh
# Copyright (C) 2019 Augentix Inc.
#
# Distributed under terms of the MIT license.
#


NOW=`date +%Y-%m-%d.%H-%M-%S`

NFS=$1

if [ ! $NFS ]; then
        echo "Please specify the NFS path to store data"
        echo "For example: sh debug.sh /mnt/nfs/ethnfs"
        exit 1;
fi

DATA_PATH=$NFS/data-$NOW

mkdir -p $DATA_PATH

# Save log into NFS file system
AGTX_LOG=/var/log/augentix.log

echo "Save augentix log ..."
if [ -e "$AGTX_LOG" ]; then
    cp -f $AGTX_LOG $DATA_PATH
else
    echo "WARNING! No augentix log file!"
fi

# Save data base into NFS file system
AGTX_DB=/tmp/ini.db

echo "Save data base ..."
if [ -e "$AGTX_DB" ]; then
    cp -f $AGTX_DB $DATA_PATH
else
    echo "WARNING! No data base!"
fi

# Collect video stream info
VIDEO_INFO=video_stream_info.log
VIDEO_INFO_PATH=$DATA_PATH/$VIDEO_INFO

echo "Collect video stream info ... "
echo "Video stream info" > $VIDEO_INFO_PATH

echo "Video buffer part ..."
echo "Video buffer system" >> $VIDEO_INFO_PATH
cat /dev/vbs >> $VIDEO_INFO_PATH
echo -e "\n" >> $VIDEO_INFO_PATH

echo "SENIF part ..."
echo "Check SENIF status twice" >> $VIDEO_INFO_PATH
cat /dev/senif >> $VIDEO_INFO_PATH
sleep 10
cat /dev/senif >> $VIDEO_INFO_PATH
echo -e "\n" >> $VIDEO_INFO_PATH

echo "IS part ..."
echo "Check IS status twice" >> $VIDEO_INFO_PATH
cat /dev/is >> $VIDEO_INFO_PATH
sleep 10
cat /dev/is >> $VIDEO_INFO_PATH
echo -e "\n" >> $VIDEO_INFO_PATH

echo "ISP part ..."
echo "Check ISP status twice" >> $VIDEO_INFO_PATH
cat /dev/isp >> $VIDEO_INFO_PATH
sleep 10
cat /dev/isp >> $VIDEO_INFO_PATH
echo -e "\n" >> $VIDEO_INFO_PATH

echo "ENC part ..."
echo "Check ENC status twice" >> $VIDEO_INFO_PATH
cat /dev/enc >> $VIDEO_INFO_PATH
sleep 10
cat /dev/enc >> $VIDEO_INFO_PATH
echo -e "\n" >> $VIDEO_INFO_PATH

# Collect system info
SYS_INFO=sys_info.log
SYS_INFO_PATH=$DATA_PATH/$SYS_INFO

echo "Collect system info ..."
echo "System info" > $SYS_INFO_PATH

# Get SDK version info
echo "Get SDK version info ..."
echo "SDK version :" >> $SYS_INFO_PATH
cat /etc/sdk-version >> $SYS_INFO_PATH

echo "PIN-MUX info ..."
PIN_INFO=pin_info.log
PIN_INFO_PATH=$DATA_PATH/$PIN_INFO
/usr/bin/pin-info.sh $PIN_INFO_PATH

echo "Debug info are collected in $DATA_PATH"
