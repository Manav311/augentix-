import socket
import sys
import time

"""
Usage: python ./install_device_cert.sh <DUT ip>

Please make sure you can ping the DUT IP address first.

This Python script is used by the factory host to connect with the DUT Unicorn, creating a unique SSL self-signed certificate and key pair for each DUT.
"""

def connect_and_send(ip_address):
    port = 6666
    command = "AgtxCrossPlatCommn 14 /system/www/cgi-bin/installCrt.sh@"
    timeout_duration = 1

    # Create a TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            # Connect to the specified IP address and port
            sock.connect((ip_address, port))
            print(f"Connected to {ip_address}:{port}")

            # Send the command
            sock.sendall(command.encode('utf-8'))
            print(f"Sent: {command}")

            # Keep the connection open for 1 second
            time.sleep(timeout_duration)

            # Receive the response from the server
            response = sock.recv(1024)
            print(f"Received: {response.decode('utf-8')}")

        except Exception as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <IP_ADDRESS>")
        sys.exit(1)

    ip_address = sys.argv[1]
    connect_and_send(ip_address)