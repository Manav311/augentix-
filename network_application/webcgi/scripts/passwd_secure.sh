#!/bin/sh
export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/root/bin:/system/bin
# Check if input is provided
if [ -z "$2" ]; then
    echo "Error: No password provided."
    exit 1
fi

# Get the length of the password
password="$2"
password_length=${#password}

# Define minimum and maximum password length
min_length=8
max_length=64

# Check if the password meets the length requirement
if [ "$password_length" -lt "$min_length" ]; then
    echo "Error: Password is too short. It must be at least $min_length characters long."
    exit 1
elif [ "$password_length" -gt "$max_length" ]; then
    echo "Error: Password is too long. It must be no more than $max_length characters long."
    exit 1
fi

# Check if the password contains only allowed characters (Unicode and whitespace are allowed)
if ! echo "$password" | awk '/^[[:alnum:][:punct:][:space:]]+$/ {exit 0} {exit 1}'; then
    echo "Error: Password contains invalid characters. Only Unicode and whitespace characters are allowed."
    exit 1
fi

# Check if truncation is required (example purpose)
if [ "$password_length" -gt "$max_length" ]; then
    echo "Warning: Password is longer than $max_length characters, truncation may occur in certain systems."
	exit 1
fi

# Password is valid
# echo "Password is valid."


# get username and password
username="$1"
password="$2"
# get the script file name
agent=$(basename "$0")

# set system arg
HTPASSWD_FILE="/etc/nginx/.htpasswd"


# set Unix socket path
SOCKET_PATH="/tmp/auth_socket"

 
# Send JSON-formatted authentication data to Unix socket
curl --unix-socket "$SOCKET_PATH" \
     -X POST \
     -H "Content-Type: application/json" \
     -d "{\"username\": \"$username\", \"password\": \"$password\", \"agent\": \"$agent\"}" \
     http://localhost/auth