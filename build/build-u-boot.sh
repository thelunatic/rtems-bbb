#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# configuration
. "${SCRIPTDIR}/configuration.sh"
LOGDIR="${PROJECTDIR}/build/"
NOW="$(date +%Y%m%d_%H%M%S)"
export PATH="${PREFIX}/bin:${PATH}"

# Build
cd "${U_BOOT_SOURCE_DIR}"
make -j `nproc` PYTHON=python2 CROSS_COMPILE=${TARGET}- mrproper
make -j `nproc` PYTHON=python2 CROSS_COMPILE=${TARGET}- ${UBOOT_CONFIG}_config
make -j `nproc` PYTHON=python2 CROSS_COMPILE=${TARGET}-

# install files
mkdir -p ${PREFIX}/uboot/${UBOOT_CONFIG}/
cp MLO u-boot.img ${PREFIX}/uboot/${UBOOT_CONFIG}/
mkdir -p ${PREFIX}/bin
cp tools/mkimage ${PREFIX}/bin
