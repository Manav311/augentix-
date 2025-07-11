#!/bin/sh

###### Configurations #######

# Times of collecting raw images
SNAPSHOT_CNT=3
DO_RAW_SNAPSHOT=1
DO_ISW_SNAPSHOT=1

# Do the jpeg snapshot by using mpi_snapshot
DO_JPEG_SNAPSHOT=1
# Image quality of the jpeg snapshot, worst [0-100] best
JPEG_QUALITY=60

# Dump the registers with DIP running / disabled
DUMP_CURRENT_CSR=1
DUMP_DISABLE_CSR=0

# If some of the programs are not put under $PATH (e.g. /system/bin/)
# please set "EXTERN_PROGRAM_PATH" variable to the path where these programs are stored.
# Programs/scripts required: busybox, cmdsender, csr, ddr2pgm, dip_dump, dump, dump_csr, mpi_snapshot
PROGRAM_PATH="$EXTERN_PROGRAM_PATH"

#### Program redirection ####

LOG_FILE="/tmp/augentix.log"
LOG_ROTATE0="${LOG_FILE}.0"

if [ "$PROGRAM_PATH" ]; then
	CMDSENDER="${PROGRAM_PATH}/cmdsender"
	CSR="${PROGRAM_PATH}/csr"
	DDR2PGM="${PROGRAM_PATH}/ddr2pgm"
	MPI_SNAPSHOT="${PROGRAM_PATH}/mpi_snapshot"
	DUMP_CSR="${PROGRAM_PATH}/dump_csr"
	DIP_DUMP="${PROGRAM_PATH}/dip_dump"
	FILE="${PROGRAM_PATH}/file"
	TR="${PROGRAM_PATH}/busybox tr"
	AWK="${PROGRAM_PATH}/busybox awk"
	CAT="${PROGRAM_PATH}/busybox cat"
	ECHO="${PROGRAM_PATH}/busybox echo"
	SLEEP="${PROGRAM_PATH}/busybox sleep"
	SEQ="${PROGRAM_PATH}/busybox seq"
	USLEEP="${PROGRAM_PATH}/busybox usleep"
	DATE="${PROGRAM_PATH}/busybox date"
	MKDIR="${PROGRAM_PATH}/busybox mkdir"
	RM="${PROGRAM_PATH}/busybox rm"
	FIND="${PROGRAM_PATH}/busybox find"
	SORT="${PROGRAM_PATH}/busybox sort"
else
	CMDSENDER="cmdsender"
	CSR="csr"
	DDR2PGM="ddr2pgm"
	MPI_SNAPSHOT="mpi_snapshot"
	DUMP_CSR="/system/bin/dump_csr"
	DIP_DUMP="dip_dump"
	FILE="file"
	TR="tr"
	AWK="awk"
	CAT="cat"
	ECHO="echo"
	SLEEP="sleep"
	SEQ="seq"
	USLEEP="usleep"
	DATE="date"
	MKDIR="mkdir"
	RM="rm"
	FIND="find"
	SORT="sort"
fi

ret=0

if [ "$(command -v ${CMDSENDER})" -a -f "$(command -v ${CMDSENDER})" ]; then :
else
	$ECHO "Error: Cannot find cmdsender program!"
	ret=1
fi

if [ "$(command -v ${CSR})" -a -f "$(command -v ${CSR})" ]; then :
else
	$ECHO "Error: Cannot find csr program!"
	ret=1
fi

if [ "$(command -v ${DDR2PGM})" -a -f "$(command -v ${DDR2PGM})" ]; then :
else
	$ECHO "Error: Cannot find ddr2pgm program!"
	ret=1
fi

if [ "$(command -v ${MPI_SNAPSHOT})" -a -f "$(command -v ${MPI_SNAPSHOT})" ]; then :
else
	$ECHO "Error: Cannot find mpi_snapshot program!"
	ret=1
fi

if [ "$(command -v ${DUMP_CSR})" -a -f "$(command -v ${DUMP_CSR})" ]; then :
else
	$ECHO "Error: Cannot find dump_csr program!"
	ret=1
fi

if [ "$(command -v ${DIP_DUMP})" -a -f "$(command -v ${DIP_DUMP})" ]; then :
else
	$ECHO "Error: Cannot find dip_dump program!"
	ret=1
fi

if [ $ret = "1" ]; then
	exit 1
fi

###### Video settings #######

if [ "$SNAPSHOT_CNT" = 0 ]; then
	DO_RAW_SNAPSHOT=0
	DO_ISW_SNAPSHOT=0
fi

# Video type:
# 0 - Single sensor
# 1 - Dual sensors
# 2 - Dual sensors with stitching
# 3 - Single HDR sensor
# 4 - Triple sensors
# 5 - Quadruple sensors
vtype=$("$DIP_DUMP" vinfo.type | $AWK '/vinfo\.type/{print $3}')

# List of active encoder channels
enc_list=$($CAT /dev/enc | $TR '[]' ' ' | $AWK 'BEGIN { i=0 } /Channel/{ i=$2 } /State: RUNNING/{ print i }')

### Current CTRL settings ###

CTRL_DEFAULT0=$("$CMDSENDER" --dip 0 0 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
CTRL_TRAIL0=$($ECHO -n "$CTRL_DEFAULT0" | $AWK '{ $1=""; print }')
CTRL_TRAIL_DISABLE=$($ECHO -n "$CTRL_TRAIL0" | $TR '[:digit:]' '0')
CAL_DEFAULT0=$("$CMDSENDER" --cal 0 0 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
CAL_TRAIL0=$($ECHO -n "$CAL_DEFAULT0" | $AWK '{ $1=""; print }')
CAL_TRAIL_DISABLE=$($ECHO -n "$CAL_TRAIL0" | $TR '[:digit:]' '0')

if [ "$vtype" = 1 ]; then
	CTRL_DEFAULT1=$("$CMDSENDER" --dip 0 1 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CTRL_TRAIL1=$($ECHO -n "$CTRL_DEFAULT1" | $AWK '{ $1=""; print }')
fi
if [ "$vtype" = 1 -o "$vtype" = 2 ]; then
	CAL_DEFAULT1=$("$CMDSENDER" --cal 0 1 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CAL_TRAIL1=$($ECHO -n "$CAL_DEFAULT1" | $AWK '{ $1=""; print }')
fi
if [ "$vtype" = 4 ]; then
	CAL_DEFAULT1=$("$CMDSENDER" --cal 0 1 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CAL_TRAIL1=$($ECHO -n "$CAL_DEFAULT1" | $AWK '{ $1=""; print }')
	CAL_DEFAULT2=$("$CMDSENDER" --cal 0 2 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CAL_TRAIL2=$($ECHO -n "$CAL_DEFAULT2" | $AWK '{ $1=""; print }')
	CTRL_DEFAULT1=$("$CMDSENDER" --dip 0 1 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CTRL_TRAIL1=$($ECHO -n "$CTRL_DEFAULT1" | $AWK '{ $1=""; print }')
	CTRL_DEFAULT2=$("$CMDSENDER" --dip 0 2 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CTRL_TRAIL2=$($ECHO -n "$CTRL_DEFAULT2" | $AWK '{ $1=""; print }')
fi
if [ "$vtype" = 5 ]; then
	CAL_DEFAULT1=$("$CMDSENDER" --cal 0 1 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CAL_TRAIL1=$($ECHO -n "$CAL_DEFAULT1" | $AWK '{ $1=""; print }')
	CAL_DEFAULT2=$("$CMDSENDER" --cal 0 2 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CAL_TRAIL2=$($ECHO -n "$CAL_DEFAULT2" | $AWK '{ $1=""; print }')
	CAL_DEFAULT3=$("$CMDSENDER" --cal 0 3 | $AWK 'BEGIN{f = 0} /cal_en/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CAL_TRAIL3=$($ECHO -n "$CAL_DEFAULT3" | $AWK '{ $1=""; print }')
	CTRL_DEFAULT1=$("$CMDSENDER" --dip 0 1 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CTRL_TRAIL1=$($ECHO -n "$CTRL_DEFAULT1" | $AWK '{ $1=""; print }')
	CTRL_DEFAULT2=$("$CMDSENDER" --dip 0 2 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CTRL_TRAIL2=$($ECHO -n "$CTRL_DEFAULT2" | $AWK '{ $1=""; print }')
	CTRL_DEFAULT3=$("$CMDSENDER" --dip 0 3 | $AWK 'BEGIN{f = 0} /dip/{f = 1} {if (f == 1) print}' | $TR '\n' ' ' | $TR -cd '[:digit:] ')
	CTRL_TRAIL3=$($ECHO -n "$CTRL_DEFAULT3" | $AWK '{ $1=""; print }')
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

# Script version, do not change this value.
CD_VERSION="v2.2.0"

get_version () {
	$ECHO "collect data version: $CD_VERSION"
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

###### JPEG snapshots #######

# Notice that jpeg can't present the color correctly due to its color space
if [ "$DO_JPEG_SNAPSHOT" = 1 ]; then
	$ECHO "Collecting jpeg snapshots..."
	for i in $enc_list; do
		"$MPI_SNAPSHOT" jpeg2 1 "$i" "$JPEG_QUALITY" "${data_path}/strm_${i}.jpg" > /dev/null 2>&1
	done
fi

######## Get IQ log #########

# The commands are wrapped within a function to support dual paths
get_iq_log () {
	path_idx="$1"

	# Generate dip_info file and dip_extend file
	if [ "$path_idx" = 0 ]; then
		dip_info_file="${data_path}/dip_info.log"
		dip_extend_file="${data_path}/dip_extend.log"
	else
		dip_info_file="${data_path}/dip_info_${path_idx}.log"
		dip_extend_file="${data_path}/dip_extend_${path_idx}.log"
	fi
	:> "$dip_info_file"
	if [ $? != 0 ]; then
		$ECHO "Cannot create log file $dip_info_file."
		return
	fi

	######## Get DIP extend Information #########
	"$DIP_DUMP" -p "$path_idx" intl >> "$dip_extend_file"

	get_version >> "$dip_info_file"

	# Environment variables
	$ECHO -----[Export]----- >> "$dip_info_file"
	export >> "$dip_info_file"

	# Library information
	$ECHO -----[Library info]----- >> "$dip_info_file"
	libmpp_path=$($FIND "/proc" -maxdepth 2 -name "maps" -exec $AWK '/libmpp\.so/{ print $6 }' {} + 2> /dev/null | $SORT -u)
	if [ ! "$(command -v $FILE)" -o ! -f "$(command -v $FILE)" ]; then
		$ECHO "Cannot find 'file' program." >> "$dip_info_file"
	elif [ "$libmpp_path" ]; then
		"$FILE" $libmpp_path >> "$dip_info_file"
	else
		$ECHO "Failed to find libmpp" >> "$dip_info_file"
	fi

	# Luma statistics
	$ECHO -----[Collect lum1 stat]----- >> "$dip_info_file"
	"$DIP_DUMP" -p "$path_idx" stat.lum >> "$dip_info_file"
	$SLEEP 1

	# AWB statistics
	$ECHO -----[Collect awb stat]----- >> "$dip_info_file"
	"$DIP_DUMP" -p "$path_idx" stat.awb >> "$dip_info_file"
	$SLEEP 1

	# Shared variables within DIP
	$ECHO -----[Collect share info]----- >> "$dip_info_file"
	"$DIP_DUMP" -p "$path_idx" share >> "$dip_info_file"
	$SLEEP 1

	# Various attributes or infos
	dip_cmd_list="EXP WB TE_INFO CTRL CAL DBC DCC LSC ROI STAT AE AWB PTA CSM SHP SHP_V2 NR CORING GAMMA TE ISO ENH HDR_SYNTH FCS DHZ DMS"
	for cmd in $dip_cmd_list; do
		cmdsender_cmd=$($ECHO $cmd | $TR '[:upper:]' '[:lower:]')
		case "$cmd" in
		CTRL)
			cmdsender_cmd="dip"
			$ECHO "-----[${cmd}]-----" >> "$dip_info_file"
			"$CMDSENDER" --"$cmdsender_cmd" 0 "$path_idx" >> "$dip_info_file" 2> /dev/null || \
			$ECHO "Failed to fetch ${cmd} information." >> "$dip_info_file"
			;;
		STAT)
			cmdsender_cmd="stat_attr"
			$ECHO "-----[${cmd}]-----" >> "$dip_info_file"
			"$CMDSENDER" --"$cmdsender_cmd" 0 "$path_idx" >> "$dip_info_file" 2> /dev/null || \
			$ECHO "Failed to fetch ${cmd} information." >> "$dip_info_file"
			;;
		CAL|DBC|DCC|LSC|ROI)
			if [ "$vtype" = 2 ]; then
				# stitching, assumes it uses path 0 AND 1
				$ECHO "-----[${cmd}]-----" >> "$dip_info_file"
				"$CMDSENDER" --"$cmdsender_cmd" 0 0 >> "$dip_info_file" 2> /dev/null || \
				$ECHO "Failed to fetch ${cmd}0 information." >> "$dip_info_file"
				"$CMDSENDER" --"$cmdsender_cmd" 0 1 >> "$dip_info_file" 2> /dev/null || \
				$ECHO "Failed to fetch ${cmd}1 information." >> "$dip_info_file"
			else
				$ECHO "-----[${cmd}]-----" >> "$dip_info_file"
				"$CMDSENDER" --"$cmdsender_cmd" 0 "$path_idx" >> "$dip_info_file" 2> /dev/null || \
				$ECHO "Failed to fetch ${cmd} information." >> "$dip_info_file"
			fi
			;;
		SHP_V2)
			cmdsender_cmd="shpv2"
			$ECHO "-----[${cmd}]-----" >> "$dip_info_file"
			"$CMDSENDER" --"$cmdsender_cmd" 0 "$path_idx" >> "$dip_info_file" 2> /dev/null || \
			$ECHO "Failed to fetch ${cmd} information." >> "$dip_info_file"
			# win shp attr
			win_list=$("$DIP_DUMP" -p "$path_idx" vinfo.valid_win | $AWK -F "=" '/valid_win/{ $1=""; FS=" "; print; FS="=" }' | $TR -d '()' )
			for cw in $win_list; do
				chn=${cw%,*}
				win=${cw#*,}
				$ECHO "win_shp[$chn][$win].strength = "$("$CMDSENDER" --winshpv2 0 $chn $win | $AWK -F "=" '/^strength */{ print $2 }' | $AWK '{$1=$1};1') >> "$dip_info_file" 2> /dev/null
				$SLEEP 1
			done
			;;
		*)
			$ECHO "-----[${cmd}]-----" >> "$dip_info_file"
			"$CMDSENDER" --"$cmdsender_cmd" 0 "$path_idx" >> "$dip_info_file" 2> /dev/null || \
			$ECHO "Failed to fetch ${cmd} information." >> "$dip_info_file"
			;;
		esac
		$USLEEP 200000
	done

	# VENC / RC attributes
	$ECHO "-----[VENC_ATTR]-----" >> "$dip_info_file"
	for i in $enc_list; do
		"$CMDSENDER" --venc --enc_idx="$i" --get_vattr >> "$dip_info_file" 2> /dev/null || \
		$ECHO "Failed to fetch VENC_ATTR of channel ${i}" >> "$dip_info_file"
	done

	# VENC info
	$ECHO "-----[VENC_INFO]-----" >> "$dip_info_file"
	for i in $enc_list; do
		"$CMDSENDER" --venc_info "$i" >> "$dip_info_file" 2> /dev/null || \
		$ECHO "Failed to fetch VENC_INFO of channel ${i}" >> "$dip_info_file"
	done

	# Driver logs
	$ECHO "-----[IS driver info]-----" >> "$dip_info_file"
	$CAT /dev/is >> "$dip_info_file" || \
	$ECHO "Failed to fetch IS driver information" >> "$dip_info_file"

	$ECHO "-----[ISP driver info]-----" >> "$dip_info_file"
	$CAT /dev/isp >> "$dip_info_file" || \
	$ECHO "Failed to fetch ISP driver information" >> "$dip_info_file"

	$ECHO "-----[ENC driver info]-----" >> "$dip_info_file"
	$CAT /dev/enc >> "$dip_info_file" || \
	$ECHO "Failed to fetch ENC driver information" >> "$dip_info_file"

	$ECHO "-----[RC driver info]-----" >> "$dip_info_file"
	$CAT /dev/rc >> "$dip_info_file" || \
	$ECHO "Failed to fetch RC driver information" >> "$dip_info_file"

	# PCA table
	if [ "$path_idx" = 0 ]; then
		pca_file="${data_path}/pca_lut.log"
	else
		pca_file="${data_path}/pca_lut_${path_idx}.log"
	fi
	"$CMDSENDER" --pca_table 0 "$path_idx" > "$pca_file" 2> /dev/null

	# Dip Cfg
	if [ "$path_idx" = 0 ]; then
		cfg_file="${data_path}/dip_cfg.log"
	else
		cfg_file="${data_path}/dip_cfg_${path_idx}.log"
	fi
	:> "$cfg_file"
	if [ $? = 0 ]; then
		get_version >> "$cfg_file"
		"$DIP_DUMP" -p "$path_idx" cfg >> "$cfg_file"
	fi
}

$ECHO "Collecting DIP information..."
get_iq_log 0
if [ $vtype = 1 ]; then
	get_iq_log 1
fi

if [ $vtype = 4 ]; then
	get_iq_log 1
	get_iq_log 2
fi

if [ $vtype = 5 ]; then
	get_iq_log 1
	get_iq_log 2
	get_iq_log 3
fi

# VENC Extend
for i in $enc_list; do
	"$CMDSENDER" --venc_extend --get "$i" "${data_path}/venc_extend_${i}.ini"
done

###### Dump raw images ######

# dump pgm
if [ "$DO_RAW_SNAPSHOT" = 1 -o "$DO_ISW_SNAPSHOT" = 1 ]; then
	$ECHO "Dumping raw images, please do not quit this script until it's finished."
fi

# Image processed after IS
if [ "$DO_ISW_SNAPSHOT" = 1 ]; then
	# bypass dip and fetch IS out
	"$CMDSENDER" --dip 0 0 2 $CTRL_TRAIL0 >> /dev/null
	if [ "$vtype" = 1 ]; then
		"$CMDSENDER" --dip 0 1 2 $CTRL_TRAIL1 >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --dip 0 1 2 $CTRL_TRAIL1 >> /dev/null
		"$CMDSENDER" --dip 0 2 2 $CTRL_TRAIL2 >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --dip 0 1 2 $CTRL_TRAIL1 >> /dev/null
		"$CMDSENDER" --dip 0 2 2 $CTRL_TRAIL2 >> /dev/null
		"$CMDSENDER" --dip 0 3 2 $CTRL_TRAIL3 >> /dev/null
	fi
	$SLEEP 1

	# Workaround: drop 1 snapshot in case the settings have not taken effect
	"$DDR2PGM" -m ISW-WP0 "/tmp/.pgm_dump" >> /dev/null
	if [ "$vtype" != 0 ]; then
		"$DDR2PGM" -m ISW-WP1 "/tmp/.pgm_dump" >> /dev/null
	fi
	$RM -f "/tmp/.pgm_dump"

	for i in $($SEQ 0 $((${SNAPSHOT_CNT} - 1)) ); do
		case "$vtype" in
		0) # single
			pgm_dir="$data_path/004.000.000.${pattern_name}_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_name" >> /dev/null
			;;
		1) # dual
			pgm_dir="$data_path/004.000.000.${pattern_name}_sns0_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p0_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_p0_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns1_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p1 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p1_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_p1_name" >> /dev/null
			;;
		2) # stitching
			pgm_dir="$data_path/004.000.000.${pattern_name}_stitch_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_name" >> /dev/null
			;;
		3) # single HDR
			pgm_dir="$data_path/004.000.000.${pattern_name}_hdr_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_se_name=$(printf '%08d_bayer_se.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_se_name" >> /dev/null
			pgm_le_name=$(printf '%08d_bayer_le.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_le_name" >> /dev/null
			;;
		4) # triple
			pgm_dir="$data_path/004.000.000.${pattern_name}_sns0_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p0_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_p0_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns1_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p1 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p1_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_p1_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns2_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p2 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p2_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP2 "$pgm_dir/$pgm_p2_name" >> /dev/null
			;;
		5) # quadruple
			pgm_dir="$data_path/004.000.000.${pattern_name}_sns0_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p0_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_p0_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns1_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p1 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p1_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_p1_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns2_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p2 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p2_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP2 "$pgm_dir/$pgm_p2_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns3_isw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p3 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p3_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP3 "$pgm_dir/$pgm_p3_name" >> /dev/null
			;;
		esac
		$SLEEP 1
	done
fi

# Raw sensor image
if [ "$DO_RAW_SNAPSHOT" = 1 ]; then
	# disable every module and fetch sensor raw
	"$CMDSENDER" --dip 0 0 1 $CTRL_TRAIL_DISABLE >> /dev/null
	if [ "$vtype" = 1 ]; then
		"$CMDSENDER" --dip 0 1 1 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --dip 0 1 1 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 2 1 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --dip 0 1 1 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 2 1 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 3 1 $CTRL_TRAIL_DISABLE >> /dev/null
	fi

	"$CMDSENDER" --cal 0 0 1 $CAL_TRAIL_DISABLE >> /dev/null
	if [ "$vtype" = 1 -o "$vtype" = 2 ]; then
		"$CMDSENDER" --cal 0 1 1 $CAL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --cal 0 1 1 $CAL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --cal 0 2 1 $CAL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --cal 0 1 1 $CAL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --cal 0 2 1 $CAL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --cal 0 3 1 $CAL_TRAIL_DISABLE >> /dev/null
	fi
	$SLEEP 1

	# Workaround: drop 1 snapshot in case the settings have not taken effect
	"$DDR2PGM" -m ISW-WP0 "/tmp/.pgm_dump" >> /dev/null
	if [ "$vtype" != 0 ]; then
		"$DDR2PGM" -m ISW-WP1 "/tmp/.pgm_dump" >> /dev/null
	fi
	$RM -f "/tmp/.pgm_dump"

	for i in $($SEQ 0 $((${SNAPSHOT_CNT} - 1)) ); do
		case "$vtype" in
		0) # single
			pgm_dir="$data_path/004.000.000.${pattern_name}_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_name" >> /dev/null
			;;
		1) # dual
			pgm_dir="$data_path/004.000.000.${pattern_name}_sns0_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p0_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_p0_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns1_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p1 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p1_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_p1_name" >> /dev/null
			;;
		2) # stitching
			pgm_dir="$data_path/004.000.000.${pattern_name}_stitch_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_name" >> /dev/null
			;;
		3) # single HDR
			pgm_dir="$data_path/004.000.000.${pattern_name}_hdr_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_se_name=$(printf '%08d_bayer_se.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_se_name" >> /dev/null
			pgm_le_name=$(printf '%08d_bayer_le.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_le_name" >> /dev/null
			;;
		4) # triple
			pgm_dir="$data_path/004.000.000.${pattern_name}_sns0_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p0_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_p0_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns1_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p1 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p1_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_p1_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns2_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p2 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p2_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP2 "$pgm_dir/$pgm_p2_name" >> /dev/null
			;;
		5) # quadruple
			pgm_dir="$data_path/004.000.000.${pattern_name}_sns0_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p0 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p0_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP0 "$pgm_dir/$pgm_p0_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns1_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p1 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p1_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP1 "$pgm_dir/$pgm_p1_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns2_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p2 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p2_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP2 "$pgm_dir/$pgm_p2_name" >> /dev/null

			pgm_dir="$data_path/004.000.000.${pattern_name}_sns3_raw"
			$MKDIR -p "$pgm_dir"
			bayer=$("$DIP_DUMP" -p3 vinfo.bayer | $AWK '/vinfo\.bayer/{print $3}')
			pgm_p3_name=$(printf '%08d_bayer.pgm' ${i})
			"$DDR2PGM" -m "-p${bayer}" ISW-WP3 "$pgm_dir/$pgm_p3_name" >> /dev/null
			;;
		esac
		$SLEEP 1
	done
fi

# Enable DIP and CAL modules after dumping raw images
if [ "$DO_RAW_SNAPSHOT" = 1 -o "$DO_ISW_SNAPSHOT" = 1 ]; then
	"$CMDSENDER" --cal 0 0 $CAL_DEFAULT0 >> /dev/null
	if [ "$vtype" = 1 -o "$vtype" = 2 ]; then
		"$CMDSENDER" --cal 0 1 $CAL_DEFAULT1 >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --cal 0 1 $CAL_DEFAULT1 >> /dev/null
		"$CMDSENDER" --cal 0 2 $CAL_DEFAULT2 >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --cal 0 1 $CAL_DEFAULT1 >> /dev/null
		"$CMDSENDER" --cal 0 2 $CAL_DEFAULT2 >> /dev/null
		"$CMDSENDER" --cal 0 3 $CAL_DEFAULT3 >> /dev/null
	fi
	"$CMDSENDER" --dip 0 0 $CTRL_DEFAULT0 >> /dev/null
	if [ "$vtype" = 1 ]; then
		"$CMDSENDER" --dip 0 1 $CTRL_DEFAULT1 >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --dip 0 1 $CTRL_DEFAULT1 >> /dev/null
		"$CMDSENDER" --dip 0 2 $CTRL_DEFAULT2 >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --dip 0 1 $CTRL_DEFAULT1 >> /dev/null
		"$CMDSENDER" --dip 0 2 $CTRL_DEFAULT2 >> /dev/null
		"$CMDSENDER" --dip 0 3 $CTRL_DEFAULT3 >> /dev/null
	fi
	$ECHO "Finished dumping raw images."
fi

######### Dump csr ##########

# Dump normal csr
if [ "$DUMP_CURRENT_CSR" = 1 ]; then
	"$CMDSENDER" --dip 0 0 2 $CTRL_TRAIL_DISABLE >> /dev/null
	if [ "$vtype" = 1 ]; then
		"$CMDSENDER" --dip 0 1 2 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --dip 0 1 2 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 2 2 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --dip 0 1 2 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 2 2 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 3 2 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	$ECHO "Dumping system registers..."
	$MKDIR -p "$data_path/current_csr/bin"
	"$DUMP_CSR" $data_path/current_csr/bin/
fi

# Dump DIP disabled csr
if [ "$DUMP_DISABLE_CSR" = 1 ]; then
	"$CMDSENDER" --dip 0 0 1 $CTRL_TRAIL_DISABLE >> /dev/null
	if [ "$vtype" = 1 ]; then
		"$CMDSENDER" --dip 0 1 1 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --dip 0 1 1 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 2 1 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --dip 0 1 1 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 2 1 $CTRL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --dip 0 3 1 $CTRL_TRAIL_DISABLE >> /dev/null
	fi
	"$CMDSENDER" --cal 0 0 1 $CAL_TRAIL_DISABLE >> /dev/null
	if [ "$vtype" = 1 -o "$vtype" = 2 ]; then
		"$CMDSENDER" --cal 0 1 1 $CAL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 4 ]; then
		"$CMDSENDER" --cal 0 1 1 $CAL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --cal 0 2 1 $CAL_TRAIL_DISABLE >> /dev/null
	fi
	if [ "$vtype" = 5 ]; then
		"$CMDSENDER" --cal 0 1 1 $CAL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --cal 0 2 1 $CAL_TRAIL_DISABLE >> /dev/null
		"$CMDSENDER" --cal 0 3 1 $CAL_TRAIL_DISABLE >> /dev/null
	fi
	$SLEEP 1
	$ECHO "Dumping system registers with DIP disabled..."
	$MKDIR -p "${data_path}/disable_csr/bin"
	"$DUMP_CSR" "${data_path}/disable_csr/bin/"
fi

# Restore settings
"$CMDSENDER" --cal 0 0 $CAL_DEFAULT0 >> /dev/null
if [ "$vtype" = 1 -o "$vtype" = 2 ]; then
	"$CMDSENDER" --cal 0 1 $CAL_DEFAULT1 >> /dev/null
fi
if [ "$vtype" = 4 ]; then
	"$CMDSENDER" --cal 0 1 $CAL_DEFAULT1 >> /dev/null
	"$CMDSENDER" --cal 0 2 $CAL_DEFAULT2 >> /dev/null
fi
if [ "$vtype" = 5 ]; then
	"$CMDSENDER" --cal 0 1 $CAL_DEFAULT1 >> /dev/null
	"$CMDSENDER" --cal 0 2 $CAL_DEFAULT2 >> /dev/null
	"$CMDSENDER" --cal 0 3 $CAL_DEFAULT3 >> /dev/null
fi
"$CMDSENDER" --dip 0 0 $CTRL_DEFAULT0 >> /dev/null
if [ "$vtype" = 1 ]; then
	"$CMDSENDER" --dip 0 1 $CTRL_DEFAULT1 >> /dev/null
fi
if [ "$vtype" = 4 ]; then
	"$CMDSENDER" --dip 0 1 $CTRL_DEFAULT1 >> /dev/null
	"$CMDSENDER" --dip 0 2 $CTRL_DEFAULT2 >> /dev/null
fi
if [ "$vtype" = 5 ]; then
	"$CMDSENDER" --dip 0 1 $CTRL_DEFAULT1 >> /dev/null
	"$CMDSENDER" --dip 0 2 $CTRL_DEFAULT2 >> /dev/null
	"$CMDSENDER" --dip 0 3 $CTRL_DEFAULT3 >> /dev/null
fi
#############################

$ECHO "Has finished collecting data to $data_path/"
