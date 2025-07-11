#!/bin/bash
i=0
case "$1" in
 help)
    echo "Usage: $0 {loop time} "
    echo "please set loop time"
    echo "$0 {help} help msg"
    exit 1
    ;;
 set)
    cmd="{\"video_strm_list\":[{\"strm_en\": 1},{\"strm_en\": 1}]}"
    echo $cmd
    ;;
 *)
    enc_num="$(cat /dev/enc | grep "Total Frames" |wc -l)"
    printf "Has $s enc \n" "$enc_num"
    while [ $i != $1 ]
    do
        if [ $enc_num == 2 ]; then
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "{\"video_strm_list\":[{\"strm_en\": 1},{\"strm_en\": 1}]}"
            echo -------------------
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "{\"video_strm_list\":[{\"strm_en\": 0},{\"strm_en\": 0}]}"
            echo -------------------
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "{\"video_strm_list\":[{\"strm_en\": 1},{\"strm_en\": 1}]}"
            echo -------------------
            echo -------------------
        elif [ $enc_num == 3 ]; then
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "{\"video_strm_list\":[{\"strm_en\": 1},{\"strm_en\": 1},{\"strm_en\": 0}]}"
            echo -------------------
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "{\"video_strm_list\":[{\"strm_en\": 1},{\"strm_en\": 0},{\"strm_en\": 0}]}"
            echo -------------------
            enc_num_tmp="$(cat /dev/enc | grep "Total Frames" |wc -l)"
            printf "Has $s enc \n" "$enc_num_tmp"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "{\"video_strm_list\":[{\"strm_en\": 1},{\"strm_en\": 1},{\"strm_en\": 1}]}"
            echo -------------------
            enc_num_tmp="$(cat /dev/enc | grep "Total Frames" |wc -l)"
            printf "Has $s enc \n" "$enc_num_tmp"
            echo -------------------
        fi
        i=$(($i+1))
    done
    ;;
esac
