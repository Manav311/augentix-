#!/bin/sh

# If some of the programs are not put under $PATH (e.g. /system/bin/)
# please set "EXTERN_PROGRAM_PATH" variable to the path where these programs are stored.
# Programs/scripts required: busybox, cmdsender, csr, ddr2pgm, dip_dump, dump, dump_csr, mpi_snapshot
PROGRAM_PATH="$EXTERN_PROGRAM_PATH"

playback_file=audio_patterns/play.wav

#### Program redirection ####

LOG_FIlE="/tmp/augentix.log"
LOG_ROTATE0="${LOG_FILE}.0"

if [ "$PROGRAM_PATH" ]; then
	ARECORD="${PROGRAM_PATH}/arecord"
	APLAY="${PROGRAM_PATH}/aplay"
	AMIXER="${PROGRAM_PATH}/amixer"
	CSR="${PROGRAM_PATH}/csr"
	DUMP="${PROGRAM_PATH}/dump"
	DUMP_CSR="${PROGRAM_PATH}/dump_csr"
	AWK="${PROGRAM_PATH}/busybox awk"
	CAT="${PROGRAM_PATH}/busybox cat"
	DATE="${PROGRAM_PATH}/busybox date"
	ECHO="${PROGRAM_PATH}/busybox echo"
	GREP="${PROGRAM_PATH}/busybox grep"
	PRINTF="${PROGRAM_PATH}/busybox printf"
	SLEEP="${PROGRAM_PATH}/busybox sleep"
	MKDIR="${PROGRAM_PATH}/busybox mkdir"
else
	ARECORD="arecord"
	APLAY="aplay"
	AMIXER="amixer"
	CSR="csr"
	DUMP="dump"
	DUMP_CSR="dump_csr"
	AWK="awk"
	CAT="cat"
	DATE="date"
	ECHO="echo"
	GREP="grep"
	PRINTF="printf"
	SLEEP="sleep"
	MKDIR="mkdir"
fi

ret=0

if [ "$(command -v ${ARECORD})" -a -f "$(command -v ${ARECORD})" ]; then :
else
	$ECHO "Error: Cannot find arecord program!"
	ret=1
fi

if [ "$(command -v ${APLAY})" -a -f "$(command -v ${APLAY})" ]; then :
else
	$ECHO "Error: Cannot find aplay program!"
	ret=1
fi

if [ "$(command -v ${AMIXER})" -a -f "$(command -v ${AMIXER})" ]; then :
else
	$ECHO "Error: Cannot find amixer program!"
	ret=1
fi

if [ "$(command -v ${CSR})" -a -f "$(command -v ${CSR})" ]; then :
else
	$ECHO "Error: Cannot find csr program!"
	ret=1
fi

if [ "$(command -v ${DUMP_CSR})" -a -f "$(command -v ${DUMP_CSR})" ]; then :
else
	$ECHO "Error: Cannot find dump_csr program!"
	ret=1
fi

if [ -f ${playback_file} ]; then :
else
	$ECHO "Warning: Cannot find playback file ${playback_file}, collect recode data only."
fi

if [ $ret = "1" ]; then
	exit 1
fi

########### Help ############

if [ $# = 0 ]; then
	$ECHO "Usage: $0 path [pattern]"
	$ECHO "path - a subdirectory will be created under this path for collecting the data"
	$ECHO "pattern - name for the pattern, will use current time of the machine if not specified"
	$ECHO "Eg. $0 /mnt/nfs/ethnfs/data"
	$ECHO "    $0 /mnt/sdcard/data laboratory"
	exit 0
fi

##### Helper functions ######

CAD_VERSION="v1.0.0"

get_version () {
	$ECHO "collect audio data version: $CAD_VERSION"
}

########### Main ############

# Create data directory
store_path="$1"

if [ "$2" ]; then
	pattern_name="$2"
else
	timestamp=$($DATE +%Y-%m-%d.%H-%M-%S )
	pattern_name="data-${timestamp}"
fi


data_path="${store_path}/${pattern_name}"

if [ -f "$data_path" -o -d "$data_path" ]; then
	$ECHO "Path '${data_path}' exists."
	$ECHO "Please specify another path or pattern name,"
	$ECHO "or remove the existing file/directory and try again."
	exit 1
fi

$MKDIR -p "$data_path"

rate_record=$($GREP dsnoop -A7 /etc/asound.conf | $AWK '/rate/{print $2}')
rate_playback=$($GREP dmix -A7 /etc/asound.conf | $AWK '/rate/{print $2}')

dump_raw_audio () {
	postfix="$1"

	if [ $rate_record -eq 16000 ]; then
		samples_hex=28000
	else
		samples_hex=14000
	fi
	samples_dec=$((0x${samples_hex}))

	$ECHO "DBGDMP $samples_hex" > /proc/audio_dsp

	delay=$(((${samples_dec} + 8000) / 8000))

	$SLEEP $delay

	raw_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $3}')
	anr_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $5}')
	buf_plb_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $7}')
	aec0_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $9}')
	aec1_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $11}')
	anr2_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $13}')
	anr1_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $15}')
	gain_out_addr=$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $17}')

	raw_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $4}') * 2)))
	anr_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $6}') * 2)))
	buf_plb_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $8}') * 2)))
	aec0_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $10}') * 2)))
	aec1_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $12}') * 2)))
	anr2_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $14}') * 2)))
	anr1_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $16}') * 2)))
	gain_out_size=$($PRINTF '%x' $((0x$($CAT /proc/audio_dsp | $AWK '$0 ~ /DBGDMP/{print $18}') * 2)))

	$ECHO "$DUMPing $((0x${raw_out_size} / 2)) samples to ${data_path}/00_raw_out_${postfix}.bin..."
	$DUMP 0x$raw_out_addr 0x$raw_out_size "${data_path}/00_raw_out_${postfix}.bin"
	$ECHO "$DUMPing $((0x${anr_out_size} / 2)) samples to ${data_path}/10_anr_out_${postfix}.bin..."
	$DUMP 0x$anr_out_addr 0x$anr_out_size "${data_path}/10_anr_out_${postfix}.bin"
	$ECHO "$DUMPing $((0x${buf_plb_size} / 2)) samples to ${data_path}/FF_buf_plb_${postfix}.bin..."
	$DUMP 0x$buf_plb_addr 0x$buf_plb_size "${data_path}/FF_buf_plb_${postfix}.bin"
	$ECHO "$DUMPing $((0x${aec0_out_size} / 2)) samples to ${data_path}/20_aec0_out_${postfix}.bin..."
	$DUMP 0x$aec0_out_addr 0x$aec0_out_size "${data_path}/20_aec0_out_${postfix}.bin"
	$ECHO "$DUMPing $((0x${aec1_out_size} / 2)) samples to ${data_path}/30_aec1_out_${postfix}.bin..."
	$DUMP 0x$aec1_out_addr 0x$aec1_out_size "${data_path}/30_aec1_out_${postfix}.bin"
	$ECHO "$DUMPing $((0x${anr2_out_size} / 2)) samples to ${data_path}/40_anr2_out_${postfix}.bin..."
	$DUMP 0x$anr2_out_addr 0x$anr2_out_size "${data_path}/40_anr2_out_${postfix}.bin"
	$ECHO "$DUMPing $((0x${anr1_out_size} / 2)) samples to ${data_path}/50_anr1_out_${postfix}.bin..."
	$DUMP 0x$anr1_out_addr 0x$anr1_out_size "${data_path}/50_anr1_out_${postfix}.bin"
	$ECHO "$DUMPing $((0x${gain_out_size} / 2)) samples to ${data_path}/60_gain_out_${postfix}.bin..."
	$DUMP 0x$gain_out_addr 0x$gain_out_size "${data_path}/60_gain_out_${postfix}.bin"
}

get_audio_log () {
	postfix="$1"

	audio_info_file="${data_path}/audio_info_${postfix}.log"

	$ECHO "-----[audio_dsp]-----" >> "$audio_info_file"
	$CAT /proc/audio_dsp >> "$audio_info_file" || \
		$ECHO "Failed to get audio_dsp information" >> "$audio_info_file"
	$ECHO "-----[asound.conf]-----" >> "$audio_info_file"
	$CAT /etc/asound.conf >> "$audio_info_file" || \
		$ECHO "Failed to get asound.conf information" >> "$audio_info_file"
	$ECHO "-----[amixer]-----" >> "$audio_info_file"
	$AMIXER >> $audio_info_file || \
		$ECHO "Failed to get amixer information" >> "$audio_info_file"
}

$ECHO "Collecting Audio information..."

# 1. Origin Setting
##### 1-1. Recording audio date #####

$ARECORD -f s16_le -c 1 -r $rate_record -d 20 ${data_path}/record_origin.wav &

##### 1-2. Playing audio date #######

if [ -f ${playback_file} ]; then :
	$SLEEP 3
	$APLAY $playback_file &
fi

##### 1-3. Dump RAW audio data and info######

dump_raw_audio origin
get_audio_log origin

# 2. Disable audio features setting

$AMIXER sset 'AGC Enable' nocap
$AMIXER sset 'FANR Enable' nocap
$AMIXER sset 'LPF1 Enable' nocap
$AMIXER sset 'LPF2 Enable' nocap
$AMIXER sset 'TANR Enable' nocap
$ECHO "AEC 0" > /proc/audio_dsp
$ECHO "AEC1 0" > /proc/audio_dsp

##### 2-1. Recording audio date #####

$ARECORD -f s16_le -c 1 -r $rate_record -d 20 ${data_path}/record_disable_sw.wav &

##### 2-2. Playing audio date #######

if [ -f ${playback_file} ]; then :
	$SLEEP 3
	$APLAY $playback_file &
fi

##### 2-3. dump raw audio data and info ######

dump_raw_audio disable_sw
get_audio_log disable_sw

###################################

$ECHO "Audio data collected to $data_path/"
