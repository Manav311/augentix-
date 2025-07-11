#!/bin/sh

# We must check if device file exist.
# If device file does not exist, test data will be written to root file system without error.
# Then, we will think SD card test passed while SD card does not exist at all.
if [ -b "/dev/mmcblk0" ]
then
	true
else
	echo "==============================="
	echo "FAIL: SD Card device not found."
	echo "==============================="
	sleep 10000
fi

if [ -f /mnt/sdcard/golden_data ]
then
	echo "golden file found in SD card"
else
	echo "Un-initialized SD card, initializing..."

	umount /mnt/sdcard > /dev/null 2>&1
	mkfs.vfat /dev/mmcblk0

	sleep 1

	sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/mmcblk0 > /dev/null 2>&1 pid=$!
	  o # clear the in memory partition table
	  n # new partition
	  p # primary partition
	  1 # partition number 1
	    # default - start at beginning of disk
	    # default, extend partition to end of disk
	  w # write the partition table
	  q # and we're done
EOF

	wait $pid

	# format USB disk
	if [ -b "/dev/mmcblk0p1" ]
	then
		mkfs.vfat /dev/mmcblk0p1
		mount /dev/mmcblk0p1 /mnt/sdcard
	else
		mkfs.vfat /dev/mmcblk0
		mount /dev/mmcblk0 /mnt/sdcard
	fi

	cp /usrdata/golden_data /mnt/sdcard/
fi

md5sum /mnt/sdcard/golden_data | awk '{print $1 > "/tmp/crc_sdc"}'
GOLDEN_CRC=$(cat /tmp/crc_sdc)

if [ $GOLDEN_CRC != "86952be8d985fc59697c7a589977a634" ]
then
	echo "========================="
	echo "FAIL: SD card SLT failed."
	echo "========================="
	sleep 10000
fi

# Clean up
umount /mnt/sdcard

# Declare success
echo "PASS: SD card SLT passed."

