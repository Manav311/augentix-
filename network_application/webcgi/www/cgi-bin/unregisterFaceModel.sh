#!/bin/sh
filename=$1
name="${filename%.*}"
deleteCmd='{ "face_name":"'$name'", "inf_cmd":"FACE_DELETE" }'
saveCmd='{ "face_name":"'$name'", "inf_cmd":"FACE_SAVE" }'
noneCmd='{ "face_name":"'$name'", "inf_cmd":"NONE" }'
/system/bin/ccclient -c AGTX_CMD_EAIF_CONF -s "$deleteCmd" \
        && /system/bin/ccclient -c AGTX_CMD_EAIF_CONF -s "$saveCmd" \
        && /system/bin/ccclient -c AGTX_CMD_EAIF_CONF -s "$noneCmd"
ret=$?
return $ret
