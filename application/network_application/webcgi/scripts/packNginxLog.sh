#! /bin/sh
#
# packNginxLog.sh
# Copyright (C) 2024 Zack Yang
#
# Distributed under terms of the MIT license.

export LD_LIBRARY_PATH=/system/lib:$LD_LIBRARY_PATH

OUTPUT_FILE="/tmp/nginx/logs_archive/log.tgz"

mkdir -p /tmp/nginx/logs_archive

tar -czf "$OUTPUT_FILE" /tmp/nginx/error.log /tmp/nginx/access.log

exit 1
