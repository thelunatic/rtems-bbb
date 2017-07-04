#!/bin/sh

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

${PROJECTDIR}/rtems-install/rtems-4.12/bin/arm-rtems4.12-gdb -x ${SCRIPTDIR}/start.gdb $@
