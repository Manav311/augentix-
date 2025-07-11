#!/bin/sh

#
# AUGENTIX INC. - PROPRIETARY
#
# factory-reset.sh: go back to factory mode after reboot
# Copyright (C) 2018 Augentix Inc. - All Rights Reserved
#
# NOTICE: The information contained herein is the property of Augentix Inc.
# Copying and distributing of this file, via any medium,
# must be licensed by Augentix Inc.
#
# * Brief: Post-installation script
# *
# * Author: ShihChieh Lin <shihchieh.lin@augentix.com>
#

reset_file=/usrdata/reset_file
touch $reset_file

mode factory

echo "[SYSUPD] System Update Finished. Factory mode ready. Reboot the system."
