#!/bin/sh

#
# AUGENTIX INC. - PROPRIETARY
#
# prefer-reset.sh: do database preference reset after system update
# Copyright (C) 2018 Augentix Inc. - All Rights Reserved
#
# NOTICE: The information contained herein is the property of Augentix Inc.
# Copying and distributing of this file, via any medium,
# must be licensed by Augentix Inc.
#
# * Brief: Post-installation script
# *
# * Author: SongTing Yang <songting.yang@augentix.com>
#

prefer_reset_file=/usrdata/prefer_reset_file
pre_version=$(head -n 1 /etc/sw-version)

echo "Previous version: $pre_version"

BLOCK_VER="INPUT_PREF_VERSION"

if [ ! -z $BLOCK_VER ]; then
	echo "Check previous version with blocking version: $BLOCK_VER!"
    pre_version1=$(echo $pre_version | cut -d "." -f 1)
    pre_version2=$(echo $pre_version | cut -d "." -f 2)
    pre_version3=$(echo $pre_version | cut -d "." -f 3)
    block_version1=$(echo $BLOCK_VER | cut -d "." -f 1)
    block_version2=$(echo $BLOCK_VER | cut -d "." -f 2)
    block_version3=$(echo $BLOCK_VER | cut -d "." -f 3)

    if [ $pre_version1 -le $block_version1 ]; then

        if [ $pre_version2 -le $block_version2 ]; then

            if [ $pre_version3 -le $block_version3 ]; then
                echo "Verify previous version fail, prepare to do database preference reset!"
                touch $prefer_reset_file
                echo "[SYSUPD] System Update Finished. User preference data reset ready. Reboot the system."
            fi
        fi
    else
        echo "Verify previous version pass! No need to do preference data reset."
        echo "Reboot the system."
    fi
else
    touch $prefer_reset_file
    echo "No blocking version! Ready to do preference data reset."
    echo "[SYSUPD] System Update Finished. User preference data reset ready. Reboot the system."
fi
