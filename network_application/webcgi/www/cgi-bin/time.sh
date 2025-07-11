#!/bin/sh
export TZ=$(cat /etc/TZ)
date -s $1

