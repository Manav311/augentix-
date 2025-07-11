#!/bin/sh
export LD_LIBRARY_PATH=/system/lib
export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/root/bin:/system/bin
filename=/usrdata/eaif/facereco/faces/$1
tmpfile=$(mktemp /tmp/validateFaceModel.XXXXXX)
/system/bin/facedet_image /system/eaif/models/facereco/inapp_scrfd.ini "$filename" "$tmpfile"
validation="$(sed -n '2p' $tmpfile)"
if [ "$validation" == "1" ]; then
    echo "Equal 1"
    return 0
else
   echo "Not qual 1"
   return 1
fi
