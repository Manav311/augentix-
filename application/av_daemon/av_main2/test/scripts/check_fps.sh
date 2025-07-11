#!/bin/sh
if [ $1 == '-h' ]
then
    echo " Usage: {driver} : {prev frame number} - {after 10 sec frame number}, FPS:{fps in 10 sec}"
    echo "-h help msg"
    echo "-s FPS {driver}:{fps}... short format"
exit 0
fi

while [ true ]
do
    format=$1
    # is prev frames
    is_prev="$(cat /dev/is | grep "Frame Count")"
    is_prev=$(echo $is_prev | tr -dc '0-9')
    # isp prev frames
    isp_prev="$(cat /dev/isp | grep "Frame Count")"
    isp_prev=$(echo $isp_prev | tr -dc '0-9')
    # enc prev frames
    enc_total_frames="$(cat /dev/enc | grep "Total Frames")"
    idx=0
    set -- $enc_total_frames
    while [ -n "$1" ]; do
        if [ $idx == 2 ]
        then
            enc_prev_0=$1
        elif [ $idx == 5 ]
        then
            enc_prev_1=$1
        elif [ $idx == 8 ]
        then
            enc_prev_2=$1
        elif [ $idx == 11 ]
        then
            enc_prev_3=$1
        fi

        idx=$(($idx+1))
        shift
    done

    enc_num="$(cat /dev/enc | grep "Total Frames" |wc -l)"
    if [ $enc_num == 0 ]
    then
        echo has no enc Running ...
        exit 1
    fi

    printf "has %s enc\n"  "$enc_num"

    sleep 10

    # is cur frames
    is_cur="$(cat /dev/is | grep "Frame Count")"
    is_cur=$(echo $is_cur | tr -dc '0-9')
    # isp cur frames
    isp_cur="$(cat /dev/isp | grep "Frame Count")"
    isp_cur=$(echo $isp_cur | tr -dc '0-9')
    # enc cur frames
    enc_total_frames="$(cat /dev/enc | grep "Total Frames")"
    idx=0
    set -- $enc_total_frames
    while [ -n "$1" ]; do
        if [ $idx == 2 ]
        then
            enc_cur_0=$1
        elif [ $idx == 5 ]
        then
            enc_cur_1=$1
        elif [ $idx == 8 ]
        then
            enc_cur_2=$1
        elif [ $idx == 11 ]
        then
            enc_cur_3=$1
        fi

        idx=$(($idx+1))
        shift
    done

    if [ $format == '-s' ]
    then
        is_fps=$(echo "scale=2;($is_cur-$is_prev)/10"| bc)
        isp_fps=$(echo "scale=2;($isp_cur-$isp_prev)/10"| bc)
        printf "FPS is:%s,isp:%s," "$is_fps" "$isp_fps"
        for i in `seq 0 $(($enc_num-1))`
        do
            if [ $i == 0 ]
            then
                prev=$enc_prev_0
                cur=$enc_cur_0
            elif [ $i == 1 ]
            then
                prev=$enc_prev_1
                cur=$enc_cur_1
            elif [ $i == 2 ]
            then
                prev=$enc_prev_2
                cur=$enc_cur_2
            elif [ $i == 3 ]
            then
                prev=$enc_prev_3
                cur=$enc_cur_3
            fi
            enc_fps=$(echo "scale=2;($cur-$prev)/10"| bc)
            printf "enc%s: %s " "$i" "$enc_fps"
        done
            printf "\n"

    else
        printf "IS :%s - %s, FPS:" "$is_prev" "$is_cur"
        echo "scale=2;($is_cur-$is_prev)/10"| bc
        printf "ISP :%s - %s, FPS:" "$isp_prev" "$isp_cur"
        echo "scale=2;($isp_cur-$isp_prev)/10"| bc

        for i in `seq 0 $(($enc_num-1))`
        do
            if [ $i == 0 ]
            then
                prev=$enc_prev_0
                cur=$enc_cur_0
            elif [ $i == 1 ]
            then
                prev=$enc_prev_1
                cur=$enc_cur_1
            elif [ $i == 2 ]
            then
                prev=$enc_prev_2
                cur=$enc_cur_2
            elif [ $i == 3 ]
            then
                prev=$enc_prev_3
                cur=$enc_cur_3
            fi
            printf "ENC %s :%s - %s, FPS:" "$i" "$prev" "$cur"
            echo "scale=2;($cur-$prev)/10"| bc
        done
    fi
done