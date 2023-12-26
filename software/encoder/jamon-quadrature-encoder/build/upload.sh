#!/bin/bash

echo "Launching upload.sh ..."

pico_sdk_path="/home/andrey-shefa/pico/pico-sdk/"
system_pico_sdk_path="$PICO_SDK_PATH"

if ["$pico_sdk_path" = "system_pico_sdk_path"]; then
	echo "The PICO_SDK_PATH is set"
else
	echo "PICO_SDK_PATH is not set"
	echo "setting..."
	export PICO_SDK_PATH=/home/andrey-shefa/pico/pico-sdk/
fi

cmake .. -DPICO_BOARD=pico_w ..

make -j4

cp project.uf2 /media/andrey-shefa/RPI-RP2/
