mod := $(notdir $(subdir))

app-$(CONFIG_APP_ACTL) += actl
PHONY += actl actl-clean actl-distclean
PHONY += actl-install actl-uninstall
actl:
	$(Q)$(MAKE) -C $(ACTL_PATH) all

actl-clean:
	$(Q)$(MAKE) -C $(ACTL_PATH) clean

actl-distclean:
	$(Q)$(MAKE) -C $(ACTL_PATH) distclean

actl-install:
	$(Q)$(MAKE) -C $(ACTL_PATH) install

actl-uninstall:
	$(Q)$(MAKE) -C $(ACTL_PATH) uninstall

app-$(CONFIG_APP_LSD_DEMO) += lsd_demo
PHONY += lsd_demo lsd_demo-clean lsd_demo-distclean
PHONY += lsd_demo-install lsd_demo-uninstall
lsd_demo:
	$(Q)$(MAKE) -C $(LSD_DEMO_PATH)/build all

lsd_demo-clean:
	$(Q)$(MAKE) -C $(LSD_DEMO_PATH)/build clean

lsd_demo-distclean:
	$(Q)$(MAKE) -C $(LSD_DEMO_PATH)/build distclean

lsd_demo-install:
	$(Q)$(MAKE) -C $(LSD_DEMO_PATH)/build install

lsd_demo-uninstall:
	$(Q)$(MAKE) -C $(LSD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_SD_DEMO) += sd_demo
PHONY += sd_demo sd_demo-clean sd_demo-distclean
PHONY += sd_demo-install sd_demo-uninstall
sd_demo:
	$(Q)$(MAKE) -C $(SD_DEMO_PATH)/build all

sd_demo-clean:
	$(Q)$(MAKE) -C $(SD_DEMO_PATH)/build clean

sd_demo-distclean:
	$(Q)$(MAKE) -C $(SD_DEMO_PATH)/build distclean

sd_demo-install:
	$(Q)$(MAKE) -C $(SD_DEMO_PATH)/build install

sd_demo-uninstall:
	$(Q)$(MAKE) -C $(SD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_G726) += g726
PHONY += g726 g726-clean g726-distclean
PHONY += g726-install g726-uninstall
g726:
	$(Q)$(MAKE) -C $(G726_PATH)/build all

g726-clean:
	$(Q)$(MAKE) -C $(G726_PATH)/build clean

g726-distclean:
	$(Q)$(MAKE) -C $(G726_PATH)/build distclean

g726-install:
	$(Q)$(MAKE) -C $(G726_PATH)/build install

g726-uninstall:
	$(Q)$(MAKE) -C $(G726_PATH)/build uninstall

app-$(CONFIG_APP_HELLO_APP) += hello_app
PHONY += hello_app hello_app-clean hello_app-distclean
PHONY += hello_app-install hello_app-uninstall
hello_app: libfoo
	$(Q)$(MAKE) -C $(HELLO_APP_PATH)/build all

hello_app-clean: libfoo-clean
	$(Q)$(MAKE) -C $(HELLO_APP_PATH)/build clean

hello_app-distclean: libfoo-distclean
	$(Q)$(MAKE) -C $(HELLO_APP_PATH)/build distclean

hello_app-install: libfoo-install
	$(Q)$(MAKE) -C $(HELLO_APP_PATH)/build install

hello_app-uninstall: libfoo-uninstall
	$(Q)$(MAKE) -C $(HELLO_APP_PATH)/build uninstall

app-$(CONFIG_APP_CMD_SENDER) += cmd_sender
PHONY += cmd_sender cmd_sender-clean cmd_sender-distclean
PHONY += cmd_sender-install cmd_sender-uninstall
cmd_sender:
	$(Q)$(MAKE) -C $(CMD_SENDER_PATH) all

cmd_sender-clean:
	$(Q)$(MAKE) -C $(CMD_SENDER_PATH) clean

cmd_sender-distclean:
	$(Q)$(MAKE) -C $(CMD_SENDER_PATH) distclean

cmd_sender-install:
	$(Q)$(MAKE) -C $(CMD_SENDER_PATH) install

cmd_sender-uninstall:
	$(Q)$(MAKE) -C $(CMD_SENDER_PATH) uninstall

app-$(CONFIG_APP_MPI_SNAPSHOT) += mpi_snapshot
PHONY += mpi_snapshot mpi_snapshot-clean mpi_snapshot-distclean
PHONY += mpi_snapshot-install mpi_snapshot-uninstall
mpi_snapshot:
	$(Q)$(MAKE) -C $(MPI_SNAPSHOT_PATH) all

mpi_snapshot-clean:
	$(Q)$(MAKE) -C $(MPI_SNAPSHOT_PATH) clean

mpi_snapshot-distclean:
	$(Q)$(MAKE) -C $(MPI_SNAPSHOT_PATH) distclean

mpi_snapshot-install:
	$(Q)$(MAKE) -C $(MPI_SNAPSHOT_PATH) install

mpi_snapshot-uninstall:
	$(Q)$(MAKE) -C $(MPI_SNAPSHOT_PATH) uninstall

app-$(CONFIG_APP_MPI_STREAM) += mpi_stream
PHONY += mpi_stream mpi_stream-clean mpi_stream-distclean
PHONY += mpi_stream-install mpi_stream-uninstall
mpi_stream: libsample libfsink
	$(Q)$(MAKE) -C $(MPI_STREAM_PATH)/build all

mpi_stream-clean: libsample-clean libfsink-clean
	$(Q)$(MAKE) -C $(MPI_STREAM_PATH)/build clean

mpi_stream-distclean: libsample-distclean libfsink-distclean
	$(Q)$(MAKE) -C $(MPI_STREAM_PATH)/build distclean

mpi_stream-install: libsample-install libfsink-install
	$(Q)$(MAKE) -C $(MPI_STREAM_PATH)/build install

mpi_stream-uninstall: libsample-uninstall libfsink-uninstall
	$(Q)$(MAKE) -C $(MPI_STREAM_PATH)/build uninstall

app-$(CONFIG_APP_REQUEST_IDR) += request_idr
PHONY += request_idr request_idr-clean request_idr-distclean
PHONY += request_idr-install request_idr-uninstall
request_idr:
	$(Q)$(MAKE) -C $(REQUEST_IDR_PATH) all

request_idr-clean:
	$(Q)$(MAKE) -C $(REQUEST_IDR_PATH) clean

request_idr-distclean:
	$(Q)$(MAKE) -C $(REQUEST_IDR_PATH) distclean

request_idr-install:
	$(Q)$(MAKE) -C $(REQUEST_IDR_PATH) install

request_idr-uninstall:
	$(Q)$(MAKE) -C $(REQUEST_IDR_PATH) uninstall

app-$(CONFIG_APP_AUTO_TRACKING) += auto_tracking
PHONY += auto_tracking auto_tracking-clean auto_tracking-distclean
PHONY += auto_tracking-install auto_tracking-uninstall
auto_tracking:
	$(Q)$(MAKE) -C $(AUTO_TRACKING_PATH)/build all

auto_tracking-clean:
	$(Q)$(MAKE) -C $(AUTO_TRACKING_PATH)/build clean

auto_tracking-distclean:
	$(Q)$(MAKE) -C $(AUTO_TRACKING_PATH)/build distclean

auto_tracking-install:
	$(Q)$(MAKE) -C $(AUTO_TRACKING_PATH)/build install

auto_tracking-uninstall:
	$(Q)$(MAKE) -C $(AUTO_TRACKING_PATH)/build uninstall

app-$(CONFIG_APP_LVGL_DEMO) += lvgl_demo
PHONY += lvgl_demo lvgl_demo-clean lvgl_demo-distclean
PHONY += lvgl_demo-install lvgl_demo-uninstall
lvgl_demo:
	$(Q)$(MAKE) -C $(LVGL_DEMO_PATH)/build all

lvgl_demo-clean:
	$(Q)$(MAKE) -C $(LVGL_DEMO_PATH)/build clean

lvgl_demo-distclean:
	$(Q)$(MAKE) -C $(LVGL_DEMO_PATH)/build distclean

lvgl_demo-install:
	$(Q)$(MAKE) -C $(LVGL_DEMO_PATH)/build install

lvgl_demo-uninstall:
	$(Q)$(MAKE) -C $(LVGL_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_SDL_DEMO) += sdl_demo
PHONY += sdl_demo sdl_demo-clean sdl_demo-distclean
PHONY += sdl_demo-install sdl_demo-uninstall
sdl_demo:
	$(Q)$(MAKE) -C $(SDL_DEMO_PATH)/build all

sdl_demo-clean:
	$(Q)$(MAKE) -C $(SDL_DEMO_PATH)/build clean

sdl_demo-distclean:
	$(Q)$(MAKE) -C $(SDL_DEMO_PATH)/build distclean

sdl_demo-install:
	$(Q)$(MAKE) -C $(SDL_DEMO_PATH)/build install

sdl_demo-uninstall:
	$(Q)$(MAKE) -C $(SDL_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_MPI_SCRIPT) += mpi_script
PHONY += mpi_script mpi_script-clean mpi_script-distclean
PHONY += mpi_script-install mpi_script-uninstall
mpi_script:
	$(Q)$(MAKE) -C $(MPI_SCRIPT_PATH) all

mpi_script-clean:
	$(Q)$(MAKE) -C $(MPI_SCRIPT_PATH) clean

mpi_script-distclean:
	$(Q)$(MAKE) -C $(MPI_SCRIPT_PATH) distclean

mpi_script-install:
	$(Q)$(MAKE) -C $(MPI_SCRIPT_PATH) install

mpi_script-uninstall:
	$(Q)$(MAKE) -C $(MPI_SCRIPT_PATH) uninstall

app-$(CONFIG_APP_AROI_DEMO) += aroi_demo
PHONY += aroi_demo aroi_demo-clean aroi_demo-distclean
PHONY += aroi_demo-install aroi_demo-uninstall
aroi_demo:
	$(Q)$(MAKE) -C $(AROI_DEMO_PATH)/build all

aroi_demo-clean:
	$(Q)$(MAKE) -C $(AROI_DEMO_PATH)/build clean

aroi_demo-distclean:
	$(Q)$(MAKE) -C $(AROI_DEMO_PATH)/build distclean

aroi_demo-install:
	$(Q)$(MAKE) -C $(AROI_DEMO_PATH)/build install

aroi_demo-uninstall:
	$(Q)$(MAKE) -C $(AROI_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_BM_DEMO) += bm_demo
PHONY += bm_demo bm_demo-clean bm_demo-distclean
PHONY += bm_demo-install bm_demo-uninstall
bm_demo:
	$(Q)$(MAKE) -C $(BM_DEMO_PATH)/build all

bm_demo-clean:
	$(Q)$(MAKE) -C $(BM_DEMO_PATH)/build clean

bm_demo-distclean:
	$(Q)$(MAKE) -C $(BM_DEMO_PATH)/build distclean

bm_demo-install:
	$(Q)$(MAKE) -C $(BM_DEMO_PATH)/build install

bm_demo-uninstall:
	$(Q)$(MAKE) -C $(BM_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_DD_DEMO) += dd_demo
PHONY += dd_demo dd_demo-clean dd_demo-distclean
PHONY += dd_demo-install dd_demo-uninstall
dd_demo:
	$(Q)$(MAKE) -C $(DD_DEMO_PATH)/build all

dd_demo-clean:
	$(Q)$(MAKE) -C $(DD_DEMO_PATH)/build clean

dd_demo-distclean:
	$(Q)$(MAKE) -C $(DD_DEMO_PATH)/build distclean

dd_demo-install:
	$(Q)$(MAKE) -C $(DD_DEMO_PATH)/build install

dd_demo-uninstall:
	$(Q)$(MAKE) -C $(DD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_DK_DEMO) += dk_demo
PHONY += dk_demo dk_demo-clean dk_demo-distclean
PHONY += dk_demo-install dk_demo-uninstall
dk_demo:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build all

dk_demo-clean:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build clean

dk_demo-distclean:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build distclean

dk_demo-install:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build install

dk_demo-uninstall:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_DFB_DEMO) += dfb_demo
PHONY += dfb_demo dfb_demo-clean dfb_demo-distclean
PHONY += dfb_demo-install dfb_demo-uninstall
dfb_demo:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build all

dfb_demo-clean:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build clean

dfb_demo-distclean:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build distclean

dfb_demo-install:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build install

dfb_demo-uninstall:
	$(Q)$(MAKE) -C $(DK_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_EF_DEMO) += ef_demo
PHONY += ef_demo ef_demo-clean ef_demo-distclean
PHONY += ef_demo-install ef_demo-uninstall
ef_demo:
	$(Q)$(MAKE) -C $(EF_DEMO_PATH)/build all

ef_demo-clean:
	$(Q)$(MAKE) -C $(EF_DEMO_PATH)/build clean

ef_demo-distclean:
	$(Q)$(MAKE) -C $(EF_DEMO_PATH)/build distclean

ef_demo-install:
	$(Q)$(MAKE) -C $(EF_DEMO_PATH)/build install

ef_demo-uninstall:
	$(Q)$(MAKE) -C $(EF_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_FLD_DEMO) += fld_demo
PHONY += fld_demo fld_demo-clean fld_demo-distclean
PHONY += fld_demo-install fld_demo-uninstall
fld_demo:
	$(Q)$(MAKE) -C $(FLD_DEMO_PATH)/build all

fld_demo-clean:
	$(Q)$(MAKE) -C $(FLD_DEMO_PATH)/build clean

fld_demo-distclean:
	$(Q)$(MAKE) -C $(FLD_DEMO_PATH)/build distclean

fld_demo-install:
	$(Q)$(MAKE) -C $(FLD_DEMO_PATH)/build install

fld_demo-uninstall:
	$(Q)$(MAKE) -C $(FLD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_HD_DEMO) += hd_demo
PHONY += hd_demo hd_demo-clean hd_demo-distclean
PHONY += hd_demo-install hd_demo-uninstall
hd_demo: libeaif libinf
	$(Q)$(MAKE) -C $(HD_DEMO_PATH)/build all

hd_demo-clean: libeaif-clean libinf-clean
	$(Q)$(MAKE) -C $(HD_DEMO_PATH)/build clean

hd_demo-distclean: libeaif-distclean libinf-distclean
	$(Q)$(MAKE) -C $(HD_DEMO_PATH)/build distclean

hd_demo-install: libeaif-install libinf-install
	$(Q)$(MAKE) -C $(HD_DEMO_PATH)/build install

hd_demo-uninstall: libeaif-uninstall libinf-uninstall
	$(Q)$(MAKE) -C $(HD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_EAI_HD_DEMO) += eai_hd_demo
PHONY += eai_hd_demo eai_hd_demo-clean eai_hd_demo-distclean
PHONY += eai_hd_demo-install eai_hd_demo-uninstall
eai_hd_demo: libinf
	$(Q)$(MAKE) -C $(EAI_HD_DEMO_PATH)/build all

eai_hd_demo-clean: libinf-clean
	$(Q)$(MAKE) -C $(EAI_HD_DEMO_PATH)/build clean

eai_hd_demo-distclean: libinf-distclean
	$(Q)$(MAKE) -C $(EAI_HD_DEMO_PATH)/build distclean

eai_hd_demo-install: libinf-install
	$(Q)$(MAKE) -C $(EAI_HD_DEMO_PATH)/build install

eai_hd_demo-uninstall: libinf-uninstall
	$(Q)$(MAKE) -C $(EAI_HD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_IR_DEMO) += ir_demo
PHONY += ir_demo ir_demo-clean ir_demo-distclean
PHONY += ir_demo-install ir_demo-uninstall
ir_demo:
	$(Q)$(MAKE) -C $(IR_DEMO_PATH)/build all

ir_demo-clean:
	$(Q)$(MAKE) -C $(IR_DEMO_PATH)/build clean

ir_demo-distclean:
	$(Q)$(MAKE) -C $(IR_DEMO_PATH)/build distclean

ir_demo-install:
	$(Q)$(MAKE) -C $(IR_DEMO_PATH)/build install

ir_demo-uninstall:
	$(Q)$(MAKE) -C $(IR_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_MD_DEMO) += md_demo
PHONY += md_demo md_demo-clean md_demo-distclean
PHONY += md_demo-install md_demo-uninstall
md_demo:
	$(Q)$(MAKE) -C $(MD_DEMO_PATH)/build all

md_demo-clean:
	$(Q)$(MAKE) -C $(MD_DEMO_PATH)/build clean

md_demo-distclean:
	$(Q)$(MAKE) -C $(MD_DEMO_PATH)/build distclean

md_demo-install:
	$(Q)$(MAKE) -C $(MD_DEMO_PATH)/build install

md_demo-uninstall:
	$(Q)$(MAKE) -C $(MD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_MD_DEMO) += od_demo
PHONY += od_demo od_demo-clean od_demo-distclean
PHONY += od_demo-install od_demo-uninstall
od_demo:
	$(Q)$(MAKE) -C $(OD_DEMO_PATH)/build all

od_demo-clean:
	$(Q)$(MAKE) -C $(OD_DEMO_PATH)/build clean

od_demo-distclean:
	$(Q)$(MAKE) -C $(OD_DEMO_PATH)/build distclean

od_demo-install:
	$(Q)$(MAKE) -C $(OD_DEMO_PATH)/build install

od_demo-uninstall:
	$(Q)$(MAKE) -C $(OD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_OSD_DEMO) += osd_demo
PHONY += osd_demo osd_demo-clean osd_demo-distclean
PHONY += osd_demo-install osd_demo-uninstall
osd_demo: libosd
	$(Q)$(MAKE) -C $(OSD_DEMO_PATH)/build all

osd_demo-clean: libosd-clean
	$(Q)$(MAKE) -C $(OSD_DEMO_PATH)/build clean

osd_demo-distclean: libosd-distclean
	$(Q)$(MAKE) -C $(OSD_DEMO_PATH)/build distclean

osd_demo-install: libosd-install
	$(Q)$(MAKE) -C $(OSD_DEMO_PATH)/build install

osd_demo-uninstall: libosd-uninstall
	$(Q)$(MAKE) -C $(OSD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_GTK3_DEMO_GUI) += gtk3_demo_gui
PHONY += gtk3_demo_gui gtk3_demo_gui-clean gtk3_demo_gui-distclean
PHONY += gtk3_demo_gui-install gtk3_demo_gui-uninstall
gtk3_demo_gui:
	$(Q)$(MAKE) -C $(GTK3_DEMO_GUI_PATH)/build all

gtk3_demo_gui-clean:
	$(Q)$(MAKE) -C $(GTK3_DEMO_GUI_PATH)/build clean

gtk3_demo_gui-distclean:
	$(Q)$(MAKE) -C $(GTK3_DEMO_GUI_PATH)/build distclean

gtk3_demo_gui-install:
	$(Q)$(MAKE) -C $(GTK3_DEMO_GUI_PATH)/build install

gtk3_demo_gui-uninstall:
	$(Q)$(MAKE) -C $(GTK3_DEMO_GUI_PATH)/build uninstall


app-$(CONFIG_APP_TD_DEMO) += pfm_demo
PHONY += pfm_demo pfm_demo-clean pfm_demo-distclean
PHONY += pfm_demo-install pfm_demo-uninstall
pfm_demo:
	$(Q)$(MAKE) -C $(PFM_DEMO_PATH)/build all

pfm_demo-clean:
	$(Q)$(MAKE) -C $(PFM_DEMO_PATH)/build clean

pfm_demo-distclean:
	$(Q)$(MAKE) -C $(PFM_DEMO_PATH)/build distclean

pfm_demo-install:
	$(Q)$(MAKE) -C $(PFM_DEMO_PATH)/build install

pfm_demo-uninstall:
	$(Q)$(MAKE) -C $(PFM_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_TD_DEMO) += td_demo
PHONY += td_demo td_demo-clean td_demo-distclean
PHONY += td_demo-install td_demo-uninstall
td_demo:
	$(Q)$(MAKE) -C $(TD_DEMO_PATH)/build all

td_demo-clean:
	$(Q)$(MAKE) -C $(TD_DEMO_PATH)/build clean

td_demo-distclean:
	$(Q)$(MAKE) -C $(TD_DEMO_PATH)/build distclean

td_demo-install:
	$(Q)$(MAKE) -C $(TD_DEMO_PATH)/build install

td_demo-uninstall:
	$(Q)$(MAKE) -C $(TD_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_FACEDET_DEMO) += facedet_demo
PHONY += facedet_demo facedet_demo-clean facedet_demo-distclean
PHONY += facedet_demo-install facedet_demo-uninstall
facedet_demo: libinf
	$(Q)$(MAKE) -C $(FACEDET_DEMO_PATH)/build all

facedet_demo-clean: libinf-clean
	$(Q)$(MAKE) -C $(FACEDET_DEMO_PATH)/build clean

facedet_demo-distclean: libinf-distclean
	$(Q)$(MAKE) -C $(FACEDET_DEMO_PATH)/build distclean

facedet_demo-install: libinf-install
	$(Q)$(MAKE) -C $(FACEDET_DEMO_PATH)/build install

facedet_demo-uninstall: libinf-uninstall
	$(Q)$(MAKE) -C $(FACEDET_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_FACERECO_DEMO) += facereco_demo
PHONY += facereco_demo facereco_demo-clean facereco_demo-distclean
PHONY += facereco_demo-install facereco_demo-uninstall
facereco_demo: libeaif
	$(Q)$(MAKE) -C $(FACERECO_DEMO_PATH)/build all

facereco_demo-clean: libeaif-clean
	$(Q)$(MAKE) -C $(FACERECO_DEMO_PATH)/build clean

facereco_demo-distclean: libeaif-distclean
	$(Q)$(MAKE) -C $(FACERECO_DEMO_PATH)/build distclean

facereco_demo-install: libeaif-install
	$(Q)$(MAKE) -C $(FACERECO_DEMO_PATH)/build install

facereco_demo-uninstall: libeaif-uninstall
	$(Q)$(MAKE) -C $(FACERECO_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_VFTR_DUMP) += vftr_dump
PHONY += vftr_dump vftr_dump-clean vftr_dump-distclean
PHONY += vftr_dump-install vftr_dump-uninstall
vftr_dump:
	$(Q)$(MAKE) -C $(VFTR_DUMP_PATH)/build all

vftr_dump-clean:
	$(Q)$(MAKE) -C $(VFTR_DUMP_PATH)/build clean

vftr_dump-distclean:
	$(Q)$(MAKE) -C $(VFTR_DUMP_PATH)/build distclean

vftr_dump-install:
	$(Q)$(MAKE) -C $(VFTR_DUMP_PATH)/build install

vftr_dump-uninstall:
	$(Q)$(MAKE) -C $(VFTR_DUMP_PATH)/build uninstall

app-$(CONFIG_APP_LPW) += lpw
PHONY += lpw lpw-clean lpw-distclean
PHONY += lpw-install lpw-uninstall
lpw:
	$(Q)$(MAKE) -C $(LPW_PATH)/build all

lpw-clean:
	$(Q)$(MAKE) -C $(LPW_PATH)/build clean

lpw-distclean:
	$(Q)$(MAKE) -C $(LPW_PATH)/build distclean

lpw-install:
	$(Q)$(MAKE) -C $(LPW_PATH)/build install

lpw-uninstall:
	$(Q)$(MAKE) -C $(LPW_PATH)/build uninstall

app-$(CONFIG_APP_LPW_SUPP) += lpw_supp
PHONY += lpw_supp lpw_supp-clean lpw_supp-distclean
PHONY += lpw_supp-install lpw_supp-uninstall
lpw_supp:
	$(Q)$(MAKE) -C $(LPW_SUPP_PATH)/build all

lpw_supp-clean:
	$(Q)$(MAKE) -C $(LPW_SUPP_PATH)/build clean

lpw_supp-distclean:
	$(Q)$(MAKE) -C $(LPW_SUPP_PATH)/build distclean

lpw_supp-install:
	$(Q)$(MAKE) -C $(LPW_SUPP_PATH)/build install

lpw_supp-uninstall:
	$(Q)$(MAKE) -C $(LPW_SUPP_PATH)/build uninstall


app-$(CONFIG_APP_HIBER_DEMO) += hiber_demo
PHONY += hiber_demo hiber_demo-clean hiber_demo-distclean
PHONY += hiber_demo-install hiber_demo-uninstall
hiber_demo:
	$(Q)$(MAKE) -C $(HIBER_DEMO_PATH)/build all

hiber_demo-clean:
	$(Q)$(MAKE) -C $(HIBER_DEMO_PATH)/build clean

hiber_demo-distclean:
	$(Q)$(MAKE) -C $(HIBER_DEMO_PATH)/build distclean

hiber_demo-install:
	$(Q)$(MAKE) -C $(HIBER_DEMO_PATH)/build install

hiber_demo-uninstall:
	$(Q)$(MAKE) -C $(HIBER_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_LPW_FW_UPG) += lpw_fw_upg
PHONY += lpw_fw_upg lpw_fw_upg-clean lpw_fw_upg-distclean
PHONY += lpw_fw_upg-install lpw_fw_upg-uninstall
lpw_fw_upg:
	$(Q)$(MAKE) -C $(LPW_FW_UPG_PATH)/build all

lpw_fw_upg-clean:
	$(Q)$(MAKE) -C $(LPW_FW_UPG_PATH)/build clean

lpw_fw_upg-distclean:
	$(Q)$(MAKE) -C $(LPW_FW_UPG_PATH)/build distclean

lpw_fw_upg-install:
	$(Q)$(MAKE) -C $(LPW_FW_UPG_PATH)/build install

lpw_fw_upg-uninstall:
	$(Q)$(MAKE) -C $(LPW_FW_UPG_PATH)/build uninstall

app-$(CONFIG_APP_LPWIO) += lpwio
PHONY += lpwio lpwio-clean lpwio-distclean
PHONY += lpwio-install lpwio-uninstall
lpwio:
	$(Q)$(MAKE) -C $(LPWIO_PATH)/build all

lpwio-clean:
	$(Q)$(MAKE) -C $(LPWIO_PATH)/build clean

lpwio-distclean:
	$(Q)$(MAKE) -C $(LPWIO_PATH)/build distclean

lpwio-install:
	$(Q)$(MAKE) -C $(LPWIO_PATH)/build install

lpwio-uninstall:
	$(Q)$(MAKE) -C $(LPWIO_PATH)/build uninstall

app-$(CONFIG_APP_SECURE_ELEMENT_DEMO) += secure_element_demo
PHONY += secure_element_demo secure_element_demo-clean secure_element_demo-distclean
PHONY += secure_element_demo-install secure_element_demo-uninstall
secure_element_demo:
	$(Q)$(MAKE) -C $(SECURE_ELEMENT_DEMO_PATH)/build all

secure_element_demo-clean:
	$(Q)$(MAKE) -C $(SECURE_ELEMENT_DEMO_PATH)/build clean

secure_element_demo-distclean:
	$(Q)$(MAKE) -C $(SECURE_ELEMENT_DEMO_PATH)/build distclean

secure_element_demo-install:
	$(Q)$(MAKE) -C $(SECURE_ELEMENT_DEMO_PATH)/build install

secure_element_demo-uninstall:
	$(Q)$(MAKE) -C $(SECURE_ELEMENT_DEMO_PATH)/build uninstall

app-$(CONFIG_APP_MP4_RECORDER) += mp4_recorder
PHONY += mp4_recorder mp4_recorder-clean mp4_recorder-distclean
PHONY += mp4_recorder-install mp4_recorder-uninstall
mp4_recorder:
	$(Q)$(MAKE) -C $(MP4_RECORDER_PATH)/build all

mp4_recorder-clean:
	$(Q)$(MAKE) -C $(MP4_RECORDER_PATH)/build clean

mp4_recorder-distclean:
	$(Q)$(MAKE) -C $(MP4_RECORDER_PATH)/build distclean

mp4_recorder-install:
	$(Q)$(MAKE) -C $(MP4_RECORDER_PATH)/build install

mp4_recorder-uninstall:
	$(Q)$(MAKE) -C $(MP4_RECORDER_PATH)/build uninstall

app-$(CONFIG_APP_SECURE_OPENSSL_DEMO) += secure_openssl_demo
PHONY += secure_openssl_demo secure_openssl_demo-clean secure_openssl_demo-distclean
PHONY += secure_openssl_demo-install secure_openssl_demo-uninstall
secure_openssl_demo:
	$(Q)$(MAKE) -C $(SECURE_OPENSSL_DEMO_PATH)/build all

secure_openssl_demo-clean:
	$(Q)$(MAKE) -C $(SECURE_OPENSSL_DEMO_PATH)/build clean

secure_openssl_demo-distclean:
	$(Q)$(MAKE) -C $(SECURE_OPENSSL_DEMO_PATH)/build distclean

secure_openssl_demo-install:
	$(Q)$(MAKE) -C $(SECURE_OPENSSL_DEMO_PATH)/build install

secure_openssl_demo-uninstall:
	$(Q)$(MAKE) -C $(SECURE_OPENSSL_DEMO_PATH)/build uninstall
	
PHONY += $(mod) $(mod)-clean $(mod)-distclean
PHONY += $(mod)-install $(mod)-uninstall
$(mod): $(app-y)
$(mod)-clean: $(addsuffix -clean,$(app-y))
$(mod)-distclean: $(addsuffix -distclean,$(app-y))
$(mod)-install: $(addsuffix -install,$(app-y))
$(mod)-uninstall: $(addsuffix -uninstall,$(app-y))

APP_BUILD_DEPS += $(mod)
APP_CLEAN_DEPS += $(mod)-clean
APP_DISTCLEAN_DEPS += $(mod)-distclean
APP_INTALL_DEPS += $(mod)-install
APP_UNINTALL_DEPS += $(mod)-uninstall
