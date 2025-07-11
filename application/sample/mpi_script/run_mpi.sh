#!/bin/sh

# default name
DEFAULT_OUTPUT="VIDEO-0"

# format SD card
#echo "Format SD card ..."
#mkdosfs /dev/mmcblk0

# mount SD card to /mnt/sdcard
#mkdir -p /mnt/sdcard
#mount /dev/$1 /mnt/sdcard
#echo "Mount SD card to /mnt/sdcard"

Output=OutputFile\=/mnt/nfs/usbnfs/$2

# Start single stream application
echo "Sample case ID: $1"
echo "Output filename: $2"
echo "Output file path: $Output"
mpi_stream -d /system/mpp/case_config/case_config_$1 -p $Output

# umount SD card
#umount /dev/$1
#echo "Umount SD card"

