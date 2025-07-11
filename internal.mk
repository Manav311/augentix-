include $(SDKSRC_DIR)/build/sdksrc.mk

ifeq ($(CONFIG_UCLIBC),y)
XTOOL_PATH := $(XTOOL_PATH_2)
CROSS_COMPILE := $(CROSS_COMPILE_2)
endif

include $(SDKSRC_DIR)/build/build_flags.mk

APP_LIB = $(APP_PATH)/library/output

AVMAIN2_PATH = $(APP_PATH)/av_daemon/av_main2/build

CONNSEL_PATH = $(APP_PATH)/network_application/connsel
EAIF_SERV_PATH = $(APP_PATH)/network_application/eaif_server
EAIF_SERV_BUILD_PATH = $(EAIF_SERV_PATH)/build
ONVIF_SERVER_PATH = $(APP_PATH)/network_application/onvif_server
IPASSIGN_PATH = $(APP_PATH)/network_application/ip_assign
STAMEN_PATH = $(APP_PATH)/network_application/stamen
UNICORN_PATH = $(APP_PATH)/network_application/unicorn
WEBCGI_PATH = $(APP_PATH)/network_application/webcgi
SE_AUTH_PATH = $(APP_PATH)/network_application/se_auth/build
WIFI_PATH= $(APP_PATH)/network_application/wifi
WS_DISCOVERY_PATH = $(APP_PATH)/network_application/ws_discovery

# ONVIF_DISCOVERY_PATH is deprecated.
# Please use WS_DISCOVERY_PATH instead.
ONVIF_DISCOVERY_PATH = $(WS_DISCOVERY_PATH)

ACTL_PATH = $(APP_PATH)/sample/actl
LSD_DEMO_PATH = $(APP_PATH)/sample/lsd_demo
SD_DEMO_PATH = $(APP_PATH)/sample/sd_demo
G726_PATH = $(APP_PATH)/sample/g726
HELLO_APP_PATH = $(APP_PATH)/sample/hello_app
AROI_DEMO_PATH = $(APP_PATH)/sample/aroi_demo
BM_DEMO_PATH = $(APP_PATH)/sample/bm_demo
DD_DEMO_PATH = $(APP_PATH)/sample/dd_demo
DK_DEMO_PATH = $(APP_PATH)/sample/dk_demo
DFB_DEMO_PATH = $(APP_PATH)/sample/dfb_demo
EF_DEMO_PATH = $(APP_PATH)/sample/ef_demo
FLD_DEMO_PATH = $(APP_PATH)/sample/fld_demo
HD_DEMO_PATH = $(APP_PATH)/sample/hd_demo
EAI_HD_DEMO_PATH = $(APP_PATH)/sample/eai_hd_demo
IR_DEMO_PATH = $(APP_PATH)/sample/ir_demo
MD_DEMO_PATH = $(APP_PATH)/sample/md_demo
OD_DEMO_PATH = $(APP_PATH)/sample/od_demo
PFM_DEMO_PATH = $(APP_PATH)/sample/pfm_demo
TD_DEMO_PATH = $(APP_PATH)/sample/td_demo
GTK3_DEMO_GUI_PATH = $(APP_PATH)/sample/gtk3_demo_gui
OSD_DEMO_PATH = $(APP_PATH)/sample/osd_demo
AUTO_TRACKING_PATH = $(APP_PATH)/sample/auto_tracking
FACEDET_DEMO_PATH = $(APP_PATH)/sample/facedet_demo
FACERECO_DEMO_PATH = $(APP_PATH)/sample/facereco_demo
VFTR_DUMP_PATH = $(APP_PATH)/sample/vftr_dump
LPW_PATH= $(APP_PATH)/sample/lpw_controller
LPW_SUPP_PATH= $(APP_PATH)/sample/lpw_supplicant
HIBER_DEMO_PATH = $(APP_PATH)/sample/hiber_demo
LPW_FW_UPG_PATH = $(APP_PATH)/sample/lpw_fw_upg
LPWIO_PATH = $(APP_PATH)/sample/lpwio
SECURE_ELEMENT_DEMO_PATH = $(APP_PATH)/sample/secure_element_demo
MP4_RECORDER_PATH = $(APP_PATH)/sample/mp4_recorder
SECURE_OPENSSL_DEMO_PATH = $(APP_PATH)/sample/secure_openssl_demo

CENTCTRL_PATH = $(APP_PATH)/setting_mgmt/centctrl
CENTCTRL_INC = $(CENTCTRL_PATH)/include
DBMONITOR_PATH = $(APP_PATH)/setting_mgmt/dbmonitor
SQLAPP_PATH = $(APP_PATH)/setting_mgmt/sqlapp
SQLAPP_INC = $(SQLAPP_PATH)/include

APP_INC = $(APP_PATH)/library/libagtx/include
LIBAGTX_PATH = $(APP_PATH)/library/libagtx
LIBAGTX_INC = $(APP_INC)
LIBAGTX_LIB = $(APP_PATH)/library/libagtx/lib
LIVE555_PATH = $(APP_PATH)/streaming_server/live555
FLV_PATH = $(APP_PATH)/streaming_server/flv_server

ACCESS_MODE_PATH = $(APP_PATH)/system_utility/access_mode
ALARMOUT_PATH = $(APP_PATH)/system_utility/alarm_out
APPVERIFY_PATH = $(APP_PATH)/system_utility/appverify
DAY_NIGHT_MODE_PATH = $(APP_PATH)/system_utility/day_night_mode
EVENTD_PATH = $(APP_PATH)/system_utility/event_daemon
GPIO_DAEMONS_PATH = $(APP_PATH)/system_utility/gpio_daemons
NRS_PATH = $(APP_PATH)/system_utility/nrs
NRS_BUILD = $(NRS_PATH)/build
NRS_INC = $(NRS_PATH)/include
NRS_LIB = $(NRS_PATH)/lib
OTP_PATH = $(APP_PATH)/system_utility/otp
OTP_BUILD = $(OTP_PATH)/build
OTP_INC = $(OTP_PATH)/include
OTP_LIB = $(OTP_PATH)/lib
SNTP_PATH = $(APP_PATH)/system_utility/sntp
SYSUPD_TOOLS_PATH = $(APP_PATH)/system_utility/sysupd_tools
UVC_PATH = $(APP_PATH)/system_utility/uvc
THERMAL_PROTECTION_PATH = $(APP_PATH)/system_utility/thermal_protection
WATCHDOG_PATH = $(APP_PATH)/system_utility/watchdog
SYSTEM_LEVEL_TEST_PATH = $(APP_PATH)/system_utility/system_level_test
FW_ENV_PATH = $(APP_PATH)/system_utility/fw_env
REALTIME_DBG_PATH = $(APP_PATH)/system_utility/realtime_dbg

VIDEO_FTR_INC = $(APP_PATH)/library/libavftr/include
LIBAVFTR_PATH = $(APP_PATH)/library/libavftr
LIBAVFTR_BUILD_PATH = $(LIBAVFTR_PATH)/build
LIBAVFTR_INC = $(LIBAVFTR_PATH)/include
LIBAVFTR_LIB = $(LIBAVFTR_PATH)/lib

LIBADO_PATH = $(APP_PATH)/library/libado
LIBADO_INC = $(LIBADO_PATH)/include
LIBADO_LIB = $(LIBADO_PATH)

LIBCM_PATH = $(APP_PATH)/library/libcm
LIBCM_INC = $(LIBCM_PATH)/include
LIBCM_LIB = $(LIBCM_PATH)

LIBEAIF_PATH = $(APP_PATH)/library/libeaif
LIBEAIF_INC = $(LIBEAIF_PATH)/include
LIBEAIF_LIB = $(LIBEAIF_PATH)/lib

LIBINF_PATH = $(APP_PATH)/library/libinf
LIBINF_INC = $(LIBINF_PATH)/include
LIBINF_LIB = $(LIBINF_PATH)/lib

LIBTBB_PATH = $(APP_PATH)/library/libinf/3rd_party/libtbb
LIBTBB_INC = $(LIBTBB_PATH)/include
LIBTBB_LIB = $(LIBTBB_PATH)/lib

LIBCPPREACT_PATH = $(APP_PATH)/library/libinf/3rd_party/libcppreact
LIBCPPREACT_INC = $(LIBCPPREACT_PATH)/include
LIBCPPREACT_LIB = $(LIBCPPREACT_PATH)/build/lib

LIBFOO_PATH = $(APP_PATH)/library/libfoo
LIBFOO_INC = $(LIBFOO_PATH)/include
LIBFOO_LIB = $(LIBFOO_PATH)/lib

LIBGPIO_PATH = $(APP_PATH)/library/libgpio
LIBLEDEVT_PATH = $(EVENTD_PATH)/libledevt
LIBLEDEVT_INC = $(LIBLEDEVT_PATH)
LIBLEDEVT_LIB = $(LIBLEDEVT_PATH)

LIBOSD_PATH = $(APP_PATH)/library/libosd
LIBOSD_INC = $(LIBOSD_PATH)/include
LIBOSD_LIB = $(LIBOSD_PATH)/lib

LIBUTILS_PATH = $(APP_PATH)/library/libutils
LIBUTILS_INC = $(LIBUTILS_PATH)/include
LIBUTILS_LIB = $(LIBUTILS_PATH)/lib

LIBPWM_PATH = $(APP_PATH)/library/libpwm

LIBMOTOR_PATH = $(APP_PATH)/library/libmotor
LIBMOTOR_INC = $(LIBMOTOR_PATH)/include
LIBMOTOR_LIB = $(LIBMOTOR_PATH)/lib

LIBSAMPLE_PATH = $(APP_PATH)/library/libsample
LIBSAMPLE_INC = $(LIBSAMPLE_PATH)/include
LIBSAMPLE_LIB = $(LIBSAMPLE_PATH)/lib

LIBSQL_PATH = $(APP_PATH)/library/libsql
LIBSQL_INC = $(LIBSQL_PATH)
LIBSQL_LIB = $(LIBSQL_PATH)

LIBTZ_PATH = $(APP_PATH)/library/libtz
LIBTZ_INC = $(LIBTZ_PATH)/include
LIBTZ_LIB = $(LIBTZ_PATH)

FSINK_PATH = $(APP_PATH)/library/libfsink
FSINK_INC = $(FSINK_PATH)/inc

FILE_PATH = $(FSINK_PATH)/file
FILE_INC = $(FSINK_PATH)/inc
FILE_LIB = $(FSINK_PATH)/file

UDPS_PATH = $(FSINK_PATH)/udps
UDPS_INC = $(FSINK_PATH)/inc
UDPS_LIB = $(FSINK_PATH)/udps

LIBAMPC_PATH = $(APP_PATH)/library/libampc
LIBAMPC_INC = $(LIBAMPC_PATH)/include
LIBAMPC_LIB = $(LIBAMPC_PATH)/lib
LIBAMPC_BUILD_PATH = $(LIBAMPC_PATH)/build

LIBMETAL_PATH = $(APP_PATH)/library/libmetal
LIBMETAL_INC = $(LIBMETAL_PATH)/include
LIBMETAL_LIB = $(LIBMETAL_PATH)/lib
LIBMETAL_BUILD_PATH = $(LIBMETAL_PATH)

LIBOPENAMP_PATH = $(APP_PATH)/library/libopen_amp
LIBOPENAMP_INC = $(LIBOPENAMP_PATH)/include
LIBOPENAMP_LIB = $(LIBOPENAMP_PATH)/lib
LIBOPENAMP_BUILD_PATH = $(LIBOPENAMP_PATH)

LIBLPW_PATH = $(APP_PATH)/library/liblpw
LIBLPW_INC = $(LIBLPW_PATH)/include
LIBLPW_LIB = $(LIBLPW_PATH)/lib
LIBLPW_BUILD_PATH = $(LIBLPW_PATH)/build

# MPI APP program
MPI_STREAM_PATH = $(APP_PATH)/sample/mpi_stream

CMD_SENDER_PATH = $(APP_PATH)/sample/cmd_sender
MPI_SNAPSHOT_PATH = $(APP_PATH)/sample/snapshot
REQUEST_IDR_PATH = $(APP_PATH)/sample/request_idr
MPI_SCRIPT_PATH = $(APP_PATH)/sample/mpi_script

LVGL_DEMO_PATH = $(APP_PATH)/sample/lvgl_demo
SDL_DEMO_PATH = $(APP_PATH)/sample/sdl_demo
