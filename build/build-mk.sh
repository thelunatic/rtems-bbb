#! /usr/bin/env sh

SCRIPTDIR=/home/lunatic/development/rtems-bbb/build
PROJECTDIR=/home/lunatic/development/rtems-bbb/build/../
. "${SCRIPTDIR}/configuration.sh"
PREFIX=${SCRIPTDIR}/../install/rtems/5

set -x
set -e -u

mkdir -p "${PREFIX}/make/custom/"
cat "${PROJECTDIR}/build/src/bsp.mk" | \
	sed 	-e "s/##RTEMS_API##/$RTEMS_VERSION/g" \
		-e "s/##RTEMS_BSP##/$BSP_NAME/g" \
		-e "s/##RTEMS_CPU##/$RTEMS_CPU/g" \
	> "${PREFIX}/make/custom/${BSP_NAME}.mk"
