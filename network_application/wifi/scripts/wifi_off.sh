#!/bin/sh

interface=wlan0

ps | grep udhcpc | grep -v grep | awk '{ print $1}' | xargs kill -9 $1
wpa_cli -i $interface terminate


