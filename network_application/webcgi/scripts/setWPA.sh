#! /bin/sh
#
# setWPA.sh
# Copyright (C) 2022 im50 <im50@aswws03>
#
# Distributed under terms of the MIT license.
#
sed -i "s/ssid=.*/ssid=\"$1\"/" /etc/wpa_supplicant.conf
sed -i "s/psk=.*/psk=\"$2\"/" /etc/wpa_supplicant.conf
