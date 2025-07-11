#!/bin/sh
echo -n $1 > /etc/nginx/.htpasswd 
echo -n ":" >> /etc/nginx/.htpasswd 
openssl passwd -apr1 $2 >> /etc/nginx/.htpasswd
