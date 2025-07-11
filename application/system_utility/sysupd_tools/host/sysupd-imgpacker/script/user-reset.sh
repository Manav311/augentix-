#!/bin/sh

#
# AUGENTIX INC. - PROPRIETARY
#
# post-install.sh: SWUpdate post-installtion script
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
echo "[SYSUPD] System Update finished. User data reset ready. Reboot the system."
