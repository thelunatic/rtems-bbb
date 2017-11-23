#! /usr/bin/env sh

# This scripts are based on revision c59479faa8eb20d4d5139d7621fd179004680d3b of
#    rtems/c/src/lib/libbsp/arm/beagle/simscripts/
# and are therefore under the RTEMS license:
#
#                       LICENSE INFORMATION
#
# RTEMS is free software; you can redistribute it and/or modify it under
# terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.  RTEMS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details. You should have received
# a copy of the GNU General Public License along with RTEMS; see
# file COPYING. If not, write to the Free Software Foundation, 675
# Mass Ave, Cambridge, MA 02139, USA.
#
# As a special exception, including RTEMS header files in a file,
# instantiating RTEMS generics or templates, or linking other files
# with RTEMS objects to produce an executable application, does not
# by itself cause the resulting executable application to be covered
# by the GNU General Public License. This exception does not
# however invalidate any other reasons why the executable file might be
# covered by the GNU Public License.

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# configuration
. "${SCRIPTDIR}/configuration.sh"
export PATH="${PREFIX}/bin:${PATH}"

# we store all generated files here.
TMPDIR=tmp_sdcard_dir.$$

FATIMG=$TMPDIR/bbxm_boot_fat.img
SIZE=65536
OFFSET=2048
FATSIZE=`expr $SIZE - $OFFSET`
UENV=uEnv.txt

rm -rf $TMPDIR
mkdir -p $TMPDIR

if [ $# -ne 2 ]
then	echo "Usage: $0 <RTEMS executable> <image name>"
	exit 1
fi

executable=$1
ubootcfg=am335x_evm
app=rtems-app.img
base=`basename $executable`

if [ ! -f "$executable" ]
then	echo "Expecting RTEMS executable as arg; $executable not found."
	exit 1
fi

IMG=$2

# Make an empty image
dd if=/dev/zero of=$IMG bs=512 seek=`expr $SIZE - 1` count=1
dd if=/dev/zero of=$FATIMG bs=512 seek=`expr $FATSIZE - 1` count=1

# Make an ms-dos FS on it
newfs_msdos -r 1 -m 0xf8 -c 4 -F16  -h 64 -u 32 -S 512 -s $FATSIZE -o 0 ./$FATIMG

# Prepare the executable.
arm-rtems5-objcopy $executable -O binary $TMPDIR/${base}.bin
gzip -9 $TMPDIR/${base}.bin
mkimage -A arm -O linux -T kernel -a 0x80000000 -e 0x80000000 -n RTEMS -d $TMPDIR/${base}.bin.gz $TMPDIR/$app
echo "setenv bootdelay 5
uenvcmd=run boot
boot=fatload mmc 0 0x80800000 $app ; fatload mmc 0 0x88000000 ${DTB_INSTALL_NAME} ; bootm 0x80800000 - 0x88000000" >$TMPDIR/$UENV

# Copy the uboot and app image onto the FAT image
mcopy -bsp -i $FATIMG $PREFIX/uboot/$ubootcfg/MLO ::MLO
mcopy -bsp -i $FATIMG $PREFIX/uboot/$ubootcfg/u-boot.img ::u-boot.img
mcopy -bsp -i $FATIMG $PREFIX/fdt/${DTB_INSTALL_NAME} ::${DTB_INSTALL_NAME}
mcopy -bsp -i $FATIMG $TMPDIR/$app ::$app
mcopy -bsp -i $FATIMG $TMPDIR/$UENV ::$UENV

# Just a single FAT partition (type C) that uses all of the image
partition -m $IMG $OFFSET c:${FATSIZE}\*

# Put the FAT image into the SD image
dd if=$FATIMG of=$IMG seek=$OFFSET

# cleanup
rm -rf $TMPDIR

echo "Result is in $IMG."
