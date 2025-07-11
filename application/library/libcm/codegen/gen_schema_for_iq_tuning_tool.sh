#!/bin/bash
if [ "$#" -lt 1 ]
then
    echo "Incorrect number of arguments"
    exit 1
fi

SCHEMA_DIR=$(dirname $0)/schema
OUTPUT_DIR=$1

SCHEMA_LIST="dip_ae_conf
             dip_awb_conf
             dip_cal_conf
             dip_coring_conf
             dip_csm_conf
             dip_ctrl_conf
             dip_dbc_conf
             dip_dcc_conf
             dip_dhz_conf
             dip_dms_conf
             dip_enh_conf
             dip_extend_conf
             dip_exp_info
             dip_fcs_conf
             dip_gamma_conf
             dip_hdr_synth_conf
             dip_iso_conf
             dip_lsc_conf
             dip_nr_conf
             dip_pca_table_conf
             dip_pta_conf
             dip_roi_conf
             dip_shp_conf
             dip_stat_conf
             dip_te_conf
             dip_te_info
             dip_wb_info
             panning_conf
             panorama_conf
             stitching_conf
             surround_conf
             video_ldc_conf"

# create output directory
mkdir -p "$OUTPUT_DIR"

for SCHEMA in $SCHEMA_LIST
do

SCHEMA_FILE="${SCHEMA_DIR}/agtx_${SCHEMA}.json"

cp "$SCHEMA_FILE" "$OUTPUT_DIR"

done

cp "${SCHEMA_DIR}/ui_dut_file_title_mapping.json" "$OUTPUT_DIR"
