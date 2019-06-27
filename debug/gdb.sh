#!/bin/sh

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export PROJECTDIR="${SCRIPTDIR}/../"

${PROJECTDIR}/install/rtems/5/bin/arm-rtems5-gdb -x ${SCRIPTDIR}/start.gdb $@
