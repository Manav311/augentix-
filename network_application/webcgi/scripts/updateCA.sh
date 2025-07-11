#!/bin/sh
export LD_LIBRARY_PATH=/system/lib:$LD_LIBRARY_PATH

chmod 600 /etc/nginx/ssl/cert.pem.upload
chmod 600 /etc/nginx/ssl/key.pem.upload

# Link CA files
ln -nfs /etc/nginx/ssl/cert.pem.upload /etc/nginx/ssl/cert.pem
ln -nfs /etc/nginx/ssl/key.pem.upload /etc/nginx/ssl/key.pem

# Restart NGINX
nginx -t
RET=$?
if [ $RET -ne 0 ]; then
  ln -nfs /etc/nginx/ssl/cert.pem.default /etc/nginx/ssl/cert.pem
  ln -nfs /etc/nginx/ssl/key.pem.default /etc/nginx/ssl/key.pem
  return 1;
fi
nginx -s reload

# Restart FLV server
/etc/init.d/develop/S95flv restart

return 0
