#!/bin/sh

# We must check if device file exist.
# If device file does not exist, written to root file system without error.
# Then, we will think USB mass storage test passed while USB mass storage does not exist at all.
if [ -b "/dev/sda" ]
then
	true
else
	echo "========================================"
	echo "FAIL: USB mass storage device not found."
	echo "========================================"
	sleep 10000
fi

# Mount USB to /mnt/usb directory
mount /dev/sda /mnt/usb

if [ -f /mnt/usb/golden_data ]
then
	echo "golden file found in USB"
else
	echo "Un-initialized USB disk, initializing..."
	umount /mnt/usb

	sed -e 's/\s*\([\+0-9a-zA-Z]*\).*/\1/' << EOF | fdisk /dev/sda > /dev/null 2>&1 pid=$!
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
	mkfs.vfat /dev/sda

	# Mount USB to /mnt/usb directory
	mount /dev/sda /mnt/usb

	cp /usrdata/golden_data /mnt/usb

fi

md5sum /mnt/usb/golden_data | awk '{print $1 > "/tmp/crc_usb"}'
GOLDEN_CRC=$(cat /tmp/crc_usb)

if [ $GOLDEN_CRC != "86952be8d985fc59697c7a589977a634" ]
then
	echo "=================================="
	echo "FAIL: USB mass storage SLT failed."
	echo "=================================="
	sleep 10000
fi	

# Clean up
umount /mnt/usb

# Declare success
echo "PASS: USB mass storage SLT passed."

