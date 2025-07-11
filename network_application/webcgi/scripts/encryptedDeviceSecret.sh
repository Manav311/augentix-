#!/bin/bash

# Define paths for keys and certificate
SSL_DIR="/etc/nginx/ssl"
PRIVATE_KEY="$SSL_DIR/device_pri_key.pem"
PUBLIC_KEY="$SSL_DIR/device_pub_key.pem"
ENCRYPTED_PRIVATE_KEY="$SSL_DIR/encrypted_priv_key.pem"
ENCRYPTED_PUBLIC_KEY="$SSL_DIR/encrypted_pub_key.pem"
ISSUER_CERT=$1
ENCRYPTED_CERT="$SSL_DIR/encrypted.crt"

if [ -z "$1" ]; then
  echo "Error: Missing argument."
  echo "Usage: $0 <install crt>"
  exit 1
fi


# Encrypt key pair and certificate using Secure Element
secure_element_demo -D 1 -P
secure_element_demo -D 1 -F -s $PRIVATE_KEY -e $ENCRYPTED_PRIVATE_KEY
secure_element_demo -D 1 -F -s $PUBLIC_KEY -e $ENCRYPTED_PUBLIC_KEY
secure_element_demo -D 1 -F -s $ISSUER_CERT -e $ENCRYPTED_CERT

echo "SE encrypted:"
echo "$ENCRYPTED_PRIVATE_KEY"
echo "$ENCRYPTED_PUBLIC_KEY"
echo "$ENCRYPTED_CERT"

# Remove unencrypted key pair and certificate
rm $PRIVATE_KEY
rm $PUBLIC_KEY

echo "Encryption complete and original keys removed."