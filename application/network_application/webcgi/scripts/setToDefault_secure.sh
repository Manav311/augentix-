#!/bin/sh
/system/www/cgi-bin/passwd_secure.sh admin Nimhs-123s@
sync
touch /system/www/.default-passwd
touch /usrdata/reset_file
reboot
