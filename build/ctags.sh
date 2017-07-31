#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# Configuration
source "${SCRIPTDIR}/configuration.sh"
BUILD_DIR="${PROJECTDIR}/build/b-$BSP_NAME"
export PATH="${PREFIX}/bin:${PATH}"

# create ctags
TAGFILE="${PROJECTDIR}/tags"
CTAGOPTS="-a -f ${PROJECTDIR}/tags --extra=+fq --recurse=yes"
[[ -e "${TAGFILE}" ]] && rm "${TAGFILE}"

ctags ${CTAGOPTS} "${RTEMS_SOURCE_DIR}"
ctags ${CTAGOPTS} --exclude="freebsd-org" "${LIBBSD_SOURCE_DIR}"
#ctags ${CTAGOPTS} "${LIBBSD_SOURCE_DIR}"
