#!/bin/bash

# Define paths for keys and certificate
SSL_DIR="/etc/nginx/ssl"
PRIVATE_KEY="$SSL_DIR/device_pri_key.pem"
PUBLIC_KEY="$SSL_DIR/device_pub_key.pem"
ENCRYPTED_PRIVATE_KEY="$SSL_DIR/encrypted_priv_key.pem"
ENCRYPTED_PUBLIC_KEY="$SSL_DIR/encrypted_pub_key.pem"
ENCRYPTED_PRIVATE_KEY_MAC="$SSL_DIR/encrypted_priv_key_mac.bin"
ENCRYPTED_PUBLIC_KEY_MAC="$SSL_DIR/encrypted_pub_key_mac.bin"
ISSUER_CERT=$1
ENCRYPTED_CERT="$SSL_DIR/encrypted.crt"
ENCRYPTED_CERT_MAC="$SSL_DIR/encrypted_mac.bin"

if [ -z "$1" ]; then
  echo "Error: Missing argument."
  echo "Usage: $0 <install crt>"
  exit 1
fi


# Encrypt key pair and certificate using Secure Element
secure_element_demo -D 1 -E -d $PRIVATE_KEY -g $ENCRYPTED_PRIVATE_KEY -m $ENCRYPTED_PRIVATE_KEY_MAC
secure_element_demo -D 1 -E -d $PUBLIC_KEY -g $ENCRYPTED_PUBLIC_KEY -m $ENCRYPTED_PUBLIC_KEY_MAC
secure_element_demo -D 1 -E -d $ISSUER_CERT -g $ENCRYPTED_CERT -m $ENCRYPTED_CERT_MAC

echo "SE encrypted:"
echo "$ENCRYPTED_PRIVATE_KEY"
echo "$ENCRYPTED_PUBLIC_KEY"
echo "$ENCRYPTED_CERT"

# Remove unencrypted key pair and certificate
rm $PRIVATE_KEY
rm $PUBLIC_KEY

echo "Encryption complete and original keys removed."
