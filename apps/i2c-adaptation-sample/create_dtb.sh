#! /usr/bin/env sh

export MACHINE='arm'
SCRIPT_DIR=$(pwd)/../../tools/devicetree-freebsd-export/sys/tools/fdt
${SCRIPT_DIR}/make_dtb.sh ${SCRIPT_DIR}/../../ ${SCRIPT_DIR}/../../gnu/dts/arm/am335x-boneblack.dts $(pwd)
dtc -@ -I dts -O dtb -o am335x-i2c-overlay.dtbo am335x-i2c-overlay.dts
#NOTE: Uncomment the following line to apply the overlay on the base tree.
#fdtoverlay -i am335x-boneblack.dtb -o am335x-boneblack.dtb am335x-i2c-overlay.dtbo
if [ $? -eq 0 ]; then
    echo "Overlay applied successfully"
#    echo "use this am335x-boneblack.dtb with the sd card image"
else
    echo "Failed to Apply overlay"
fi
