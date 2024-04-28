#!/bin/bash

# Set the broadcast address and port
BROADCAST_ADDRESS="255.255.255.255"
PORT="12345"

echo "Sending command: $1"

# Convert the data to bytes
BYTE_DATA=$(echo -n "$1" | xxd -pu -c 256)

# Send the data
echo -n "$BYTE_DATA" | socat - UDP-DATAGRAM:$BROADCAST_ADDRESS:$PORT,broadcast

# Check if the data has been sent successfully
if [ $? -eq 0 ]; then
echo "Data has been sent successfully."
echo "$BYTE_DATA"
else
echo "Failed to send data."
fi
