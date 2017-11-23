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

export PREFIX

# Build
cd "${NEWFS_MSDOS_SOURCE_DIR}"
make
make install
