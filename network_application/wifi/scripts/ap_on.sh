#!/bin/sh

interface=wlan0
static_ip=192.168.0.1
#abs_hostapd=/system/bin/hostapd
abs_hostapd=/usr/sbin/hostapd
#abs_hostapd_conf=/system/script/rtl_hostapd_2G.conf
abs_hostapd_conf=/tmp/augentix_ap.conf
#abs_hostapd_conf=/etc/hostapd.conf

ifconfig wlan0 up
ifconfig wlan0 192.168.0.1
$abs_hostapd $abs_hostapd_conf -B

