#!/bin/sh
export TZ=$(cat /etc/TZ)
date -u -s $1

