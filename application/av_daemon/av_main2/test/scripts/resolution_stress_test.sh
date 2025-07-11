#!/bin/bash
i=0
case "$1" in
 help)
    echo "Usage: $0 {loop time} {ch0 width} {ch0 height} {ch1 width} {ch1 height}.."
    echo "please set dft resolution by chn"
    echo "$0 {help} help msg"
    exit 1
    ;;
 set)
    cmd="{\"video_strm_list\":[{\"width\": $2, \"height\": $3},{\"width\": $4, \"height\": $5}]}"
    echo $cmd
    ;;
 *)
    # how to diff 5M and 2M
    enc_num="$(cat /dev/enc | grep "Total Frames" |wc -l)"
    printf "Has $s enc \n" "$enc_num"
    while [ $i != $1 ]
        do
        printf "test %s time:\n" "$i"
        if [ $enc_num == 2 ]; then
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"width": 704, "height": 576},{"width": 1280, "height": 720}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Resolution")"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"width": 640, "height": 360},{"width": 1920, "height": 1080}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Resolution")"
            echo -------------------
            sleep 5
            # original resolution
            cmd="{\"video_strm_list\":[{\"width\": $2, \"height\": $3},{\"width\": $4, \"height\": $5}]}"
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "$cmd"
            echo -------------------
            echo "$(cat /dev/enc | grep "Resolution")"
            echo -------------------
            sleep 5
        elif [ $enc_num == 3 ]; then
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"width": 704, "height": 576},{"width": 1280, "height": 720}, {"width": 704, "height": 576}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Resolution")"
            echo -------------------
            sleep 5
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s '{"video_strm_list":[{"width": 640, "height": 360},{"width": 1920, "height": 1080},{"width": 640, "height": 360}]}'
            echo -------------------
            echo "$(cat /dev/enc | grep "Resolution")"
            echo -------------------
            sleep 5
            # original resolution
            cmd="{\"video_strm_list\":[{\"width\": $2, \"height\": $3},{\"width\": $4, \"height\": $5},{\"width\": $6, \"height\": $7}]}"
            /system/bin/ccclient -c AGTX_CMD_VIDEO_STRM_CONF -s "$cmd"
            echo -------------------
            echo "$(cat /dev/enc | grep "Resolution")"
            echo -------------------
            sleep 5
        fi

        i=$(($i+1))
        done
    exit 0
    ;;
esac
