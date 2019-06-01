#! /usr/bin/env sh

BUILD=$(pwd)/../../build
${BUILD}/build-dtb.sh am335x-boneblack.dtb
cp $(pwd)/../../install/rtems/5/fdt/am335x-boneblack.dtb .
dtc -@ -I dts -O dtb -o am335x-i2c-overlay.dtb am335x-i2c-overlay.dts
fdtoverlay -i am335x-boneblack.dtb -o am335x-boneblack.dtb am335x-i2c-overlay.dtb
if [ $? -eq 0 ]; then
    echo "Overlay applied successfully"
    echo "use this am335x-boneblack.dtb with the sd card image"
else
    echo "Failed to Apply overlay"
fi
