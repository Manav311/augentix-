#!/bin/bash
i=0
case "$1" in
 help)
    echo "Usage: $0 {loop time}"
    echo "$0 {help} help msg"
    exit 1
    ;;
 *)
    enc_num="$(cat /dev/enc | grep "Total Frames" |wc -l)"
    printf "Has $s enc \n" "$enc_num"
    while [ $i != $1 ]
        do
        printf "test %s time:\n" "$i"
        if [ $enc_num == 2 ]; then
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"venc_type":1},{"venc_type":0}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Codec")"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"venc_type":0},{"venc_type":1}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Codec")"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"venc_type":0},{"venc_type":0}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Codec")"
            echo -------------------
            sleep 5
        elif [ $enc_num == 3 ]; then
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"venc_type":1},{"venc_type":0},{"venc_type":1}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Codec")"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"venc_type":0},{"venc_type":1},{"venc_type":0}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Codec")"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"venc_type":0},{"venc_type":0},{"venc_type":0}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Codec")"
            echo -------------------
            sleep 5
        fi

        i=$(($i+1))
        done
    exit 0
    ;;
esac
