#!/bin/sh

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/root/bin:/system/bin

# set video streaming client IP
CLIENT_IP=192.168.10.xx

do_aplay () {
	for i in $(seq 0 1000)
	do
		aplay -t raw -f S16_LE -r 48000 /tmp/test.raw > /dev/null 2>&1
		sleep 1
	done
}

if [ "$CLIENT_IP" == "192.168.10.xx" ]
then
    echo "==============================="
    echo "Please set CLIENT_IP in slt.sh"
    echo "==============================="
    exit
else
	true
fi

# remount /usrdata to avoid data broken
mount -o remount,ro /usrdata

# verify eFuse
/usrdata/efuse_verify.sh

# load mpp drivers for video and audio
/system/mpp/script/load_mpp.sh -i

# reset ADC workaround
csr ADOADC_CTR.ADCCTR.ENV_INIT 1

ADC_OUT_MON_0=`csr ADOADC_CFG.ADCCFG.ADC_OUT_MON`
ADC_OUT_MON_1=`csr ADOADC_CFG.ADCCFG.ADC_OUT_MON`

for i in $(seq 0 10)
do
	if [ $i -eq 10 ]
	then
        echo "==============================="
        echo "FAIL: unable to reset ADC."
        echo "==============================="
        sleep 10000
	else
		if [ "$ADC_OUT_MON_0" == "$ADC_OUT_MON_1" ]
		then
			csr ADOADC_CTR.ADCCTR.ENV_INIT 1
			ADC_OUT_MON_1=`csr ADOADC_CFG.ADCCFG.ADC_OUT_MON`
		else
			break
		fi
	fi
done

# test audio input
arecord -t raw -f S16_LE -r 48000 -d 5 /tmp/test.raw > /dev/null 2>&1 &

sleep 3

# test video streaming with ethernet
mpi_stream -d /usrdata/case_config_slt_1m_single_sensor > /dev/null 2>&1 &

sleep 3

testOnDemandRTSPServer 0 -n 2>&1 | grep URL &

# check video frame rate and also test audio output at the same time
sleep 1
echo "==============================="
cat /dev/is | grep "^Frame"
echo "==============================="
sleep 10
cat /dev/is | grep "^Frame"
echo "==============================="

# test usb
/usrdata/slt_usb.sh

# test sd card
/usrdata/slt_sdc.sh

do_aplay &
