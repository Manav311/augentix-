#! /bin/sh
#
# codegen.sh
#
# Distributed under terms of the MIT license.
#
# ******************************************************************************
#
# Copyright (c) MultiTek Inc. - All Rights Reserved
#
# Unauthorized copying of this file, via any medium is strictly prohibited.
#
# Proprietary and confidential.
#
# ******************************************************************************

APP_INC=../../libagtx/include
SCHEMA_DIR=./schema
AUTOGEN_DIR=./autogen

# uncomment the following line to parse every jsons inside $SCHEMA_DIR
# SCHEMA_NAME=`cd ${SCHEMA_DIR}; echo *.json | sed 's/agtx_//g' | sed 's/.json//g'`

# ... or parse some specific files only by naming them inside the quotes below
# remember to remove "agtx_" prefix and ".json" suffix
SCHEMA_NAME="color_conf"

HEADER_DIR="$AUTOGEN_DIR/include"

# mkdir -p "$AUTOGEN_DIR"
mkdir -p "$HEADER_DIR"

for SCHEMA in $SCHEMA_NAME
do

SCHEMA_FILE="${SCHEMA_DIR}/agtx_${SCHEMA}.json"
LIBAGTX_HEADER="${HEADER_DIR}/agtx_${SCHEMA}.h"
AUTOGEN_HEADER="${AUTOGEN_DIR}/cm_${SCHEMA}.h"
AUTOGEN_SOURCE="${AUTOGEN_DIR}/cm_${SCHEMA}.c"

# Generate header files
# echo python schema2header.py "$SCHEMA_FILE" "$LIBAGTX_HEADER"
python schema2header.py "$SCHEMA_FILE" "$LIBAGTX_HEADER"

# Generate source files
# echo python schema2combin.py "$SCHEMA_FILE" "$AUTOGEN_SOURCE"
python schema2combin.py "$SCHEMA_FILE" "$AUTOGEN_SOURCE"

cp "$LIBAGTX_HEADER" "$APP_INC"
cp "$AUTOGEN_HEADER" ../include
cp "$AUTOGEN_SOURCE" ../

done
