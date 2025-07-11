#!/bin/sh

interface=wlan0
wpa_exist=0

# check if the wpa_supplicant is running
wpa_exist=$(ps | grep "wpa_supplicant -B" | grep -v grep | awk '{ print $1 }')
if [ ! -z $wpa_exist ]; then
   echo "[error] wpa_supplicant is running now. don't overwrite it."
   echo "please execute <wifi_off.sh> or <kill -9 pid> to terimate the running wpa_supplicant"
   exit
fi

# check if the command line is correct
if [ -z "$1" ]; then
   echo "[error] no input parameter"
   echo "usage: wifi_on.sh wpa_supplicant.conf" 
   exit
fi

# check if the wpa_supplicant.conf is exist
if [ -f "$1" ]; then
   conf=$1
else
   echo "[error] no such file:" $1
   exit
fi

# start wpa_suppliant to connect to wifi ap
wpa_supplicant -B -i $interface -c $conf

# clear the old ip-address and request ip-address from wifi-ap
# Do NOT modify the file path of PID file, otherwise connmngr will fail
udhcpc -n -i $interface -R -t 20 -T 2 -p /var/run/udhcpc.pid

ret=$?
if [ $ret != 0 ]; then
   echo "Start udhcpc again"
   udhcpc -b -i $interface -R -t 20 -p /var/run/udhcpc.pid
fi

# check if connection is established.
#wlan_ip=$(ifconfig $interface | awk '/inet addr/{print substr($2,6)}')
#if [ -z "$wlan_ip" ]; then
#   echo "[error] connect failure: don't get ip-address."
#   echo "please check if the settings of wpa_supplicant.conf is correct."
#else
#   echo "[finished] connect success: ip-address<$wlan_ip>"
#fi
