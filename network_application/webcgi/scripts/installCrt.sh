#!/bin/sh

# Define paths for keys and certificate
SSL_DIR="/etc/nginx/ssl"
PRIVATE_KEY="$SSL_DIR/device_pri_key.pem"
PUBLIC_KEY="$SSL_DIR/device_pub_key.pem"
CSR_PATH="/usrdata/"
OPENSSL="/usr/bin/openssl"


echo "CSR path : $CSR_PATH"

# Generate ECDSA key pair
$OPENSSL ecparam -genkey -name prime256v1 -out $PRIVATE_KEY
$OPENSSL ec -pubout -in $PRIVATE_KEY -out $PUBLIC_KEY
echo "openssl gen $PUBLIC_KEY $PRIVATE_KEY"

$OPENSSL req -new -key $PRIVATE_KEY -out $CSR_PATH/device.csr -subj "/C=TW/ST=Taiwan/L=Taipei/O=Augentix/OU=SD2/CN=ALL"
echo "Use $PRIVATE_KEY gen $CSR_PATH/device.csr"

$OPENSSL ecparam -genkey -name prime256v1 -out $CSR_PATH/ca-priv-key.pem
$OPENSSL req -new -x509 -key $CSR_PATH/ca-priv-key.pem -out $CSR_PATH/ca-cert.pem -days 365 -subj "/C=TW/ST=Taiwan/L=Taipei/O=Augentix/OU=SD2/CN=ALL"
echo "openssl gen CA key pair $CSR_PATH/ca-priv-key.pem, $CSR_PATH/ca-cert.pem"

$OPENSSL x509 -req -in $CSR_PATH/device.csr -CA $CSR_PATH/ca-cert.pem -CAkey $CSR_PATH/ca-priv-key.pem -CAcreateserial -out $CSR_PATH/device-cert.crt -days 365 -sha256

if [ -f "/system/www/cgi-bin/encryptedDeviceSecret.sh" ]; then
    sh /system/www/cgi-bin/encryptedDeviceSecret.sh "$CSR_PATH/device-cert.crt"
else
    sh /system/www/cgi-bin/encryptedDeviceSecret_sq7131s.sh "$CSR_PATH/device-cert.crt"
fi
