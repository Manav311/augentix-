#!/bin/sh
export LD_LIBRARY_PATH=/system/lib:$LD_LIBRARY_PATH

# Link CA files
ln -nfs /etc/nginx/ssl/cert.pem.default /etc/nginx/ssl/cert.pem
ln -nfs /etc/nginx/ssl/key.pem.default /etc/nginx/ssl/key.pem

# Restart NGINX
nginx -s reload

# Restart FLV server
/etc/init.d/develop/S95flv restart
